#include "imaged.h"
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <babl/babl.h>

Image *imageNew(ImageMeta meta) {
  Image *image = malloc(sizeof(Image));
  if (!image) {
    return NULL;
  }

  image->owner = true;
  image->meta = meta;

  size_t channels = imageColorNumChannels(meta.color);

  image->data = calloc(meta.width * meta.height * channels, meta.bits / 8);
  if (image->data == NULL) {
    free(image);
    return NULL;
  }

  return image;
}

Image *imageNewWithData(ImageMeta meta, void *data) {
  Image *image = malloc(sizeof(Image));
  if (!image) {
    return NULL;
  }

  image->owner = false;
  image->meta = meta;
  image->data = data;
  if (image->data == NULL) {
    free(image);
    return NULL;
  }

  return image;
}

void imageMetaInit(uint64_t w, uint64_t h, ImageColor color, ImageKind kind,
                   uint8_t bits, ImageMeta *meta) {
  if (!meta) {
    return;
  }

  meta->width = w;
  meta->height = h;
  meta->color = color;
  meta->kind = kind;
  meta->bits = bits;
}

Image *imageAlloc(uint64_t w, uint64_t h, ImageColor color, ImageKind kind,
                  uint8_t bits, const void *data) {
  ImageMeta meta;
  imageMetaInit(w, h, color, kind, bits, &meta);

  Image *image = imageNew(meta);
  if (image == NULL) {
    return NULL;
  }

  if (data) {
    memcpy(image->data, data,
           w * h * imageColorNumChannels(meta.color) * (meta.bits / 8));
  }

  return image;
}

void imageFree(Image *image) {
  if (image == NULL) {
    return;
  }

  if (image->data != NULL) {
    if (image->owner) {
      free(image->data);
      image->data = NULL;
    }
  }

  free(image);
}

Image *imageClone(const Image *image) {
  return imageAlloc(image->meta.width, image->meta.height, image->meta.color,
                    image->meta.kind, image->meta.bits, image->data);
}

size_t imagePixelBytes(Image *image) {
  return (size_t)image->meta.bits / 8 *
         imageColorNumChannels(image->meta.color);
}

size_t imageDataNumBytes(Image *image) {
  return imagePixelBytes(image) * image->meta.width * image->meta.height *
         imageColorNumChannels(image->meta.color);
}

size_t imageIndex(Image *image, size_t x, size_t y) {
  size_t bits = (size_t)image->meta.bits / 8;
  size_t channels = imageColorNumChannels(image->meta.color);
  return (image->meta.width * channels * y + x * channels) * bits;
}

void *imageAt(Image *image, size_t x, size_t y) {
  if (x >= image->meta.width || y >= image->meta.height) {
    return NULL;
  }

  return image->data + imageIndex(image, x, y);
}

#define norm(x, min, max) (((float)x - (float)min) / ((float)max - (float)min))

#define GET(t, min, max)                                                       \
  for (i = 0; i < channels; i++) {                                             \
    pixel->data[i] = norm(((t *)px)[i], min, max);                             \
  }

bool imageGetPixel(Image *image, size_t x, size_t y, Pixel *pixel) {
  size_t channels = imageColorNumChannels(image->meta.color);
  if (channels > 4) {
    channels = 4;
  }

  pixel->data[3] = 1.0;
  void *px = imageAt(image, x, y);
  if (!px) {
    return false;
  }

  size_t i = 0;
  switch (image->meta.kind) {
  case IMAGE_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      GET(int8_t, INT8_MIN, INT8_MAX);
      break;
    case 16:
      GET(int16_t, INT16_MIN, INT16_MAX);
      break;
    case 32:
      GET(int32_t, INT32_MIN, INT32_MAX);
      break;
    case 64:
      GET(int64_t, INT64_MIN, INT64_MAX);
      break;
    default:
      return false;
    }
    break;
  case IMAGE_KIND_UINT:
    switch (image->meta.bits) {
    case 8:
      GET(uint8_t, 0, UINT8_MAX);
      break;
    case 16:
      GET(uint16_t, 0, UINT16_MAX);
      break;
    case 32:
      GET(uint32_t, 0, UINT32_MAX);
      break;
    case 64:
      GET(uint64_t, 0, UINT64_MAX);
      break;
    default:
      return false;
    }
    break;
  case IMAGE_KIND_FLOAT:
    switch (image->meta.bits) {
    case 16:
      for (i = 0; i < channels; i++) {
        uint16_t h = ((uint16_t *)px)[i];
        pixel->data[i] = ((h & 0x8000) << 16) |
                         (((h & 0x7c00) + 0x1C000) << 13) |
                         ((h & 0x03FF) << 13);
      }
      break;
    case 32:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = ((float *)px)[i];
      }
      break;
    case 64:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = (float)(((double *)px)[i]);
      }
      break;
    default:
      return false;
    }
    break;
  }

  return true;
}

#define denorm(x, min, max)                                                    \
  ((((float)max - (float)min) * ((float)x / 1.0)) + (float)min)

#define SET(t, min, max)                                                       \
  for (i = 0; i < channels; i++) {                                             \
    ((t *)px)[i] = (t)denorm(pixel->data[i], min, max);                        \
  }

bool imageSetPixel(Image *image, size_t x, size_t y, const Pixel *pixel) {
  void *px = imageAt(image, x, y);
  if (px == NULL) {
    return false;
  }

  size_t channels = imageColorNumChannels(image->meta.color);
  if (channels > 4) {
    channels = 4;
  }

  size_t i = 0;

  switch (image->meta.kind) {
  case IMAGE_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      SET(int8_t, INT8_MIN, INT8_MAX);
      break;
    case 16:
      SET(int16_t, INT16_MIN, INT16_MAX);
      break;
    case 32:
      SET(int32_t, INT32_MIN, INT32_MAX);
      break;
    case 64:
      SET(int64_t, INT64_MIN, INT64_MAX);
      break;
    default:
      return false;
    }
    break;
  case IMAGE_KIND_UINT:
    switch (image->meta.bits) {
    case 8:
      SET(uint8_t, 0, UINT8_MAX);
      break;
    case 16:
      SET(uint16_t, 0, UINT16_MAX);
      break;
    case 32:
      SET(uint32_t, 0, UINT32_MAX);
      break;
    case 64:
      SET(uint64_t, 0, UINT64_MAX);
      break;

    default:
      return false;
    }
    break;
  case IMAGE_KIND_FLOAT:
    switch (image->meta.bits) {
    case 16:
      for (i = 0; i < channels; i++) {
        uint32_t x = *((uint32_t *)(pixel->data) + i);
        uint16_t h = ((x >> 16) & 0x8000) |
                     ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) |
                     ((x >> 13) & 0x03ff);
        ((uint16_t *)px)[i] = h;
      }
      break;
    case 32:
      for (i = 0; i < channels; i++) {
        ((float *)px)[i] = (float)pixel->data[i];
      }
      break;
    case 64:
      for (i = 0; i < channels; i++) {
        ((double *)px)[i] = (double)pixel->data[i];
      }
      break;
    default:
      return false;
    }
    break;
  }

  return true;
}

const Babl *format(ImageColor color, ImageKind kind, uint8_t bits) {
  const char *colorName = imageColorName(color);
  const char *typeName = imageTypeName(kind, bits);

  if (colorName == NULL || typeName == NULL) {
    return NULL;
  }

  size_t len = strlen(colorName) + strlen(typeName) + 2;
  char fmt[len];
  snprintf(fmt, len, "%s %s", colorName, typeName);

  return babl_format(fmt);
}

__thread bool imagedBablIsInitialized = false;

bool imageConvertTo(const Image *src, Image *dest) {
  if (!imagedBablIsInitialized) {
    imagedBablIsInitialized = true;
    babl_init();
    atexit(babl_exit);
  }

  if (dest->meta.width != src->meta.width ||
      dest->meta.height != src->meta.height) {
    return false;
  }

  const Babl *in = format(src->meta.color, src->meta.kind, src->meta.bits);
  const Babl *out = format(dest->meta.color, dest->meta.kind, dest->meta.bits);
  if (in == NULL || out == NULL) {
    return false;
  }

  const Babl *fish = babl_fish(in, out);
  babl_process(fish, src->data, dest->data, src->meta.width * src->meta.height);
  return true;
}

Image *imageConvert(const Image *src, ImageColor color, ImageKind kind,
                    uint8_t bits) {
  Image *dest =
      imageAlloc(src->meta.width, src->meta.height, color, kind, bits, NULL);
  if (dest == NULL) {
    return NULL;
  }

  if (!imageConvertTo(src, dest)) {
    imageFree(dest);
    return NULL;
  }

  return dest;
}

bool imageConvertInPlace(Image **src, ImageColor color, ImageKind kind,
                         uint8_t bits) {
  if (!src) {
    return false;
  }

  if ((*src)->meta.color == color && (*src)->meta.kind == kind &&
      (*src)->meta.bits == bits) {
    return true;
  }

  return imageConsume(imageConvert(*src, color, kind, bits), src) != NULL;
}

void imageAdjustGamma(Image *src, float gamma) {
  Pixel px = pixelEmpty();
#define X(i) px.data[i] = pow(px.data[i], 1.0 / gamma)
  IMAGE_ITER_ALL(src, x, y) {
    imageGetPixel(src, x, y, &px);
    X(0);
    X(1);
    X(2);
    X(3);
    pixelClamp(&px);
    imageSetPixel(src, x, y, &px);
  }
#undef X
}

void imageRotate(Image *im, Image *dst, float deg) {
  float midX, midY;
  float dx, dy;
  int32_t rotX, rotY;

  midX = im->meta.width / 2.0f;
  midY = im->meta.height / 2.0f;

  float angle = 2 * M_PI * deg / 360.0f;

  Pixel px;
  IMAGE_ITER_ALL(dst, i, j) {
    dx = i + 0.5 - midX;
    dy = j + 0.5 - midY;

    rotX = (uint64_t)(midX + dx * cos(angle) - dy * sin(angle));
    rotY = (uint64_t)(midY + dx * sin(angle) + dy * cos(angle));
    if (rotX >= 0 && rotY >= 0) {
      if (imageGetPixel(im, rotX, rotY, &px)) {
        imageSetPixel(dst, i, j, &px);
      }
    }
  }
}

void imageFilter(Image *im, Image *dst, float *K, int Ks, float divisor,
                 float offset) {
  Ks = Ks / 2;

  int kx, ky;
  Pixel p, px;
  px.data[3] = 1.0;

  // Divisor can never be zero
  if (divisor == 0.0) {
    divisor = 1.0;
  }

  IMAGE_ITER_ALL(im, ix, iy) {
    px.data[0] = px.data[1] = px.data[2] = 0.0;
    for (kx = -Ks; kx <= Ks; kx++) {
      for (ky = -Ks; ky <= Ks; ky++) {
        if (imageGetPixel(im, ix + kx, iy + ky, &p)) {
#define X(l)                                                                   \
  px.data[l] +=                                                                \
      (K[(kx + Ks) + (ky + Ks) * (2 * Ks + 1)] / divisor) * p.data[l] + offset
          X(0);
          X(1);
          X(2);
#undef X
        }
      }
    }
    pixelClamp(&px);
    imageSetPixel(dst, ix, iy, &px);
  }
}

Image *imageConsume(Image *x, Image **dest) {
  if (!dest) {
    return x;
  }

  imageFree(*dest);
  *dest = x;
  return *dest;
}

Image *imageScale(Image *src, double scale_x, double scale_y) {
  double targetWidth = (double)src->meta.width * scale_x;
  double targetHeight = (double)src->meta.height * scale_y;

  return imageResize(src, (size_t)targetWidth, (size_t)targetHeight);
}

void imageResizeTo(Image *src, Image *dest) {
  double targetWidth = (double)dest->meta.width;
  double sourceWidth = (double)src->meta.width;
  double targetHeight = (double)dest->meta.height;
  double sourceHeight = (double)src->meta.height;

  double xr = sourceWidth / targetWidth;
  double yr = sourceHeight / targetHeight;

  Pixel px = pixelEmpty(), tmp = pixelEmpty();
  IMAGE_ITER_ALL(dest, x, y) {
    size_t xx = floor((double)x * xr);
    size_t yy = floor((double)y * yr);
    size_t count = 0;

#define X(yy)                                                                  \
  if (imageGetPixel(src, xx - 1, yy, &tmp)) {                                  \
    pixelAdd(&tmp, &px);                                                       \
    count += 1;                                                                \
  }                                                                            \
  if (imageGetPixel(src, xx, yy, &tmp)) {                                      \
    pixelAdd(&tmp, &px);                                                       \
    count += 1;                                                                \
  }                                                                            \
  if (imageGetPixel(src, xx + 1, yy, &tmp)) {                                  \
    pixelAdd(&tmp, &px);                                                       \
    count += 1;                                                                \
  }
    X(yy - 1);
    X(yy);
    X(yy + 1);
#undef X

    if (count > 0) {
      px.data[0] /= (float)count;
      px.data[1] /= (float)count;
      px.data[2] /= (float)count;
      px.data[3] /= (float)count;

      pixelClamp(&px);
      imageSetPixel(dest, x, y, &px);
    }
  }
}

Image *imageResize(Image *src, size_t x, size_t y) {
  Image *dest =
      imageAlloc(x, y, src->meta.color, src->meta.kind, src->meta.bits, NULL);
  if (dest == NULL) {
    return dest;
  }

  imageResizeTo(src, dest);
  return dest;
}

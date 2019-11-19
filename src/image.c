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

Image *imageNew(ImagedMeta meta) {
  Image *image = malloc(sizeof(Image));
  if (!image) {
    return NULL;
  }

  image->meta = meta;

  size_t channels = imagedColorNumChannels(meta.color);

  image->data = calloc(meta.width * meta.height * channels, meta.bits / 8);
  if (image->data == NULL) {
    free(image);
    return NULL;
  }

  return image;
}

Image *imageAlloc(uint64_t w, uint64_t h, ImagedColor color, ImagedKind kind,
                  uint8_t bits, const void *data) {
  ImagedMeta meta = {
      .width = w,
      .height = h,
      .color = color,
      .kind = kind,
      .bits = bits,
  };

  Image *image = imageNew(meta);
  if (image == NULL) {
    return NULL;
  }

  if (data) {
    memcpy(image->data, data,
           w * h * imagedColorNumChannels(meta.color) * (meta.bits / 8));
  }

  return image;
}

void imageFree(Image *image) {
  if (image == NULL) {
    return;
  }

  if (image->data != NULL) {
    free(image->data);
    image->data = NULL;
  }

  free(image);
}

Image *imageClone(const Image *image) {
  return imageAlloc(image->meta.width, image->meta.height, image->meta.color,
                    image->meta.kind, image->meta.bits, image->data);
}

size_t imagePixelBytes(Image *image) {
  return (size_t)image->meta.bits / 8 *
         imagedColorNumChannels(image->meta.color);
}

size_t imageBytes(Image *image) {
  return imagePixelBytes(image) * image->meta.width * image->meta.height *
         imagedColorNumChannels(image->meta.color);
}

size_t imageIndex(Image *image, size_t x, size_t y) {
  size_t bits = (size_t)image->meta.bits / 8;
  size_t channels = imagedColorNumChannels(image->meta.color);
  return (image->meta.width * channels * y + x * channels) * bits;
}

void *imageAt(Image *image, size_t x, size_t y) {
  if (x >= image->meta.width || y >= image->meta.height) {
    return NULL;
  }

  return image->data + imageIndex(image, x, y);
}

#define norm(x, min, max) (((float)x - (float)min) / ((float)max - (float)min))

bool imageGetPixel(Image *image, size_t x, size_t y, Pixel *pixel) {
  size_t channels = imagedColorNumChannels(image->meta.color);
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
  case IMAGED_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = norm(((int8_t *)px)[i], INT8_MIN, INT8_MAX);
      }
      break;
    case 16:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = norm(((int16_t *)px)[i], INT16_MIN, INT16_MAX);
      }
      break;
    case 32:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = norm(((uint32_t *)px)[i], INT32_MIN, INT32_MAX);
      }
      break;
    default:
      return false;
    }
    break;
  case IMAGED_KIND_UINT:
    switch (image->meta.bits) {
    case 8:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = norm(((uint8_t *)px)[i], 0, UINT8_MAX);
      }
      break;
    case 16:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = norm(((uint16_t *)px)[i], 0, UINT16_MAX);
      }
      break;
    case 32:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = norm(((uint32_t *)px)[i], 0, UINT32_MAX);
      }
      break;
    default:
      return false;
    }
    break;
  case IMAGED_KIND_FLOAT:
    switch (image->meta.bits) {
    case 32:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = ((float *)px)[i];
      }
      break;
    case 64:
      for (i = 0; i < channels; i++) {
        pixel->data[i] = (float)((double *)px)[i];
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

bool imageSetPixel(Image *image, size_t x, size_t y, const Pixel *pixel) {
  void *px = imageAt(image, x, y);
  if (px == NULL) {
    return false;
  }

  size_t channels = imagedColorNumChannels(image->meta.color);
  if (channels > 4) {
    channels = 4;
  }

  size_t i = 0;

  switch (image->meta.kind) {
  case IMAGED_KIND_INT:
    switch (image->meta.bits) {
    case 8:

      for (i = 0; i < channels; i++) {
        ((int8_t *)px)[i] = (int8_t)denorm(pixel->data[i], INT8_MIN, INT8_MAX);
      }
      break;
    case 16:

      for (i = 0; i < channels; i++) {
        ((int16_t *)px)[i] =
            (int16_t)denorm(pixel->data[i], INT16_MIN, INT16_MAX);
      }
      break;
    case 32:

      for (i = 0; i < channels; i++) {
        ((int32_t *)px)[i] =
            (int32_t)denorm(pixel->data[i], INT32_MIN, INT32_MAX);
      }
      break;
    default:
      return false;
    }
    break;
  case IMAGED_KIND_UINT:
    switch (image->meta.bits) {
    case 8:

      for (i = 0; i < channels; i++) {
        ((uint8_t *)px)[i] = (uint8_t)denorm(pixel->data[i], 0, UINT8_MAX);
      }
      break;
    case 16:

      for (i = 0; i < channels; i++) {
        ((uint16_t *)px)[i] = (uint16_t)denorm(pixel->data[i], 0, UINT16_MAX);
      }
      break;
    case 32:

      for (i = 0; i < channels; i++) {
        ((uint32_t *)px)[i] = (uint32_t)denorm(pixel->data[i], 0, UINT32_MAX);
      }
      break;
    default:
      return false;
    }
    break;
  case IMAGED_KIND_FLOAT:
    switch (image->meta.bits) {
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

const Babl *format(ImagedColor color, ImagedKind kind, uint8_t bits) {
  const char *colorName = imagedColorName(color);
  const char *typeName = imagedTypeName(kind, bits);

  if (colorName == NULL || typeName == NULL) {
    return NULL;
  }

  size_t len = strlen(colorName) + strlen(typeName) + 2;
  char fmt[len];
  snprintf(fmt, len, "%s %s", colorName, typeName);

  return babl_format(fmt);
}

static bool bablInit = false;

bool imageConvertTo(const Image *src, Image *dest) {
  if (!bablInit) {
    babl_init();
    atexit(babl_exit);
    bablInit = true;
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

Image *imageConvert(const Image *src, ImagedColor color, ImagedKind kind,
                    uint8_t bits) {
  Image *dest =
      imageAlloc(src->meta.width, src->meta.height, color, kind, bits, NULL);
  if (dest == NULL) {
    return NULL;
  }
  if (!imageConvertTo(src, dest)) {
    return NULL;
  }

  return dest;
}

bool imageConvertInPlace(Image **src, ImagedColor color, ImagedKind kind,
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

Image *imageConvertACES0(Image *src) {
  Image *dest = imageAlloc(src->meta.width, src->meta.height,
                           IMAGED_COLOR_CIEXYZ, IMAGED_KIND_FLOAT, 32, NULL);
  if (!dest) {
    return NULL;
  }

  float r, g, b;
  float *data = dest->data;
  for (size_t i = 0; i < dest->meta.width * dest->meta.height *
                             imagedColorNumChannels(dest->meta.color);
       i += 3) {
    r = data[i];
    g = data[i + 1];
    b = data[i + 2];

    data[i] = 1.0498110175 * r + 0.0000000000 * g + -0.0000974845 * b;
    data[i + 1] = -0.4959030231 * r + 1.3733130458 * g + 0.0982400361 * b;
    data[i + 2] = 0.0000000000 * r + 0.0000000000 * g + 0.9912520182 * b;
  }

  dest->meta.color = IMAGED_COLOR_RGB;

  return dest;
}

Image *imageConvertACES1(Image *src) {
  Image *dest = imageAlloc(src->meta.width, src->meta.height,
                           IMAGED_COLOR_CIEXYZ, IMAGED_KIND_FLOAT, 32, NULL);
  if (!dest) {
    return NULL;
  }

  float r, g, b;
  float *data = dest->data;
  for (size_t i = 0; i < dest->meta.width * dest->meta.height *
                             imagedColorNumChannels(dest->meta.color);
       i += 3) {
    r = data[i];
    g = data[i + 1];
    b = data[i + 2];

    data[i] = 1.6410233797 * r + -0.3248032942 * g + -0.2364246952 * b;
    data[i + 1] = -0.6636628587 * r + 1.6153315917 * g + 0.0167563477 * b;
    data[i + 2] = 0.0117218943 * r + -0.0082844420 * g + 0.9883948585 * b;
  }

  dest->meta.color = IMAGED_COLOR_RGB;

  return dest;
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

  size_t channels = imagedColorNumChannels(im->meta.color);

  // Ignore alpha channel
  if (channels > 3) {
    channels = 3;
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

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
  bool hasAlpha = channels == 4;
  pixel->data[3] = 1.0;
  void *px = imageAt(image, x, y);
  switch (image->meta.kind) {
  case IMAGED_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = norm(((int8_t *)px)[i % channels], INT8_MIN, INT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] =
            norm(((int16_t *)px)[i % channels], INT16_MIN, INT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] =
            norm(((uint32_t *)px)[i % channels], INT32_MIN, INT32_MAX);
      }
      break;
    default:
      return false;
    }
    break;
  case IMAGED_KIND_UINT:
    switch (image->meta.bits) {
    case 8:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = norm(((uint8_t *)px)[i % channels], 0, UINT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = norm(((uint8_t *)px)[i % channels], 0, UINT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = norm(((uint32_t *)px)[i % channels], 0, UINT32_MAX);
      }
      break;
    default:
      return false;
    }
    break;
  case IMAGED_KIND_FLOAT:
    switch (image->meta.bits) {
    case 32:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = ((float *)px)[i % channels];
      }
      break;
    case 64:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = (float)((double *)px)[i % channels];
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

  switch (image->meta.kind) {
  case IMAGED_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      for (size_t i = 0; i < channels; i++) {
        ((int8_t *)px)[i] = (int8_t)denorm(pixel->data[i], INT8_MIN, INT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < channels; i++) {
        ((int16_t *)px)[i] =
            (int16_t)denorm(pixel->data[i], INT16_MIN, INT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < channels; i++) {
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
      for (size_t i = 0; i < channels; i++) {
        ((uint8_t *)px)[i] = (uint8_t)denorm(pixel->data[i], 0, UINT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < channels; i++) {
        ((uint16_t *)px)[i] = (uint16_t)denorm(pixel->data[i], 0, UINT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < channels; i++) {
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
      for (size_t i = 0; i < channels; i++) {
        ((float *)px)[i] = (float)pixel->data[i];
      }
      break;
    case 64:
      for (size_t i = 0; i < channels; i++) {
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

struct imageParallelIterator {
  uint32_t x0, y0, x1, y1;
  Image *image0, *image1;
  imageParallelFn f;
  void *userdata;
};

void *imageParallelWrapper(void *_iter) {
  struct imageParallelIterator *iter = (struct imageParallelIterator *)_iter;
  Pixel px;
  uint32_t i, j;
  for (j = iter->y0; j < iter->y1; j++) {
    for (i = iter->x0; i < iter->x1; i++) {
      if (imageGetPixel(iter->image1, i, j, &px)) {
        if (iter->f(i, j, &px, iter->userdata)) {
          imageSetPixel(iter->image0, i, j, &px);
        }
      }
    }
  }

  return NULL;
}

ImagedStatus imageEachPixel2(Image *src, Image *dst, imageParallelFn fn,
                             int nthreads, void *userdata) {
  if (src == NULL) {
    return IMAGED_ERR;
  }

  if (dst == NULL) {
    dst = src;
  }

  if (nthreads <= 0) {
    nthreads = sysconf(_SC_NPROCESSORS_ONLN);
  } else if (nthreads == 1) {
    uint64_t i, j;
    Pixel px;
    for (j = 0; j < src->meta.height; j++) {
      for (i = 0; i < src->meta.width; i++) {
        if (imageGetPixel(src, i, j, &px)) {
          if (fn(i, j, &px, userdata)) {
            imageSetPixel(dst, i, j, &px);
          }
        }
      }
    }
    return IMAGED_OK;
  }

  pthread_t threads[nthreads];
  int tries = 1, n;
  uint64_t height, x;

  height = src->meta.height / nthreads;

  for (x = 0; x < (size_t)nthreads; x++) {
    struct imageParallelIterator iter;
    iter.x1 = src->meta.width;
    iter.x0 = 0;
    iter.y1 = height * x;
    iter.y0 = height;
    iter.userdata = userdata;
    iter.image0 = dst;
    iter.image1 = src;
    iter.f = fn;
    if (pthread_create(&threads[x], NULL, imageParallelWrapper, &iter) != 0) {
      if (tries <= 5) {
        x -= 1;
        tries += 1;
      } else {
        return IMAGED_ERR;
      }
    } else {
      tries = 1;
    }
  }

  for (n = 0; n < nthreads; n++) {
    // Maybe do something if this fails?
    pthread_join(threads[n], NULL);
  }

  return IMAGED_OK;
}

ImagedStatus imageEachPixel(Image *im, imageParallelFn fn, int nthreads,
                            void *userdata) {
  return imageEachPixel2(NULL, im, fn, nthreads, userdata);
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

  Image *tmp = *src;
  if (!tmp) {
    return false;
  }

  *src = imageConvert(*src, color, kind, bits);
  if (*src == NULL) {
    *src = tmp;
    return false;
  }

  imageFree(tmp);
  return true;
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

    rotX = (uint32_t)(midX + dx * cos(angle) - dy * sin(angle));
    rotY = (uint32_t)(midY + dx * sin(angle) + dy * cos(angle));
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

  size_t channels = imagedColorNumChannels(im->meta.color), l;

  // Ignore alpha channel
  if (channels > 3) {
    channels = 3;
  }

  IMAGE_ITER_ALL(im, ix, iy) {
    px.data[0] = px.data[1] = px.data[2] = 0.0;
    for (kx = -Ks; kx <= Ks; kx++) {
      for (ky = -Ks; ky <= Ks; ky++) {
        imageGetPixel(im, ix + kx, iy + ky, &p);
        for (l = 0; l < channels; l++) {
          px.data[l] +=
              (K[(kx + Ks) + (ky + Ks) * (2 * Ks + 1)] / divisor) * p.data[l] +
              offset;
        }
      }
    }

    pixelClamp(&px);
    imageSetPixel(dst, ix, iy, &px);
  }
}

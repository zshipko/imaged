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

#ifdef IMAGED_BABL
#include <babl/babl.h>
#endif

Image *imageAlloc(uint64_t w, uint64_t h, uint8_t c, ImagedKind kind,
                  uint8_t bits, const void *data) {
  Image *image = malloc(sizeof(Image));
  if (!image) {
    return NULL;
  }

  image->meta.width = w;
  image->meta.height = h;
  image->meta.channels = c;
  image->meta.kind = kind;
  image->meta.bits = bits;

  image->data = calloc(w * h * c, bits / 8);
  if (image->data == NULL) {
    free(image);
    return NULL;
  }

  if (data) {
    memcpy(image->data, data, w * h * c * (bits / 8));
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
  return imageAlloc(image->meta.width, image->meta.height, image->meta.channels,
                    image->meta.kind, image->meta.bits, image->data);
}

size_t imagePixelBytes(Image *image) {
  return (size_t)image->meta.bits / 8 * (size_t)image->meta.channels;
}

size_t imageBytes(Image *image) {
  return imagePixelBytes(image) * image->meta.width * image->meta.height *
         (size_t)image->meta.channels;
}

size_t imageIndex(Image *image, size_t x, size_t y) {
  size_t bits = (size_t)image->meta.bits / 8;
  return (image->meta.width * (size_t)image->meta.channels * y +
          x * (size_t)image->meta.channels) *
         bits;
}

void *imageAt(Image *image, size_t x, size_t y) {
  if (x >= image->meta.width || y >= image->meta.height) {
    return NULL;
  }

  return image->data + imageIndex(image, x, y);
}

#define norm(x, min, max) (((float)x - (float)min) / ((float)max - (float)min))

bool imageGetPixel(Image *image, size_t x, size_t y, Pixel *pixel) {
  bool hasAlpha = image->meta.channels == 4;
  pixel->data[3] = 1.0;
  void *px = imageAt(image, x, y);
  switch (image->meta.kind) {
  case IMAGED_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] =
            norm(((int8_t *)px)[i % image->meta.channels], INT8_MIN, INT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = norm(((int16_t *)px)[i % image->meta.channels],
                              INT16_MIN, INT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = norm(((uint32_t *)px)[i % image->meta.channels],
                              INT32_MIN, INT32_MAX);
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
        pixel->data[i] =
            norm(((uint8_t *)px)[i % image->meta.channels], 0, UINT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] =
            norm(((uint8_t *)px)[i % image->meta.channels], 0, UINT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] =
            norm(((uint32_t *)px)[i % image->meta.channels], 0, UINT32_MAX);
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
        pixel->data[i] = ((float *)px)[i % image->meta.channels];
      }
      break;
    case 64:
      for (size_t i = 0; i < (hasAlpha ? 4 : 3); i++) {
        pixel->data[i] = (float)((double *)px)[i % image->meta.channels];
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

  size_t channels = image->meta.channels <= 4 ? image->meta.channels : 4;
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

void *bimageParallelWrapper(void *_iter) {
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

ImagedStatus bimageEachPixel2(Image *src, Image *dst, imageParallelFn fn,
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
    if (pthread_create(&threads[x], NULL, bimageParallelWrapper, &iter) != 0) {
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

ImagedStatus bimageEachPixel(Image *im, imageParallelFn fn, int nthreads,
                             void *userdata) {
  return bimageEachPixel2(NULL, im, fn, nthreads, userdata);
}

#ifdef IMAGED_BABL
void imageConvertTo(Image *src, const char *srcfmt, Image *dest,
                    const char *destfmt) {
  babl_init();
  const Babl *fish = babl_fish(srcfmt, destfmt);
  babl_process(fish, src->data, dest->data, src->meta.width * src->meta.height);
  babl_exit();
}

Image *imageConvert(Image *src, const char *srcfmt, ImagedKind kind,
                    const char *destfmt) {
  babl_init();
  const Babl *out = babl_format(destfmt);
  int outchan = babl_format_get_n_components(out);
  printf("NCOMP: %d\n", outchan);
  printf("BYTES: %d\n", babl_format_get_bytes_per_pixel(out));
  Image *dest =
      imageAlloc(src->meta.width, src->meta.height, outchan, kind,
                 babl_format_get_bytes_per_pixel(out) * 8 / outchan, NULL);
  if (dest == NULL) {
    babl_exit();
    return NULL;
  }
  const Babl *fish = babl_fish(srcfmt, out);
  babl_process(fish, src->data, dest->data, src->meta.width * src->meta.height);
  babl_exit();
  return dest;
}
#else
void imageConvertTo(Image *src, const char *srcfmt, Image *dest,
                    const char *destfmt) {
  (void)src;
  (void)srcfmt;
  (void)dest;
  (void)destfmt;
  fputs("ERROR: Imaged was not compiled with Babl support", stderr);
  abort();
}

Image *imageConvert(Image *src, const char *srcfmt, ImagedKind kind,
                    const char *destfmt) {
  (void)src;
  (void)srcfmt;
  (void)kind;
  (void)destfmt;
  fputs("ERROR: Imaged was not compiled with Babl support", stderr);
  abort();
  return NULL;
}
#endif

void bimageRotate(Image *im, Image *dst, float deg) {
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

#ifdef __SSE__
  __m128 divi = _mm_load_ps1(&divisor), offs = _mm_load_ps1(&offset);
#else
  int channels = im->meta.channels, l;

  // Ignore alpha channel
  if (channels > 3) {
    channels = 3;
  }
#endif

  IMAGE_ITER_ALL(im, ix, iy) {
    px.data[0] = px.data[1] = px.data[2] = 0.0;
    for (kx = -Ks; kx <= Ks; kx++) {
      for (ky = -Ks; ky <= Ks; ky++) {
        imageGetPixel(im, ix + kx, iy + ky, &p);
#ifdef __SSE__
        px.data +=
            (_mm_load_ps1(&K[(kx + Ks) + (ky + Ks) * (2 * Ks + 1)]) / divi) *
                p.data +
            offs;
#else
        for (l = 0; l < channels; l++) {
          px.data[l] +=
              (K[(kx + Ks) + (ky + Ks) * (2 * Ks + 1)] / divisor) * p.data[l] +
              offset;
        }
#endif
      }
    }

    pixelClamp(&px);
    imageSetPixel(dst, ix, iy, &px);
  }
}

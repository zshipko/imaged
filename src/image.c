#include "imaged.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

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

#define norm(x, min, max)                                                      \
  (((float)x - (float)min) / ((float)max - 1 - (float)min))

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
  (((float)max - 1 - (float)min) * ((x - 0) / (1.0 - 0))) + (float)min

bool imageSetPixel(Image *image, size_t x, size_t y, const Pixel *pixel) {
  void *px = imageAt(image, x, y);
  switch (image->meta.kind) {
  case IMAGED_KIND_INT:
    switch (image->meta.bits) {
    case 8:
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
        ((int8_t *)px)[i] = (int8_t)denorm(pixel->data[i], INT8_MIN, INT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
        ((int16_t *)px)[i] =
            (int16_t)denorm(pixel->data[i], INT16_MIN, INT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
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
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
        ((uint8_t *)px)[i] = (uint8_t)denorm(pixel->data[i], 0, UINT8_MAX);
      }
      break;
    case 16:
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
        ((uint16_t *)px)[i] = (uint16_t)denorm(pixel->data[i], 0, UINT16_MAX);
      }
      break;
    case 32:
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
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
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
        ((float *)px)[i] = (float)pixel->data[i];
      }
      break;
    case 64:
      for (size_t i = 0; i < image->meta.channels || i < 4; i++) {
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

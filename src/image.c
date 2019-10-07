#include "imaged.h"
#include <stdlib.h>
#include <string.h>

Image *imageAlloc(uint64_t w, uint64_t h, uint8_t c, ImagedKind kind,
                  uint8_t bits, void *data) {
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

size_t imagePixelBytes(Image *image) {
  return (size_t)image->meta.bits / 8 * (size_t)image->meta.channels;
}

size_t imageIndex(Image *image, size_t x, size_t y) {
  size_t bits = (size_t)image->meta.bits;
  return image->meta.width * (size_t)image->meta.channels * y * bits +
         x * (size_t)image->meta.channels * bits;
}

void *imageAt(Image *image, size_t x, size_t y) {
  if (x >= image->meta.width || y >= image->meta.height) {
    return NULL;
  }

  return image->data + imageIndex(image, x, y);
}

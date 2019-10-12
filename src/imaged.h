#ifndef __IMAGED_H
#define __IMAGED_H

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __SSE__
#include <immintrin.h>
#endif

char *imagedStringPrintf(const char *fmt, ...);

typedef enum {
  IMAGED_OK,
  IMAGED_ERR,
  IMAGED_ERR_CANNOT_CREATE_FILE,
  IMAGED_ERR_FILE_DOES_NOT_EXIST,
  IMAGED_ERR_FILE_ALREADY_EXISTS,
  IMAGED_ERR_SEEK,
  IMAGED_ERR_MAP_FAILED,
  IMAGED_ERR_INVALID_KEY,
  IMAGED_ERR_INVALID_FILE,
} ImagedStatus;

const char *imagedError(ImagedStatus status);
void imagedPrintError(ImagedStatus status, const char *message);

typedef struct {
  char *root;
} Imaged;

typedef enum {
  IMAGED_KIND_INT,
  IMAGED_KIND_UINT,
  IMAGED_KIND_FLOAT,
} ImagedKind;

typedef struct {
  uint64_t width, height;
  uint8_t channels, bits;
  ImagedKind kind;
} ImagedMeta;

size_t imagedMetaTotalBytes(const ImagedMeta *meta);

typedef struct {
  ImagedMeta meta;
  void *data;
} Image;

Image *imageAlloc(uint64_t w, uint64_t h, uint8_t c, ImagedKind kind,
                  uint8_t bits, const void *data);
Image *imageClone(const Image *image);
void imageFree(Image *image);
size_t imagePixelBytes(Image *image);
size_t imageBytes(Image *image);
size_t imageIndex(Image *image, size_t x, size_t y);
void *imageAt(Image *image, size_t x, size_t y);

typedef struct {
#ifdef __SSE__
  __m128 data;
#else
  float data[4];
#endif
} Pixel;

Pixel pixelEmpty();
Pixel pixelGray(float r);
Pixel pixelRGB(float r, float g, float b);
Pixel pixelRGBA(float r, float g, float b, float a);
bool imageGetPixel(Image *image, size_t x, size_t y, Pixel *pixel);
bool imageSetPixel(Image *image, size_t x, size_t y, const Pixel *pixel);

typedef struct {
  int fd;
  Image image;
} ImagedHandle;

void imagedResetLocks(Imaged *db);

// Open a new imaged context
Imaged *imagedOpen(const char *path);

// Close an imaged context
void imagedClose(Imaged *db);

// Destroy an imaged context, removing all contents from disk
ImagedStatus imagedDestroy(Imaged *db);

// Returns true when there is a value associated with the given key
bool imagedHasKey(Imaged *db, const char *key, ssize_t keylen);

// Set a key
ImagedStatus imagedSet(Imaged *db, const char *key, ssize_t keylen,
                       ImagedMeta meta, const void *imagedata,
                       ImagedHandle *handle);

// Get a key
ImagedStatus imagedGet(Imaged *db, const char *key, ssize_t keylen,
                       bool editable, ImagedHandle *handle);

// Remove the value associated with the provided key
ImagedStatus imagedRemove(Imaged *db, const char *key, ssize_t keylen);

// Release ImageHandle resources including all memory and file descriptors
void imagedHandleFree(ImagedHandle *handle);

typedef struct {
  Imaged *db;
  DIR *d;
  struct dirent *ent;
  const char *key;
  ImagedHandle handle;
} ImagedIter;

ImagedIter *imagedIterNew(Imaged *db);
Image *imagedIterNext(ImagedIter *iter);
const char *imagedIterNextKey(ImagedIter *iter);
void imagedIterFree(ImagedIter *iter);
void imagedIterReset(ImagedIter *iter);

// UTIL
#define IMAGED_UNUSED __attribute__((unused))

#ifndef IMAGED_NO_DEFER
void defer_free(void *data);
void defer_Imaged(Imaged **db);
void defer_ImagedIter(ImagedIter **iter);
void defer_ImagedHandle(ImagedHandle **image);
#define defer_(b, t, v) t *v __attribute__((cleanup(defer_##b)))
#define defer(t, v) defer_(t, t, v)
#endif

#ifdef IMAGED_EZIMAGE
#include <ezimage.h>
IMAGED_UNUSED static ezimage_shape imagedMetaToEzimageShape(ImagedMeta meta) {
  ezimage_shape shape = {
      .width = meta.width,
      .height = meta.height,
      .channels = meta.channels,
      .t = {.bits = meta.bits, .kind = (ezimage_kind)meta.kind}};
  return shape;
}

IMAGED_UNUSED static ImagedMeta
imagedMetaFromEzimageShape(ezimage_shape shape) {
  ImagedMeta meta = {
      .width = shape.width,
      .height = shape.height,
      .channels = shape.channels,
      .bits = shape.t.bits,
      .kind = (ImagedKind)shape.t.kind,
  };
  return meta;
}

IMAGED_UNUSED static Image *imagedReadImage(const char *path) {
  ezimage_shape shape;
  void *data = ezimage_imread(path, NULL, &shape);
  if (!data) {
    return NULL;
  }

  Image *image = malloc(sizeof(Image));
  if (!image) {
    free(data);
    return NULL;
  }

  image->meta = imagedMetaFromEzimageShape(shape);
  image->data = data;

  return image;
}

IMAGED_UNUSED static bool imagedWriteImage(const char *path,
                                           const Image *image) {
  ezimage_shape shape = imagedMetaToEzimageShape(image->meta);
  return ezimage_imwrite(path, image->data, &shape);
}

#endif // IMAGED_EZIMAGE

#ifdef IMAGED_HALIDE
#include "HalideRuntime.h"
static halide_type_code_t getType(ImagedKind kind) {
  switch (kind) {
  case IMAGED_KIND_UINT:
    return halide_type_uint;
  case IMAGED_KIND_INT:
    return halide_type_uint;
  case IMAGED_KIND_FLOAT:
    return halide_type_float;
  }
}

IMAGED_UNUSED static void imageNewHalideBuffer(Image *image,
                                               halide_buffer_t *buffer) {
  buffer->device = 0;
  buffer->device_interface = NULL;
  buffer->host = image->data;
  buffer->dimensions = image->meta.channels < 3 ? 2 : 3;
  buffer->dim = malloc(sizeof(halide_buffer_t) * buffer->dimensions);
  assert(buffer->dim);

  if (buffer->dimensions == 2) {
    // width
    buffer->dim[0].min = 0;
    buffer->dim[0].extent = image->meta.width;
    buffer->dim[0].stride = 1;
    buffer->dim[0].flags = 0;

    // height
    buffer->dim[1].min = 0;
    buffer->dim[1].extent = image->meta.height;
    buffer->dim[1].stride = image->meta.width;
    buffer->dim[1].flags = 0;
  } else {
    // channels
    buffer->dim[0].min = 0;
    buffer->dim[0].extent = image->meta.channels;
    buffer->dim[0].stride = 1;
    buffer->dim[0].flags = 0;

    // width
    buffer->dim[1].min = 0;
    buffer->dim[1].extent = image->meta.width;
    buffer->dim[1].stride = image->meta.channels;
    buffer->dim[1].flags = 0;

    // height
    buffer->dim[2].min = 0;
    buffer->dim[2].extent = image->meta.height;
    buffer->dim[2].stride = image->meta.width * image->meta.channels;
    buffer->dim[2].flags = 0;
  }

  struct halide_type_t t;
  t.code = getType(image->meta.kind);
  t.bits = image->meta.bits;
  t.lanes = 1;

  buffer->type = t;
}

IMAGED_UNUSED static void imageFreeHalideBuffer(halide_buffer_t *buffer) {
  free(buffer->dim);
}

#endif // IMAGED_HALIDE

#endif

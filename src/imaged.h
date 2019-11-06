#ifndef __IMAGED_H
#define __IMAGED_H

#if defined(__cplusplus) && defined(IMAGED_HALIDE_UTIL)
#include "Halide.h"

using namespace Halide;

template <typename T>
void interleave_input(T &input, Expr n, Var x, Var y, Var c) {
  input.dim(0).set_stride(n).dim(2).set_stride(1);
  input.dim(2).set_bounds(0, n);
}

template <typename T>
void interleave_output(T &output, Expr n, Var x, Var y, Var c) {
  output.dim(0).set_stride(n).dim(2).set_stride(1);
  output.dim(2).set_bounds(0, n);
  output.reorder(c, x, y).unroll(c);
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef _WIN32
#define IMAGED_PATH_SEP '\\'
#else
#define IMAGED_PATH_SEP '/'
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
  IMAGED_ERR_LOCKED,
} ImagedStatus;

const char *imagedError(ImagedStatus status);
void imagedPrintError(ImagedStatus status, const char *message);

struct ImagedHandle;

typedef struct {
  char *root;
} Imaged;

typedef enum {
  IMAGED_KIND_INT,
  IMAGED_KIND_UINT,
  IMAGED_KIND_FLOAT,
} ImagedKind;

typedef enum {
  IMAGED_COLOR_GRAY = 1,
  IMAGED_COLOR_GRAYA = 2,
  IMAGED_COLOR_RGB = 3,
  IMAGED_COLOR_RGBA = 4,
  IMAGED_COLOR_CMYK = 5,
  IMAGED_COLOR_CMYKA = 6,
  IMAGED_COLOR_YCBCR = 7,
  IMAGED_COLOR_YCBCRA = 8,
  IMAGED_COLOR_CIELAB = 9,
  IMAGED_COLOR_CIELABA = 10,
  IMAGED_COLOR_CIELCH = 11,
  IMAGED_COLOR_CIELCHA = 12,
  IMAGED_COLOR_CIEXYZ = 13,
  IMAGED_COLOR_CIEXYZA = 14,
  IMAGED_COLOR_YUV = 15,
  IMAGED_COLOR_YUVA = 16,
  IMAGED_COLOR_HSL = 17,
  IMAGED_COLOR_HSLA = 18,
  IMAGED_COLOR_HSV = 19,
  IMAGED_COLOR_HSVA = 20,
  IMAGED_COLOR_LAST = IMAGED_COLOR_HSVA,
} ImagedColor;

extern size_t imagedColorChannelMap[];
extern const char *imagedColorNameMap[];
const char *imagedColorName(ImagedColor color);
const char *imagedTypeName(ImagedKind kind, uint8_t bits);
size_t imagedColorNumChannels(ImagedColor color);
bool imagedParseColorAndType(const char *color, const char *t, ImagedColor *c,
                             ImagedKind *kind, uint8_t *bits);
bool imagedIsValidType(ImagedKind kind, uint8_t bits);

typedef struct {
  uint64_t width, height;
  uint8_t bits;
  ImagedKind kind;
  ImagedColor color;
} ImagedMeta;

size_t imagedMetaNumPixels(const ImagedMeta *meta);
size_t imagedMetaTotalBytes(const ImagedMeta *meta);

typedef struct {
  ImagedMeta meta;
  void *data;
} Image;

Image *imageRead(const char *filename, ImagedColor color, ImagedKind kind,
                 uint8_t bits);
Image *imageReadDefault(const char *filename);
ImagedStatus imageWrite(const char *path, const Image *image);

Image *imageNew(ImagedMeta meta);
Image *imageAlloc(uint64_t w, uint64_t h, ImagedColor color, ImagedKind kind,
                  uint8_t bits, const void *data);
Image *imageClone(const Image *image);
void imageFree(Image *image);
size_t imagePixelBytes(Image *image);
size_t imageBytes(Image *image);
size_t imageIndex(Image *image, size_t x, size_t y);
void *imageAt(Image *image, size_t x, size_t y);

typedef struct {
  float data[4];
} Pixel;

bool imageGetPixel(Image *image, size_t x, size_t y, Pixel *pixel);
bool imageSetPixel(Image *image, size_t x, size_t y, const Pixel *pixel);

void pixelClamp(Pixel *px);
Pixel pixelEmpty(void);
Pixel pixelGray(float r);
Pixel pixelRGB(float r, float g, float b);
Pixel pixelRGBA(float r, float g, float b, float a);
void pixelAdd(const Pixel *src, Pixel *dest);
void pixelSub(const Pixel *src, Pixel *dest);
void pixelMul(const Pixel *src, Pixel *dest);
void pixelDiv(const Pixel *src, Pixel *dest);
bool pixelEq(const Pixel *a, const Pixel *b);
bool pixelEqAll(const Pixel *a, float v);
float pixelSum(const Pixel *a);

#define IMAGE_ITER(im, x, y, _x, _y, _w, _h, sx, sy)                           \
  uint64_t x, y;                                                               \
  for (y = _y; y < im->meta.height && y < _y + _h; y += sy)                    \
    for (x = _x; x < im->meta.width && x < _x + _w; x += sx)
#define IMAGE_ITER_ALL(im, x, y)                                               \
  uint64_t x, y;                                                               \
  for (y = 0; y < im->meta.height; y++)                                        \
    for (x = 0; x < im->meta.width; x++)

bool imageConvertTo(const Image *src, Image *dest);
Image *imageConvert(const Image *src, ImagedColor color, ImagedKind kind,
                    uint8_t bits);
bool imageConvertInPlace(Image **src, ImagedColor color, ImagedKind kind,
                         uint8_t bits);

void imageResizeTo(Image *src, Image *dest);
Image *imageResize(Image *src, size_t x, size_t y);
Image *imageScale(Image *src, double scale_x, double scale_y);
Image *imageConsume(Image *x, Image **dest);

typedef struct ImagedHandle {
  int fd;
  Image image;
} ImagedHandle;

void imagedResetLocks(Imaged *db);
bool imagedKeyIsLocked(Imaged *db, const char *key, ssize_t keylen);
bool imagedIsValidFile(Imaged *db, const char *key, ssize_t keylen);
bool imagedWait(ImagedStatus status);

// Open a new imaged context
Imaged *imagedOpen(const char *path);

// Close an imaged context
void imagedClose(Imaged *db);

// Destroy an imaged store, removing all contents from disk
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

// Release ImagedHandle resources including all memory and file descriptors
void imagedHandleClose(ImagedHandle *handle);

void imagedHandleInit(ImagedHandle *handle);

typedef struct {
  Imaged *db;
  DIR *d;
  struct dirent *ent;
  const char *key;
  size_t keylen;
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
void defer_Image(Image **db);
void defer_Imaged(Imaged **db);
void defer_ImagedIter(ImagedIter **iter);
void defer_ImagedHandle(ImagedHandle *h);
#define $(b, t, v) t v __attribute__((cleanup(defer_##b)))
#define $_(t, v) $(t, t, *v)
#define $free(t, v) $_(free, t, v)
#define $Imaged(v) $_(Imaged, v)
#define $Image(v) $_(Image, v)
#define $ImagedIter(v) $_(ImagedIter, v)
#define $ImagedHandle(v)                                                       \
  $(ImagedHandle, ImagedHandle, v);                                            \
  imagedHandleInit(&v);
#endif

#ifdef __cplusplus
}
#endif

#ifdef IMAGED_HALIDE
#include "HalideRuntime.h"
#include <stdlib.h>
static halide_type_code_t getType(ImagedKind kind) {
  switch (kind) {
  case IMAGED_KIND_UINT:
    return halide_type_uint;
  case IMAGED_KIND_INT:
    return halide_type_int;
  case IMAGED_KIND_FLOAT:
    return halide_type_float;
  }
}

IMAGED_UNUSED static void imageNewHalideBuffer(Image *image,
                                               halide_buffer_t *buffer) {
  size_t channels = imagedColorNumChannels(image->meta.color);
  buffer->device = 0;
  buffer->device_interface = NULL;
  buffer->host = (uint8_t *)image->data;
  buffer->dimensions = channels == 1 ? 2 : 3;
  buffer->dim = (halide_dimension_t *)malloc(sizeof(halide_dimension_t) *
                                             buffer->dimensions);
  if (buffer->dim == NULL) {
    buffer->host = NULL;
    return;
  }

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
    buffer->dim[2].min = 0;
    buffer->dim[2].extent = channels;
    buffer->dim[2].stride = 1;
    buffer->dim[2].flags = 0;

    // width
    buffer->dim[0].min = 0;
    buffer->dim[0].extent = image->meta.width;
    buffer->dim[0].stride = channels;
    buffer->dim[0].flags = 0;

    // height
    buffer->dim[1].min = 0;
    buffer->dim[1].extent = image->meta.height;
    buffer->dim[1].stride = image->meta.width * channels;
    buffer->dim[1].flags = 0;
  }

  buffer->type.code = getType(image->meta.kind);
  buffer->type.bits = image->meta.bits;
  buffer->type.lanes = 1;
}

IMAGED_UNUSED static void imageFreeHalideBuffer(halide_buffer_t *buffer) {
  free(buffer->dim);
}

IMAGED_UNUSED static void defer_HalideBuffer(halide_buffer_t *b) {
  if (b) {
    imageFreeHalideBuffer(b);
  }
}

#define $HalideBuffer(v) $(HalideBuffer, halide_buffer_t, v)

#endif // IMAGED_HALIDE

#endif

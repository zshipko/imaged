#ifndef IMAGED_HEADER_FILE
#define IMAGED_HEADER_FILE

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

/** Utility for creating new strings */
char *imagedStringPrintf(const char *fmt, ...);

/** Status types: IMAGED_OK implies the function executed successfully, while
 * any other response signifies failure */
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

/** Convert ImagedStatus to an error message */
const char *imagedError(ImagedStatus status);

/** Dump ImagedStatus error message to stderr */
void imagedPrintError(ImagedStatus status, const char *message);

struct ImagedHandle;

/** Image database */
typedef struct {
  char *root;
} Imaged;

/** Image kinds, specifies the image data base type */
typedef enum {
  IMAGE_KIND_INT,
  IMAGE_KIND_UINT,
  IMAGE_KIND_FLOAT,
} ImageKind;

/** Image colors, specifies the image color type */
typedef enum {
  IMAGE_COLOR_GRAY = 1,
  IMAGE_COLOR_GRAYA = 2,
  IMAGE_COLOR_RGB = 3,
  IMAGE_COLOR_RGBA = 4,
  IMAGE_COLOR_CMYK = 5,
  IMAGE_COLOR_CMYKA = 6,
  IMAGE_COLOR_YCBCR = 7,
  IMAGE_COLOR_YCBCRA = 8,
  IMAGE_COLOR_CIELAB = 9,
  IMAGE_COLOR_CIELABA = 10,
  IMAGE_COLOR_CIELCH = 11,
  IMAGE_COLOR_CIELCHA = 12,
  IMAGE_COLOR_CIEXYZ = 13,
  IMAGE_COLOR_CIEXYZA = 14,
  IMAGE_COLOR_YUV = 15,
  IMAGE_COLOR_YUVA = 16,
  IMAGE_COLOR_HSL = 17,
  IMAGE_COLOR_HSLA = 18,
  IMAGE_COLOR_HSV = 19,
  IMAGE_COLOR_HSVA = 20,
  IMAGE_COLOR_CIEXYY = 21,
  IMAGE_COLOR_CIEXYYA = 22,
  IMAGE_COLOR_HCY = 23,
  IMAGE_COLOR_HCYA = 24,
  IMAGE_COLOR_LAST = IMAGE_COLOR_HCYA,
} ImageColor;

extern size_t imageColorChannelMap[];
extern const char *imageColorNameMap[];

/** Get name of color */
const char *imageColorName(ImageColor color);

/** Get name of type */
const char *imageTypeName(ImageKind kind, uint8_t bits);

/** Get number of channels in a color */
size_t imageColorNumChannels(ImageColor color);

/** Parse color and type names */
bool imageParseColorAndType(const char *color, const char *t, ImageColor *c,
                            ImageKind *kind, uint8_t *bits);

/** Returns true if the kind/bits create a valid image type */
bool imageIsValidType(ImageKind kind, uint8_t bits);

/** ImageMeta is used to store image metadata with information about the image
 * shape and type */
typedef struct {
  uint64_t width, height;
  uint8_t bits;
  ImageKind kind;
  ImageColor color;
} ImageMeta;

/** Get the number of pixels in an image */
size_t imageMetaNumPixels(const ImageMeta *meta);

/** Get the number of bytes in an image */
size_t imageMetaTotalBytes(const ImageMeta *meta);

/* Initialize a valid meta pointer with the given values */
void imageMetaInit(uint64_t w, uint64_t h, ImageColor color, ImageKind kind,
                   uint8_t bits, ImageMeta *meta);

/** Stores image data with associated metadata */
typedef struct {
  bool owner;
  ImageMeta meta;
  void *data;
} Image;

void imageRAWUseAutoBrightness(bool b);
void imageRAWUseCameraWhiteBalance(bool b);

/** Read an image from disk, the resulting image will be converted to match
 * color/kind/bits if needed */
Image *imageRead(const char *filename, ImageColor color, ImageKind kind,
                 uint8_t bits);

/** Read an image from disk, using the default format **/
Image *imageReadDefault(const char *filename);

/** Write an image to disk */
ImagedStatus imageWrite(const char *path, const Image *image);

/** Create a new image with the given metadata */
Image *imageNew(ImageMeta meta);

/** Create a new image from an existing buffer */
Image *imageNewWithData(ImageMeta meta, void *data);

/** Create a new image and copy data if provided */
Image *imageAlloc(uint64_t w, uint64_t h, ImageColor color, ImageKind kind,
                  uint8_t bits, const void *data);

/** Duplicate an existing image */
Image *imageClone(const Image *image);

/** Free allocated image */
void imageFree(Image *image);

/** Get the number of bytes in a pixel for the given image */
size_t imagePixelBytes(Image *image);

/** Get the number of bytes in an image */
size_t imageBytes(Image *image);

/** Get the data offset at the position (x, y) */
size_t imageIndex(Image *image, size_t x, size_t y);

/** Get a pointer to the data at the position (x, y) */
void *imageAt(Image *image, size_t x, size_t y);

/** 4-channel floating point pixel */
typedef struct {
  float data[4];
} Pixel;

/** Get pixel at position (x, y) */
bool imageGetPixel(Image *image, size_t x, size_t y, Pixel *pixel);

/** Set pixel at position (x, y) */
bool imageSetPixel(Image *image, size_t x, size_t y, const Pixel *pixel);

/** Ensures pixel values are between 0 and 1 */
void pixelClamp(Pixel *px);

/** Create a new empty pixel */
Pixel pixelEmpty(void);

/** Create a new gray pixel */
Pixel pixelGray(float r);

/** Create a new RGB pixel */
Pixel pixelRGB(float r, float g, float b);

/** Create a new RGBA pixel */
Pixel pixelRGBA(float r, float g, float b, float a);

/** Pixel addition */
void pixelAdd(const Pixel *src, Pixel *dest);

/** Pixel subtraction */
void pixelSub(const Pixel *src, Pixel *dest);

/** Pixel multiplication */
void pixelMul(const Pixel *src, Pixel *dest);

/** Pixel division */
void pixelDiv(const Pixel *src, Pixel *dest);

/** Pixel equality */
bool pixelEq(const Pixel *a, const Pixel *b);

/** Pixel equality against a single value */
bool pixelEqAll(const Pixel *a, float v);

/** Sun of all pixel channels */
float pixelSum(const Pixel *a);

#define IMAGE_ITER(im, x, y, _x, _y, _w, _h, sx, sy)                           \
  uint64_t x, y;                                                               \
  for (y = _y; y < im->meta.height && y < _y + _h; y += sy)                    \
    for (x = _x; x < im->meta.width && x < _x + _w; x += sx)
#define IMAGE_ITER_ALL(im, x, y)                                               \
  uint64_t x, y;                                                               \
  for (y = 0; y < im->meta.height; y++)                                        \
    for (x = 0; x < im->meta.width; x++)

/** Adjust image gamma */
void imageAdjustGamma(Image *src, float gamma);

/** Convert source image to the format specified by the destination image */
bool imageConvertTo(const Image *src, Image *dest);

/** Convert source image to the specified type, returning the new converted
 * image */
Image *imageConvert(const Image *src, ImageColor color, ImageKind kind,
                    uint8_t bits);

/** Convert source image to the specified type */
bool imageConvertInPlace(Image **src, ImageColor color, ImageKind kind,
                         uint8_t bits);

Image *imageConvertACES0(Image *src);
Image *imageConvertACES0ToXYZ(Image *src);
Image *imageConvertACES1(Image *src);
Image *imageConvertACES1ToXYZ(Image *src);

/** Resize source image to size specified by destination image */
void imageResizeTo(Image *src, Image *dest);

/** Resize image to the given size, returns a new image */
Image *imageResize(Image *src, size_t x, size_t y);

/** Scale an image using the given factors, returns a new image */
Image *imageScale(Image *src, double scale_x, double scale_y);

Image *imageConsume(Image *x, Image **dest);

/** A handle is used to refer to an imgd image in an Imaged database */
typedef struct ImagedHandle {
  int fd;
  Image image;
} ImagedHandle;

/** Remove all image locks */
void imagedResetLocks(Imaged *db);

/** Returns true when an image is locked */
bool imagedKeyIsLocked(Imaged *db, const char *key, ssize_t keylen);

/** Returns true when the specified file is an valid imgd file */
bool imagedIsValidFile(Imaged *db, const char *key, ssize_t keylen);

/** Wait for an image to become available */
bool imagedWait(ImagedStatus status);

/** Open a new imaged context */
Imaged *imagedOpen(const char *path);

/** Close an imaged context */
void imagedClose(Imaged *db);

/** Destroy an imaged store, removing all contents from disk */
ImagedStatus imagedDestroy(Imaged *db);

/** Returns true when there is a value associated with the given key */
bool imagedHasKey(Imaged *db, const char *key, ssize_t keylen);

/** Set a key */
ImagedStatus imagedSet(Imaged *db, const char *key, ssize_t keylen,
                       ImageMeta meta, const void *imagedata,
                       ImagedHandle *handle);

/** Get a key */
ImagedStatus imagedGet(Imaged *db, const char *key, ssize_t keylen,
                       bool editable, ImagedHandle *handle);

/** &Remove the value associated with the provided key */
ImagedStatus imagedRemove(Imaged *db, const char *key, ssize_t keylen);

/** Release ImagedHandle resources including all memory and file descriptors
 */
void imagedHandleClose(ImagedHandle *handle);

/** Initialize an new handle */
void imagedHandleInit(ImagedHandle *handle);

/** Iterator over imgd files in an Imaged database */
typedef struct {
  Imaged *db;
  DIR *d;
  struct dirent *ent;
  const char *key;
  size_t keylen;
  ImagedHandle handle;
} ImagedIter;

/** Create a new iterator */
ImagedIter *imagedIterNew(Imaged *db);

/** Get next image */
Image *imagedIterNext(ImagedIter *iter);

/** Get next key */
const char *imagedIterNextKey(ImagedIter *iter);

/** Free iterator */
void imagedIterFree(ImagedIter *iter);
void imagedIterReset(ImagedIter *iter);

typedef bool (*imageParallelFn)(uint64_t, uint64_t, Pixel *, void *);
ImagedStatus imageEachPixel2(Image *im, Image *dst, imageParallelFn fn,
                             int nthreads, void *userdata);
ImagedStatus imageEachPixel(Image *im, imageParallelFn fn, int nthreads,
                            void *userdata);

// UTIL
#define IMAGED_UNUSED __attribute__((unused))

typedef struct halide_buffer_t halide_buffer_t;

#ifndef IMAGED_NO_DEFER
void defer_free(void *data);
void defer_Image(Image **db);
void defer_Imaged(Imaged **db);
void defer_ImagedIter(ImagedIter **iter);
void defer_ImagedHandle(ImagedHandle *h);
void defer_HalideBuffer(halide_buffer_t *b);
#define $(b, t, v) t v __attribute__((cleanup(defer_##b)))
#define $_(t, v) $(t, t, *v)
#define $free(t, v) $_(free, t, v)
#define $Imaged(v) $_(Imaged, v)
#define $Image(v) $_(Image, v)
#define $ImagedIter(v) $_(ImagedIter, v)
#define $ImagedHandle(v)                                                       \
  $(ImagedHandle, ImagedHandle, v);                                            \
  imagedHandleInit(&v);
#define $HalideBuffer(v) $(HalideBuffer, halide_buffer_t, v)
#endif

#ifdef __cplusplus
}
#endif

void imageNewHalideBuffer(Image *image, halide_buffer_t *buffer);
void imageFreeHalideBuffer(halide_buffer_t *buffer);

#endif

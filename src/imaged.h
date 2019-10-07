#ifndef __IMAGED_H
#define __IMAGED_H

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

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

Image *imageAlloc(uint64_t w, uint64_t h, uint64_t c, ImagedKind kind,
                  uint8_t bits, void *data);
void imageFree(Image *image);
size_t imagePixelBytes(Image *image);
size_t imageIndex(Image *image, size_t x, size_t y);
void *imageAt(Image *image, size_t x, size_t y);

typedef struct {
  int fd;
  Image image;
} ImagedHandle;

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
                       ImagedHandle *handle);

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
void imagedIterFree(ImagedIter *iter);

// UTIL
#ifndef unused
#define unused __attribute__((unused))
#endif

#ifndef IMAGED_NO_DEFER
void defer_free(void *data);
void defer_Imaged(Imaged **db);
void defer_ImagedIter(ImagedIter **iter);
void defer_ImagedHandle(ImagedHandle **image);
#define defer_(b, t, v) t *v __attribute__((cleanup(defer_##b)))
#define defer(t, v) defer_(t, t, v)
#endif

#endif

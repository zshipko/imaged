#define _GNU_SOURCE
#include "imaged.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void defer_free(void *data) {
  void **ptr = (void **)data;
  if (ptr != NULL && *ptr != NULL) {
    free(*ptr);
    *ptr = NULL;
  }
}

void defer_Image(Image **image) {
  if (image && *image) {
    imageFree(*image);
    *image = NULL;
  }
}

void defer_Imaged(Imaged **db) {
  if (db && *db) {
    imagedClose(*db);
    *db = NULL;
  }
}

void defer_ImagedIter(ImagedIter **iter) {
  if (iter && *iter) {
    imagedIterFree(*iter);
    *iter = NULL;
  }
}

void defer_ImagedHandle(ImagedHandle *h) {
  if (h) {
    imagedHandleClose(h);
  }
}

char *imagedStringPrintf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *ptr = NULL;
  if (vasprintf(&ptr, fmt, args) == -1) {
    ptr = NULL;
  }
  va_end(args);
  return ptr;
}

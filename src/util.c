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
};

void defer_Imaged(Imaged **db) {
  if (db && *db) {
    imagedClose(*db);
  }
}

void defer_ImagedIter(ImagedIter **iter) {
  if (iter && *iter) {
    imagedIterFree(*iter);
  }
}

void defer_ImagedHandle(ImagedHandle **image) {
  if (image && *image) {
    imagedHandleFree(*image);
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

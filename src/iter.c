#define _DEFAULT_SOURCE
#include "imaged.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

ImagedIter *imagedIterNew(Imaged *db) {
  ImagedIter *iter = malloc(sizeof(ImagedIter));
  if (iter == NULL) {
    return NULL;
  }

  iter->d = opendir(db->root);
  if (iter->d == NULL) {
    free(iter);
    return NULL;
  }

  iter->db = db;
  iter->ent = NULL;
  iter->handle.image.data = NULL;
  iter->handle.fd = -1;

  return iter;
}

void imagedIterReset(ImagedIter *iter) {
  if (iter != NULL && iter->d != NULL) {
    rewinddir(iter->d);
  }
}

Image *imagedIterNext(ImagedIter *iter) {
  if (iter == NULL) {
    return NULL;
  }

  if (iter->handle.image.data != NULL) {
    imagedHandleClose(&iter->handle);
  }

  struct dirent *ent = readdir(iter->d);
  if (ent == NULL) {
    return NULL;
  }
  iter->key = ent->d_name;
  iter->keylen = strlen(iter->key);

  if (ent->d_type == DT_DIR) {
    return imagedIterNext(iter);
  }

  if (imagedGet(iter->db, ent->d_name, -1, true, &iter->handle) != IMAGED_OK) {
    return imagedIterNext(iter);
  }

  iter->handle.image.owner = false;

  return &iter->handle.image;
}

const char *imagedIterNextKey(ImagedIter *iter) {
  if (iter == NULL) {
    return NULL;
  }

  struct dirent *ent = readdir(iter->d);
  if (ent == NULL) {
    return NULL;
  }
  iter->key = ent->d_name;
  iter->keylen = strlen(iter->key);
  iter->handle.image.data = NULL;

  if (ent->d_type == DT_DIR || !imagedIsValidFile(iter->db, ent->d_name, -1)) {
    return imagedIterNextKey(iter);
  }

  return iter->key;
}

void imagedIterFree(ImagedIter *iter) {
  if (iter == NULL) {
    return;
  }

  if (iter->handle.image.data != NULL) {
    imagedHandleClose(&iter->handle);
  }

  closedir(iter->d);
  free(iter);
}

#define _DEFAULT_SOURCE
#include "imaged.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static void mkdir_all(const char *dir) {
  char tmp[PATH_MAX];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", dir);
  len = strlen(tmp);
  if (tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for (p = tmp + 1; *p; p++)
    if (*p == '/') {
      *p = 0;
      mkdir(tmp, 0755);
      *p = '/';
    }
  mkdir(tmp, 0755);
}

static void close_unlock(int fd) {
  flock(fd, LOCK_UN);
  close(fd);
}

static bool fileExists(const char *path, struct stat *st) {
  struct stat stx;
  if (stat(path, &stx) == -1) {
    return false;
  }

  if (st) {
    *st = stx;
  }

  return true;
}

static char *pathJoin(const char *a, const char *b, ssize_t blen) {
  size_t a_len = strlen(a);
  size_t b_len = blen <= 0 ? strlen(b) : (size_t)blen;

  if (a[a_len - 1] == '/') {
    a_len -= 1;
  }

  if (b[0] == '/') {
    b += 1;
    b_len -= 1;
  }

  size_t outlen = a_len + b_len + 2;
  char *s = malloc(outlen);
  if (s == NULL) {
    return NULL;
  }

  s[a_len] = '/';
  memcpy(s, a, a_len);
  memcpy(s + a_len + 1, b, b_len);
  s[outlen - 1] = '\0';
  return s;
}

static bool isValidKey(const char *key, ssize_t len) {
  if (len <= 0) {
    len = (ssize_t)strlen(key);
  }

  for (ssize_t i = 0; i < len; i++) {
    if (key[i] == '/') {
      return false;
    }
  }

  return true;
}

const char *imagedError(ImagedStatus status) {
  if (errno != 0) {
    return strerror(errno);
  }

  switch (status) {
  case IMAGED_OK:
    return "OK";
  case IMAGED_ERR:
    return "error";
  case IMAGED_ERR_CANNOT_CREATE_FILE:
    return "cannot create file";
  case IMAGED_ERR_FILE_DOES_NOT_EXIST:
    return "file does not exist";
  case IMAGED_ERR_FILE_ALREADY_EXISTS:
    return "file already exists";
  case IMAGED_ERR_SEEK:
    return "seek error";
  case IMAGED_ERR_MAP_FAILED:
    return "unable to map file";
  case IMAGED_ERR_INVALID_KEY:
    return "invalid key";
  case IMAGED_ERR_INVALID_FILE:
    return "invalid file";
  }

  return "unknown";
}

void imagedPrintError(ImagedStatus status, const char *message) {
  fprintf(stderr, "Error: %s - %s\n", message, imagedError(status));
}

size_t imagedMetaTotalBytes(const ImagedMeta *meta) {
  return meta->width * meta->height * (size_t)meta->channels *
         ((size_t)meta->bits / 8);
}

Imaged *imagedOpen(const char *path) {
  char template[] = "/tmp/imaged.XXXXXX";
  if (path == NULL) {
    path = mkdtemp(template);
    if (path == NULL) {
      return NULL;
    }
  }

  char *root = strndup(path, strlen(path));
  if (root == NULL) {
    return NULL;
  }

  mkdir_all(path);

  Imaged *db = malloc(sizeof(Imaged));
  if (db == NULL) {
    return NULL;
  }

  db->root = root;
  return db;
}

void imagedClose(Imaged *db) {
  if (db == NULL) {
    free(db->root);
    free(db);
  }
}

ImagedStatus imagedDestroy(Imaged *db) {
  Image *img = NULL;
  defer(ImagedIter, iter) = imagedIterNew(db);
  while ((img = imagedIterNext(iter))) {
    imagedHandleFree(&iter->handle);
    imagedRemove(db, iter->key, -1);
  }

  if (rmdir(db->root) == -1) {
    return IMAGED_ERR;
  }

  imagedClose(db);

  return IMAGED_OK;
}

bool imagedHasKey(Imaged *db, const char *key, ssize_t keylen) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  defer_(free, char, path) = pathJoin(db->root, key, keylen);
  return fileExists(path, NULL);
}

ImagedStatus imagedSet(Imaged *db, const char *key, ssize_t keylen,
                       ImagedMeta meta, const void *imagedata,
                       ImagedHandle *handle) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  defer_(free, char, path) = pathJoin(db->root, key, keylen);

  int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0655);
  if (fd < 0) {
    return IMAGED_ERR;
  }
  flock(fd, LOCK_EX);

  size_t map_size = sizeof(ImagedMeta) + imagedMetaTotalBytes(&meta);
  if (lseek(fd, map_size, SEEK_SET) == -1) {
    close_unlock(fd);
    return IMAGED_ERR_SEEK;
  }

  if (write(fd, "", 1) != 1) {
    close_unlock(fd);
    return IMAGED_ERR;
  }

  void *data = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close_unlock(fd);
    return IMAGED_ERR_MAP_FAILED;
  }

  bzero(data, map_size);
  memcpy(data, &meta, sizeof(ImagedMeta));

  if (imagedata != NULL) {
    memcpy(data + sizeof(ImagedMeta), imagedata, imagedMetaTotalBytes(&meta));
  }

  if (handle == NULL) {
    close_unlock(fd);
    return IMAGED_OK;
  }

  handle->fd = fd;
  handle->image.meta = meta;
  handle->image.data = data + sizeof(ImagedMeta);

  return IMAGED_OK;
}

ImagedStatus imagedGet(Imaged *db, const char *key, ssize_t keylen,
                       bool editable, ImagedHandle *handle) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  defer_(free, char, path) = pathJoin(db->root, key, keylen);

  struct stat st;
  if (!fileExists(path, &st)) {
    return IMAGED_ERR_FILE_DOES_NOT_EXIST;
  }

  size_t map_size = st.st_size;
  if (map_size <= sizeof(ImagedMeta)) {
    unlink(path);
    return IMAGED_ERR_INVALID_FILE;
  }

  if (handle == NULL) {
    return IMAGED_OK;
  }

  int fd = open(path, O_CREAT | O_RDWR, 0655);
  if (fd < 0) {
    return IMAGED_ERR;
  }
  flock(fd, LOCK_EX);

  int flags = PROT_READ;
  if (editable) {
    flags |= PROT_WRITE;
  }
  void *data = mmap(0, map_size, flags, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close_unlock(fd);
    return IMAGED_ERR_MAP_FAILED;
  }

  handle->fd = fd;
  memcpy(&handle->image.meta, data, sizeof(ImagedMeta));
  handle->image.data = data + sizeof(ImagedMeta);

  if (imagedMetaTotalBytes(&handle->image.meta) + sizeof(ImagedMeta) !=
      st.st_size) {
    imagedHandleFree(handle);
    unlink(path);
    return IMAGED_ERR_INVALID_FILE;
  }

  return IMAGED_OK;
}

ImagedStatus imagedRemove(Imaged *db, const char *key, ssize_t keylen) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  defer_(free, char, path) = pathJoin(db->root, key, keylen);

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return IMAGED_ERR_FILE_DOES_NOT_EXIST;
  }
  flock(fd, LOCK_EX);
  remove(path);
  close_unlock(fd);
  return IMAGED_OK;
}

void imagedHandleFree(ImagedHandle *handle) {
  if (handle == NULL) {
    return;
  }

  if (handle->image.data != NULL) {
    void *ptr = handle->image.data - sizeof(ImagedMeta);
    size_t size =
        sizeof(ImagedMeta) + imagedMetaTotalBytes(&handle->image.meta);
    msync(ptr, size, MS_SYNC);
    munmap(ptr, size);
    handle->image.data = NULL;
  }

  if (handle->fd > 0) {
    close_unlock(handle->fd);
    handle->fd = -1;
  }
}

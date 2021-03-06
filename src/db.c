#define _DEFAULT_SOURCE
#include "imaged.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <assert.h>

static const char _header[4] = "imgd";
static size_t _header_size = sizeof(_header);

static void mkdirAll(const char *dir) {
  char tmp[PATH_MAX];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", dir);
  len = strlen(tmp);
  if (tmp[len - 1] == IMAGED_PATH_SEP)
    tmp[len - 1] = 0;
  for (p = tmp + 1; *p; p++)
    if (*p == IMAGED_PATH_SEP) {
      *p = 0;
      mkdir(tmp, 0755);
      *p = IMAGED_PATH_SEP;
    }
  mkdir(tmp, 0755);
}

static void close_unlock(int fd) {
  flock(fd, LOCK_UN);
  close(fd);
}

static bool fileExists(const char *path, struct stat *st) {
  struct stat tmp;
  if (stat(path, &tmp) == -1) {
    return false;
  }

  if (st) {
    *st = tmp;
  }

  return true;
}

static char *pathJoin(const char *a, const char *b, ssize_t blen) {
  size_t a_len = strlen(a);
  size_t b_len = blen <= 0 ? strlen(b) : (size_t)blen;

  if (a[a_len - 1] == IMAGED_PATH_SEP) {
    a_len -= 1;
  }

  if (b[0] == IMAGED_PATH_SEP) {
    b += 1;
    b_len -= 1;
  }

  size_t outlen = a_len + b_len + 2;
  char *s = malloc(outlen);
  if (s == NULL) {
    return NULL;
  }

  s[a_len] = IMAGED_PATH_SEP;
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
    if (key[i] == IMAGED_PATH_SEP) {
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
  case IMAGED_ERR_LOCKED:
    return "file already locked";
  }

  return "unknown";
}

void imagedPrintError(ImagedStatus status, const char *message) {
  fprintf(stderr, "Error: %s - %s\n", message, imagedError(status));
}

size_t imageMetaNumPixels(const ImageMeta *meta) {
  return meta->width * meta->height;
}

size_t imageMetaTotalBytes(const ImageMeta *meta) {
  return imageMetaNumPixels(meta) * imageColorNumChannels(meta->color) *
         ((size_t)meta->bits / 8);
}

bool imagedIsValidFile(const Imaged *db, const char *key, ssize_t keylen) {
  char *path = pathJoin(db->root, key, keylen);
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    free(path);
    return false;
  }

  struct stat st;
  stat(path, &st);

  char header[4];
  if (read(fd, header, 4) != 4 || strncmp(header, _header, _header_size) != 0) {
    close(fd);
    free(path);
    return false;
  }

  ImageMeta meta;

  if (read(fd, &meta, sizeof(ImageMeta)) != sizeof(ImageMeta)) {
    close(fd);
    free(path);
    return false;
  }

  close(fd);
  free(path);

  return imageMetaTotalBytes(&meta) + _header_size + sizeof(ImageMeta) + 1 ==
         (size_t)st.st_size;
}

bool imagedKeyIsLocked(const Imaged *db, const char *key, ssize_t keylen) {
  char *path = pathJoin(db->root, key, keylen);
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    free(path);
    return false;
  }

  bool r = flock(fd, LOCK_EX | LOCK_NB) != 0;
  if (!r) {
    flock(fd, LOCK_UN);
  }

  close(fd);
  free(path);
  return r;
}

void imagedResetLocks(Imaged *db) {
  DIR *dir = opendir(db->root);
  if (!dir) {
    return;
  }

  struct dirent *ent;

  while ((ent = readdir(dir))) {
    char *filename = pathJoin(db->root, ent->d_name, -1);
    if (filename) {
      int fd = open(filename, O_RDONLY);
      if (fd != -1) {
        flock(fd, LOCK_UN);
      }
      close(fd);
      free(filename);
    }
  }

  closedir(dir);
}

static char cwdBuf[PATH_MAX];

Imaged *imagedOpen(const char *path) {
  if (path == NULL) {
    path = getcwd(cwdBuf, PATH_MAX - 1);
  }

  char *root = strndup(path, strlen(path));
  if (root == NULL) {
    return NULL;
  }

  mkdirAll(path);

  Imaged *db = malloc(sizeof(Imaged));
  if (db == NULL) {
    free(root);
    return NULL;
  }

  db->root = root;
  return db;
}

void imagedClose(Imaged *db) {
  if (db != NULL) {
    free(db->root);
    free(db);
  }
}

ImagedStatus imagedDestroy(Imaged *db) {
  $ImagedIter(iter) = imagedIterNew(db);
  while (imagedIterNext(iter) != NULL) {
    imagedHandleClose(&iter->handle);
    imagedRemove(db, iter->key, -1);
  }

  rmdir(db->root);
  return IMAGED_OK;
}

bool imagedHasKey(const Imaged *db, const char *key, ssize_t keylen) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  char *path = pathJoin(db->root, key, keylen);
  bool res = fileExists(path, NULL);
  free(path);
  return res;
}

ImagedStatus imagedSet(Imaged *db, const char *key, ssize_t keylen,
                       const ImageMeta *meta, const void *imagedata,
                       ImagedHandle *handle) {
  imagedHandleInit(handle);
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  char *path = pathJoin(db->root, key, keylen);

  int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0655);
  if (fd < 0) {
    free(path);
    return IMAGED_ERR;
  }

  if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
    close(fd);
    free(path);
    return IMAGED_ERR_LOCKED;
  }

  size_t map_size =
      _header_size + sizeof(ImageMeta) + imageMetaTotalBytes(meta);
  if (lseek(fd, map_size, SEEK_SET) == -1) {
    close_unlock(fd);
    free(path);
    return IMAGED_ERR_SEEK;
  }

  if (write(fd, "", 1) != 1) {
    close_unlock(fd);
    free(path);
    return IMAGED_ERR;
  }

  void *data = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close_unlock(fd);
    free(path);
    return IMAGED_ERR_MAP_FAILED;
  }

  bzero(data, map_size);
  memcpy(data, _header, _header_size);
  memcpy((uint8_t *)data + _header_size, meta, sizeof(ImageMeta));

  if (imagedata != NULL) {
    memcpy((uint8_t *)data + _header_size + sizeof(ImageMeta), imagedata,
           imageMetaTotalBytes(meta));
  }

  if (handle == NULL) {
    close_unlock(fd);
    free(path);
    return IMAGED_OK;
  }

  handle->fd = fd;
  handle->image.meta = *(ImageMeta *)((uint8_t *)data + _header_size);
  handle->image.data = (uint8_t *)data + _header_size + sizeof(ImageMeta);
  handle->image.owner = false;

  free(path);

  return IMAGED_OK;
}

ImagedStatus imagedStat(Imaged *db, const char *key, ssize_t keylen,
                        struct stat *st) {
  char *path = pathJoin(db->root, key, keylen);
  ImagedStatus status = stat(path, st) == -1 ? IMAGED_ERR : IMAGED_OK;
  free(path);
  return status;
}

ImagedStatus imagedGet(Imaged *db, const char *key, ssize_t keylen,
                       bool editable, ImagedHandle *handle) {
  imagedHandleInit(handle);
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  char *path = pathJoin(db->root, key, keylen);

  struct stat st;
  if (!fileExists(path, &st)) {
    free(path);
    return IMAGED_ERR_FILE_DOES_NOT_EXIST;
  }

  size_t map_size = st.st_size;
  if (map_size <= _header_size + sizeof(ImageMeta)) {
    free(path);
    return IMAGED_ERR_INVALID_FILE;
  }

  if (handle == NULL) {
    free(path);
    return IMAGED_OK;
  }

  int fd = open(path, (editable ? O_RDWR : O_RDONLY), 0655);
  if (fd < 0) {
    free(path);
    return IMAGED_ERR;
  }

  if (flock(fd, (editable ? LOCK_EX : LOCK_SH) | LOCK_NB) < 0) {
    free(path);
    close(fd);
    return IMAGED_ERR_LOCKED;
  }

  int flags = PROT_READ;
  if (editable) {
    flags |= PROT_WRITE;
  }
  void *data = mmap(0, map_size, flags, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close_unlock(fd);
    free(path);
    return IMAGED_ERR_MAP_FAILED;
  }

  if (strncmp(data, _header, _header_size) != 0) {
    munmap(data, map_size);
    close_unlock(fd);
    free(path);
    return IMAGED_ERR_INVALID_FILE;
  }

  handle->fd = fd;

  memcpy(&handle->image.meta, (uint8_t *)data + _header_size,
         sizeof(ImageMeta));
  handle->image.data = (uint8_t *)data + _header_size + sizeof(ImageMeta);
  handle->image.owner = false;

  if (imageMetaTotalBytes(&handle->image.meta) + _header_size +
          sizeof(ImageMeta) + 1 !=
      (size_t)st.st_size) {

    imagedHandleClose(handle);
    free(path);
    return IMAGED_ERR_INVALID_FILE;
  }

  free(path);
  return IMAGED_OK;
}

ImagedStatus imagedRemove(Imaged *db, const char *key, ssize_t keylen) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  char *path = pathJoin(db->root, key, keylen);

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    free(path);
    return IMAGED_ERR_FILE_DOES_NOT_EXIST;
  }

  char header[4];
  if ((read(fd, &header, _header_size)) != 4 ||
      strncmp(header, _header, _header_size) != 0) {
    free(path);
    close(fd);
    return IMAGED_ERR_INVALID_FILE;
  }

  if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
    free(path);
    close(fd);
    return IMAGED_ERR_LOCKED;
  }

  remove(path);
  free(path);
  close_unlock(fd);
  return IMAGED_OK;
}

void imagedHandleInit(ImagedHandle *handle) {
  if (handle) {
    bzero(&handle->image, sizeof(Image));
    handle->fd = -1;
  }
}

void imagedHandleClose(ImagedHandle *handle) {
  if (handle == NULL) {
    return;
  }

  if (handle->image.data != NULL && handle->fd >= 0) {
    void *ptr = handle->image.data - _header_size - sizeof(ImageMeta);
    size_t size = _header_size + sizeof(ImageMeta) +
                  imageMetaTotalBytes(&handle->image.meta);
    // msync(ptr, size, MS_SYNC);
    munmap(ptr, size);
    handle->image.data = NULL;
  }

  if (handle->fd >= 0) {
    close_unlock(handle->fd);
    handle->fd = -1;
  }
}

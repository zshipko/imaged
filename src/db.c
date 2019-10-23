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

size_t imagedMetaNumPixels(const ImagedMeta *meta) {
  return meta->width * meta->height;
}

size_t imagedMetaTotalBytes(const ImagedMeta *meta) {
  return imagedMetaNumPixels(meta) * imagedColorNumChannels(meta->color) *
         ((size_t)meta->bits / 8);
}

bool imagedKeyIsLocked(Imaged *db, const char *key, ssize_t keylen) {
  $(free, char, *path) = pathJoin(db->root, key, keylen);
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return false;
  }
  bool r = flock(fd, LOCK_EX | LOCK_NB) != 0;
  if (!r) {
    flock(fd, LOCK_UN);
  }
  close(fd);
  return r;
}

bool imagedWait(ImagedStatus status) { return status == IMAGED_ERR_LOCKED; }

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

#ifdef _WIN32
static char defaultRoot[] = "\\tmp\\imaged";
#else
static char defaultRoot[] = "/tmp/imaged";
#endif

Imaged *imagedOpen(const char *path) {
  if (path == NULL) {
    path = defaultRoot;
  }

  char *root = strndup(path, strlen(path));
  if (root == NULL) {
    return NULL;
  }

  mkdir_all(path);

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
  Image *img = NULL;

  $ImagedIter(iter) = imagedIterNew(db);
  while ((img = imagedIterNext(iter))) {
    imagedHandleClose(&iter->handle);
    imagedRemove(db, iter->key, -1);
  }

  if (rmdir(db->root) == -1) {
    return IMAGED_ERR;
  }

  return IMAGED_OK;
}

bool imagedHasKey(Imaged *db, const char *key, ssize_t keylen) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  $(free, char, *path) = pathJoin(db->root, key, keylen);
  return fileExists(path, NULL);
}

ImagedStatus imagedSet(Imaged *db, const char *key, ssize_t keylen,
                       ImagedMeta meta, const void *imagedata,
                       ImagedHandle *handle) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  $(free, char, *path) = pathJoin(db->root, key, keylen);

  int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0655);
  if (fd < 0) {
    return IMAGED_ERR;
  }

  if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
    close(fd);
    return IMAGED_ERR_LOCKED;
  }

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

  $(free, char, *path) = pathJoin(db->root, key, keylen);

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

  int fd = open(path, (editable ? O_CREAT | O_RDWR : O_RDONLY), 0655);
  if (fd < 0) {
    return IMAGED_ERR;
  }

  if (flock(fd, (editable ? LOCK_EX : LOCK_SH) | LOCK_NB) < 0) {
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
    return IMAGED_ERR_MAP_FAILED;
  }

  handle->fd = fd;
  memcpy(&handle->image.meta, data, sizeof(ImagedMeta));
  handle->image.data = data + sizeof(ImagedMeta);

  if (imagedMetaTotalBytes(&handle->image.meta) + sizeof(ImagedMeta) + 1 !=
      (size_t)st.st_size) {
    imagedHandleClose(handle);
    unlink(path);
    return IMAGED_ERR_INVALID_FILE;
  }

  return IMAGED_OK;
}

ImagedStatus imagedRemove(Imaged *db, const char *key, ssize_t keylen) {
  if (!isValidKey(key, keylen)) {
    return IMAGED_ERR_INVALID_KEY;
  }

  $(free, char, *path) = pathJoin(db->root, key, keylen);

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return IMAGED_ERR_FILE_DOES_NOT_EXIST;
  }
  if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
    close(fd);
    return IMAGED_ERR_LOCKED;
  }
  remove(path);
  close_unlock(fd);
  return IMAGED_OK;
}

void imagedHandleClose(ImagedHandle *handle) {
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

  if (handle->fd >= 0) {
    close_unlock(handle->fd);
    handle->fd = -1;
  }
}

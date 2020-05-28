#include <pthread.h>
#include <unistd.h>

#include "imaged.h"

struct imageParallelIterator {
  uint64_t x0, y0, x1, y1;
  Image *im, *dst;
  imageParallelFn f;
  void *userdata;
};

static void *imageParallelWrapper(void *_iter) {
  struct imageParallelIterator *iter = (struct imageParallelIterator *)_iter;
  uint64_t x_start = iter->x0, x_end = iter->x1;
  uint64_t y_start = iter->y0, y_end = iter->y1;
  for (uint64_t j = y_start; j < y_end; j++) {
    for (uint64_t i = x_start; i < x_end; i++) {
      Pixel px;
      imageGetPixel(iter->im, i, j, &px);
      if (iter->f(i, j, iter->im, &px, iter->userdata)) {
        imageSetPixel(iter->dst, i, j, &px);
      }
    }
  }

  return NULL;
}

ImagedStatus imageEachPixel2(Image *im, Image *dst, imageParallelFn fn,
                             int nthreads, void *userdata) {
  if (im == NULL) {
    return IMAGED_ERR;
  }

  if (dst == NULL) {
    dst = im;
  }

  if (nthreads <= 0) {
    nthreads = sysconf(_SC_NPROCESSORS_ONLN);
  } else if (nthreads == 1) {
    Pixel px = pixelEmpty();
    IMAGE_ITER_ALL(im, i, j) {
      imageGetPixel(im, i, j, &px);
      if (fn(i, j, im, &px, userdata)) {
        imageSetPixel(dst, i, j, &px);
      }
    }
    return IMAGED_OK;
  }

  pthread_t threads[nthreads];
  struct imageParallelIterator iter[nthreads];
  int n = 0;
  uint64_t chunk = im->meta.height / (uint64_t)nthreads;
  for (int64_t x = 0; x < nthreads; x++) {
    iter[x].x1 = im->meta.width;
    iter[x].x0 = 0;
    iter[x].y1 = chunk * x + chunk;
    iter[x].y0 = chunk * x;
    iter[x].userdata = userdata;
    iter[x].dst = dst;
    iter[x].im = im;
    iter[x].f = fn;
    if (pthread_create(&threads[x], NULL, imageParallelWrapper, &iter[x]) !=
        0) {
      for (n = 0; n < x; n++) {
        pthread_join(threads[n], NULL);
      }
      return IMAGED_ERR;
    }
  }

  for (n = 0; n < nthreads; n++) {
    // Maybe do something if this fails?
    pthread_join(threads[n], NULL);
  }

  return IMAGED_OK;
}

ImagedStatus imageEachPixel(Image *im, imageParallelFn fn, int nthreads,
                            void *userdata) {
  return imageEachPixel2(im, NULL, fn, nthreads, userdata);
}

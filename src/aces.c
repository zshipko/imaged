#include "imaged.h"

#include <stdlib.h>

Image *imageConvertACES0ToXYZ(Image *src) {
  Image *dest = imageConvert(src, IMAGED_COLOR_CIEXYZ, IMAGED_KIND_FLOAT, 32);
  if (!dest) {
    return NULL;
  }

  float r, g, b;
  float *data = dest->data;
  for (size_t i = 0; i < imagedMetaNumPixels(&src->meta) * 3; i += 3) {
    r = data[i];
    g = data[i + 1];
    b = data[i + 2];
    data[i] = 0.9525523959 * r + 0.0000000000 * g + 0.0000936786 * b;
    data[i + 1] = 0.3439664498 * r + 0.7281660966 * g + -0.0721325464 * b;
    data[i + 2] = 0.0000000000 * r + 0.0000000000 * g + 1.0088251844 * b;
  }

  return dest;
}

Image *imageConvertACES0(Image *src) {
  Image *dest = imageConvert(src, IMAGED_COLOR_CIEXYZ, IMAGED_KIND_FLOAT, 32);
  if (!dest) {
    return NULL;
  }

  float r, g, b;
  float *data = dest->data;
  for (size_t i = 0; i < imagedMetaNumPixels(&src->meta) * 3; i += 3) {
    r = data[i];
    g = data[i + 1];
    b = data[i + 2];

    data[i] = 1.0498110175 * r + 0.0000000000 * g + -0.0000974845 * b;
    data[i + 1] = -0.4959030231 * r + 1.3733130458 * g + 0.0982400361 * b;
    data[i + 2] = 0.0000000000 * r + 0.0000000000 * g + 0.9912520182 * b;
  }

  dest->meta.color = IMAGED_COLOR_RGB;

  return dest;
}

Image *imageConvertACES1ToXYZ(Image *src) {
  Image *dest = imageAlloc(src->meta.width, src->meta.height,
                           IMAGED_COLOR_CIEXYZ, IMAGED_KIND_FLOAT, 32, NULL);
  if (!dest) {
    return NULL;
  }

  float r, g, b;
  float *data = dest->data;
  for (size_t i = 0; i < imagedMetaNumPixels(&src->meta) * 3; i += 3) {
    r = data[i];
    g = data[i + 1];
    b = data[i + 2];

    data[i] = 0.6624541811 * r + 0.1340042065 * g + 0.1561876870 * b;
    data[i + 1] = 0.2722287168 * r + 0.6740817658 * g + 0.0536895174 * b;
    data[i + 2] = -0.0055746495 * r + 0.0040607335 * g + 1.0103391003 * b;
  }

  return dest;
}

Image *imageConvertACES1(Image *src) {
  Image *dest = imageAlloc(src->meta.width, src->meta.height,
                           IMAGED_COLOR_CIEXYZ, IMAGED_KIND_FLOAT, 32, NULL);
  if (!dest) {
    return NULL;
  }

  float r, g, b;
  float *data = dest->data;
  for (size_t i = 0; i < imagedMetaNumPixels(&src->meta) * 3; i += 3) {
    r = data[i];
    g = data[i + 1];
    b = data[i + 2];

    data[i] = 1.6410233797 * r + -0.3248032942 * g + -0.2364246952 * b;
    data[i + 1] = -0.6636628587 * r + 1.6153315917 * g + 0.0167563477 * b;
    data[i + 2] = 0.0117218943 * r + -0.0082844420 * g + 0.9883948585 * b;
  }

  dest->meta.color = IMAGED_COLOR_RGB;

  return dest;
}

#include "imaged.h"

Pixel pixelEmpty() {
#ifdef __SSE__
  Pixel px = {.data = _mm_set_ps1(0.0f)};
#else
  Pixel px = {.data = {0.0, 0.0, 0.0, 0.0}};
#endif
  return px;
}

Pixel pixelRGBA(float r, float g, float b, float a) {
#ifdef __SSE__
  Pixel px = {.data = _mm_set_ps(a, b, g, r)};
#else
  Pixel px = {.data = {r, g, b, a}};
#endif
  return px;
}

Pixel pixelGray(float r) { return pixelRGBA(r, r, r, 1.0); }

Pixel pixelRGB(float r, float g, float b) { return pixelRGBA(r, g, b, 1.0); }

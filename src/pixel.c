#include "imaged.h"

Pixel pixelEmpty(void) {
#ifdef __SSE__
  Pixel px = {.data = _mm_setzero_ps()};
#else
  Pixel px = {.data = {0.0, 0.0, 0.0, 0.0}};
#endif
  return px;
}

Pixel pixelNew(float r, float g, float b, float a) {
#ifdef __SSE__
  Pixel px = {.data = _mm_set_ps(a, b, g, r)};
#else
  Pixel px = {.data = {r, g, b, a}};
#endif
  return px;
}

Pixel pixelNew1(float r) { return pixelNew(r, r, r, 1.0); }

Pixel pixelNew3(float r, float g, float b) { return pixelNew(r, g, b, 1.0); }

void pixelClamp(Pixel *px) {
#define CLAMP(i)                                                               \
  px->data[i] = px->data[i] < 0.0 ? 0.0 : px->data[i] > 1.0 ? 1.0 : px->data[i];
  CLAMP(0);
  CLAMP(1);
  CLAMP(2);
  CLAMP(3);
#undef CLAMP
}

#if defined(__SSE__) || defined(__ARM_NEON)
#define PIXEL_OP(name, op)                                                     \
  void pixel##name(const Pixel *src, Pixel *dest) {                            \
    dest->data = src->data op dest->data;                                      \
  }
#else
#define PIXEL_OP(name, op)                                                     \
  void pixel##name(const Pixel *src, Pixel *dest) {                            \
    dest->data[0] = src->data[0] op dest->data[0];                             \
    dest->data[1] = src->data[1] op dest->data[1];                             \
    dest->data[2] = src->data[2] op dest->data[2];                             \
    dest->data[3] = src->data[3] op dest->data[3];                             \
  }
#endif

#if defined(__SSE__) || defined(__ARM_NEON)
#define PIXEL_OP_F(name, op)                                                   \
  void pixel##name##F(Pixel *src, float f) { src->data = src->data op f; }
#else
#define PIXEL_OP_F(name, op)                                                   \
  void pixel##name##F(Pixel *src, float f) {                                   \
    src->data[0] = src->data[0] op f;                                          \
    src->data[1] = src->data[1] op f;                                          \
    src->data[2] = src->data[2] op f;                                          \
    src->data[3] = src->data[3] op f;                                          \
  }
#endif

PIXEL_OP(Add, +);
PIXEL_OP(Sub, -);
PIXEL_OP(Mul, *);
PIXEL_OP(Div, /);

PIXEL_OP_F(Add, +);
PIXEL_OP_F(Sub, -);
PIXEL_OP_F(Mul, *);
PIXEL_OP_F(Div, /);

bool pixelEq(const Pixel *a, const Pixel *b) {
  return a->data[0] == b->data[0] && a->data[1] == b->data[1] &&
         a->data[2] == b->data[2] && a->data[3] == b->data[3];
}

bool pixelEqAll(const Pixel *a, float v) {
  return a->data[0] == v && a->data[1] == v && a->data[2] == v &&
         a->data[3] == v;
}

float pixelSum(const Pixel *a) {
  return a->data[0] + a->data[1] + a->data[2] + a->data[3];
}

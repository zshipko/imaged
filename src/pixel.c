#include "imaged.h"

Pixel pixelEmpty() {
  Pixel px = {.data = {0.0, 0.0, 0.0, 0.0}};
  return px;
}

Pixel pixelRGBA(float r, float g, float b, float a) {
  Pixel px = {.data = {r, g, b, a}};
  return px;
}

Pixel pixelGray(float r) { return pixelRGBA(r, r, r, 1.0); }

Pixel pixelRGB(float r, float g, float b) { return pixelRGBA(r, g, b, 1.0); }

void pixelClamp(Pixel *px) {
  int i;
  for (i = 0; i < 4; i++) {
    px->data[i] =
        px->data[i] < 0.0 ? 0.0 : px->data[i] > 1.0 ? 1.0 : px->data[i];
  }
}

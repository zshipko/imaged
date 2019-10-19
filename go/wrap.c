#include "../src/imaged.h"

ImagedMeta _meta(size_t w, size_t h, int color, ImagedKind kind, uint8_t bits) {
  ImagedMeta meta = {
      .width = w,
      .height = h,
      .color = color,
      .kind = kind,
      .bits = bits,
  };
  return meta;
}

#include "../src/imaged.h"

ImageMeta _meta(size_t w, size_t h, ImageColor color, ImageKind kind,
                uint8_t bits) {
  ImageMeta meta = {
      .width = w,
      .height = h,
      .color = color,
      .kind = kind,
      .bits = bits,
  };
  return meta;
}

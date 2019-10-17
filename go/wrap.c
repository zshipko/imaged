#include "../src/imaged.h"

ImagedMeta _meta(size_t w, size_t h, uint8_t channels, ImagedKind kind,
                 uint8_t bits) {
  ImagedMeta meta = {
      .width = w,
      .height = h,
      .channels = channels,
      .kind = kind,
      .bits = bits,
  };
  return meta;
}

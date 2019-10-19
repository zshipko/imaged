#include "imaged.h"

#include <string.h>

#include <babl/babl.h>

size_t imagedColorChannelMap[] = {
    0, // Undefined
    1, // GRAY
    2, // GRAYA
    3, // RGB
    4, // RGBA
    4, // CMYK
    5, // CMYKA
    3, // YCBCR
    4, // YCBCRA
    3, // CIELAB
    4, // CIELABA
    3, // CIELCH
    4, // CIELCHA
    3, // CIEXYZ
    4, // CIEXYZA
    3, // YUV
    4, // YUVA
    3, // HSL
    4, // HSLA
    3, // HSV
    4, // HSVA
};

size_t imagedColorNumChannels(ImagedColor color) {
  return imagedColorChannelMap[color];
}

const char *imagedColorNameMap[] = {
    NULL,                // Undefined
    "Y",                 // GRAY
    "YA",                // GRAYA
    "RGB",               // RGB
    "RGBA",              // RGBA
    "CYMK",              // CMYK
    "CMYKA",             // CMYKA
    "Y'CbCr",            // YCBCR
    "Y'CbCrA",           // YCBCRA
    "CIE Lab",           // LAB
    "CIE Lab alpha",     // LABA
    "CIE LCH(ab)",       // LCH
    "CIE LCH(ab) alpha", // LCHA
    "CIE XYZ",           // XYZ
    "CIE XYZ alpha",     // XYZA
    "CIE Yuv",           // YUV
    "CIE Yuv alpha",     // YUVA
    "HSL",               // HSL
    "HSLA",              // HSLA
    "HSV",               // HSV
    "HSVA",              // HSVA
};

const char *imagedColorName(ImagedColor color) {
  return imagedColorNameMap[color];
}

const char *imagedTypeName(ImagedKind kind, uint8_t bits) {
  switch (kind) {
  case IMAGED_KIND_INT:
    switch (bits) {
    case 8:
      return "i8";
    case 16:
      return "i16";
    case 32:
      return "i32";
    case 64:
      return "i64";
    }
    break;
  case IMAGED_KIND_UINT:
    switch (bits) {
    case 8:
      return "u8";
    case 16:
      return "u16";
    case 32:
      return "u32";
    case 64:
      return "u64";
    }
    break;
  case IMAGED_KIND_FLOAT:
    switch (bits) {
    case 16:
      return "half";
    case 32:
      return "float";
    case 64:
      return "double";
    }
    break;
  }

  return NULL;
}

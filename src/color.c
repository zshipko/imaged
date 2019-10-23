#define _GNU_SOURCE
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

bool imagedIsValidType(ImagedKind kind, uint8_t bits) {
  return imagedTypeName(kind, bits) != NULL;
}

bool imagedParseColorAndType(const char *color, const char *t, ImagedColor *c,
                             ImagedKind *kind, uint8_t *bits) {
  if (kind != NULL) {
    bool foundColor = false;
    for (ImagedColor d = IMAGED_COLOR_GRAY;
         !foundColor && d <= IMAGED_COLOR_LAST; d++) {
      if (strncasecmp(color, imagedColorName(d), strlen(color)) == 0) {
        *c = d;
        foundColor = true;
      }
    }

    if (!foundColor) {
      return false;
    }
  }

  if (t != NULL) {
    if (strncasecmp(t, "f32", 3) == 0 || strncasecmp(t, "float", 5) == 0) {
      *bits = 32;
      *kind = IMAGED_KIND_FLOAT;
    } else if (strncasecmp(t, "f16", 3) == 0 ||
               strncasecmp(t, "half", 4) == 0) {
      *bits = 16;
      *kind = IMAGED_KIND_FLOAT;
    } else if (strncasecmp(t, "f64", 3) == 0 ||
               strncasecmp(t, "double", 6) == 0) {
      *bits = 64;
      *kind = IMAGED_KIND_FLOAT;
    } else if (strncasecmp(t, "u8", 2) == 0 ||
               strncasecmp(t, "uint8", 5) == 0) {
      *bits = 8;
      *kind = IMAGED_KIND_UINT;
    } else if (strncasecmp(t, "u16", 3) == 0 ||
               strncasecmp(t, "uint16", 6) == 0) {
      *bits = 16;
      *kind = IMAGED_KIND_UINT;
    } else if (strncasecmp(t, "u32", 2) == 0 ||
               strncasecmp(t, "uint32", 6) == 0) {
      *bits = 32;
      *kind = IMAGED_KIND_UINT;
    } else if (strncasecmp(t, "i8", 2) == 0 ||
               strncasecmp(t, "iint8", 5) == 0) {
      *bits = 8;
      *kind = IMAGED_KIND_INT;
    } else if (strncasecmp(t, "i16", 3) == 0 ||
               strncasecmp(t, "iint16", 6) == 0) {
      *bits = 16;
      *kind = IMAGED_KIND_INT;
    } else if (strncasecmp(t, "i32", 2) == 0 ||
               strncasecmp(t, "iint32", 6) == 0) {
      *bits = 32;
      *kind = IMAGED_KIND_INT;
    } else {
      return false;
    }
  }

  return true;
}
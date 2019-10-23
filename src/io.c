#include "imaged.h"
#include <ezimage.h>

static ezimage_shape imagedMetaToEzimageShape(ImagedMeta meta) {
  ezimage_shape shape = {
      .width = meta.width,
      .height = meta.height,
      .channels = imagedColorNumChannels(meta.color),
      .t = {.bits = meta.bits, .kind = (ezimage_kind)meta.kind}};
  return shape;
}

static ImagedMeta imagedMetaFromEzimageShape(ezimage_shape shape) {
  ImagedMeta meta = {
      .width = shape.width,
      .height = shape.height,
      .color = shape.channels == 1
                   ? IMAGED_COLOR_GRAY
                   : shape.channels == 2
                         ? IMAGED_COLOR_GRAYA
                         : shape.channels == 3 ? IMAGED_COLOR_RGB
                                               : IMAGED_COLOR_RGBA,
      .bits = shape.t.bits,
      .kind = (ImagedKind)shape.t.kind,
  };
  return meta;
}

ImagedStatus imageWrite(const char *path, const Image *image) {
  // Only Gray/RGB/RGBA images
  if (image->meta.color > IMAGED_COLOR_RGBA) {
    $Image(tmp) = imageConvert(image, IMAGED_COLOR_RGB, IMAGED_KIND_UINT, 8);
    if (tmp == NULL) {
      return IMAGED_ERR;
    }

    return imageWrite(path, tmp);
  }

  ezimage_shape shape = imagedMetaToEzimageShape(image->meta);
  return ezimage_imwrite(path, image->data, &shape) ? IMAGED_OK : IMAGED_ERR;
}

Image *imageRead(const char *filename, ImagedColor color, ImagedKind kind,
                 uint8_t bits) {
  ezimage_shape shape;
  void *data = ezimage_imread(filename, NULL, &shape);
  if (!data) {
    return NULL;
  }

  if (shape.channels > 4) {
    free(data);
    return NULL;
  }

  Image *image = malloc(sizeof(Image));
  if (!image) {
    free(data);
    return NULL;
  }

  image->meta = imagedMetaFromEzimageShape(shape);
  image->data = data;

  if ((image->meta.color != color || color < 0) ||
      (image->meta.kind != kind || kind < 0) ||
      (image->meta.bits != bits || bits == 0)) {
    Image *tmp = imageConvert(image, color, kind, bits);
    imageFree(image);
    return tmp;
  }

  return image;
}

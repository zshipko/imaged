#include "imaged.h"
#include <ezimage.h>

static ezimage_shape imageMetaToEzimageShape(ImageMeta meta) {
  ezimage_shape shape = {
      .width = meta.width,
      .height = meta.height,
      .channels = imageColorNumChannels(meta.color),
      .t = {.bits = meta.bits, .kind = (ezimage_kind)meta.kind}};
  return shape;
}

static ImageMeta imageMetaFromEzimageShape(ezimage_shape shape) {
  ImageMeta meta = {
      .width = shape.width,
      .height = shape.height,
      .color =
          shape.channels == 1
              ? IMAGE_COLOR_GRAY
              : shape.channels == 2
                    ? IMAGE_COLOR_GRAYA
                    : shape.channels == 3 ? IMAGE_COLOR_RGB : IMAGE_COLOR_RGBA,
      .bits = shape.t.bits,
      .kind = (ImageKind)shape.t.kind,
  };
  return meta;
}

ImagedStatus imageWrite(const char *path, const Image *image) {
  // Only Gray/RGB/RGBA images
  if (image->meta.color > IMAGE_COLOR_RGBA) {
    $Image(tmp) = imageConvert(image, IMAGE_COLOR_RGB, IMAGE_KIND_UINT, 8);
    if (tmp == NULL) {
      return IMAGED_ERR;
    }

    return imageWrite(path, tmp);
  }

  ezimage_shape shape = imageMetaToEzimageShape(image->meta);
  return ezimage_imwrite(path, image->data, &shape) ? IMAGED_OK : IMAGED_ERR;
}

static bool imaged_raw_auto_bright = false;
static bool imaged_raw_camera_wb = true;

void imageRAWUseAutoBrightness(bool b) { imaged_raw_auto_bright = b; }
void imageRAWUseCameraWhiteBalance(bool b) { imaged_raw_camera_wb = b; }

#ifndef IMAGED_NO_RAW
#include <libraw/libraw.h>

Image *imageReadRAWNoProcess(const char *filename) {
  libraw_data_t *ctx = libraw_init(0);
  if (!ctx) {
    return NULL;
  }

  if (libraw_open_file(ctx, filename) != LIBRAW_SUCCESS) {
    goto err;
  }

  ctx->params.output_bps = 16;
  ctx->params.use_rawspeed = 1;
  ctx->params.no_auto_bright = imaged_raw_auto_bright == false;
  ctx->params.use_camera_wb = imaged_raw_camera_wb == true;
  ctx->params.use_auto_wb = imaged_raw_camera_wb == false;

  if (libraw_unpack(ctx) != LIBRAW_SUCCESS) {
    goto err;
  }

  Image *image =
      imageAlloc(ctx->rawdata.sizes.raw_width, ctx->rawdata.sizes.raw_height,
                 IMAGE_COLOR_GRAY, IMAGE_KIND_UINT, 16, ctx->rawdata.raw_image);
  if (image == NULL) {
    goto err;
  }

  return image;

err:
  libraw_free_image(ctx);
  libraw_close(ctx);
  return NULL;
}

Image *imageReadRAW(const char *filename) {
  libraw_data_t *ctx = libraw_init(0);
  if (!ctx) {
    return NULL;
  }

  if (libraw_open_file(ctx, filename) != LIBRAW_SUCCESS) {
    goto err;
  }

  ctx->params.output_bps = 16;
  ctx->params.use_rawspeed = 1;
  ctx->params.no_auto_bright = imaged_raw_auto_bright == false;
  ctx->params.use_camera_wb = imaged_raw_camera_wb == true;
  ctx->params.use_auto_wb = imaged_raw_camera_wb == false;

  if (libraw_unpack(ctx) != LIBRAW_SUCCESS) {
    goto err;
  }

  if (libraw_dcraw_process(ctx) != LIBRAW_SUCCESS) {
    goto err;
  }

  int errcode = LIBRAW_SUCCESS;
  libraw_processed_image_t *raw = libraw_dcraw_make_mem_image(ctx, &errcode);
  if (raw == NULL || errcode != LIBRAW_SUCCESS) {
    goto err;
  }

  Image *image = imageAlloc(raw->width, raw->height, IMAGE_COLOR_RGB,
                            IMAGE_KIND_UINT, raw->bits, raw->data);
  if (image == NULL) {
    goto err1;
  }

  libraw_dcraw_clear_mem(raw);
  libraw_free_image(ctx);
  libraw_close(ctx);
  return image;

err1:
  libraw_dcraw_clear_mem(raw);
err:
  libraw_free_image(ctx);
  libraw_close(ctx);
  return NULL;
}
#else

#include <stdio.h>
Image *imageReadRAWNoProcess(const char *filename) {
  (void)filename;
  fprintf(stderr, "LibRAW not found: imageReadRAWNoProcess");
  return NULL;
}

Image *imageReadRAW(const char *filename) {
  (void)filename;
  fprintf(stderr, "LibRAW not found: imageReadRAW");
  return NULL;
}

#endif

static Image *imageReadFile(const char *filename, ImageKind kind,
                            uint8_t bits) {
  ezimage_shape shape;
  ezimage_type ty = {
      .kind = (ezimage_kind)kind,
      .bits = bits,
  };
  void *data = ezimage_imread(filename, &ty, &shape);
  if (!data) {
    return NULL;
  }

  if (shape.channels > 4) {
    free(data);
    return NULL;
  }

  Image *image = imageNewWithData(imageMetaFromEzimageShape(shape), data);
  if (!image) {
    free(data);
    return NULL;
  }
  image->owner = true;
  return image;
}

Image *imageRead(const char *filename, ImageColor color, ImageKind kind,
                 uint8_t bits) {
  Image *image = imageReadFile(filename, kind, bits);
  if (image == NULL) {
#ifndef IMAGED_NO_RAW
    image = imageReadRAW(filename);
#else
    return NULL;
#endif
  }

  if (image == NULL) {
    return NULL;
  }

  if (color < 0 || color > IMAGE_COLOR_LAST) {
    color = IMAGE_COLOR_RGB;
  }

  if (kind < 0 || kind > IMAGE_KIND_FLOAT) {
    kind = IMAGE_KIND_FLOAT;
  }

  if (bits == 0) {
    bits = 32;
  }

  if (image->meta.color != color || image->meta.kind != kind ||
      image->meta.bits != bits) {
    imageConvertInPlace(&image, color, kind, bits);
  }

  return image;
}

Image *imageReadDefault(const char *filename) {
  return imageRead(filename, -1, -1, 0);
}

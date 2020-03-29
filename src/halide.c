#include "imaged.h"

#ifdef IMAGED_HALIDE

#include "HalideRuntime.h"
#include <stdlib.h>
static halide_type_code_t getType(ImageKind kind) {
  switch (kind) {
  case IMAGE_KIND_INT:
    return halide_type_int;
  case IMAGE_KIND_FLOAT:
    return halide_type_float;
  default:
    return halide_type_uint;
  }
}

void imageNewHalideBuffer(Image *image, halide_buffer_t *buffer) {
  size_t channels = imageColorNumChannels(image->meta.color);
  buffer->device = 0;
  buffer->device_interface = NULL;
  buffer->host = (uint8_t *)image->data;
  buffer->dimensions = channels == 1 ? 2 : 3;
  buffer->dim = (halide_dimension_t *)malloc(sizeof(halide_dimension_t) *
                                             buffer->dimensions);
  if (buffer->dim == NULL) {
    buffer->host = NULL;
    return;
  }

  if (buffer->dimensions == 2) {
    // width
    buffer->dim[0].min = 0;
    buffer->dim[0].extent = image->meta.width;
    buffer->dim[0].stride = 1;
    buffer->dim[0].flags = 0;

    // height
    buffer->dim[1].min = 0;
    buffer->dim[1].extent = image->meta.height;
    buffer->dim[1].stride = image->meta.width;
    buffer->dim[1].flags = 0;
  } else {
    // channels
    buffer->dim[2].min = 0;
    buffer->dim[2].extent = channels;
    buffer->dim[2].stride = 1;
    buffer->dim[2].flags = 0;

    // width
    buffer->dim[0].min = 0;
    buffer->dim[0].extent = image->meta.width;
    buffer->dim[0].stride = channels;
    buffer->dim[0].flags = 0;

    // height
    buffer->dim[1].min = 0;
    buffer->dim[1].extent = image->meta.height;
    buffer->dim[1].stride = image->meta.width * channels;
    buffer->dim[1].flags = 0;
  }

  buffer->type.code = getType(image->meta.kind);
  buffer->type.bits = image->meta.bits;
  buffer->type.lanes = 1;
}

void imageFreeHalideBuffer(halide_buffer_t *buffer) { free(buffer->dim); }

void defer_HalideBuffer(halide_buffer_t *b) {
  if (b) {
    imageFreeHalideBuffer(b);
  }
}

#else

#include <stdio.h>
#include <stdlib.h>

void imageNewHalideBuffer(IMAGED_UNUSED Image *image,
                          IMAGED_UNUSED halide_buffer_t *buffer) {
  fputs("IMAGED_HALIDE not enabled\n", stderr);
  abort();
}
void imageFreeHalideBuffer(IMAGED_UNUSED halide_buffer_t *buffer) { return; }

void defer_HalideBuffer(IMAGED_UNUSED halide_buffer_t *b) {
  (void)b;
  return;
}

#endif

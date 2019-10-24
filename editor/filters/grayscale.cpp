#define IMAGED_HALIDE_UTIL
#include "../../src/imaged.h"

class Grayscale : public Generator<Grayscale> {
public:
  Input<Buffer<float>> input{"input", 3};
  Output<Buffer<float>> grayscale{"grayscale", 3};
  Var x, y, c;

  void generate() {
    grayscale(x, y, c) = input(x, y, 0) * 0.299f + input(x, y, 1) * 0.587f +
                         input(x, y, 2) * 0.114f;
    interleave_input(input, 3, x, y, c);
    interleave_output(grayscale, 3, x, y, c);
  }
};

HALIDE_REGISTER_GENERATOR(Grayscale, grayscale);

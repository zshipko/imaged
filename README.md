# imaged - image processing and storage framework

<a href="https://crates.io/crates/imaged">
    <img src="https://img.shields.io/crates/v/imaged.svg">
</a>

`imaged` provides a simple interface for working with image data across applications.

## Features

- Share image data safely between processes
- Encode/decode many popular formats using [ezimage](https://github.com/zshipko/ezimage) and [libraw](https://github.com/libraw/libraw) (should be detected automatically, or enable using `RAW=1` when compiling)
- Convert between colorspaces using [babl](https://github.com/GNOME/babl)
- Developed for use with [Halide](https://github.com/halide/halide), just set `HALIDE=1` when building

## Installation

To get started to will need to following:

- GNU Make
- C compiler
- [babl](https://github.com/GNOME/babl)
  * `apt install libbabl-dev` (on Debian based distros)
  * `brew install babl` (on macOS)

Once all of the dependencies have been installed:

```shell
$ make
$ sudo make install
```

Or to build the library without the `imaged` utility:

```shell
$ make lib
$ sudo make install-lib
```

To include `imaged` in your application you can use pkg-config: `pkg-config --cflags --libs imaged`

## imaged utility

Run `imaged --help` for more information

## Bindings

- [Rust](https://github.com/zshipko/imaged/tree/master/rust)
- [Go](https://github.com/zshipko/imaged/tree/master/go)

## libimaged

```c
#include <imaged.h>
#include <stdio.h>

int main(void){
  // Open the current directory, this will allow you to locate and iterate over
  // `imgd` files in the specified directory
  $Imaged(db) = imagedOpen(".");
  if (db == NULL){
    return 1;
  }


  // Note: Using $Imaged will automatically free `db` when it goes out of scope,
  // you can also use imagedClose if you don't like the `$` syntax

  // Create a new floating-point file
  ImageMeta meta = {
    .width = 800,
    .height = 600,
    .color = IMAGED_COLOR_RGB,
    .kind = IMAGED_KIND_FLOAT,
    .bits = 32,
  };

  // $ImagedHandle will automatically free `handle` when it goes out of scope
  $ImagedHandle(handle); // when not using the `$` syntax, you will need to use
                         // imagedHandleInit to initialize a handle, and
                         // imagedHandleClose to free it

  // Set a new image named "test"
  if (imagedSet(db, "test", -1, meta, NULL, &handle) != IMAGED_OK){
    return 2;
  }

  // Set the center pixel
  Pixel px = pixelRGB(1.0, 0, 0);
  if (imageSetPixel(&handle.image, 400, 300, &px) != IMAGED_OK){
    // handle out of bounds error
  }

  // NOTE: the image is automatically saved, since we were working directly with data
  // from disk!

  return 0;
}
```

For a full listing of available types and functions see [src/imaged.h](https://github.com/zshipko/imaged/blob/master/src/imaged.h)

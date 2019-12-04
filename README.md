# imaged - image processing and storage framework

`imaged` provides a simple interface for working with images. This includes the ability to encode/decode many popular formats and methods of storing image data on disk for use in multiple applications.

## Installation

To get started to will need to following:

- GNU Make
- C compiler
- [babl](https://github.com/GNOME/babl)
  * `apt install libbabl-dev` on Debian based distros
  * `brew install babl` on macOS
- [ezimage](https://github.com/zshipko/ezimage)

Once all of the dependencies have been installed:

```shell
$ make
$ [sudo] make install
```

Or to build the library without the `imaged` executable:

```shell
$ make lib
$ [sudo] make install-lib
```

## Bindings

- `./rust`: contains Rust bindings
- `./go`: contains Go bindings

## Command-line

Run `imaged --help` for more information

## Library

```c
int main(void){
  // Open the current directory, this will allow you to locate and iterate over `imgd` files in the specified directory
  // Using $Imaged will automatically free `db` when it goes out of scope
  $Imaged(db) = imagedOpen(".");
  if (db == NULL){
    return 1;
  }

  // Create a new floating-point imgd file
  ImagedMeta meta = {
    .width = 800,
    .height = 600,
    .color = IMAGED_COLOR_RGB,
    .kind = IMAGED_KIND_FLOAT,
    .bits = 32,
  };

  // $ImagedHandle will automatically free `handle` when it goes out of scope
  $ImagedHandle(handle);

  // Set a new image named "test"
  if (imagedSet(db, "test", -1, meta, NULL, &handle) != IMAGED_OK){
    return 2;
  }

  // Set the center pixel
  Pixel px = pixelRGB(1.0, 0, 0);
  imageSet(&handle.image, 400, 300, &px);

  // NOTE: the image is automatically saved, since you set the image data on
  // disk directly!

  return 0;
}
```

For a full listing of available types and functions see [src/imaged.h](https://github.com/zshipko/imaged/blob/master/src/imaged.h)

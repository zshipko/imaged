# imaged - image processing and storage

`imaged` provides basic `Image` and `Pixel` types in addition to the `Imaged` image store type, which is used
to store raw pixel data on disk.

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

## GUI

`./editor` contains code for displaying images using GLFW

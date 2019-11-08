# imaged - image processing and storage framework

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

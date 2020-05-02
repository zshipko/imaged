# imaged

An image processing library in Rust built on [libimaged](https://github.com/zshipko/imaged).

`imaged` is focused on supporting a wide range of color conversions using [babl](http://www.gegl.org/babl/)

## Example

```rust
use imaged::*;

fn run() -> Result<(), Error>
    // Read image
    let image = Image::read_default("test.jpg")?;

    let mut px = Pixel::new();
    image.get_pixel(10, 10, &mut px)?;

    // Convert colorspace and typ
    let a = image.convert(Color::HSV, Type::F(32))?;

    // Save imag
    a.write("out.tiff")?;

    Ok(()
}
```

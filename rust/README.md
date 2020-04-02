# image2

Rust bindings for [libimaged](https://github.com/zshipko/imaged)


```rust
use image2::*;
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

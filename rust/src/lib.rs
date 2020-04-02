//! imaged is an image processing and storage library for working with a wide range of image types
//! For more information see [imaged](https://github.com/zshipko/imaged)
//!
//! ## Getting started
//!
//! ```rust
//! use image2::*;
//!
//! fn run() -> Result<(), Error> {
//!     // Read image
//!     let image = Image::read_default("test.jpg")?;
//!
//!     // Convert colorspace and type
//!     let a = image.convert(Color::HSV, Type::F(32))?;
//!
//!     // Save image
//!     a.write("out.tiff")?;
//!
//!     Ok(())
//! }
//!

use std::ffi::c_void;

/// Raw imaged bindings
pub mod ffi;

#[cfg(feature = "halide")]
mod halide_wrapper;

#[cfg(feature = "halide")]
pub use halide_runtime as halide;

#[cfg(feature = "viewer")]
pub mod viewer;

mod db;
mod error;
mod image;
mod iter;
mod pixel;

pub use db::*;
pub use error::Error;
pub use image::*;
pub use iter::*;
pub use pixel::*;

/// Global configuration flags
pub mod conf {
    use crate::*;

    /// Use autobrightness in libraw
    pub fn use_raw_auto_brightness(b: bool) {
        unsafe { ffi::imageRAWUseAutoBrightness(b) }
    }

    /// Use camera white balance in libraw
    pub fn use_raw_camera_white_balance(b: bool) {
        unsafe { ffi::imageRAWUseCameraWhiteBalance(b) }
    }
}

extern "C" {
    fn babl_init();
}

/// Initialize global data related to babl conversions
/// NOTE: This is only needed when working with multiple threads
pub fn init() {
    unsafe {
        babl_init();
    }
}

#[cfg(test)]
mod tests {
    use crate::*;

    #[test]
    fn it_works() {
        let db = DB::open("./testing").unwrap();
        let image = Image::new(Meta::new(800, 600, Color::RGB, Type::F(32))).unwrap();
        db.set("testing", &image).unwrap();
        {
            let mut iter = db.iter().unwrap();
            let (k, i) = iter.next().unwrap();
            assert!(k == "testing");
            assert!(i.meta().width() == 800, i.meta().height() == 600);
        }

        db.remove("testing").unwrap();

        {
            let mut iter = db.iter().unwrap();
            assert!(iter.next().is_none());
        }

        db.destroy().unwrap();
    }

    #[test]
    fn each_pixel() {
        let mut image = Image::new(Meta::new(800, 600, Color::RGB, Type::F(32))).unwrap();
        let mut image2 = Image::new(Meta::new(800, 600, Color::RGB, Type::F(32))).unwrap();
        image
            .each_pixel(None, |x, y, px| {
                let data = px.data_mut();
                data[0] = 1.0;
                data[2] = 0.5;
                data[3] = 0.25;
                image2.set_pixel(x, y, px);
                Ok(true)
            })
            .unwrap();

        for y in 0..600 {
            for x in 0..800 {
                assert!(image.at::<f32>(x, y).unwrap() == image2.at::<f32>(x, y).unwrap());
            }
        }
    }
}

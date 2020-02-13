//! imaged is an image processing and storage library for working with a wide range of image types
//! For more information see [imaged](https://github.com/zshipko/imaged)
//!
//! ## Getting started
//!
//! ```rust
//! use imaged::*;
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
    pub fn use_raw_auto_brighness(b: bool) {
        unsafe { ffi::imageRAWUseAutoBrightness(b) }
    }

    /// Use camera white balancei in libraw
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

lazy_static::lazy_static! {
    static ref _UNUSED: () = unsafe { babl_init() };
}

#[cfg(test)]
mod tests {
    use crate::*;

    #[test]
    fn it_works() {
        let db = DB::open("./testing").unwrap();
        db.destroy().unwrap();
    }
}

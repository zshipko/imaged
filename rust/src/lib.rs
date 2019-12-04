use std::ffi::c_void;

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

pub mod conf {
    use crate::*;

    pub fn use_raw_auto_brighness(b: bool) {
        unsafe { ffi::imageRAWUseAutoBrightness(b) }
    }

    pub fn use_raw_camera_white_balance(b: bool) {
        unsafe { ffi::imageRAWUseCameraWhiteBalance(b) }
    }
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

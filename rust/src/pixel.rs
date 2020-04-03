use crate::*;

/// 4-channel Pixel type
pub use ffi::Pixel;

impl Default for Pixel {
    fn default() -> Pixel {
        Pixel::new()
    }
}

impl std::ops::Add for Pixel {
    type Output = Pixel;
    fn add(self, mut other: Pixel) -> Pixel {
        unsafe {
            ffi::pixelAdd(&self, &mut other);
        }
        other
    }
}

impl std::ops::Sub for Pixel {
    type Output = Pixel;
    fn sub(self, mut other: Pixel) -> Pixel {
        unsafe {
            ffi::pixelSub(&self, &mut other);
        }
        other
    }
}

impl std::ops::Mul for Pixel {
    type Output = Pixel;
    fn mul(self, mut other: Pixel) -> Pixel {
        unsafe {
            ffi::pixelMul(&self, &mut other);
        }
        other
    }
}

impl std::ops::Div for Pixel {
    type Output = Pixel;
    fn div(self, mut other: Pixel) -> Pixel {
        unsafe {
            ffi::pixelDiv(&self, &mut other);
        }
        other
    }
}

impl Pixel {
    /// Empty pixel
    pub fn new() -> Pixel {
        unsafe { ffi::pixelEmpty() }
    }

    /// RGB pixel, 3 channels
    pub fn rgb(r: f32, g: f32, b: f32) -> Pixel {
        unsafe { ffi::pixelRGB(r, g, b) }
    }

    /// RGBA pixel, 4 channels
    pub fn rgba(r: f32, g: f32, b: f32, a: f32) -> Pixel {
        unsafe { ffi::pixelRGBA(r, g, b, a) }
    }

    /// Gray pixel, 3 channels
    pub fn gray(x: f32) -> Pixel {
        unsafe { ffi::pixelGray(x) }
    }

    /// Get pixel data
    pub fn data(&self) -> [f32; 4] {
        self.data
    }

    /// Get a mutable reference to the pixel data
    pub fn data_mut(&mut self) -> &mut [f32] {
        &mut self.data
    }
}

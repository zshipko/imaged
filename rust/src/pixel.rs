use crate::*;

/// 4-channel Pixel type
pub use sys::Pixel;

impl Default for Pixel {
    fn default() -> Pixel {
        Pixel::new()
    }
}

impl std::ops::Add for Pixel {
    type Output = Pixel;
    fn add(self, mut other: Pixel) -> Pixel {
        unsafe {
            sys::pixelAdd(&self, &mut other);
        }
        other
    }
}

impl std::ops::Sub for Pixel {
    type Output = Pixel;
    fn sub(self, mut other: Pixel) -> Pixel {
        unsafe {
            sys::pixelSub(&self, &mut other);
        }
        other
    }
}

impl std::ops::Mul for Pixel {
    type Output = Pixel;
    fn mul(self, mut other: Pixel) -> Pixel {
        unsafe {
            sys::pixelMul(&self, &mut other);
        }
        other
    }
}

impl std::ops::Div for Pixel {
    type Output = Pixel;
    fn div(self, mut other: Pixel) -> Pixel {
        unsafe {
            sys::pixelDiv(&self, &mut other);
        }
        other
    }
}

impl Pixel {
    /// Empty pixel
    pub fn new() -> Pixel {
        unsafe { sys::pixelEmpty() }
    }

    /// RGB pixel, 3 channels
    pub fn rgb(r: f32, g: f32, b: f32) -> Pixel {
        unsafe { sys::pixelRGB(r, g, b) }
    }

    /// RGBA pixel, 4 channels
    pub fn rgba(r: f32, g: f32, b: f32, a: f32) -> Pixel {
        unsafe { sys::pixelRGBA(r, g, b, a) }
    }

    /// Gray pixel, 3 channels
    pub fn gray(x: f32) -> Pixel {
        unsafe { sys::pixelGray(x) }
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

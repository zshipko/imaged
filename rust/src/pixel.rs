use crate::*;

/// 4-channel Pixel type
pub use sys::Pixel;

impl Default for Pixel {
    fn default() -> Pixel {
        Pixel::empty()
    }
}

impl std::ops::Add for Pixel {
    type Output = Pixel;
    fn add(mut self, other: Pixel) -> Pixel {
        unsafe {
            sys::pixelAdd(&mut self, &other);
        }
        other
    }
}

impl std::ops::Sub for Pixel {
    type Output = Pixel;
    fn sub(mut self, other: Pixel) -> Pixel {
        unsafe {
            sys::pixelSub(&mut self, &other);
        }
        other
    }
}

impl std::ops::Mul for Pixel {
    type Output = Pixel;
    fn mul(mut self, other: Pixel) -> Pixel {
        unsafe {
            sys::pixelMul(&mut self, &other);
        }
        other
    }
}

impl std::ops::Div for Pixel {
    type Output = Pixel;
    fn div(mut self, other: Pixel) -> Pixel {
        unsafe {
            sys::pixelDiv(&mut self, &other);
        }
        other
    }
}

impl Pixel {
    /// Empty pixel
    pub fn empty() -> Pixel {
        unsafe { sys::pixelEmpty() }
    }

    /// 3 channel pixel
    pub fn new3(r: f32, g: f32, b: f32) -> Pixel {
        unsafe { sys::pixelNew3(r, g, b) }
    }

    /// 4 channel pixel
    pub fn new(r: f32, g: f32, b: f32, a: f32) -> Pixel {
        unsafe { sys::pixelNew(r, g, b, a) }
    }

    /// 1 channel pixel
    pub fn gray(x: f32) -> Pixel {
        unsafe { sys::pixelNew1(x) }
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

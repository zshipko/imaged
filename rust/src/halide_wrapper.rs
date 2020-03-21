#![cfg(feature = "halide")]
use crate::Error;

impl<'a> crate::Image<'a> {
    /// Use the image as a mutable halide_buffer_t
    pub fn as_mut_halide_buffer<'b>(&'b mut self) -> Result<halide_runtime::Buffer<'b>, Error> {
        let meta = self.meta();

        // This only works because imaged Kind is modeled after halide
        let kind = unsafe { std::mem::transmute_copy(&meta.kind) };
        Ok(halide_runtime::Buffer::new(
            meta.width as i32,
            meta.height as i32,
            meta.channels() as i32,
            halide_runtime::Type(kind, meta.bits),
            self.buffer_mut()?,
        ))
    }

    /// Use the image as a halide_buffer_t
    pub fn as_halide_buffer<'b>(&'b self) -> Result<halide_runtime::Buffer<'b>, Error> {
        let meta = self.meta();

        // This only works because imaged Kind is modeled after halide
        let kind = unsafe { std::mem::transmute_copy(&meta.kind) };
        Ok(halide_runtime::Buffer::new(
            meta.width as i32,
            meta.height as i32,
            meta.channels() as i32,
            halide_runtime::Type(kind, meta.bits),
            unsafe { &mut *(self.buffer()? as *const [u8] as *mut [u8]) },
        ))
    }
}

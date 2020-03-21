#![cfg(feature = "halide")]
use crate::Error;

impl<'a> crate::Image<'a> {
    /// Use the image as halide_buffer_t
    pub unsafe fn as_halide_buffer(&'a mut self) -> Result<halide_runtime::Buffer<'a>, Error> {
        let meta = self.meta();

        // This only works because imaged Kind is modeled after halide
        let kind = std::mem::transmute_copy(&meta.kind);
        Ok(halide_runtime::Buffer::new(
            meta.width as i32,
            meta.height as i32,
            meta.channels() as i32,
            halide_runtime::Type(kind, meta.bits),
            self.buffer_mut()?,
        ))
    }
}

#![cfg(feature = "halide")]
use crate::Error;

impl<'a> crate::Image<'a> {
    /// Use the image as a mutable halide_buffer_t
    pub fn as_mut_halide_buffer(&mut self) -> Result<halide_runtime::Buffer, Error> {
        let meta = self.meta();

        // This only works because imaged Kind is modeled after halide
        let kind = unsafe { std::mem::transmute_copy(&meta.kind) };
        Ok(halide_runtime::Buffer::new(
            meta.width as i32,
            meta.height as i32,
            meta.channels() as i32,
            halide_runtime::Type::new(kind, meta.bits),
            self.buffer_mut()?,
        ))
    }

    /// Use the image as a halide_buffer_t
    ///
    /// NOTE: This buffer should only be used immutably, it is not safe to
    /// use the resulting Buffer as an output argument. Use `as_mut_halide_buffer`
    /// if you will be mutating the contents
    pub fn as_halide_buffer(&self) -> Result<halide_runtime::Buffer, Error> {
        let meta = self.meta();

        // This only works because imaged Kind is modeled after halide
        let kind = unsafe { std::mem::transmute_copy(&meta.kind) };
        Ok(halide_runtime::Buffer::new(
            meta.width as i32,
            meta.height as i32,
            meta.channels() as i32,
            halide_runtime::Type::new(kind, meta.bits),
            #[allow(clippy::cast_ref_to_mut)]
            unsafe {
                &mut *(self.buffer()? as *const [u8] as *mut [u8])
            },
        ))
    }
}

#![cfg(feature = "halide")]

pub fn halide_buffer(im: &mut crate::Image) -> halide_runtime::Buffer {
    let meta = im.meta();

    // This only works because imaged Kind is modeled after halide
    let kind = unsafe { std::mem::transmute_copy(&meta.kind) };
    halide_runtime::Buffer::new(
        meta.width as i32,
        meta.height as i32,
        meta.channels() as i32,
        halide_runtime::Type(kind, meta.bits),
        im.data_ptr() as *mut u8,
    )
}

impl<'a> From<&'a mut crate::Image> for halide_runtime::Buffer {
    fn from(x: &'a mut crate::Image) -> halide_runtime::Buffer {
        halide_buffer(x)
    }
}

impl<'a> From<&'a crate::Image> for halide_runtime::Buffer {
    fn from(im: &'a crate::Image) -> halide_runtime::Buffer {
        let meta = im.meta();
        let kind = unsafe { std::mem::transmute_copy(&meta.kind) };
        halide_runtime::Buffer::new(
            meta.width as i32,
            meta.height as i32,
            meta.channels() as i32,
            halide_runtime::Type(kind, meta.bits),
            im.data_ptr() as *mut u8, // Cheating by converting the const pointer to mut,
                                      // this should only be used for const halide buffers
        )
    }
}

pub fn halide_buffer(im: &imaged::Image) -> halide_runtime::Buffer {
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

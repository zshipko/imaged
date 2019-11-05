/* automatically generated by rust-bindgen */

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

pub type __uint8_t = ::std::os::raw::c_uchar;
pub type __uint32_t = ::std::os::raw::c_uint;
pub type __uint64_t = ::std::os::raw::c_ulong;
pub type __ino_t = ::std::os::raw::c_ulong;
pub type __off_t = ::std::os::raw::c_long;
pub type __ssize_t = ::std::os::raw::c_long;
#[repr(C)]
#[derive(Copy, Clone)]
pub struct dirent {
    pub d_ino: __ino_t,
    pub d_off: __off_t,
    pub d_reclen: ::std::os::raw::c_ushort,
    pub d_type: ::std::os::raw::c_uchar,
    pub d_name: [::std::os::raw::c_char; 256usize],
}
#[test]
fn bindgen_test_layout_dirent() {
    assert_eq!(
        ::std::mem::size_of::<dirent>(),
        280usize,
        concat!("Size of: ", stringify!(dirent))
    );
    assert_eq!(
        ::std::mem::align_of::<dirent>(),
        8usize,
        concat!("Alignment of ", stringify!(dirent))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<dirent>())).d_ino as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(dirent),
            "::",
            stringify!(d_ino)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<dirent>())).d_off as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(dirent),
            "::",
            stringify!(d_off)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<dirent>())).d_reclen as *const _ as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(dirent),
            "::",
            stringify!(d_reclen)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<dirent>())).d_type as *const _ as usize },
        18usize,
        concat!(
            "Offset of field: ",
            stringify!(dirent),
            "::",
            stringify!(d_type)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<dirent>())).d_name as *const _ as usize },
        19usize,
        concat!(
            "Offset of field: ",
            stringify!(dirent),
            "::",
            stringify!(d_name)
        )
    );
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __dirstream {
    _unused: [u8; 0],
}
pub type DIR = __dirstream;
extern "C" {
    pub fn imagedStringPrintf(
        fmt: *const ::std::os::raw::c_char,
        ...
    ) -> *mut ::std::os::raw::c_char;
}
#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, PartialOrd)]
pub enum ImagedStatus {
    IMAGED_OK = 0,
    IMAGED_ERR = 1,
    IMAGED_ERR_CANNOT_CREATE_FILE = 2,
    IMAGED_ERR_FILE_DOES_NOT_EXIST = 3,
    IMAGED_ERR_FILE_ALREADY_EXISTS = 4,
    IMAGED_ERR_SEEK = 5,
    IMAGED_ERR_MAP_FAILED = 6,
    IMAGED_ERR_INVALID_KEY = 7,
    IMAGED_ERR_INVALID_FILE = 8,
    IMAGED_ERR_LOCKED = 9,
}
extern "C" {
    pub fn imagedError(status: ImagedStatus) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn imagedPrintError(status: ImagedStatus, message: *const ::std::os::raw::c_char);
}
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct Imaged {
    pub root: *mut ::std::os::raw::c_char,
}
#[test]
fn bindgen_test_layout_Imaged() {
    assert_eq!(
        ::std::mem::size_of::<Imaged>(),
        8usize,
        concat!("Size of: ", stringify!(Imaged))
    );
    assert_eq!(
        ::std::mem::align_of::<Imaged>(),
        8usize,
        concat!("Alignment of ", stringify!(Imaged))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Imaged>())).root as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(Imaged),
            "::",
            stringify!(root)
        )
    );
}
#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, PartialOrd)]
pub enum ImagedKind {
    IMAGED_KIND_INT = 0,
    IMAGED_KIND_UINT = 1,
    IMAGED_KIND_FLOAT = 2,
}
impl ImagedColor {
    pub const IMAGED_COLOR_LAST: ImagedColor = ImagedColor::IMAGED_COLOR_HSVA;
}
#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, PartialOrd)]
pub enum ImagedColor {
    IMAGED_COLOR_GRAY = 1,
    IMAGED_COLOR_GRAYA = 2,
    IMAGED_COLOR_RGB = 3,
    IMAGED_COLOR_RGBA = 4,
    IMAGED_COLOR_CMYK = 5,
    IMAGED_COLOR_CMYKA = 6,
    IMAGED_COLOR_YCBCR = 7,
    IMAGED_COLOR_YCBCRA = 8,
    IMAGED_COLOR_CIELAB = 9,
    IMAGED_COLOR_CIELABA = 10,
    IMAGED_COLOR_CIELCH = 11,
    IMAGED_COLOR_CIELCHA = 12,
    IMAGED_COLOR_CIEXYZ = 13,
    IMAGED_COLOR_CIEXYZA = 14,
    IMAGED_COLOR_YUV = 15,
    IMAGED_COLOR_YUVA = 16,
    IMAGED_COLOR_HSL = 17,
    IMAGED_COLOR_HSLA = 18,
    IMAGED_COLOR_HSV = 19,
    IMAGED_COLOR_HSVA = 20,
}
extern "C" {
    pub fn imagedColorName(color: ImagedColor) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn imagedTypeName(kind: ImagedKind, bits: u8) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn imagedColorNumChannels(color: ImagedColor) -> usize;
}
extern "C" {
    pub fn imagedParseColorAndType(
        color: *const ::std::os::raw::c_char,
        t: *const ::std::os::raw::c_char,
        c: *mut ImagedColor,
        kind: *mut ImagedKind,
        bits: *mut u8,
    ) -> bool;
}
extern "C" {
    pub fn imagedIsValidType(kind: ImagedKind, bits: u8) -> bool;
}
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct ImagedMeta {
    pub width: u64,
    pub height: u64,
    pub bits: u8,
    pub kind: ImagedKind,
    pub color: ImagedColor,
}
#[test]
fn bindgen_test_layout_ImagedMeta() {
    assert_eq!(
        ::std::mem::size_of::<ImagedMeta>(),
        32usize,
        concat!("Size of: ", stringify!(ImagedMeta))
    );
    assert_eq!(
        ::std::mem::align_of::<ImagedMeta>(),
        8usize,
        concat!("Alignment of ", stringify!(ImagedMeta))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedMeta>())).width as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedMeta),
            "::",
            stringify!(width)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedMeta>())).height as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedMeta),
            "::",
            stringify!(height)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedMeta>())).bits as *const _ as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedMeta),
            "::",
            stringify!(bits)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedMeta>())).kind as *const _ as usize },
        20usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedMeta),
            "::",
            stringify!(kind)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedMeta>())).color as *const _ as usize },
        24usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedMeta),
            "::",
            stringify!(color)
        )
    );
}
extern "C" {
    pub fn imagedMetaNumPixels(meta: *const ImagedMeta) -> usize;
}
extern "C" {
    pub fn imagedMetaTotalBytes(meta: *const ImagedMeta) -> usize;
}
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct Image {
    pub meta: ImagedMeta,
    pub data: *mut ::std::os::raw::c_void,
}
#[test]
fn bindgen_test_layout_Image() {
    assert_eq!(
        ::std::mem::size_of::<Image>(),
        40usize,
        concat!("Size of: ", stringify!(Image))
    );
    assert_eq!(
        ::std::mem::align_of::<Image>(),
        8usize,
        concat!("Alignment of ", stringify!(Image))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Image>())).meta as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(Image),
            "::",
            stringify!(meta)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Image>())).data as *const _ as usize },
        32usize,
        concat!(
            "Offset of field: ",
            stringify!(Image),
            "::",
            stringify!(data)
        )
    );
}
extern "C" {
    pub fn imageRead(
        filename: *const ::std::os::raw::c_char,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
    ) -> *mut Image;
}
extern "C" {
    pub fn imageReadDefault(filename: *const ::std::os::raw::c_char) -> *mut Image;
}
extern "C" {
    pub fn imageWrite(path: *const ::std::os::raw::c_char, image: *const Image) -> ImagedStatus;
}
extern "C" {
    pub fn imageNew(meta: ImagedMeta) -> *mut Image;
}
extern "C" {
    pub fn imageAlloc(
        w: u64,
        h: u64,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
        data: *const ::std::os::raw::c_void,
    ) -> *mut Image;
}
extern "C" {
    pub fn imageClone(image: *const Image) -> *mut Image;
}
extern "C" {
    pub fn imageFree(image: *mut Image);
}
extern "C" {
    pub fn imagePixelBytes(image: *mut Image) -> usize;
}
extern "C" {
    pub fn imageBytes(image: *mut Image) -> usize;
}
extern "C" {
    pub fn imageIndex(image: *mut Image, x: usize, y: usize) -> usize;
}
extern "C" {
    pub fn imageAt(image: *mut Image, x: usize, y: usize) -> *mut ::std::os::raw::c_void;
}
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct Pixel {
    pub data: [f32; 4usize],
}
#[test]
fn bindgen_test_layout_Pixel() {
    assert_eq!(
        ::std::mem::size_of::<Pixel>(),
        16usize,
        concat!("Size of: ", stringify!(Pixel))
    );
    assert_eq!(
        ::std::mem::align_of::<Pixel>(),
        4usize,
        concat!("Alignment of ", stringify!(Pixel))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Pixel>())).data as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(Pixel),
            "::",
            stringify!(data)
        )
    );
}
extern "C" {
    pub fn pixelClamp(px: *mut Pixel);
}
extern "C" {
    pub fn pixelEmpty() -> Pixel;
}
extern "C" {
    pub fn pixelGray(r: f32) -> Pixel;
}
extern "C" {
    pub fn pixelRGB(r: f32, g: f32, b: f32) -> Pixel;
}
extern "C" {
    pub fn pixelRGBA(r: f32, g: f32, b: f32, a: f32) -> Pixel;
}
extern "C" {
    pub fn imageGetPixel(image: *mut Image, x: usize, y: usize, pixel: *mut Pixel) -> bool;
}
extern "C" {
    pub fn imageSetPixel(image: *mut Image, x: usize, y: usize, pixel: *const Pixel) -> bool;
}
pub type imageParallelFn = ::std::option::Option<
    unsafe extern "C" fn(
        arg1: u32,
        arg2: u32,
        arg3: *mut Pixel,
        arg4: *mut ::std::os::raw::c_void,
    ) -> bool,
>;
extern "C" {
    pub fn imageEachPixel2(
        src: *mut Image,
        dst: *mut Image,
        fn_: imageParallelFn,
        nthreads: ::std::os::raw::c_int,
        userdata: *mut ::std::os::raw::c_void,
    ) -> ImagedStatus;
}
extern "C" {
    pub fn imageEachPixel(
        im: *mut Image,
        fn_: imageParallelFn,
        nthreads: ::std::os::raw::c_int,
        userdata: *mut ::std::os::raw::c_void,
    ) -> ImagedStatus;
}
extern "C" {
    pub fn imageConvertTo(src: *const Image, dest: *mut Image) -> bool;
}
extern "C" {
    pub fn imageConvert(
        src: *const Image,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
    ) -> *mut Image;
}
extern "C" {
    pub fn imageConvertInPlace(
        src: *mut *mut Image,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
    ) -> bool;
}
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct ImagedHandle {
    pub fd: ::std::os::raw::c_int,
    pub image: Image,
}
#[test]
fn bindgen_test_layout_ImagedHandle() {
    assert_eq!(
        ::std::mem::size_of::<ImagedHandle>(),
        48usize,
        concat!("Size of: ", stringify!(ImagedHandle))
    );
    assert_eq!(
        ::std::mem::align_of::<ImagedHandle>(),
        8usize,
        concat!("Alignment of ", stringify!(ImagedHandle))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedHandle>())).fd as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedHandle),
            "::",
            stringify!(fd)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedHandle>())).image as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedHandle),
            "::",
            stringify!(image)
        )
    );
}
extern "C" {
    pub fn imagedResetLocks(db: *mut Imaged);
}
extern "C" {
    pub fn imagedKeyIsLocked(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: isize,
    ) -> bool;
}
extern "C" {
    pub fn imagedIsValidFile(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: isize,
    ) -> bool;
}
extern "C" {
    pub fn imagedWait(status: ImagedStatus) -> bool;
}
extern "C" {
    pub fn imagedOpen(path: *const ::std::os::raw::c_char) -> *mut Imaged;
}
extern "C" {
    pub fn imagedClose(db: *mut Imaged);
}
extern "C" {
    pub fn imagedDestroy(db: *mut Imaged) -> ImagedStatus;
}
extern "C" {
    pub fn imagedHasKey(db: *mut Imaged, key: *const ::std::os::raw::c_char, keylen: isize)
        -> bool;
}
extern "C" {
    pub fn imagedSet(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: isize,
        meta: ImagedMeta,
        imagedata: *const ::std::os::raw::c_void,
        handle: *mut ImagedHandle,
    ) -> ImagedStatus;
}
extern "C" {
    pub fn imagedGet(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: isize,
        editable: bool,
        handle: *mut ImagedHandle,
    ) -> ImagedStatus;
}
extern "C" {
    pub fn imagedRemove(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: isize,
    ) -> ImagedStatus;
}
extern "C" {
    pub fn imagedHandleClose(handle: *mut ImagedHandle);
}
extern "C" {
    pub fn imagedHandleInit(handle: *mut ImagedHandle);
}
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct ImagedIter {
    pub db: *mut Imaged,
    pub d: *mut DIR,
    pub ent: *mut dirent,
    pub key: *const ::std::os::raw::c_char,
    pub keylen: usize,
    pub handle: ImagedHandle,
}
#[test]
fn bindgen_test_layout_ImagedIter() {
    assert_eq!(
        ::std::mem::size_of::<ImagedIter>(),
        88usize,
        concat!("Size of: ", stringify!(ImagedIter))
    );
    assert_eq!(
        ::std::mem::align_of::<ImagedIter>(),
        8usize,
        concat!("Alignment of ", stringify!(ImagedIter))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedIter>())).db as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedIter),
            "::",
            stringify!(db)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedIter>())).d as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedIter),
            "::",
            stringify!(d)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedIter>())).ent as *const _ as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedIter),
            "::",
            stringify!(ent)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedIter>())).key as *const _ as usize },
        24usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedIter),
            "::",
            stringify!(key)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedIter>())).keylen as *const _ as usize },
        32usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedIter),
            "::",
            stringify!(keylen)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<ImagedIter>())).handle as *const _ as usize },
        40usize,
        concat!(
            "Offset of field: ",
            stringify!(ImagedIter),
            "::",
            stringify!(handle)
        )
    );
}
extern "C" {
    pub fn imagedIterNew(db: *mut Imaged) -> *mut ImagedIter;
}
extern "C" {
    pub fn imagedIterNext(iter: *mut ImagedIter) -> *mut Image;
}
extern "C" {
    pub fn imagedIterNextKey(iter: *mut ImagedIter) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn imagedIterFree(iter: *mut ImagedIter);
}
extern "C" {
    pub fn imagedIterReset(iter: *mut ImagedIter);
}

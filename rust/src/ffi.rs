/* automatically generated by rust-bindgen */

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

pub type __uint8_t = ::std::os::raw::c_uchar;
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
pub type size_t = ::std::os::raw::c_ulong;
pub type ssize_t = __ssize_t;
extern "C" {
    #[doc = " Utility for creating new strings"]
    pub fn imagedStringPrintf(
        fmt: *const ::std::os::raw::c_char,
        ...
    ) -> *mut ::std::os::raw::c_char;
}
#[repr(u32)]
#[doc = " Status types: IMAGED_OK implies the function executed successfully, while"]
#[doc = " any other response signifies failure"]
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
    #[doc = " Convert ImagedStatus to an error message"]
    pub fn imagedError(status: ImagedStatus) -> *const ::std::os::raw::c_char;
}
extern "C" {
    #[doc = " Dump ImagedStatus error message to stderr"]
    pub fn imagedPrintError(status: ImagedStatus, message: *const ::std::os::raw::c_char);
}
#[doc = " Image database"]
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
#[doc = " Image kinds, specifies the image data base type"]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, PartialOrd)]
pub enum ImagedKind {
    IMAGED_KIND_INT = 0,
    IMAGED_KIND_UINT = 1,
    IMAGED_KIND_FLOAT = 2,
}
impl ImagedColor {
    pub const IMAGED_COLOR_LAST: ImagedColor = ImagedColor::IMAGED_COLOR_HCYA;
}
#[repr(u32)]
#[doc = " Image colors, specifies the image color type"]
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
    IMAGED_COLOR_CIEXYY = 21,
    IMAGED_COLOR_CIEXYYA = 22,
    IMAGED_COLOR_HCY = 23,
    IMAGED_COLOR_HCYA = 24,
}
extern "C" {
    #[doc = " Get name of color"]
    pub fn imagedColorName(color: ImagedColor) -> *const ::std::os::raw::c_char;
}
extern "C" {
    #[doc = " Get name of type"]
    pub fn imagedTypeName(kind: ImagedKind, bits: u8) -> *const ::std::os::raw::c_char;
}
extern "C" {
    #[doc = " Get number of channels in a color"]
    pub fn imagedColorNumChannels(color: ImagedColor) -> size_t;
}
extern "C" {
    #[doc = " Parse color and type names"]
    pub fn imagedParseColorAndType(
        color: *const ::std::os::raw::c_char,
        t: *const ::std::os::raw::c_char,
        c: *mut ImagedColor,
        kind: *mut ImagedKind,
        bits: *mut u8,
    ) -> bool;
}
extern "C" {
    #[doc = " Returns true if the kind/bits create a valid image type"]
    pub fn imagedIsValidType(kind: ImagedKind, bits: u8) -> bool;
}
#[doc = " ImagedMeta is used to store image metadata with information about the image"]
#[doc = " shape and type"]
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
    #[doc = " Get the number of pixels in an image"]
    pub fn imagedMetaNumPixels(meta: *const ImagedMeta) -> size_t;
}
extern "C" {
    #[doc = " Get the number of bytes in an image"]
    pub fn imagedMetaTotalBytes(meta: *const ImagedMeta) -> size_t;
}
extern "C" {
    pub fn imagedMetaInit(
        w: u64,
        h: u64,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
        meta: *mut ImagedMeta,
    );
}
#[doc = " Stores image data with associated metadata"]
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct Image {
    pub owner: bool,
    pub meta: ImagedMeta,
    pub data: *mut ::std::os::raw::c_void,
}
#[test]
fn bindgen_test_layout_Image() {
    assert_eq!(
        ::std::mem::size_of::<Image>(),
        48usize,
        concat!("Size of: ", stringify!(Image))
    );
    assert_eq!(
        ::std::mem::align_of::<Image>(),
        8usize,
        concat!("Alignment of ", stringify!(Image))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Image>())).owner as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(Image),
            "::",
            stringify!(owner)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Image>())).meta as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(Image),
            "::",
            stringify!(meta)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<Image>())).data as *const _ as usize },
        40usize,
        concat!(
            "Offset of field: ",
            stringify!(Image),
            "::",
            stringify!(data)
        )
    );
}
extern "C" {
    pub fn imageRAWUseAutoBrightness(b: bool);
}
extern "C" {
    pub fn imageRAWUseCameraWhiteBalance(b: bool);
}
extern "C" {
    #[doc = " Read an image from disk, the resulting image will be converted to match"]
    #[doc = " color/kind/bits if needed"]
    pub fn imageRead(
        filename: *const ::std::os::raw::c_char,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
    ) -> *mut Image;
}
extern "C" {
    #[doc = " Read an image from disk, using the default format"]
    pub fn imageReadDefault(filename: *const ::std::os::raw::c_char) -> *mut Image;
}
extern "C" {
    #[doc = " Write an image to disk"]
    pub fn imageWrite(path: *const ::std::os::raw::c_char, image: *const Image) -> ImagedStatus;
}
extern "C" {
    #[doc = " Create a new image with the given metadata"]
    pub fn imageNew(meta: ImagedMeta) -> *mut Image;
}
extern "C" {
    #[doc = " Create a new image from an existing buffer"]
    pub fn imageNewWithData(meta: ImagedMeta, data: *mut ::std::os::raw::c_void) -> *mut Image;
}
extern "C" {
    #[doc = " Create a new image and copy data if provided"]
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
    #[doc = " Duplicate an existing image"]
    pub fn imageClone(image: *const Image) -> *mut Image;
}
extern "C" {
    #[doc = " Free allocated image"]
    pub fn imageFree(image: *mut Image);
}
extern "C" {
    #[doc = " Get the number of bytes in a pixel for the given image"]
    pub fn imagePixelBytes(image: *mut Image) -> size_t;
}
extern "C" {
    #[doc = " Get the number of bytes in an image"]
    pub fn imageBytes(image: *mut Image) -> size_t;
}
extern "C" {
    #[doc = " Get the data offset at the position (x, y)"]
    pub fn imageIndex(image: *mut Image, x: size_t, y: size_t) -> size_t;
}
extern "C" {
    #[doc = " Get a pointer to the data at the position (x, y)"]
    pub fn imageAt(image: *mut Image, x: size_t, y: size_t) -> *mut ::std::os::raw::c_void;
}
#[doc = " 4-channel floating point pixel"]
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
    #[doc = " Get pixel at position (x, y)"]
    pub fn imageGetPixel(image: *mut Image, x: size_t, y: size_t, pixel: *mut Pixel) -> bool;
}
extern "C" {
    #[doc = " Set pixel at position (x, y)"]
    pub fn imageSetPixel(image: *mut Image, x: size_t, y: size_t, pixel: *const Pixel) -> bool;
}
extern "C" {
    #[doc = " Ensures pixel values are between 0 and 1"]
    pub fn pixelClamp(px: *mut Pixel);
}
extern "C" {
    #[doc = " Create a new empty pixel"]
    pub fn pixelEmpty() -> Pixel;
}
extern "C" {
    #[doc = " Create a new gray pixel"]
    pub fn pixelGray(r: f32) -> Pixel;
}
extern "C" {
    #[doc = " Create a new RGB pixel"]
    pub fn pixelRGB(r: f32, g: f32, b: f32) -> Pixel;
}
extern "C" {
    #[doc = " Create a new RGBA pixel"]
    pub fn pixelRGBA(r: f32, g: f32, b: f32, a: f32) -> Pixel;
}
extern "C" {
    #[doc = " Pixel addition"]
    pub fn pixelAdd(src: *const Pixel, dest: *mut Pixel);
}
extern "C" {
    #[doc = " Pixel subtraction"]
    pub fn pixelSub(src: *const Pixel, dest: *mut Pixel);
}
extern "C" {
    #[doc = " Pixel multiplication"]
    pub fn pixelMul(src: *const Pixel, dest: *mut Pixel);
}
extern "C" {
    #[doc = " Pixel division"]
    pub fn pixelDiv(src: *const Pixel, dest: *mut Pixel);
}
extern "C" {
    #[doc = " Pixel equality"]
    pub fn pixelEq(a: *const Pixel, b: *const Pixel) -> bool;
}
extern "C" {
    #[doc = " Pixel equality against a single value"]
    pub fn pixelEqAll(a: *const Pixel, v: f32) -> bool;
}
extern "C" {
    #[doc = " Sun of all pixel channels"]
    pub fn pixelSum(a: *const Pixel) -> f32;
}
extern "C" {
    #[doc = " Adjust image gamma"]
    pub fn imageAdjustGamma(src: *mut Image, gamma: f32);
}
extern "C" {
    #[doc = " Convert source image to the format specified by the destination image"]
    pub fn imageConvertTo(src: *const Image, dest: *mut Image) -> bool;
}
extern "C" {
    #[doc = " Convert source image to the specified type, returning the new converted"]
    #[doc = " image"]
    pub fn imageConvert(
        src: *const Image,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
    ) -> *mut Image;
}
extern "C" {
    #[doc = " Convert source image to the specified type"]
    pub fn imageConvertInPlace(
        src: *mut *mut Image,
        color: ImagedColor,
        kind: ImagedKind,
        bits: u8,
    ) -> bool;
}
extern "C" {
    pub fn imageConvertACES0(src: *mut Image) -> *mut Image;
}
extern "C" {
    pub fn imageConvertACES0ToXYZ(src: *mut Image) -> *mut Image;
}
extern "C" {
    pub fn imageConvertACES1(src: *mut Image) -> *mut Image;
}
extern "C" {
    pub fn imageConvertACES1ToXYZ(src: *mut Image) -> *mut Image;
}
extern "C" {
    #[doc = " Resize source image to size specified by destination image"]
    pub fn imageResizeTo(src: *mut Image, dest: *mut Image);
}
extern "C" {
    #[doc = " Resize image to the given size, returns a new image"]
    pub fn imageResize(src: *mut Image, x: size_t, y: size_t) -> *mut Image;
}
extern "C" {
    #[doc = " Scale an image using the given factors, returns a new image"]
    pub fn imageScale(src: *mut Image, scale_x: f64, scale_y: f64) -> *mut Image;
}
extern "C" {
    pub fn imageConsume(x: *mut Image, dest: *mut *mut Image) -> *mut Image;
}
#[doc = " A handle is used to refer to an imgd image in an Imaged database"]
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
        56usize,
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
    #[doc = " Remove all image locks"]
    pub fn imagedResetLocks(db: *mut Imaged);
}
extern "C" {
    #[doc = " Returns true when an image is locked"]
    pub fn imagedKeyIsLocked(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: ssize_t,
    ) -> bool;
}
extern "C" {
    #[doc = " Returns true when the specified file is an valid imgd file"]
    pub fn imagedIsValidFile(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: ssize_t,
    ) -> bool;
}
extern "C" {
    #[doc = " Wait for an image to become available"]
    pub fn imagedWait(status: ImagedStatus) -> bool;
}
extern "C" {
    #[doc = " Open a new imaged context"]
    pub fn imagedOpen(path: *const ::std::os::raw::c_char) -> *mut Imaged;
}
extern "C" {
    #[doc = " Close an imaged context"]
    pub fn imagedClose(db: *mut Imaged);
}
extern "C" {
    #[doc = " Destroy an imaged store, removing all contents from disk"]
    pub fn imagedDestroy(db: *mut Imaged) -> ImagedStatus;
}
extern "C" {
    #[doc = " Returns true when there is a value associated with the given key"]
    pub fn imagedHasKey(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: ssize_t,
    ) -> bool;
}
extern "C" {
    #[doc = " Set a key"]
    pub fn imagedSet(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: ssize_t,
        meta: ImagedMeta,
        imagedata: *const ::std::os::raw::c_void,
        handle: *mut ImagedHandle,
    ) -> ImagedStatus;
}
extern "C" {
    #[doc = " Get a key"]
    pub fn imagedGet(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: ssize_t,
        editable: bool,
        handle: *mut ImagedHandle,
    ) -> ImagedStatus;
}
extern "C" {
    #[doc = " &Remove the value associated with the provided key"]
    pub fn imagedRemove(
        db: *mut Imaged,
        key: *const ::std::os::raw::c_char,
        keylen: ssize_t,
    ) -> ImagedStatus;
}
extern "C" {
    #[doc = " Release ImagedHandle resources including all memory and file descriptors"]
    pub fn imagedHandleClose(handle: *mut ImagedHandle);
}
extern "C" {
    #[doc = " Initialize an new handle"]
    pub fn imagedHandleInit(handle: *mut ImagedHandle);
}
#[doc = " Iterator over imgd files in an Imaged database"]
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialOrd, PartialEq)]
pub struct ImagedIter {
    pub db: *mut Imaged,
    pub d: *mut DIR,
    pub ent: *mut dirent,
    pub key: *const ::std::os::raw::c_char,
    pub keylen: size_t,
    pub handle: ImagedHandle,
}
#[test]
fn bindgen_test_layout_ImagedIter() {
    assert_eq!(
        ::std::mem::size_of::<ImagedIter>(),
        96usize,
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
    #[doc = " Create a new iterator"]
    pub fn imagedIterNew(db: *mut Imaged) -> *mut ImagedIter;
}
extern "C" {
    #[doc = " Get next image"]
    pub fn imagedIterNext(iter: *mut ImagedIter) -> *mut Image;
}
extern "C" {
    #[doc = " Get next key"]
    pub fn imagedIterNextKey(iter: *mut ImagedIter) -> *const ::std::os::raw::c_char;
}
extern "C" {
    #[doc = " Free iterator"]
    pub fn imagedIterFree(iter: *mut ImagedIter);
}
extern "C" {
    pub fn imagedIterReset(iter: *mut ImagedIter);
}
pub type imageParallelFn = ::std::option::Option<
    unsafe extern "C" fn(
        arg1: u64,
        arg2: u64,
        arg3: *mut Pixel,
        arg4: *mut ::std::os::raw::c_void,
    ) -> bool,
>;
extern "C" {
    pub fn imageEachPixel2(
        im: *mut Image,
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
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct halide_buffer_t {
    _unused: [u8; 0],
}
extern "C" {
    pub fn imageNewHalideBuffer(image: *mut Image, buffer: *mut halide_buffer_t);
}
extern "C" {
    pub fn imageFreeHalideBuffer(buffer: *mut halide_buffer_t);
}

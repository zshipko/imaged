use crate::*;

use std::os::raw::c_char;

pub use ffi::ImagedMeta as Meta;

/// Image type
pub struct Image<'a>(pub &'a mut ffi::Image, pub bool);

unsafe impl<'a> Sync for Image<'a> {}
unsafe impl<'a> Send for Image<'a> {}

/// Type defines an image's underlying data type
/// For example, `Type::U(16)` is `uint16_t`, `Type::F(32)` is `float`
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub enum Type {
    I(u8),
    U(u8),
    F(u8),
}

/// Color defines an image's colorspace
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub enum Color {
    Unknown = 0,
    Gray = 1,
    GrayA = 2,
    RGB = 3,
    RGBA = 4,
    CMYK = 5,
    CMYKA = 6,
    YCBCR = 7,
    YCBCRA = 8,
    CIELAB = 9,
    CIELABA = 10,
    CIELCH = 11,
    CIELCHA = 12,
    CIEXYZ = 13,
    CIEXYZA = 14,
    YUV = 15,
    YUVA = 16,
    HSL = 17,
    HSLA = 18,
    HSV = 19,
    HSVA = 20,
    CIEXYY = 21,
    CIEXYYA = 22,
    HCY = 23,
    HCYA = 24,
}

impl Color {
    pub fn ffi(&self) -> ffi::ImagedColor {
        unsafe { std::mem::transmute_copy(self) }
    }

    /// Get the number of channels
    pub fn channels(&self) -> usize {
        unsafe { ffi::imagedColorNumChannels(self.ffi()) }
    }
}

impl Type {
    /// Get kind and bits
    pub fn info(&self) -> (ffi::ImagedKind, u8) {
        let (kind, bits) = match self {
            Type::I(x) => (ffi::ImagedKind::IMAGED_KIND_INT, x),
            Type::U(x) => (ffi::ImagedKind::IMAGED_KIND_UINT, x),
            Type::F(x) => (ffi::ImagedKind::IMAGED_KIND_FLOAT, x),
        };
        (kind, *bits)
    }
}

impl Meta {
    /// Create a new Meta instance
    pub fn new(w: usize, h: usize, color: Color, ty: Type) -> Meta {
        let (kind, bits) = ty.info();
        Meta {
            width: w as u64,
            height: h as u64,
            color: color.ffi(),
            kind,
            bits,
        }
    }

    pub fn width(&self) -> usize {
        self.width as usize
    }

    pub fn height(&self) -> usize {
        self.height as usize
    }

    /// Create a new Meta instance from an existing one with the color changed
    pub fn with_color(&self, color: Color) -> Self {
        let mut meta = self.clone();
        meta.color = color.ffi();
        meta
    }

    /// Create a new Meta instance from an existing one with the type changed
    pub fn with_type(&self, t: Type) -> Self {
        let mut meta = self.clone();
        let info = t.info();
        meta.kind = info.0;
        meta.bits = info.1;
        meta
    }

    /// Get the underlying data type
    pub fn get_type(&self) -> Type {
        match self.kind {
            ffi::ImagedKind::IMAGED_KIND_INT => Type::I(self.bits),
            ffi::ImagedKind::IMAGED_KIND_UINT => Type::U(self.bits),
            ffi::ImagedKind::IMAGED_KIND_FLOAT => Type::F(self.bits),
        }
    }

    /// Get image colorspace
    pub fn get_color(&self) -> Color {
        unsafe { std::mem::transmute(self.color) }
    }

    /// Get total number of bytes occupied by the image data
    pub fn total_bytes(&self) -> usize {
        return unsafe { ffi::imagedMetaTotalBytes(self) };
    }

    /// Get the number of channels
    pub fn channels(&self) -> usize {
        unsafe { ffi::imagedColorNumChannels(self.color) }
    }
}

impl<'a> Drop for Image<'a> {
    fn drop(&mut self) {
        match self.1 {
            true => unsafe { ffi::imageFree(self.0) },
            _ => (),
        }
    }
}

impl<'a> Image<'a> {
    /// Read default colorspace/type from disk using ezimage
    pub fn read_default<P: AsRef<std::path::Path>>(path: P) -> Result<Image<'a>, Error> {
        let path = format!("{}\0", path.as_ref().display());
        let im = unsafe { ffi::imageReadDefault(path.as_ptr() as *const c_char) };
        if im.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *im, true)) }
    }

    /// Read from disk using ezimage
    pub fn read<P: AsRef<std::path::Path>>(
        path: P,
        color: Color,
        t: Type,
    ) -> Result<Image<'a>, Error> {
        let path = format!("{}\0", path.as_ref().display());
        let (kind, bits) = t.info();
        let im = unsafe {
            ffi::imageRead(
                path.as_ptr() as *const c_char,
                std::mem::transmute(color as i32),
                std::mem::transmute(kind as i32),
                bits,
            )
        };
        if im.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *im, true)) }
    }

    /// Write image to disk using ezimage
    pub fn write<P: AsRef<std::path::Path>>(&self, path: P) -> Result<(), Error> {
        let path = format!("{}\0", path.as_ref().display());
        let rc = unsafe { ffi::imageWrite(path.as_ptr() as *const c_char, self.0) };

        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(())
    }

    /// Allocate a new image
    pub fn new(meta: Meta) -> Result<Self, Error> {
        let image = unsafe {
            ffi::imageAlloc(
                meta.width,
                meta.height,
                meta.color,
                meta.kind,
                meta.bits,
                std::ptr::null_mut(),
            )
        };
        if image.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *image, true)) }
    }

    /// Wrap existing data
    pub fn new_with_data<T, X: AsMut<[T]>>(meta: Meta, mut data: X) -> Result<Self, Error> {
        let image =
            unsafe { ffi::imageNewWithData(meta, data.as_mut().as_mut_ptr() as *mut c_void) };
        if image.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *image, true)) }
    }

    /// Create a new image similar to an existing image with the color changed
    pub fn new_like_with_color(&self, color: Color) -> Result<Self, Error> {
        let meta = self.meta().clone().with_color(color);
        Self::new(meta)
    }

    /// Create a new image similar to an existing image with the type changed
    pub fn new_like_with_type(&self, t: Type) -> Result<Self, Error> {
        let meta = self.meta().clone().with_type(t);
        Self::new(meta)
    }

    /// Get metadata
    pub fn meta(&self) -> &Meta {
        &self.0.meta
    }

    /// Get image (width, height, channels)
    pub fn shape(&self) -> (usize, usize, usize) {
        let meta = self.meta();
        (meta.width(), meta.height(), meta.channels())
    }

    pub fn get_type(&self) -> Type {
        let meta = self.meta();
        meta.get_type()
    }

    pub fn get_color(&self) -> Color {
        let meta = self.meta();
        meta.get_color()
    }

    /// Size of each channel in the underlying data
    pub fn elem_size(&self) -> usize {
        self.meta().bits as usize / 8
    }

    /// Get a pointer to the underlying data
    pub fn data_ptr(&self) -> *mut c_void {
        self.0.data
    }

    /// Get the underlying data as a slice
    pub fn data<T>(&self) -> Result<&[T], Error> {
        let size = std::mem::size_of::<T>();
        if size != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let data = unsafe {
            std::slice::from_raw_parts(
                (&*self.0).data as *const T,
                self.meta().total_bytes() / size,
            )
        };
        Ok(data)
    }

    /// Get the underlying data as a byte slice
    pub fn buffer(&self) -> Result<&[u8], Error> {
        let data = unsafe {
            std::slice::from_raw_parts((&*self.0).data as *const u8, self.meta().total_bytes())
        };
        Ok(data)
    }

    /// Get the underlying data as a mutable slice
    pub fn data_mut<T>(&mut self) -> Result<&mut [T], Error> {
        let size = std::mem::size_of::<T>();
        if size != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let data = unsafe {
            std::slice::from_raw_parts_mut(
                (&mut *self.0).data as *mut T,
                self.meta().total_bytes() / size,
            )
        };
        Ok(data)
    }

    /// Get the underlying data as a mutable byte slice
    pub fn buffer_mut(&mut self) -> Result<&mut [u8], Error> {
        let data = unsafe {
            std::slice::from_raw_parts_mut(
                (&mut *self.0).data as *mut u8,
                self.meta().total_bytes(),
            )
        };
        Ok(data)
    }

    /// Determines if the image matches the type parameter `T`
    pub fn is_type<T>(&self) -> bool {
        std::mem::size_of::<T>() == self.elem_size()
    }

    /// Get a mutable reference to the data at position (x, y)
    pub fn at<T>(&mut self, x: usize, y: usize) -> Result<&mut [T], Error> {
        if std::mem::size_of::<T>() != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let meta = self.meta();
        if x as u64 >= meta.width || y as u64 >= meta.height {
            return Err(Error::OutOfBounds);
        }

        let ptr = unsafe { ffi::imageAt(self.0 as *const ffi::Image as *mut ffi::Image, x, y) };
        if ptr.is_null() {
            return Err(Error::NullPointer);
        }

        let data = unsafe { std::slice::from_raw_parts_mut(ptr as *mut T, meta.channels()) };

        Ok(data)
    }

    /// Get the pixel at (x, y)
    pub fn get_pixel(&self, x: usize, y: usize, px: &mut Pixel) -> bool {
        unsafe { ffi::imageGetPixel(self.0 as *const ffi::Image as *mut ffi::Image, x, y, px) }
    }

    /// Set the pixel at (x, y)
    pub fn set_pixel(&self, x: usize, y: usize, px: &Pixel) -> bool {
        unsafe { ffi::imageSetPixel(self.0 as *const ffi::Image as *mut ffi::Image, x, y, px) }
    }

    /// Iterate over each pixel and apply `f`
    pub fn for_each<T, F: FnMut((usize, usize), &mut [T])>(
        &mut self,
        mut f: F,
    ) -> Result<(), Error> {
        if std::mem::size_of::<T>() != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let meta = self.meta().clone();

        self.data_mut()?
            .chunks_exact_mut(meta.channels())
            .enumerate()
            .for_each(|(n, pixel)| {
                let y = n / meta.width as usize;
                let x = n - (y * meta.width as usize);
                f((x, y), pixel)
            });
        Ok(())
    }

    /// Iterate over each pixel in two images of the same size and apply `f`
    pub fn for_each2<T, F: FnMut((usize, usize), &mut [T], &[T])>(
        &mut self,
        other: &Image,
        mut f: F,
    ) -> Result<(), Error> {
        if std::mem::size_of::<T>() != self.elem_size()
            || std::mem::size_of::<T>() != other.elem_size()
        {
            return Err(Error::IncorrectImageType);
        }

        let meta = self.meta().clone();

        let b = other.data()?.chunks(meta.channels());
        self.data_mut()?
            .chunks_mut(meta.channels())
            .zip(b)
            .enumerate()
            .for_each(|(n, (pixel, pixel1))| {
                let y = n / meta.width as usize;
                let x = n - (y * meta.width as usize);
                f((x, y), pixel, pixel1)
            });
        Ok(())
    }

    /// Convert an image colorspace in-place, overwriting the source image
    pub fn convert_in_place(&mut self, color: Color, t: Type) -> Result<(), Error> {
        let (kind, bits) = t.info();
        let rc = unsafe {
            ffi::imageConvertInPlace(&mut (self.0 as *mut ffi::Image), color.ffi(), kind, bits)
        };
        if !rc {
            return Err(Error::IncorrectImageType);
        }
        Ok(())
    }

    /// Convert an image colorspace based on the destination image type, allocating a new image
    pub fn convert_to(&self, dest: &mut Image) -> Result<(), Error> {
        let rc = unsafe { ffi::imageConvertTo(self.0, dest.0) };
        if !rc {
            return Err(Error::IncorrectImageType);
        }
        Ok(())
    }

    /// Convert an image colorspace, allocating a new image
    pub fn convert(&self, color: Color, t: Type) -> Result<Image, Error> {
        let (kind, bits) = t.info();
        let dest = unsafe { ffi::imageConvert(self.0, color.ffi(), kind, bits) };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }

    /// Gamma correctoin
    pub fn adjust_gamma(&self, g: f32) -> Result<(), Error> {
        unsafe { ffi::imageAdjustGamma(self.0 as *const ffi::Image as *mut ffi::Image, g) };
        Ok(())
    }

    pub fn convert_aces0(&self) -> Result<Image, Error> {
        let dest =
            unsafe { ffi::imageConvertACES0(self.0 as *const ffi::Image as *mut ffi::Image) };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }

    pub fn convert_aces1(&self) -> Result<Image, Error> {
        let dest =
            unsafe { ffi::imageConvertACES1(self.0 as *const ffi::Image as *mut ffi::Image) };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }

    pub fn convert_aces0_to_xyz(&self) -> Result<Image, Error> {
        let dest =
            unsafe { ffi::imageConvertACES0ToXYZ(self.0 as *const ffi::Image as *mut ffi::Image) };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }

    pub fn convert_aces1_to_xyz(&self) -> Result<Image, Error> {
        let dest =
            unsafe { ffi::imageConvertACES1ToXYZ(self.0 as *const ffi::Image as *mut ffi::Image) };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }

    /// Scale an image using the given factor for each dimension
    pub fn scale(&self, x: f64, y: f64) -> Result<Image, Error> {
        let dest = unsafe { ffi::imageScale(self.0 as *const ffi::Image as *mut ffi::Image, x, y) };

        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }

    /// Resize an image to match the destination image size
    pub fn resize_to(&self, dest: &mut Image) -> Result<(), Error> {
        unsafe { ffi::imageResizeTo(self.0 as *const ffi::Image as *mut ffi::Image, dest.0) };
        Ok(())
    }

    /// Resize an image to the given size
    pub fn resize(&self, width: usize, height: usize) -> Result<Image, Error> {
        let dest = unsafe {
            ffi::imageResize(
                self.0 as *const ffi::Image as *mut ffi::Image,
                width,
                height,
            )
        };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }

        unsafe { Ok(Image(&mut *dest, true)) }
    }
}

impl<'a> Clone for Image<'a> {
    /// Clone and image
    fn clone(&self) -> Image<'a> {
        let img = unsafe { ffi::imageClone(self.0) };
        unsafe { Image(&mut *img, true) }
    }
}

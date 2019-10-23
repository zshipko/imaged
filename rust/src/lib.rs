use std::ffi::c_void;

pub mod ffi;

#[derive(Debug)]
pub enum Error {
    CannotOpenDB,
    InvalidPath,
    NullPointer,
    IncorrectImageType,
    OutOfBounds,
    FFI(ffi::ImagedStatus),
}

pub use ffi::ImagedMeta as Meta;
pub struct DB(*mut ffi::Imaged);
pub struct Iter<'a>(*mut ffi::ImagedIter, &'a DB);
pub struct KeyIter<'a>(*mut ffi::ImagedIter, &'a DB);
pub struct Handle<'a>(ffi::ImagedHandle, &'a DB);
pub struct Image<'a>(*mut ffi::Image, Option<&'a DB>);
pub use ffi::Pixel;

impl Pixel {
    pub fn new() -> Pixel {
        unsafe { ffi::pixelEmpty() }
    }

    pub fn rgb(r: f32, g: f32, b: f32) -> Pixel {
        unsafe { ffi::pixelRGB(r, g, b) }
    }

    pub fn rgba(r: f32, g: f32, b: f32, a: f32) -> Pixel {
        unsafe { ffi::pixelRGBA(r, g, b, a) }
    }

    pub fn gray(x: f32) -> Pixel {
        unsafe { ffi::pixelGray(x) }
    }

    pub fn data(&self) -> &[f32] {
        &self.data
    }

    pub fn data_mut(&mut self) -> &mut [f32] {
        &mut self.data
    }
}

pub enum Type {
    I(u8),
    U(u8),
    F(u8),
}

#[repr(C)]
pub enum Color {
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
}

impl Color {
    pub fn ffi(&self) -> ffi::ImagedColor {
        unsafe { std::mem::transmute_copy(self) }
    }
}

impl Type {
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

    pub fn with_color(&self, color: Color) -> Self {
        let mut meta = self.clone();
        meta.color = color.ffi();
        meta
    }

    pub fn with_type(&self, t: Type) -> Self {
        let mut meta = self.clone();
        let info = t.info();
        meta.kind = info.0;
        meta.bits = info.1;
        meta
    }

    pub fn get_type(&self) -> Type {
        match self.kind {
            ffi::ImagedKind::IMAGED_KIND_INT => Type::I(self.bits),
            ffi::ImagedKind::IMAGED_KIND_UINT => Type::U(self.bits),
            ffi::ImagedKind::IMAGED_KIND_FLOAT => Type::F(self.bits),
        }
    }

    pub fn total_bytes(&self) -> usize {
        return unsafe { ffi::imagedMetaTotalBytes(self) };
    }

    pub fn channels(&self) -> usize {
        unsafe { ffi::imagedColorNumChannels(self.color) }
    }
}

impl<'a> Drop for Image<'a> {
    fn drop(&mut self) {
        match self.1 {
            None => unsafe { ffi::imageFree(self.0) },
            _ => (),
        }
    }
}

impl<'a> Drop for Handle<'a> {
    fn drop(&mut self) {
        unsafe { ffi::imagedHandleClose(&mut self.0) }
    }
}

impl<'a> Drop for Iter<'a> {
    fn drop(&mut self) {
        unsafe { ffi::imagedIterFree(self.0) }
    }
}

impl<'a> Drop for KeyIter<'a> {
    fn drop(&mut self) {
        unsafe { ffi::imagedIterFree(self.0) }
    }
}

impl<'a> Handle<'a> {
    pub fn image(&self) -> Image<'a> {
        Image(
            &self.0.image as *const ffi::Image as *mut ffi::Image,
            Some(self.1),
        )
    }
}

impl DB {
    pub fn open<P: AsRef<std::path::Path>>(path: P) -> Result<Self, Error> {
        let path = path.as_ref();
        let path = match path.to_str() {
            Some(x) => x,
            None => return Err(Error::InvalidPath),
        };

        let path = match std::ffi::CString::new(path) {
            Ok(x) => x,
            Err(_) => return Err(Error::InvalidPath),
        };

        let db = unsafe { ffi::imagedOpen(path.as_ptr()) };
        if db.is_null() {
            return Err(Error::CannotOpenDB);
        }

        return Ok(DB(db));
    }

    pub fn destroy(self) -> Result<(), Error> {
        let rc = unsafe { ffi::imagedDestroy(self.0) };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }
        Ok(())
    }

    pub fn iter<'a>(&'a self) -> Result<Iter<'a>, Error> {
        let iter = unsafe { ffi::imagedIterNew(self.0) };
        if iter.is_null() {
            return Err(Error::NullPointer);
        }

        return Ok(Iter(iter, self));
    }

    pub fn iter_keys<'a>(&'a self) -> Result<KeyIter<'a>, Error> {
        let iter = unsafe { ffi::imagedIterNew(self.0) };
        if iter.is_null() {
            return Err(Error::NullPointer);
        }

        return Ok(KeyIter(iter, self));
    }

    pub fn get<'a, S: AsRef<str>>(&'a self, key: S, editable: bool) -> Result<Handle<'a>, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            ffi::imagedGet(
                self.0,
                key.as_ref().as_ptr() as *const i8,
                key.as_ref().len() as isize,
                editable,
                &mut handle,
            )
        };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(Handle(handle, self))
    }

    pub fn set_image<'a, S: AsRef<str>>(
        &'a self,
        key: S,
        image: &Image,
    ) -> Result<Handle<'a>, Error> {
        self.set(key, image.meta().clone(), Some(image.data_ptr()))
    }

    pub fn set<'a, S: AsRef<str>>(
        &'a self,
        key: S,
        meta: Meta,
        data: Option<*const c_void>,
    ) -> Result<Handle<'a>, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            ffi::imagedSet(
                self.0,
                key.as_ref().as_ptr() as *const i8,
                key.as_ref().len() as isize,
                meta,
                data.unwrap_or(std::ptr::null()),
                &mut handle,
            )
        };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(Handle(handle, self))
    }

    pub fn remove<S: AsRef<str>>(&self, key: S) -> Result<(), Error> {
        let rc = unsafe {
            ffi::imagedRemove(
                self.0,
                key.as_ref().as_ptr() as *const i8,
                key.as_ref().len() as isize,
            )
        };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(())
    }
}

impl Drop for DB {
    fn drop(&mut self) {
        unsafe { ffi::imagedClose(self.0) }
    }
}

impl<'a> Iter<'a> {
    pub fn reset(&mut self) {
        unsafe { ffi::imagedIterReset(self.0) }
    }
}

impl<'a> KeyIter<'a> {
    pub fn reset(&mut self) {
        unsafe { ffi::imagedIterReset(self.0) }
    }
}

impl<'a> Iterator for Iter<'a> {
    type Item = (&'a str, Image<'a>);

    fn next(&mut self) -> Option<Self::Item> {
        let ptr = unsafe { ffi::imagedIterNext(self.0) };
        if ptr.is_null() {
            return None;
        }

        unsafe {
            let iter = &*self.0;
            let key = std::slice::from_raw_parts(iter.key as *const u8, iter.keylen);
            let key = std::str::from_utf8_unchecked(key);
            Some((key, Image(ptr, Some(self.1))))
        }
    }
}

impl<'a> Iterator for KeyIter<'a> {
    type Item = &'a str;

    fn next(&mut self) -> Option<Self::Item> {
        let key = unsafe { ffi::imagedIterNextKey(self.0) };
        if key.is_null() {
            return None;
        }

        unsafe {
            let key = std::slice::from_raw_parts(key as *const u8, (&*self.0).keylen);
            let key = std::str::from_utf8_unchecked(key);
            Some(key)
        }
    }
}

impl<'a> Image<'a> {
    pub fn read<P: AsRef<std::path::Path>>(path: P) -> Result<Image<'a>, Error> {
        let path = format!("{}\0", path.as_ref().display());
        let im = unsafe {
            ffi::imageRead(
                path.as_ptr() as *const i8,
                std::mem::transmute(-1),
                std::mem::transmute(-1),
                0,
            )
        };
        if im.is_null() {
            return Err(Error::NullPointer);
        }

        Ok(Image(im, None))
    }

    pub fn write<P: AsRef<std::path::Path>>(&self, path: P) -> Result<(), Error> {
        let path = format!("{}\0", path.as_ref().display());
        let rc = unsafe { ffi::imageWrite(path.as_ptr() as *const i8, self.0) };

        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(())
    }

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

        Ok(Image(image, None))
    }

    pub fn new_like_with_color(&self, color: Color) -> Result<Self, Error> {
        let meta = self.meta().clone().with_color(color);
        Self::new(meta)
    }

    pub fn new_like_with_type(&self, t: Type) -> Result<Self, Error> {
        let meta = self.meta().clone().with_type(t);
        Self::new(meta)
    }

    pub fn meta(&self) -> &Meta {
        return unsafe { &(&*self.0).meta };
    }

    pub fn elem_size(&self) -> usize {
        self.meta().bits as usize / 8
    }

    pub fn data_ptr(&self) -> *mut c_void {
        if self.0.is_null() {
            return std::ptr::null_mut();
        }

        unsafe { (*self.0).data }
    }

    pub fn data<T>(&self) -> Result<&[T], Error> {
        if std::mem::size_of::<T>() != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let data = unsafe {
            std::slice::from_raw_parts((&*self.0).data as *const T, self.meta().total_bytes())
        };
        Ok(data)
    }

    pub fn data_mut<T>(&mut self) -> Result<&mut [T], Error> {
        if std::mem::size_of::<T>() != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let data = unsafe {
            std::slice::from_raw_parts_mut((&mut *self.0).data as *mut T, self.meta().total_bytes())
        };
        Ok(data)
    }

    pub fn is_type<T>(&self) -> bool {
        std::mem::size_of::<T>() == self.elem_size()
    }

    pub fn at<T>(&mut self, x: usize, y: usize) -> Result<&mut [T], Error> {
        if std::mem::size_of::<T>() != self.elem_size() {
            return Err(Error::IncorrectImageType);
        }

        let meta = self.meta();
        if x as u64 >= meta.width || y as u64 >= meta.height {
            return Err(Error::OutOfBounds);
        }

        let ptr = unsafe { ffi::imageAt(self.0, x, y) };
        if ptr.is_null() {
            return Err(Error::NullPointer);
        }

        let data = unsafe { std::slice::from_raw_parts_mut(ptr as *mut T, meta.channels()) };

        Ok(data)
    }

    pub fn get_pixel(&self, x: usize, y: usize, px: &mut Pixel) -> bool {
        unsafe { ffi::imageGetPixel(self.0, x, y, px) }
    }

    pub fn set_pixel(&self, x: usize, y: usize, px: &Pixel) -> bool {
        unsafe { ffi::imageSetPixel(self.0, x, y, px) }
    }

    pub fn for_each<T, F: Fn((usize, usize), &mut [T])>(&mut self, f: F) -> Result<(), Error> {
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

    pub fn for_each2<T, F: Fn((usize, usize), &mut [T], &[T])>(
        &mut self,
        other: &Image,
        f: F,
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

    pub fn convert_to(&self, dest: &mut Image) -> Result<(), Error> {
        let rc = unsafe { ffi::imageConvertTo(self.0, dest.0) };
        if !rc {
            return Err(Error::IncorrectImageType);
        }
        Ok(())
    }

    pub fn convert(&self, color: Color, t: Type) -> Result<Image, Error> {
        let (kind, bits) = t.info();
        let dest = unsafe { ffi::imageConvert(self.0, color.ffi(), kind, bits) };
        if dest.is_null() {
            return Err(Error::NullPointer);
        }
        Ok(Image(dest, None))
    }

    pub fn clone<'b>(&self) -> Image<'b> {
        let img = unsafe { ffi::imageClone(self.0) };
        Image(img, None)
    }
}

#[cfg(test)]
mod tests {
    use crate::*;

    #[test]
    fn it_works() {
        let db = DB::open("./testing").unwrap();
        db.destroy().unwrap();
    }
}

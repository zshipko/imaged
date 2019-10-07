pub mod ffi;

#[derive(Debug)]
pub enum Error {
    CannotOpenDB,
    InvalidPath,
    NullPointer,
    FFI(ffi::ImagedStatus),
}

pub use ffi::ImagedMeta as Meta;
pub struct Imaged(*mut ffi::Imaged);
pub struct Iter<'a>(*mut ffi::ImagedIter, &'a Imaged);
pub struct Handle<'a>(ffi::ImagedHandle, &'a Imaged);
pub struct Image<'a>(*mut ffi::Image, Option<&'a Imaged>);

pub enum Type {
    I(u8),
    U(u8),
    F(u8),
}

impl Meta {
    pub fn new(w: usize, h: usize, channels: u8, ty: Type) -> Meta {
        let (kind, bits) = match ty {
            Type::I(x) => (ffi::ImagedKind::IMAGED_KIND_INT, x),
            Type::U(x) => (ffi::ImagedKind::IMAGED_KIND_UINT, x),
            Type::F(x) => (ffi::ImagedKind::IMAGED_KIND_FLOAT, x),
        };

        Meta {
            width: w as u64,
            height: h as u64,
            channels,
            kind,
            bits,
        }
    }

    pub fn typ(&self) -> Type {
        match self.kind {
            ffi::ImagedKind::IMAGED_KIND_INT => Type::I(self.bits),
            ffi::ImagedKind::IMAGED_KIND_UINT => Type::U(self.bits),
            ffi::ImagedKind::IMAGED_KIND_FLOAT => Type::F(self.bits),
        }
    }
}

impl<'a> Drop for Image<'a> {
    fn drop(&mut self) {
        match self.1 {
            Some(_) => unsafe { ffi::imageFree(self.0) },
            _ => (),
        }
    }
}

impl<'a> Drop for Handle<'a> {
    fn drop(&mut self) {
        unsafe { ffi::imagedHandleFree(&mut self.0) }
    }
}

impl<'a> Drop for Iter<'a> {
    fn drop(&mut self) {
        unsafe { ffi::imagedIterFree(self.0) }
    }
}

impl Imaged {
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

        return Ok(Imaged(db));
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

    pub fn get<S: AsRef<str>>(&self, key: S) -> Result<Handle, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            ffi::imagedGet(
                self.0,
                key.as_ref().as_ptr() as *const i8,
                key.as_ref().len() as isize,
                &mut handle,
            )
        };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(Handle(handle, self))
    }

    pub fn set<S: AsRef<str>>(
        &self,
        key: S,
        meta: Meta,
        data: Option<*const std::ffi::c_void>,
    ) -> Result<Handle, Error> {
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

impl Drop for Imaged {
    fn drop(&mut self) {
        unsafe { ffi::imagedClose(self.0) }
    }
}

extern "C" {
    fn strlen(_: *const i8) -> usize;
}

impl<'a> Iter<'a> {
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
            let key = std::slice::from_raw_parts(iter.key as *const u8, strlen(iter.key));
            let key = std::str::from_utf8_unchecked(key);
            Some((key, Image(ptr, Some(self.1))))
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::*;

    #[test]
    fn it_works() {
        let db = Imaged::open("./testing").unwrap();
        db.destroy().unwrap();
    }
}

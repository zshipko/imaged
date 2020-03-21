use crate::*;

use std::os::raw::c_char;

/// DB wraps the libimaged's `Imaged` type
pub struct DB(*mut ffi::Imaged);

impl Drop for DB {
    fn drop(&mut self) {
        unsafe { ffi::imagedClose(self.0) }
    }
}

/// Handle wraps the `ImagedHandle` type
pub struct Handle<'a>(ffi::ImagedHandle, &'a DB);

impl<'a> Drop for Handle<'a> {
    fn drop(&mut self) {
        unsafe { ffi::imagedHandleClose(&mut self.0) }
    }
}

impl<'a> Handle<'a> {
    /// Get the underlying image
    pub fn image(&self) -> Image {
        unsafe {
            Image(
                &mut *(&self.0.image as *const ffi::Image as *mut ffi::Image),
                false,
            )
        }
    }
}

impl DB {
    /// Open an imaged instance at the given path
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

    /// Destroy all imgd files at the DB's root
    pub fn destroy(self) -> Result<(), Error> {
        let rc = unsafe { ffi::imagedDestroy(self.0) };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }
        Ok(())
    }

    /// Returns a new image iterator
    pub fn iter<'a>(&'a self) -> Result<Iter<'a>, Error> {
        let iter = unsafe { ffi::imagedIterNew(self.0) };
        if iter.is_null() {
            return Err(Error::NullPointer);
        }

        return Ok(Iter(iter, self));
    }

    /// Returns a new key iterator
    pub fn iter_keys<'a>(&'a self) -> Result<KeyIter<'a>, Error> {
        let iter = unsafe { ffi::imagedIterNew(self.0) };
        if iter.is_null() {
            return Err(Error::NullPointer);
        }

        return Ok(KeyIter(iter, self));
    }

    /// Get a key, `editable` determines whether or not the pixels can be edited
    pub fn get<'a, S: AsRef<str>>(&'a self, key: S, editable: bool) -> Result<Handle<'a>, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            ffi::imagedGet(
                self.0,
                key.as_ref().as_ptr() as *const c_char,
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

    /// Check if a key is in use
    pub fn is_locked(&self, key: &str) -> bool {
        unsafe { ffi::imagedKeyIsLocked(self.0, key.as_ptr() as *const c_char, key.len() as isize) }
    }

    /// Check if a key is a valid imgd file
    pub fn file_is_valid(&self, key: &str) -> bool {
        unsafe { ffi::imagedIsValidFile(self.0, key.as_ptr() as *const c_char, key.len() as isize) }
    }

    /// Set a key
    pub fn set<'a, S: AsRef<str>>(&'a self, key: S, image: &Image) -> Result<Handle<'a>, Error> {
        self.set_raw(key, image.meta().clone(), Some(image.data_ptr()))
    }

    /// Set a key using metadata and a pointer to the image data
    pub fn set_raw<'a, S: AsRef<str>>(
        &'a self,
        key: S,
        meta: Meta,
        data: Option<*const c_void>,
    ) -> Result<Handle<'a>, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            ffi::imagedSet(
                self.0,
                key.as_ref().as_ptr() as *const c_char,
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

    /// Remove a key
    pub fn remove<S: AsRef<str>>(&self, key: S) -> Result<(), Error> {
        let rc = unsafe {
            ffi::imagedRemove(
                self.0,
                key.as_ref().as_ptr() as *const c_char,
                key.as_ref().len() as isize,
            )
        };
        if rc != ffi::ImagedStatus::IMAGED_OK {
            return Err(Error::FFI(rc));
        }

        Ok(())
    }
}

use crate::*;

use std::marker::PhantomData;
use std::os::raw::c_char;

/// DB wraps the libimaged's `Imaged` type
pub struct DB(*mut sys::Imaged);

impl Drop for DB {
    fn drop(&mut self) {
        unsafe { sys::imagedClose(self.0) }
    }
}

/// Handle wraps the `ImagedHandle` type
pub struct Handle<'a>(sys::ImagedHandle, &'a DB);

impl<'a> Drop for Handle<'a> {
    fn drop(&mut self) {
        unsafe { sys::imagedHandleClose(&mut self.0) }
    }
}

impl<'a> Handle<'a> {
    /// Get the underlying image
    pub fn image(&mut self) -> Image {
        Image(&mut self.0.image, false, PhantomData)
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

        let db = unsafe { sys::imagedOpen(path.as_ptr()) };
        if db.is_null() {
            return Err(Error::CannotOpenDB);
        }

        Ok(DB(db))
    }

    /// Destroy all imgd files at the DB's root
    pub fn destroy(self) -> Result<(), Error> {
        let rc = unsafe { sys::imagedDestroy(self.0) };
        if rc != sys::ImagedStatus::IMAGED_OK {
            return Err(Error::Sys(rc));
        }
        Ok(())
    }

    /// Returns a new image iterator
    pub fn iter(&self) -> Result<Iter, Error> {
        let iter = unsafe { sys::imagedIterNew(self.0) };
        if iter.is_null() {
            return Err(Error::NullPointer);
        }

        Ok(Iter(iter, self))
    }

    /// Returns a new key iterator
    pub fn iter_keys(&self) -> Result<KeyIter, Error> {
        let iter = unsafe { sys::imagedIterNew(self.0) };
        if iter.is_null() {
            return Err(Error::NullPointer);
        }

        Ok(KeyIter(iter, self))
    }

    /// Get a key, `editable` determines whether or not the pixels can be edited
    pub fn get<S: AsRef<str>>(&self, key: S, editable: bool) -> Result<Handle, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            sys::imagedGet(
                self.0,
                key.as_ref().as_ptr() as *const c_char,
                key.as_ref().len() as i64,
                editable,
                &mut handle,
            )
        };
        if rc != sys::ImagedStatus::IMAGED_OK {
            return Err(Error::Sys(rc));
        }

        Ok(Handle(handle, self))
    }

    /// Check if a key is in use
    pub fn is_locked(&self, key: &str) -> bool {
        unsafe { sys::imagedKeyIsLocked(self.0, key.as_ptr() as *const c_char, key.len() as i64) }
    }

    /// Check if a key is a valid imgd file
    pub fn file_is_valid(&self, key: &str) -> bool {
        unsafe { sys::imagedIsValidFile(self.0, key.as_ptr() as *const c_char, key.len() as i64) }
    }

    /// Set a key
    pub fn set<'a, S: AsRef<str>>(&'a self, key: S, image: &Image) -> Result<Handle<'a>, Error> {
        self.set_raw(key, image.meta().clone(), Some(image.data_ptr()))
    }

    /// Set a key using metadata and a pointer to the image data
    pub fn set_raw<S: AsRef<str>>(
        &self,
        key: S,
        meta: Meta,
        data: Option<*const c_void>,
    ) -> Result<Handle, Error> {
        let mut handle = unsafe { std::mem::zeroed() };
        let rc = unsafe {
            sys::imagedSet(
                self.0,
                key.as_ref().as_ptr() as *const c_char,
                key.as_ref().len() as i64,
                &meta,
                data.unwrap_or(std::ptr::null()),
                &mut handle,
            )
        };
        if rc != sys::ImagedStatus::IMAGED_OK {
            return Err(Error::Sys(rc));
        }

        Ok(Handle(handle, self))
    }

    /// Remove a key
    pub fn remove<S: AsRef<str>>(&self, key: S) -> Result<(), Error> {
        let rc = unsafe {
            sys::imagedRemove(
                self.0,
                key.as_ref().as_ptr() as *const c_char,
                key.as_ref().len() as i64,
            )
        };
        if rc != sys::ImagedStatus::IMAGED_OK {
            return Err(Error::Sys(rc));
        }

        Ok(())
    }
}

use crate::*;

/// Iterator over Imaged images
pub struct Iter<'a>(pub(crate) *mut ffi::ImagedIter, pub(crate) &'a DB);

/// Iterator over Imaged keys
pub struct KeyIter<'a>(pub(crate) *mut ffi::ImagedIter, pub(crate) &'a DB);

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
    type Item = (&'a str, Image);

    fn next(&mut self) -> Option<Self::Item> {
        let ptr = unsafe { ffi::imagedIterNext(self.0) };
        if ptr.is_null() {
            return None;
        }

        unsafe {
            let iter = &*self.0;
            let key = std::slice::from_raw_parts(iter.key as *const u8, iter.keylen);
            let key = std::str::from_utf8_unchecked(key);
            Some((key, Image(ptr, false)))
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

use crate::*;

#[derive(Debug)]
pub enum Error {
    /// Occurs when a database can't be opened due to permission issues, etc...
    CannotOpenDB,

    /// Occurs when a path can't be converted to valid UTF-8
    InvalidPath,

    /// Occurs when a NULL pointer is encountered
    NullPointer,

    /// Occurs when image data is accessed using the wrong type
    IncorrectImageType,

    /// Occurs when an image is accessed beyond its bounds
    OutOfBounds,

    /// Wraps ImagedStatus
    FFI(ffi::ImagedStatus),
}

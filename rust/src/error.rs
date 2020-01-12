use crate::*;

use thiserror::Error as TError;

/// Crate-wide error type
#[derive(TError, Debug)]
pub enum Error {
    /// Occurs when a database can't be opened due to permission issues, etc...
    #[error("There was an error when attempting to open the database")]
    CannotOpenDB,

    /// Occurs when a path can't be converted to valid UTF-8
    #[error("An invalid path was encountered")]
    InvalidPath,

    /// Occurs when a NULL pointer is encountered
    #[error("A NULL pointer was encountered")]
    NullPointer,

    /// Occurs when image data is accessed using the wrong type
    #[error("Image data was accessed using the wrong data type")]
    IncorrectImageType,

    /// Occurs when an image is accessed beyond its bounds
    #[error("An image has been accessed beyond the available dimensions")]
    OutOfBounds,

    /// Wraps ImagedStatus
    #[error("Imaged specified error")]
    FFI(ffi::ImagedStatus),
}

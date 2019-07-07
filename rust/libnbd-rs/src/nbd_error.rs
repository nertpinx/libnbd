use libnbd_sys::{nbd_get_errno, nbd_get_error};
use std::ffi::CStr;
use std::fmt;

#[derive(Debug, Copy, Clone)]
pub struct NbdError {
    errno: i32,
    strerr: &'static CStr,
}

impl NbdError {
    pub fn from_libnbd() -> Self {
        Self {
            errno: unsafe { nbd_get_errno() },
            strerr: unsafe { CStr::from_ptr(nbd_get_error()) },
        }
    }
}

impl std::error::Error for NbdError {}

impl fmt::Display for NbdError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "NBD Error (errno = {}): {}",
            self.errno,
            self.strerr.to_string_lossy()
        )
    }
}

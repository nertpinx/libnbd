use super::nbd_error::*;
use libnbd_sys::*;

pub struct Nbd {
    pub(crate) handle: *mut nbd_handle,
}

impl Nbd {
    pub fn create() -> Result<Self, NbdError> {
        let handle = unsafe { nbd_create() };
        if handle.is_null() {
            return Err(NbdError::from_libnbd());
        }
        Ok(Self { handle: handle })
    }
}

impl Drop for Nbd {
    fn drop(&mut self) {
        unsafe { nbd_close(self.handle) };
    }
}

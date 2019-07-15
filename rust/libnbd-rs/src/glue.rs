/* NBD client library in userspace
 * WARNING: THIS FILE IS GENERATED FROM
 * generator/generator
 * ANY CHANGES YOU MAKE TO THIS FILE WILL BE LOST.
 *
 * Copyright (C) 2013-2019 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

//use libc;
use libnbd_sys::*;
use std::ffi::CStr;
#[allow(unused_imports)]
use std::os::raw::{c_char, c_int, c_uint, c_void};
//use std::os::unix::io::RawFd;

pub use super::base::*;
use super::nbd_error::*;

impl Nbd {
    /// Set or clear the debug flag
    ///
    /// Set or clear the debug flag. When debugging is enabled,
    /// debugging messages from the library are printed to
    /// stderr, unless a debugging callback has been defined too
    /// (see [`Nbd::set_debug_callback`]) in which case they are
    /// sent to that function. This flag defaults to false on
    /// newly created handles, except if `LIBNBD_DEBUG=1` is set
    /// in the environment in which case it defaults to true.
    pub fn set_debug(&self, debug: bool) -> Result<(), NbdError> {
        let ret = unsafe { nbd_set_debug(self.handle, debug) };
        if ret == -1 {
            return Err(NbdError::from_libnbd());
        }
        Ok(())
    }

    /// Set the debug callback
    ///
    /// Set the debug callback. This function is called when the
    /// library emits debug messages, when debugging is enabled
    /// on a handle. The callback parameters are `data` passed
    /// to this function, the name of the libnbd function
    /// emitting the debug message (`context`), and the message
    /// itself (`msg`). If no debug callback is set on a handle
    /// then messages are printed on `stderr`.
    ///
    /// The callback should not call [`libnbd_rs::*`] APIs on
    /// the same handle since it can be called while holding the
    /// handle lock and will cause a deadlock.
    pub fn set_debug_callback(
        &mut self,
        debug_fn: Option<Box<dyn Fn(&'static str, &'static str)>>,
    ) -> Result<(), NbdError> {
        extern "C" fn cb_wrapper(
            data: *mut c_void,
            context: *const c_char,
            msg: *const c_char,
        ) -> c_int {
            eprintln!("BLA_data: {:#?}", data);
            let cb = data as *mut Option<&dyn Fn(&'static str, &'static str)>;
            eprintln!("BLA_cb: {:#?}", cb);
            let cb2 = match unsafe { *cb } {
                Some(x) => x,
                None => {
                    eprintln!("WAT!?!?");
                    return 0;
                }
            };
            let context = unsafe { CStr::from_ptr(context) }.to_str().unwrap();
            let msg = unsafe { CStr::from_ptr(msg) }.to_str().unwrap();

            eprintln!("BLA_cb2: {:p}", cb2);

            cb2(context, msg);
            0
        };

        eprintln!("MEH: {:#?}", &debug_fn as *const _);
        let debug_fn2 = match debug_fn {
            Some(f) => {
                let ptr = Box::leak(f) as *const _ as *mut _;
                eprintln!("MEH_d: {:p}", ptr);
                let tmp = unsafe { Box::from_raw(ptr) };
                self.debug_callback = Some(tmp);
                Some(ptr)
            }
            None => {
                self.debug_callback = None;
                None
            }
        };

        eprintln!("MEH2: {:#?}", &debug_fn2 as *const _);
        let debug_fn3 = &debug_fn2 as *const _ as *mut c_void;
        eprintln!("MEH3: {:#?}", debug_fn3);

        let a = std::ffi::CString::new("asdf").unwrap();
        let b = std::ffi::CString::new("fdsa").unwrap();
        cb_wrapper(debug_fn3, a.as_ptr(), b.as_ptr());

        let ret = unsafe { nbd_set_debug_callback(self.handle, debug_fn3, Some(cb_wrapper)) };
        if ret == -1 {
            return Err(NbdError::from_libnbd());
        }
        Ok(())
    }

    /// Return a descriptive string for the state of the connection
    ///
    /// Return the version of libnbd. This is returned as a
    /// string in the form `"major.minor.release"` where each of
    /// major, minor and release is a small positive integer.
    /// For example `"1.0.3"`.
    ///
    /// The major number is `0` for the early experimental
    /// versions of libnbd where we still had an unstable API,
    /// or `1` for the versions of libnbd with a long-term
    /// stable API and ABI.
    ///
    /// The minor number is even (`0`, `2`, etc) for stable
    /// releases, and odd (`1`, `3`, etc) for development
    /// versions. Note that new APIs added in a development
    /// version remain experimental and subject to change in
    /// that branch until they appear in a stable release.
    ///
    /// The release number is incremented for each release along
    /// a particular branch.
    pub fn get_version(&self) -> &'static str {
        let ret = unsafe { nbd_get_version(self.handle) };
        let ret = unsafe { CStr::from_ptr(ret) };
        let ret = ret.to_str().unwrap();
        ret
    }
}

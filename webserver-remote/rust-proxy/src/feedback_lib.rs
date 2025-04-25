use std::ffi::{c_char, CString};

unsafe extern "C" {
    fn broadcast_to_frontend(ip: *const c_char);
}

pub fn broadcast(ip: String) {
    let str = CString::new(ip).unwrap();
    unsafe {
        broadcast_to_frontend(str.as_ptr() as *const c_char);
    }
}

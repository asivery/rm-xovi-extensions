use std::ffi::{c_char, CStr};

mod proxy;
#[path = "feedback_lib.rs"]
mod feedback;

// metadata-invoked by xovi-message-broker
#[unsafe(no_mangle)] extern "C" fn add_blocked(addr: *const c_char) -> *const c_char {
    let addr: String = unsafe { CStr::from_ptr(addr) }.to_str().unwrap().into();
    proxy::add_address(addr, proxy::IpRefAction::Blocked);
    std::ptr::null()
}

// metadata-invoked by xovi-message-broker
#[unsafe(no_mangle)] extern "C" fn add_allowed(addr: *const c_char) -> *const c_char {
    let addr: String = unsafe { CStr::from_ptr(addr) }.to_str().unwrap().into();
    proxy::add_address(addr, proxy::IpRefAction::Allowed);
    std::ptr::null()
}

#[unsafe(no_mangle)] extern "C" fn start_proxy_server() {
    proxy::start_proxy();
}

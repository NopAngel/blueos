#![no_std]

use core::panic::PanicInfo;

#[no_mangle]
pub extern "C" fn suma_rust(a: i32, b: i32) -> i32 {
    a + b
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
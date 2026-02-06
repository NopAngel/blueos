#![no_std]

pub mod security;
pub mod virt;
pub mod mm;

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
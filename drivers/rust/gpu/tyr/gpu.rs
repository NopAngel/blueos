// LICENSE: GPL 2.0

// (schema)

#![no_std]
#![no_main]
#![feature(lang_items)]

use core::panic::PanicInfo;


mod tyr_registers {
    pub const TYR_CTRL_REG: u32 = 0x0000;
    pub const TYR_STATUS_REG: u32 = 0x0004;
    pub const TYR_FB_ADDR: u32 = 0x0008;
    pub const TYR_RES_X: u32 = 0x000C;
    pub const TYR_RES_Y: u32 = 0x0010;
    // ... 
}

struct TyrGpuDriver {
    base_address: *mut u32,
    framebuffer: *mut u8,
}

impl TyrGpuDriver {
    unsafe fn new(addr: usize) -> Self {
        let base = addr as *mut u32;

        // Reset GPU
        base.write_volatile(0x1);

        Self {
            base_address: base,
            framebuffer: core::ptr::null_mut(),
        }
    }

    unsafe fn set_resolution(&mut self, width: u32, height: u32) {
        let base = self.base_address;
        base.offset(tyr_registers::TYR_RES_X as isize / 4).write_volatile(width);
        base.offset(tyr_registers::TYR_RES_Y as isize / 4).write_volatile(height);
    }

   
    unsafe fn enable(&self) {
        let base = self.base_address;
        let ctrl = base.read_volatile();
        base.write_volatile(ctrl | 0x1);
    }
}


#[no_mangle]
pub extern "C" fn _start() -> ! {
    unsafe {
        let mut gpu = TyrGpuDriver::new(0xF0000000);
        gpu.set_resolution(1024, 768);
        gpu.enable();
    }

    loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[lang = "eh_personality"]
extern "C" fn eh_personality() {}


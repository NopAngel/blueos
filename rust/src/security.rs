extern "C" {
    fn printk(s: *const u8, row: i32, color: i32);
}


#[no_mangle]
pub extern "C" fn is_command_safe(command_ptr: *const u8, length: i32) -> i32 {
    if length > 20 { return 0; } 
    let cmd = unsafe { core::slice::from_raw_parts(command_ptr, length as usize) };

   
    let dangerous = b"rm-rf";
    if cmd == dangerous {
        return 0;
    }

    1
}

#[no_mangle]
pub static mut __stack_chk_guard: u32 = 0x595a5553; // "SYSU" en hex, por ejemplo

#[no_mangle]
pub extern "C" fn __stack_chk_fail() -> ! {
   
    loop {}
}


#[no_mangle]
pub extern "C" fn ipe_verify_binary(buffer_ptr: *const u8, size: usize) -> i32 {
    if buffer_ptr.is_null() || size < 4 {
        return 0;
    }


    let binary = unsafe { core::slice::from_raw_parts(buffer_ptr, size) };

    let magic = b"BLUS";
    if &binary[0..4] != magic {
        return -1;
    }

   
    let mut checksum: u8 = 0;
    for &byte in binary.iter().skip(4) {
        checksum = checksum.wrapping_add(byte);
    }

    
    1
}


static mut TRUSTED_DEVICE_ID: i32 = -1;

#[no_mangle]
pub extern "C" fn loadpin_init(device_id: i32) {
    unsafe {
        if TRUSTED_DEVICE_ID == -1 {
            TRUSTED_DEVICE_ID = device_id;
        }
    }
}

#[no_mangle]
pub extern "C" fn loadpin_check(source_device_id: i32) -> i32 {
    unsafe {
        if TRUSTED_DEVICE_ID != -1 && source_device_id != TRUSTED_DEVICE_ID {
            return 0; 
        }
    }
    1 
}



struct Sandbox {
    min_memory: u32,
    max_memory: u32,
    can_access_io: bool,
}

static mut POLICIES: [Option<Sandbox>; 4] = [None, None, None, None];

#[no_mangle]
pub extern "C" fn landlock_restrict(pid: usize, min_mem: u32, max_mem: u32, io: bool) -> i32 {
    if pid >= 4 { return -1; }
    unsafe {
        POLICIES[pid] = Some(Sandbox {
            min_memory: min_mem,
            max_memory: max_mem,
            can_access_io: io,
        });
    }
    0
}

#[no_mangle]
pub extern "C" fn landlock_check_mem(pid: usize, address: u32) -> i32 {
    unsafe {
        if let Some(ref policy) = POLICIES[pid] {
            if address >= policy.min_memory && address <= policy.max_memory {
                return 1; 
            }
            return 0; 
        }
    }
    1 
}

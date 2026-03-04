// HOW TO USE:

/*

use crate::_DRIVER_INIT;
use crate::_DRIVER_VERSION;
use crate::_DRIVER_PANIC;

_DRIVER_INIT!("PS2_Keyboard", "Standard PS/2 Keyboard Driver for BlueOS");
_DRIVER_VERSION!("1.0.0");

#[no_mangle]
pub extern "C" fn _DRIVER_PS2_Keyboard() {
    let hardware_ok = true; 
    if !hardware_ok {
        _DRIVER_PANIC!("ERR TEST");
    }
}

#[no_mangle]
pub extern "C" fn keyboard_process_scancode(code: u8) {

}*/
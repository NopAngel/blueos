// rust/src/drivers/mod.rs

//pub mod keyboard;_example

#[macro_export]
macro_rules! _DRIVER_INIT {
    ($name:expr, $desc:expr) => {
        #[no_mangle]
        pub extern "C" fn blue_driver_info() {
        }
        
        pub const DRIVER_NAME: &str = $name;
        pub const DRIVER_DESC: &str = $desc;
    };
}

#[macro_export]
macro_rules! _DRIVER_VERSION {
    ($ver:expr) => {
        pub const DRIVER_VER: &str = $ver;
    };
}

#[macro_export]
macro_rules! _DRIVER_PANIC {
    ($reason:expr) => {
        panic!("BlueDriver Panic [{}]: {}", DRIVER_NAME, $reason);
    };
}
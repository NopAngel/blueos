#![no_std]
#![no_main]
#![feature(lang_items, start, asm, naked_functions, panic_info_message)]

use core::ptr::{read_volatile, write_volatile};
use core::mem::zeroed;


const DRM_IOCTL_BASE: u32 = 0x64;
const DRM_IOCTL_MODE_GETRESOURCES: u32 = 0xC0106400;
const DRM_IOCTL_MODE_GETCONNECTOR: u32 = 0xC01C6407;
const DRM_IOCTL_MODE_GETENCODER: u32 = 0xC0186402;
const DRM_IOCTL_MODE_CREATE_DUMB: u32 = 0xC0206401;
const DRM_IOCTL_MODE_MAP_DUMB: u32 = 0xC0106403;
const DRM_IOCTL_MODE_PAGE_FLIP: u32 = 0xC018640D;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeRes {
    count_fbs: u32,
    fbs: u64,
    count_crtcs: u32,
    crtcs: u64,
    count_connectors: u32,
    connectors: u64,
    count_encoders: u32,
    encoders: u64,
    min_width: u32,
    max_width: u32,
    min_height: u32,
    max_height: u32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeConnector {
    connector_id: u32,
    encoder_id: u32,
    connector_type: u32,
    connector_type_id: u32,
    connection: u32,
    mm_width: u32,
    mm_height: u32,
    subpixel: u32,
    count_modes: u32,
    modes: u64,
    count_props: u32,
    props: u64,
    prop_values: u64,
    count_encoders: u32,
    encoders: u64,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeModeInfo {
    clock: u32,
    hdisplay: u16,
    hsync_start: u16,
    hsync_end: u16,
    htotal: u16,
    hskew: u16,
    vdisplay: u16,
    vsync_start: u16,
    vsync_end: u16,
    vtotal: u16,
    vscan: u16,
    vrefresh: u32,
    flags: u32,
    type_: u32,
    name: [u8; 32],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeCrtc {
    crtc_id: u32,
    buffer_id: u32,
    x: u32,
    y: u32,
    width: u32,
    height: u32,
    mode_valid: i32,
    mode: DrmModeModeInfo,
    gamma_size: i32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeGetResources {
    fb_id_ptr: u64,
    crtc_id_ptr: u64,
    connector_id_ptr: u64,
    encoder_id_ptr: u64,
    count_fbs: u32,
    count_crtcs: u32,
    count_connectors: u32,
    count_encoders: u32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeGetConnector {
    encoders_ptr: u64,
    modes_ptr: u64,
    props_ptr: u64,
    prop_values_ptr: u64,
    count_modes: u32,
    count_props: u32,
    count_encoders: u32,
    encoder_id: u32,
    connector_id: u32,
    connector_type: u32,
    connector_type_id: u32,
    connection: u32,
    mm_width: u32,
    mm_height: u32,
    subpixel: u32,
    pad: u32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeCreateDumb {
    height: u32,
    width: u32,
    bpp: u32,
    flags: u32,
    handle: u32,
    pitch: u32,
    size: u64,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct DrmModeMapDumb {
    handle: u32,
    pad: u32,
    offset: u64,
}

struct DrmDevice {
    fd: i32,
    resources: DrmModeRes,
    connector: DrmModeConnector,
    crtc: DrmModeCrtc,
    mode: DrmModeModeInfo,
    fb_handle: u32,
    fb_size: usize,
    fb_ptr: *mut u8,
}

impl DrmDevice {
    fn new() -> Self {
        unsafe {
            Self {
                fd: -1,
                resources: zeroed(),
                connector: zeroed(),
                crtc: zeroed(),
                mode: zeroed(),
                fb_handle: 0,
                fb_size: 0,
                fb_ptr: core::ptr::null_mut(),
            }
        }
    }

    unsafe fn open(&mut self, path: &[u8]) -> i32 {
        let fd: i32;
        asm!(
            "mov x8, #56",    // syscall openat
            "mov x0, #-100",  // AT_FDCWD
            "mov x1, {path}",
            "mov x2, #2",     // O_RDWR
            "svc #0",
            "mov {fd}, x0",
            path = in(reg) path.as_ptr(),
            fd = out(reg) fd,
        );
        self.fd = fd;
        fd
    }

    unsafe fn ioctl(&self, cmd: u32, arg: u64) -> i32 {
        let mut ret: i32;
        asm!(
            "mov x8, #29",    // syscall ioctl
            "mov x0, {fd}",
            "mov x1, {cmd}",
            "mov x2, {arg}",
            "svc #0",
            "mov {ret}, x0",
            fd = in(reg) self.fd,
            cmd = in(reg) cmd,
            arg = in(reg) arg,
            ret = out(reg) ret,
        );
        ret
    }

    unsafe fn mmap(&self, addr: u64, length: usize, prot: i32, flags: i32, offset: i64) -> *mut u8 {
        let ptr: u64;
        asm!(
            "mov x8, #222",   // syscall mmap
            "mov x0, {addr}",
            "mov x1, {length}",
            "mov x2, {prot}",
            "mov x3, {flags}",
            "mov x4, {offset}",
            "svc #0",
            "mov {ptr}, x0",
            addr = in(reg) addr,
            length = in(reg) length,
            prot = in(reg) prot,
            flags = in(reg) flags,
            offset = in(reg) offset,
            ptr = out(reg) ptr,
        );
        ptr as *mut u8
    }

    unsafe fn get_resources(&mut self) -> i32 {
        let mut res = DrmModeGetResources {
            fb_id_ptr: 0,
            crtc_id_ptr: 0,
            connector_id_ptr: 0,
            encoder_id_ptr: 0,
            count_fbs: 0,
            count_crtcs: 0,
            count_connectors: 0,
            count_encoders: 0,
        };

        let ret = self.ioctl(DRM_IOCTL_MODE_GETRESOURCES, &mut res as *mut _ as u64);
        if ret < 0 {
            return ret;
        }

        // Allocate arrays
        let fb_ids = Self::alloc::<u32>(res.count_fbs as usize);
        let crtc_ids = Self::alloc::<u32>(res.count_crtcs as usize);
        let connector_ids = Self::alloc::<u32>(res.count_connectors as usize);
        let encoder_ids = Self::alloc::<u32>(res.count_encoders as usize);

        res.fb_id_ptr = fb_ids as u64;
        res.crtc_id_ptr = crtc_ids as u64;
        res.connector_id_ptr = connector_ids as u64;
        res.encoder_id_ptr = encoder_ids as u64;

        let ret = self.ioctl(DRM_IOCTL_MODE_GETRESOURCES, &mut res as *mut _ as u64);
        
        if ret >= 0 {
            // Find first connected connector
            for i in 0..res.count_connectors {
                let connector_id = *connector_ids.offset(i as isize);
                if self.get_connector(connector_id) == 0 && self.connector.connection == 1 {
                    break;
                }
            }
        }

        Self::dealloc(fb_ids);
        Self::dealloc(crtc_ids);
        Self::dealloc(connector_ids);
        Self::dealloc(encoder_ids);

        ret
    }

    unsafe fn get_connector(&mut self, connector_id: u32) -> i32 {
        let mut get_conn = DrmModeGetConnector {
            encoders_ptr: 0,
            modes_ptr: 0,
            props_ptr: 0,
            prop_values_ptr: 0,
            count_modes: 0,
            count_props: 0,
            count_encoders: 0,
            encoder_id: 0,
            connector_id,
            connector_type: 0,
            connector_type_id: 0,
            connection: 0,
            mm_width: 0,
            mm_height: 0,
            subpixel: 0,
            pad: 0,
        };

        let ret = self.ioctl(DRM_IOCTL_MODE_GETCONNECTOR, &mut get_conn as *mut _ as u64);
        if ret < 0 {
            return ret;
        }

        // Allocate arrays
        let encoders = Self::alloc::<u32>(get_conn.count_encoders as usize);
        let modes = Self::alloc::<DrmModeModeInfo>(get_conn.count_modes as usize);
        let props = Self::alloc::<u32>(get_conn.count_props as usize);
        let prop_values = Self::alloc::<u64>(get_conn.count_props as usize);

        get_conn.encoders_ptr = encoders as u64;
        get_conn.modes_ptr = modes as u64;
        get_conn.props_ptr = props as u64;
        get_conn.prop_values_ptr = prop_values as u64;

        let ret = self.ioctl(DRM_IOCTL_MODE_GETCONNECTOR, &mut get_conn as *mut _ as u64);
        
        if ret >= 0 {
            self.connector = DrmModeConnector {
                connector_id: get_conn.connector_id,
                encoder_id: get_conn.encoder_id,
                connector_type: get_conn.connector_type,
                connector_type_id: get_conn.connector_type_id,
                connection: get_conn.connection,
                mm_width: get_conn.mm_width,
                mm_height: get_conn.mm_height,
                subpixel: get_conn.subpixel,
                count_modes: get_conn.count_modes,
                modes: get_conn.modes_ptr,
                count_props: get_conn.count_props,
                props: get_conn.props_ptr,
                prop_values: get_conn.prop_values_ptr,
                count_encoders: get_conn.count_encoders,
                encoders: get_conn.encoders_ptr,
            };

            // Use first mode if available
            if get_conn.count_modes > 0 {
                self.mode = *modes;
            }
        }

        Self::dealloc(encoders);
        Self::dealloc(modes);
        Self::dealloc(props);
        Self::dealloc(prop_values);

        ret
    }

    unsafe fn create_framebuffer(&mut self, width: u32, height: u32) -> i32 {
        let mut create_dumb = DrmModeCreateDumb {
            height,
            width,
            bpp: 32,
            flags: 0,
            handle: 0,
            pitch: 0,
            size: 0,
        };

        let ret = self.ioctl(DRM_IOCTL_MODE_CREATE_DUMB, &mut create_dumb as *mut _ as u64);
        if ret < 0 {
            return ret;
        }

        self.fb_handle = create_dumb.handle;
        self.fb_size = create_dumb.size as usize;

        // Map framebuffer
        let mut map_dumb = DrmModeMapDumb {
            handle: create_dumb.handle,
            pad: 0,
            offset: 0,
        };

        let ret = self.ioctl(DRM_IOCTL_MODE_MAP_DUMB, &mut map_dumb as *mut _ as u64);
        if ret < 0 {
            return ret;
        }

        self.fb_ptr = self.mmap(0, self.fb_size, 3, 1, map_dumb.offset as i64);
        if self.fb_ptr.is_null() {
            return -1;
        }

        0
    }

    unsafe fn draw_pixel(&self, x: u32, y: u32, color: u32) {
        if x >= self.mode.hdisplay as u32 || y >= self.mode.vdisplay as u32 {
            return;
        }

        let pitch = (self.mode.hdisplay * 4) as u32;
        let offset = (y * pitch) + (x * 4);
        
        if offset < self.fb_size as u32 {
            write_volatile(self.fb_ptr.add(offset as usize) as *mut u32, color);
        }
    }

    unsafe fn fill_rect(&self, x: u32, y: u32, w: u32, h: u32, color: u32) {
        let pitch = (self.mode.hdisplay * 4) as u32;
        
        for dy in 0..h {
            let row_start = ((y + dy) * pitch) + (x * 4);
            let row_end = row_start + (w * 4);
            
            let mut offset = row_start;
            while offset < row_end {
                if offset < self.fb_size as u32 {
                    write_volatile(self.fb_ptr.add(offset as usize) as *mut u32, color);
                }
                offset += 4;
            }
        }
    }

    unsafe fn clear_screen(&self, color: u32) {
        let size = self.fb_size / 4;
        for i in 0..size {
            write_volatile(self.fb_ptr.add(i * 4) as *mut u32, color);
        }
    }

    unsafe fn alloc<T>(count: usize) -> *mut T {
        let size = core::mem::size_of::<T>() * count;
        let mut ptr: u64;
        
        asm!(
            "mov x8, #9",     // syscall mmap (anonymous)
            "mov x0, #0",     // NULL address
            "mov x1, {size}",
            "mov x2, #3",     // PROT_READ | PROT_WRITE
            "mov x3, #0x22",  // MAP_PRIVATE | MAP_ANONYMOUS
            "mov x4, #-1",    // fd = -1
            "mov x5, #0",     // offset = 0
            "svc #0",
            "mov {ptr}, x0",
            size = in(reg) size,
            ptr = out(reg) ptr,
        );
        
        ptr as *mut T
    }

    unsafe fn dealloc(ptr: *mut u8) {
        if !ptr.is_null() {
            asm!(
                "mov x8, #11",    // syscall munmap
                "mov x0, {ptr}",
                "svc #0",
                ptr = in(reg) ptr,
            );
        }
    }
}

fn rgb(r: u8, g: u8, b: u8) -> u32 {
    (b as u32) << 16 | (g as u32) << 8 | (r as u32)
}

#[no_mangle]
pub extern "C" fn _start() -> ! {
    unsafe {
        let mut drm = DrmDevice::new();
       
        let card_path = b"/dev/dri/card0\0";
        if drm.open(card_path) < 0 {
            panic!("No se pudo abrir dispositivo DRM");
        }
   
        if drm.get_resources() < 0 {
            panic!("No se pudieron obtener recursos DRM");
        }
        
        if drm.connector.connection != 1 {
            panic("Monitor no conectado");
        }
        
       
        let width = drm.mode.hdisplay as u32;
        let height = drm.mode.vdisplay as u32;
        
        if drm.create_framebuffer(width, height) < 0 {
            panic("No se pudo crear framebuffer");
        }
        
        
        drm.clear_screen(rgb(0, 0, 0));
        
       
        drm.fill_rect(100, 100, 200, 150, rgb(255, 0, 0));
        
        drm.fill_rect(400, 200, 150, 200, rgb(0, 255, 0));
        
      
        for i in 0..50 {
            drm.draw_pixel(50 + i, 50 + i, rgb(0, 0, 255));
        }
        
      
        loop {
            asm!("wfe");
        }
    }
}

#[panic_handler]
fn panic(info: &core::panic::PanicInfo) -> ! {
    unsafe {
        let msg = b"PANIC\0";
        asm!(
            "mov x8, #64",    // syscall write
            "mov x0, #2",     // stderr
            "mov x1, {msg}",
            "mov x2, #6",
            "svc #0",
            msg = in(reg) msg.as_ptr(),
        );
        loop {
            asm!("wfi");
        }
    }
}

#[lang = "eh_personality"]
extern "C" fn eh_personality() {}

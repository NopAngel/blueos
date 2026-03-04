



#[repr(C, align(4096))]
struct VmxMemory {
    data: [u8; 4096],
}

#[no_mangle]
pub extern "C" fn virt_is_vm() -> i32 {
    let mut eax: u32 = 0x40000000;
    let mut ebx: u32 = 0;
    let mut ecx: u32 = 0;
    let mut edx: u32 = 0;

    unsafe {
        core::arch::asm!(
            "cpuid",
            inout("eax") eax,
            out("ebx") ebx,
            out("ecx") ecx,
            out("edx") edx,
        );
    }

    if ebx != 0 { 1 } else { 0 }
}


struct VirtualIRQChip {
    pending_irqs: u16,    
    masked_irqs: u16,    
}

static mut VIRT_IRQ_CHIP: VirtualIRQChip = VirtualIRQChip {
    pending_irqs: 0,
    masked_irqs: 0xFFFF, 
};

#[no_mangle]
pub extern "C" fn virt_irq_raise(irq: u8) {
    if irq < 16 {
        unsafe {
            VIRT_IRQ_CHIP.pending_irqs |= 1 << irq;
        }
    }
}

#[no_mangle]
pub extern "C" fn virt_irq_ack(irq: u8) {
    unsafe {
        VIRT_IRQ_CHIP.pending_irqs &= !(1 << irq);
    }
}
#[no_mangle]
pub extern "C" fn virt_irq_is_pending(irq: u8) -> i32 {
    unsafe {
        if (VIRT_IRQ_CHIP.pending_irqs & (1 << irq)) != 0 { 1 } else { 0 }
    }
}

#[no_mangle]
pub extern "C" fn virt_irq_unmask(irq: u8) {
    if irq < 16 {
        unsafe {
            VIRT_IRQ_CHIP.masked_irqs &= !(1 << irq);
        }
    }
}



#[repr(C, align(4096))]
pub struct VmControlStructure {
    pub revision_id: u32,
    pub abort_indicator: u32,
    pub data: [u8; 4088], 
}

#[no_mangle]
pub extern "C" fn rust_init_vmcs(ptr: *mut VmControlStructure, rev_id: u32) {
    if ptr.is_null() { return; }

    unsafe {
        core::ptr::write_bytes(ptr, 0, 1);
        
        (*ptr).revision_id = rev_id;
    }
}

static mut VMXON_REGION: VmxMemory = VmxMemory { data: [0; 4096] };

#[no_mangle]
pub extern "C" fn rust_allocate_vmx_region() -> *mut u8 {
    unsafe {
        VMXON_REGION.data.as_mut_ptr()
    }
}
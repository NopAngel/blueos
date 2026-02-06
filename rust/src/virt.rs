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
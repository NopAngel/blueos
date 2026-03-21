#![no_std]

// --- Estructuras de Memoria ---
#[repr(C, align(4096))]
struct VmxMemory {
    data: [u8; 4096],
}

// Estructura de control (Universal)
#[repr(C, align(4096))]
pub struct VmControlStructure {
    pub revision_id: u32,
    pub abort_indicator: u32,
    pub data: [u8; 4088], 
}

// --- Detección de VM (Arquitectura dependiente) ---

#[no_mangle]
pub extern "C" fn virt_is_vm() -> i32 {
    // Si estamos en x86 o i386 usamos CPUID
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        let mut ebx: u32;
        unsafe {
            core::arch::asm!(
                "cpuid",
                inout("eax") 0x40000000 => _,
                out("ebx") ebx,
                out("ecx") _,
                out("edx") _,
            );
        }
        if ebx != 0 { return 1; }
    }

    // Si estamos en RISC-V, la detección suele hacerse vía Device Tree
    // o chequeando la extensión H en el registro misa.
    #[cfg(target_arch = "riscv32")]
    {
        let misa: usize;
        unsafe {
            core::arch::asm!("csrr {}, misa", out(reg) misa);
        }
        if (misa & (1 << 7)) != 0 { return 1; }
    }

    0 // Por defecto no detectado
}

// --- Gestión de IRQs Virtuales (Universal) ---

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
        unsafe { VIRT_IRQ_CHIP.pending_irqs |= 1 << irq; }
    }
}

#[no_mangle]
pub extern "C" fn virt_irq_ack(irq: u8) {
    if irq < 16 {
        unsafe { VIRT_IRQ_CHIP.pending_irqs &= !(1 << irq); }
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
        unsafe { VIRT_IRQ_CHIP.masked_irqs &= !(1 << irq); }
    }
}

// --- Inicialización de Estructuras de Control ---

#[no_mangle]
pub extern "C" fn rust_init_vmcs(ptr: *mut VmControlStructure, rev_id: u32) {
    if ptr.is_null() { return; }
    unsafe {
        // Limpiamos la estructura
        core::ptr::write_bytes(ptr as *mut u8, 0, 4096);
        // En RISC-V el revision_id no se usa igual, pero lo dejamos por compatibilidad
        (*ptr).revision_id = rev_id;
    }
}

// --- Región de Memoria Estática ---

static mut VMXON_REGION: VmxMemory = VmxMemory { data: [0; 4096] };

#[no_mangle]
pub extern "C" fn rust_allocate_vmx_region() -> *mut u8 {
    unsafe {
        VMXON_REGION.data.as_mut_ptr()
    }
}
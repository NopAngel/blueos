#ifndef INITCALL_H
#define INITCALL_H

typedef int (*initcall_t)(void);

#define __define_initcall(fn, id) \
    static initcall_t __initcall_##fn##id __attribute__((__used__, \
    __section__(".initcall" #id ".init"))) = fn

#define pure_initcall(fn)      __define_initcall(fn, 0) // simples and CPU
#define core_initcall(fn)      __define_initcall(fn, 1) // memory
#define arch_initcall(fn)      __define_initcall(fn, 2) // arch (IRQ/PIC)
#define fs_initcall(fn)        __define_initcall(fn, 3) // VFS and fs
#define device_initcall(fn)    __define_initcall(fn, 4) // Drivers (keyboard,rtc, etc)
#define late_initcall(fn)      __define_initcall(fn, 5) // shell and apps

#endif
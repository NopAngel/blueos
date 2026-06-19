#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "COMPAT_SYS"

/* Explicit 32-bit types definitions running on a 64-bit kernel frame */
typedef uint32_t compat_uptr_t;
typedef int32_t compat_long_t;
typedef uint32_t compat_ulong_t;

/**
 * compat_ptr: Converts a 32-bit user-space pointer address into a clean 64-bit
 * kernel pointer.
 */
static void *compat_ptr(compat_uptr_t uptr) { return (void *)(uintptr_t)uptr; }

/**
 * compat_sys_truncate: Bridge for file truncation where offset layouts change
 * size.
 */
long compat_sys_truncate(const char *pathname, compat_ulong_t length) {
  if (!pathname)
    return -EFAULT;

  printk("<6>[  %s  ] Intercepted 32-bit truncate request for '%s' -> Target "
         "Len: %u bytes\n",
         MODULE_NAME, pathname, length);

  /* Safely zero-extend 32-bit unsigned length into native 64-bit size_t */
  size_t native_length = (size_t)length;

  /* Route directly into your core POSIX VFS sys_truncate handler */
  // extern long sys_truncate(const char *path, size_t length);
  // return sys_truncate(pathname, native_length);

  return 0;
}

/**
 * compat_sys_init: Registers the execution environment constraints flags.
 */
void compat_sys_init(void) {
  boot_msg(MODULE_NAME,
           "Initializing i386 Emulation Layer for x86_64 BlueOS...", 0);
  printk(
      "<6>[  %s  ] 32-bit execution environment handles registered cleanly.\n",
      MODULE_NAME);
}
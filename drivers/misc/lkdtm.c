#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* LKDTM Panic and Corruption Injection Codes for BlueOS */
#define LKDTM_PANIC_EXPLICIT 1
#define LKDTM_NULL_POINTER 2
#define LKDTM_BUFFER_OVERFLOW 3
#define LKDTM_DIV_BY_ZERO 4
#define LKDTM_STACK_CORRUPTION 5

#define MODULE_NAME "LKDTM"

/* External reference to your native kernel panic engine */
extern void k_panic(int code, const char *reason);

/**
 * lkdtm_execute_crash: Master trigger for simulated crashes and memory
 * corruptions. Dispatches targeted destructive routines based on the crash type
 * identifier.
 */
void lkdtm_execute_crash(int crash_type) {
  printk("<6>[  LKDTM  ] Initiating fault injection type: %d\n", crash_type);

  switch (crash_type) {
  case LKDTM_PANIC_EXPLICIT:
    boot_msg(MODULE_NAME, "Triggering explicit kernel k_panic...", 2);
    /* Invokes your custom native panic handler */
    k_panic(0x1990, "LKDTM explicit hardware testing panic");
    break;

  case LKDTM_NULL_POINTER:
    boot_msg(MODULE_NAME, "Forcing dangerous NULL pointer dereference...", 1);
    volatile uint32_t *null_ptr = (volatile uint32_t *)NULL;
    /* This will trap into your VMM / MMU Page Fault handler */
    *null_ptr = 0xDEADBEEF;
    break;

  case LKDTM_BUFFER_OVERFLOW: {
    boot_msg(MODULE_NAME, "Executing controlled memory buffer overflow...", 1);
    char target_buffer[8] = "BlueOS";
    volatile char *corrupt_ptr = target_buffer;

    /* Purposely overrunning the assigned stack boundary */
    for (int i = 0; i < 64; i++) {
      corrupt_ptr[i] = 'A';
    }
    printk(
        "<3>[  LKDTM  ] Buffer overrun completed. Memory state unpredicted.\n");
    break;
  }

  case LKDTM_DIV_BY_ZERO: {
    boot_msg(MODULE_NAME,
             "Forcing illegal arithmetic exception (Divide by Zero)...", 1);
    volatile int divisor = 0;
    volatile int result = 100 / divisor;
    /* Prevents the compiler from optimizing out the variable statement */
    (void)result;
    break;
  }

  case LKDTM_STACK_CORRUPTION:
    boot_msg(MODULE_NAME, "Corrupting the active execution stack frame...", 2);
    /* Modifying stack variables to sabotage return addresses */
    volatile uint8_t stack_var;
    uint8_t *stack_probe = (uint8_t *)&stack_var;
    for (int i = 0; i < 32; i++) {
      *(stack_probe + i) = 0xFF;
    }
    k_panic(0x5555, "LKDTM Stack frame smashed maliciously");
    break;

  default:
    printk("<4>[  LKDTM  ] Unknown crash signature ID (%d). Aborting fault "
           "injection.\n",
           crash_type);
    break;
  }
}

/**
 * lkdtm_handler_command: Pseudo-debugfs command interface mapped to your qsh.c
 * terminal
 */
void lkdtm_handler_command(const char *cmd) {
  if (!cmd)
    return;

  if (vsprintf(NULL, cmd, NULL) == 0)
    return; // Basic input safety guard check

  printk("<5>[  LKDTM  ] Input token processed in virtual diagnostic "
         "subsystem.\n");

  if (cmd[0] == '1')
    lkdtm_execute_crash(LKDTM_PANIC_EXPLICIT);
  else if (cmd[0] == '2')
    lkdtm_execute_crash(LKDTM_NULL_POINTER);
  else if (cmd[0] == '3')
    lkdtm_execute_crash(LKDTM_BUFFER_OVERFLOW);
  else if (cmd[0] == '4')
    lkdtm_execute_crash(LKDTM_DIV_BY_ZERO);
  else if (cmd[0] == '5')
    lkdtm_execute_crash(LKDTM_STACK_CORRUPTION);
  else
    printk("<4>[  LKDTM  ] Invalid stress payload argument.\n");
}
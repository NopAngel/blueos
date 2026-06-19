#include <stdbool.h>
#include <stdint.h>

static bool hyper_is_qemu = false;
static bool hyper_is_guest = false;
static const char *hyper_name = "Bare Metal";

bool x86_is_guest(void) { return hyper_is_qemu; }

const char *x86_hyper_name(void) {
  return hyper_is_qemu ? "QEMU/KVM" : "Bare Metal";
}

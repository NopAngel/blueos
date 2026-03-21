#
# BlueOS Main Wrapper Makefile
#

-include .config

# --- Magic Export ---
# Export ALL variables from .config to sub-makefiles
export $(shell sed 's/=.*//' .config 2>/dev/null)

# --- Architecture Setup ---
ARCH ?= i386
export ARCH

# --- Verbosity & Jobs ---
Q ?= @
MAKE += --no-print-directory
JOBS := $(shell nproc 2>/dev/null || echo 1)

# --- Global Compiler Flags ---
# Convert CONFIG_...=y into -DCONFIG_... for C code
CONF_FLAGS += $(patsubst CONFIG_%, -DCONFIG_%, $(filter CONFIG_%, $(.VARIABLES)))
export CONF_FLAGS

.PHONY: all compile run iso menuconfig clean help prepare

all: compile

# Step 0: Prepare generated headers (Like Linux does)
prepare:
	@echo "  CHK     include/generated/utsrelease.h"
	@mkdir -p include/generated
	@echo "#define BLUEOS_VERSION \"0.1.0-alpha\"" > include/generated/utsrelease.h
	@echo "#define BLUEOS_COMPILE_BY \"$(USER)\"" >> include/generated/utsrelease.h
	@echo "#define BLUEOS_COMPILE_HOST \"$(shell hostname)\"" >> include/generated/utsrelease.h

compile: prepare
	@echo "  BUILD   Arch: $(ARCH) (Jobs: $(JOBS))"
	$(Q)$(MAKE) -j$(JOBS) -f Makefile.$(ARCH)

run: prepare
	$(Q)$(MAKE) -f Makefile.$(ARCH) run

menuconfig:
	$(Q)$(MAKE) -f scripts/Makefile all

clean:
	@echo "  CLEAN   Build artifacts"
	$(Q)rm -rf obj/ build/ include/generated/
	$(Q)if [ -d "rust" ]; then cd rust && cargo clean; fi
	$(Q)find . -type f \( -name "*.o" -o -name "*.a" -o -name "*.elf" \) -delete

help:
	@echo "BlueOS Build System"
	@echo "  make [ARCH=riscv|i386]  - Build kernel"
	@echo "  make run                - Boot in QEMU"
	@echo "  make clean              - Remove all objects"
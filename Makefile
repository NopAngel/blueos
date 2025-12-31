CC = @gcc
ASM = @nasm
L = @ld
QEMU = qemu-system-i386





KBIN = kernel
CFLAG = -m32 -fno-stack-protector -c -std=c99 -nostdlib -I. -Iinclude
LDFLAG = -m elf_i386 -T link.ld -o $(KBIN)
LDPATH = kasm.o printk.o mm.o bg.o fs.o panic.o init.o gui-k.o utils.o speaker.o string.o keyboard.o vfs.o bls_snd.o
QEMUFLAG =



SHOW_LOG = && echo [CC] Compiling
SHOW_LOG_LD = && echo [LD] Linking
SHOW_LOG_S = && echo [S] Compiling


all:
	@clear
	$(ASM) -f elf32 kernel.asm -o kasm.o $(SHOW_LOG_S) kasm.s
	$(ASM) -f elf32 sound/blaster/blaster.s -o bls_snd.o $(SHOW_LOG_S) bls_snd.s
	@echo "[CC] Compiling C"
	$(CC) $(CFLAG) init/kernel.c -o kernel.o $(SHOW_LOG) kernel.o
	$(CC) $(CFLAG) init/init_fnc.c -o init.o $(SHOW_LOG) init.o
	$(CC) $(CFLAG) arch/panic.c -o panic.o $(SHOW_LOG) panic.o
	$(CC) $(CFLAG) arch/mm/memory.c -o mm.o $(SHOW_LOG) mm.o
	$(CC) $(CFLAG) arch/printk.c -o printk.o $(SHOW_LOG) printk.o
	$(CC) $(CFLAG) arch/bg.c -o bg.o $(SHOW_LOG) bg.o
	$(CC) $(CFLAG) sound/core/pcspeaker.c -o speaker.o $(SHOW_LOG) speaker.o
	$(CC) $(CFLAG) drivers/keyboard/keyboard.c -o keyboard.o $(SHOW_LOG) keyboard.o
	$(CC) $(CFLAG) fs/fs.c -o fs.o $(SHOW_LOG) fs.o
	$(CC) $(CFLAG) arch/string/string.c -o string.o $(SHOW_LOG) string.o
	$(CC) $(CFLAG) fs/vfs.c -o vfs.o $(SHOW_LOG) vfs.o

	$(CC) $(CFLAG) gui/dosbox/main.c -o gui-k.o $(SHOW_LOG) gui-k.o
	$(CC) $(CFLAG) gui/dosbox/utils.c -o utils.o $(SHOW_LOG) utils.o



	@echo "[LD] Linking C"
	$(L) $(LDFLAG) $(LDPATH) kernel.o $(SHOW_LOG_LD) $(LDPATH)


gui: all
	$(CC) $(CFLAG) gui/vga.c -o vga.o $(SHOW_LOG) vga.o
	$(CC) $(CFLAG) arch/os/os.c -o os.o $(SHOW_LOG) os.o
	$(CC) $(CFLAG) gui/bitmap.c -o bitmap.o $(SHOW_LOG) bitmap.o
	$(L) $(LDFLAG) $(LDPATH) vga.o bitmap.o os.o $(SHOW_LOG_LD) $(LDPATH)
	@make debug

kernel: all debug


compile_drivers:
	@echo [MAKE] Compile DRIVERS.
	$(CC) $(CFLAG) init/wrapper.c -o wrapper.o $(SHOW_LOG) wrapper.o
	$(CC) $(CFLAG) drivers/nvidia/core.c -o nvcore.o $(SHOW_LOG) nvcore.o
	$(CC) $(CFLAG) arch/timer.c -o timer.o $(SHOW_LOG) timer.o
	@echo [MAKE] Success
	$(L) $(LDFLAG) $(LDPATH) wrapper.o nvcore.o timer.o






log:
	@echo [MAKE] SHOW LOGS *FILE COMPILES*
	@ls *.o






debug:
	@echo [DEBUG] Debug Blue-Kernel *WITH QEMU*
	@$(QEMU) -kernel $(QEMUFLAG) $(KBIN)



clean:
	@echo [CLEAN] Clean Project..
	@rm -rf *.o kernel
	@echo [CLEAN] Success

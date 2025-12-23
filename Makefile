C = gcc
ASM = nasm

KBIN = kernel
CFLAG = -m32 -fno-stack-protector -c -std=c99 

# ld
LDFLAG = -m elf_i386 -T link.ld -o $(KBIN)
LDPATH = kasm.o kernel.o printk.o mm.o bg.o fs.o panic.o speaker.o string.o nvcore.o timer.o keyboard.o vfs.o bls_snd.o
QEMU = qemu-system-i386
QEMUFLAG =
all:
	$(ASM) -f elf32 kernel.asm -o kasm.o
	$(ASM) -f elf32 sound/blaster/blaster.s -o bls_snd.o
	@echo "[C] Compiling and linking C"
	$(CC) $(CFLAG) arch/kernel.c -o kernel.o
	$(CC) $(CFLAG) arch/panic.c -o panic.o
	$(CC) $(CFLAG) arch/mm/memory.c -o mm.o
	$(CC) $(CFLAG) arch/printk.c -o printk.o
	$(CC) $(CFLAG) arch/bg.c -o bg.o
	$(CC) $(CFLAG) sound/core/pcspeaker.c -o speaker.o
	$(CC) $(CFLAG) drivers/keyboard/keyboard.c -o keyboard.o
	$(CC) $(CFLAG) fs/fs.c -o fs.o
	$(CC) $(CFLAG) arch/string/string.c -o string.o
	$(CC) $(CFLAG) fs/vfs.c -o vfs.o
	# $(CC) $(CFLAG) init/wrapper.c -o wrapper.o
	#$(CC) $(CFLAG) drivers/nvidia/core.c -o nvcore.o
	#$(CC) $(CFLAG) arch/timer.c -o timer.o
	ld $(LDFLAG) $(LDPATH)

debug:
	$(QEMU) -kernel $(QEMUFLAG) $(KBIN)

clean:
	rm -rf *.o kernel 

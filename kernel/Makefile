
include Makefile.cfg

OBJ = boot.o kernel.o
ALL_OBJ = boot.o kernel.o io.o serial.o debug.o string.o kmem.o gdt.o idt.o \
		  cpu.o isr.o irq.o vfs.o initramfs.o process.o syscall.o scheduler.o \
		  tty.o kbd.o
OUT = obj

ASFLAGS = -f elf64
CFLAGS = -c -I include/ -mcmodel=large -nostdlib -ffreestanding
LDFLAGS = -T ../link.ld -mcmodel=large -nostdlib -ffreestanding \
		  -z max-page-size=0x1000

kernel.elf: $(OBJ)
	cp $(OBJ) $(OUT)
	cd misc; make
	cd mem; make
	cd cpu; make
	cd fs; make
	cd sys; make
	cd devices; make
	cd obj; $(CC) $(LDFLAGS) $(ALL_OBJ) -o ../$@
	
%.o : %.asm
	$(AS) $(ASFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	cd misc; make clean
	cd mem; make clean
	cd cpu; make clean
	cd fs; make clean
	cd sys; make clean
	cd devices; make clean
	rm -f obj/*.o
	rm -f *.o kernel.elf


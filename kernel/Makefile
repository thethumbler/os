
include Makefile.cfg

OBJ = boot.o kernel.o
ALL_OBJ = boot.o kernel.o io.o serial.o debug.o string.o kmem.o gdt.o idt.o \
		  cpu.o isr.o irq.o vfs.o initramfs.o process.o syscall.o scheduler.o \
		  signal.o fpu.o pit.o ramdev.o ttydev.o ext2.o\
		  devfs.o console.o kbd.o ata.o devman.o fsman.o

OUT = obj

ASFLAGS = -f elf64
CFLAGS = -c -I include/ -mcmodel=large -nostdlib -ffreestanding
LDFLAGS = -T ../link.ld -mcmodel=large -nostdlib -ffreestanding \
		  -z max-page-size=0x1000
CPPFLAGS = -I include/ -P
NODIR = --no-print-directory

kernel.elf: $(OBJ)
	${CP} $(OBJ) $(OUT) 
	
	@cd misc; make $(NODIR)
	@cd mem; make $(NODIR)
	@cd cpu; make $(NODIR)
	@cd fs; make $(NODIR)
	@cd sys; make $(NODIR)
	@cd devices; make $(NODIR)
	@cd obj; $(LD) $(LDFLAGS) $(ALL_OBJ) -o ../$@
	
%.o : %.asm
	$(CPP) $(CPPFLAGS) $< -o $<"_tmp__"
	$(AS) $(ASFLAGS) -o $@ $<"_tmp__"
	@rm $<"_tmp__"

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	@echo -n -e "\033[0;31m"; \
	cd misc; make clean $(NODIR)
	
	@echo -n -e "\033[0;31m"; \
	cd mem; make clean $(NODIR)
	
	@echo -n -e "\033[0;31m"; \
	cd cpu; make clean $(NODIR)
	
	@echo -n -e "\033[0;31m"; \
	cd fs; make clean $(NODIR)
	
	@echo -n -e "\033[0;31m"; \
	cd sys; make clean $(NODIR)
	
	@echo -n -e "\033[0;31m"; \
	cd devices; make clean $(NODIR)
	
	@rm -f obj/*.o
	@rm -f *.o kernel.elf


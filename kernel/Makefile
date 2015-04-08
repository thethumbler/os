
include Makefile.cfg

OBJ = boot.o kernel.o
ALL_OBJ = boot.o kernel.o io.o serial.o debug.o mem.o
OUT = obj

ASFLAGS = -f elf64
CFLAGS = -c -I include/ -mcmodel=large -nostdlib -ffreestanding
LDFLAGS = -T ../link.ld -mcmodel=large -nostdlib -ffreestanding \
		  -z max-page-size=0x1000

kernel.elf: $(OBJ)
	cp $(OBJ) $(OUT)
	cd misc; make
	cd mem; make
	cd obj; $(CC) $(LDFLAGS) $(ALL_OBJ) -o ../$@
	
%.o : %.asm
	$(AS) $(ASFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	cd misc; make clean
	cd mem; make clean
	rm -f obj/*.o
	rm -f *.o kernel.elf


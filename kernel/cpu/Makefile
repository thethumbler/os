
include ../Makefile.cfg

OBJ = idt.o cpu.o
OUT=../obj

ASFLAGS = -f elf64
CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	cp $(OBJ) $(OUT) 
		
%.o : %.asm
	$(AS) $(ASFLAGS) $< -o $@

$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -f $(OBJ)


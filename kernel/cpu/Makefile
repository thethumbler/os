include ../Makefile.cfg

OBJ = gdt.o idt.o cpu.o isr.o irq.o
OUT=../obj

ASFLAGS = -f elf64
CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	${CP} $(OBJ) $(OUT) 

%.o : %.asm
	@$(AS) $(ASFLAGS) $< -o $@

#$(OUT)/%.o: %.c
#	@$(CC) $(CFLAGS) $< -o $@ 
	
clean:
	${RM} -f $(OBJ)


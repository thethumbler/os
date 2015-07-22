
include ../Makefile.cfg

OBJ = fpu.o pit.o ramdev.o ttydev.o console.o kbd.o ata.o devman.o
OUT=../obj

CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	${CP} $(OBJ) $(OUT) 
	
$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	${RM} -f $(OBJ)

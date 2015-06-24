
include ../Makefile.cfg

OBJ = vfs.o initramfs.o ext2.o
OUT=../obj

CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	${CP} $(OBJ) $(OUT) 
	
$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	${RM} -f $(OBJ)


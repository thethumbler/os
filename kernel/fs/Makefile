
include ../Makefile.cfg

OBJ = vfs.o initramfs.o
OUT=../obj

CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	cp $(OBJ) $(OUT) 
	
$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -f $(OBJ)


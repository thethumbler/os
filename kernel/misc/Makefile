
include ../Makefile.cfg

OBJ = io.o serial.o debug.o string.o
OUT=../obj

CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	${CP} $(OBJ) $(OUT) 
	
$(OUT)/%.o: %.c
	${CC} $(CFLAGS) $< -o $@
	
clean:
	${RM} -f $(OBJ)

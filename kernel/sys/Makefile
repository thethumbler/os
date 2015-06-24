
include ../Makefile.cfg

OBJ = process.o syscall.o scheduler.o signal.o
OUT=../obj

CFLAGS = -c -I ../include/ -mcmodel=large -nostdlib -ffreestanding

all: $(OBJ)
	${CP} $(OBJ) $(OUT) 
	
$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	${RM} -f $(OBJ)


include ../kernel/Makefile.cfg

SRC = src
OBJ = system.o stdio.o stdlib.o unistd.o fcntl.o signal.o

libc.a: $(OBJ)
	${AR} rcs libc.a obj/*.o

%.o : $(SRC)/%.c
	${CC} -mcmodel=large -c -fno-builtin -ffreestanding -I include/ $<
	@mkdir -p obj/
	@mv $@ obj/

clean:
	${RM} -f $(OBJ)

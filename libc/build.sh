x86_64-elf-gcc -mcmodel=large -c -I include/ src/*.c
mv *.o obj/
x86_64-elf-ar rcs libc.a obj/*.o

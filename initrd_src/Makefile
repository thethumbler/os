include ../kernel/Makefile.cfg

APPS = 	init login shell hw echo clear title syntax sayhi cat mbr ls sig \
		bf malloctest ext2chk mount

all: $(APPS)
	@echo -n -e "\033[1;33mCreating initrd : \033[1;31m"
	@cd ../initrd; find . | cpio -H bin -o > ../iso/initrd
	@echo -n -e "\033[0m"
	
% : %.c
	@echo -n -e "\033[1;33mCC " $< "\033[1;31m"
	@~/myos/bin/x86_64-elektros-gcc $< -o ../initrd/bin/$@ -std=c99
	@echo -e "\033[0m"

#~/myos/bin/tcc $< ../mymalloc.o -o ../initrd/$@
#strip ../initrd/$@	
#$(CC) $(CFLAGS) -z max-page-size=0x1000 -nostdlib -Wl,-Ttext,0x0 \
#-fno-builtin -ffreestanding -w $< ~/myos/usr/lib/crt1.o ../mymalloc.o \
#~/myos/usr/lib/libc.a -o ../initrd/$@ -I ~/myos/usr/include \

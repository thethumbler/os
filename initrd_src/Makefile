include ../kernel/Makefile.cfg

APPS = init shell hw ext shell2 sig bf

all: $(APPS)
	@echo -n -e "\033[1;33mCreating initrd : \033[1;31m"
	@cd ../initrd; find . | cpio -H bin -o > ../iso/initrd
	@echo -n -e "\033[0m"
	
% : %.c
	$(CC) $(CFLAGS) -z max-page-size=0x1000 -nostdlib -Wl,-Ttext,0x0 \
	-fno-builtin -ffreestanding -w $< ../libc/libc.a -o ../initrd/$@ -I ../libc/include \
	#-mno-sse -mno-sse2 


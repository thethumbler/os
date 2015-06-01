include ../kernel/Makefile.cfg

APPS = init shell hw shell2 ext hwl

all: $(APPS)
	cd ../initrd; find . | cpio -H bin -o > ../iso/initrd
	
% : %.c
	$(CC) $(CFLAGS) -z max-page-size=0x1000 -nostdlib -Wl,-Ttext,0x0 \
	$< -o ../initrd/$@ ../libc/libc.a -I ../libc/include


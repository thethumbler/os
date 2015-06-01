cd: cd.iso

cd.iso: kernel/kernel.elf libc/libc.a iso/initrd hd.img
	cp kernel/kernel.elf iso/
	grub2-mkrescue iso/ -o cd.iso
	
kernel/kernel.elf:
	cd kernel; make
	
iso/initrd:
	cd initrd_src; make

libc/libc.a:
	cd libc; sh build.sh
	
hd.img:
	#NOTE : hd.img is created only once! you have to update it manually
	touch hd.img
	dd if=/dev/zero of=hd.img bs=1MiB count=64
	mkfs.ext2 hd.img
	mkdir tmp
	sudo mount hd.img tmp
	sudo chmod 777 tmp/.
	cp hd/* tmp/ -rf
	sudo umount tmp
	rm tmp -rf
	
clean:
	cd kernel; make clean
	rm -f cd.iso
	rm -f iso/kernel.elf
	rm -f iso/initrd
	
try: cd.iso
	qemu-system-x86_64 -cdrom cd.iso -m 1G -serial stdio -hda hd.img #-enable-kvm

cd: cd.iso

cd.iso: kernel/kernel.elf
	cp kernel/kernel.elf iso/
	grub2-mkrescue iso/ -o cd.iso
	
kernel/kernel.elf:
	cd kernel; make

clean:
	cd kernel; make clean
	rm -f cd.iso
	rm -f iso/kernel.elf
try: cd.iso
	qemu-system-x86_64 cd.iso -m 256M -serial stdio

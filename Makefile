cd: cd.iso

NODIR = --no-print-directory

cd.iso: kernel/kernel.elf libc/libc.a iso/initrd hd.img
	@cp kernel/kernel.elf iso/
	@grub2-mkrescue iso/ -o cd.iso &>/dev/null
	@echo -e "\033[0;32mcd.iso is ready\033[0m"
	
kernel/kernel.elf:
	@cd kernel; make $(NODIR)
	
iso/initrd:
	@rm -f -r initrd/dev
	@mkdir initrd/dev
	@cd initrd_src; make $(NODIR)

libc/libc.a:
	@cd libc; make $(NODIR)
	
hd.img:
	@echo -e "\033[0;34mCreating hd.img\n\033[0;31m"\
	"NOTE : hd.img is created only once, you need to update it manualy\033[0m"
	@touch hd.img
	@dd if=/dev/zero of=hd.img bs=1MiB count=64 &>/dev/null
	@mkfs.ext2 hd.img &>/dev/null
	@rm -f -r tmp
	@mkdir tmp
	@sudo mount hd.img tmp
	@sudo chmod 777 tmp/.
	@cp hd/* tmp/ -rf
	@sudo umount tmp
	@rm tmp -rf
	
clean:
	@cd kernel; make clean --no-print-directory
	@rm -f cd.iso
	@rm -f iso/kernel.elf
	@rm -f iso/initrd
	
try: cd.iso
	qemu-system-x86_64 -cdrom cd.iso -m 1G -serial stdio -hda hd.img #-enable-kvm

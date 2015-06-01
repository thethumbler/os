#include <system.h>
#include <multiboot.h>
#include <io.h>
#include <serial.h>
#include <debug.h>
#include <kmem.h>
#include <vfs.h>
#include <initramfs.h>
#include <elf.h>
#include <process.h>
#include <tty.h>
#include <kbd.h>
#include <vbe.h>
#include <ext2.h>
#include <ata.h>
#include <string.h>

void map_mem(multiboot_info_t*);

uint8_t *vga = 0x8000000000;

void init_video()
{
	ModeInfoBlock *vbe = mboot_info->vbe_mode_info;
	debug("Resolution %dx%d\n", vbe->XResolution, vbe->YResolution);
	debug("VBE 0x%x\n", vbe->PhysBasePtr);
	mman.set_unusable(vbe->PhysBasePtr, 2*1024*1024);
	uint64_t *vga_pdpt = mman.get_frame();
	uint64_t *vga_pd   = mman.get_frame();
	uint64_t *vga_pt   = mman.get_frame();
	
	uint64_t *i = vbe->PhysBasePtr - 512, k = 0;
	while( k < 512 )	// 2 MiB
		vga_pt[k++] = (uint64_t)(i += 512) | 3;
	*vga_pd = (uint64_t)vga_pt | 3;
	*vga_pdpt = (uint64_t)vga_pd | 3;
	*(PML4 + 1) = (uint64_t)vga_pdpt | 3;
}

void kmain(void)
{
	serial.init();
	//serial.write_str("Hello, World\n");
	kernel_end = (uint64_t)heap_addr; //(uint64_t)&kend/0x1000*0x1000 + ((uint64_t)&kend%0x1000?0x1000:0x0);
	kernel_heap_ptr = (uint8_t*)((uint64_t)&VMA + kernel_end + heap_size) ;
	UPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x3000);
	KPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x4000);
	PML4 = (uint64_t*)((uint64_t)&VMA + kernel_end);
	map_mem(mboot_info);
	//init_video();
	vmem_init();
	idt_install();
	isr_install();

	extern void load_tss(void);
	load_tss();
	extern uint64_t k_tss64_sp;
	*(uint64_t*)( (uint64_t)&VMA + (uint64_t)&k_tss64_sp ) = 0xFFFFFFFFC0008000;
	//asm("movw %%ax, 0x28; ltr %%ax":::"ax" );
	//for(;;);
	outb(0x43, 0x36);
	uint32_t div = 1193180/50;	// 50 Hz
	outb(0x40, div & 0xFF);
	outb(0x40, (div >> 8) & 0xFF);
	extern void timer();
	extern void schedule();
	irq_install_handler(0, schedule);
	irq_install();

	//asm( "movw %%ax, 0x28; ltr %%ax":::"ax" );
	//for(;;);
 	void *ramdisk = 
 		((multiboot_module_t*)mboot_info->mods_addr)->mod_start;

	inode_t *rootfs = initramfs.load(ramdisk);

	vfs_mount_root(rootfs);

	// Create /dev/ttym	[Master TTY driver]
	devtty.load(0xFFFFFFFFC00B8000);
	// Create /dev/tty0 [Slave TTY driver]
	devtty.load(kmalloc(0x1000));
	devtty.load(kmalloc(0x1000));
	// Open /dev/tty0 as a file
	inode_t *f = vfs_trace_path(vfs_root, "/dev/tty0");
	// Write to /dev/tty0
	vfs_write(f, (void*)"tty0\n", strlen("tty0\n"));
	vfs_write(vfs_trace_path(vfs_root, "/dev/tty1"), (void*)"tty1\n", strlen("tty1\n"));
	//tty_switch(1);
	//vfs_write(vfs_trace_path(vfs_root, "/dev/ttym"), 0, 0);
	
	//for(;;);
	
	irq_install_handler(1, kbd_handler);

	process_t *init = load_elf("/init");
	
	extern inode_t *ext2_load(void *_);
	ext2_load(&atadev);
	
	
	
	//assert(1, 0, "Critical");
	
	extern void spawn_init(process_t*);
	spawn_init(init);
	
	for(;;);
}

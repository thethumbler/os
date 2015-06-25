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
#include <signal.h>
#include <pit.h>

void map_mem(multiboot_info_t*);

uint8_t *vga = (uint8_t*)0xA0000000000;
void init_video()
{
	VbeInfoBlock *vbe_info = (VbeInfoBlock*)(uint64_t)mboot_info->vbe_control_info;
	ModeInfoBlock *vbe = (ModeInfoBlock*)(uint64_t)(mboot_info->vbe_mode_info);
	debug("Resolution %dx%d\n", vbe->XResolution, vbe->YResolution);
	debug("VBE V:%d.%d #0x%x [%d]\n", vbe_info->VbeVersion >> 2*4, 
		vbe_info->VbeVersion & 0xF , vbe->PhysBasePtr, vbe->LinBytesPerScanLine);
	
	mman.set_unusable(vbe->PhysBasePtr, 2*1024*1024);
	uint64_t *vga_pdpt = (uint64_t*)mman.get_frame();
	uint64_t *vga_pd   = (uint64_t*)mman.get_frame();
	uint64_t *vga_pt   = (uint64_t*)mman.get_frame();
	
	uint64_t i = vbe->PhysBasePtr - 4*1024, k = 0;
	while( k < 512 )	// 2 MiB
		vga_pt[k++] = (uint64_t)(i += 4*1024) | 3;
	*vga_pd = (uint64_t)vga_pt | 3;
	*vga_pdpt = (uint64_t)vga_pd | 3;
	*(PML4 + 20) = (uint64_t)vga_pdpt | 3;	// 0xA0000000000
	
	TLB_flush();
}

void kmain(void)
{
	serial.init();
	kernel_end = (uint64_t)heap_addr;
	kernel_heap_ptr = (uint8_t*)((uint64_t)&VMA + kernel_end + heap_size) ;
	UPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x3000);
	KPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x4000);
	PML4 = (uint64_t*)((uint64_t)&VMA + kernel_end);
	map_mem(mboot_info);

#if _GFX_
	init_video();
#endif
	vmem_init();
	
	idt_install();
	isr_install();
	
	extern void load_tss(void);
	load_tss();
	extern uint64_t k_tss64_sp;
	*(uint64_t*)( (uint64_t)&VMA + (uint64_t)&k_tss64_sp ) = 0xFFFFFFFFC0008000;

	pit_set_freq(2000);	// timer tick = 500 us
	pit_install();
	irq_install();
	
	extern void mouse_init();
	//mouse_init();
	extern void mouse_handler(void*);
	//irq_install_handler(12, mouse_handler);

	debug("Loading Ramdisk\n");
 	void *ramdisk = 
 		(void*)(uint64_t)(((multiboot_module_t*)(uint64_t)mboot_info->mods_addr)->mod_start);

	inode_t *rootfs = initramfs.load(ramdisk);

	vfs_mount_root(rootfs);
	
	// Create /dev/ttym	[Master TTY driver]
	devtty.load((void*)0xFFFFFFFFC00B8000);
	// Create /dev/tty0 [Slave TTY driver]
	devtty.load(kmalloc(0x1000));
	// Create /dev/tty1 [Slave TTY driver]
	devtty.load(kmalloc(0x1000));
	
	vfs_write(vfs_trace_path(vfs_root, "/dev/tty0"), (void*)"tty0\n", strlen("tty0\n"));
	vfs_write(vfs_trace_path(vfs_root, "/dev/tty1"), (void*)"tty1\n", strlen("tty1\n"));

	irq_install_handler(1, kbd_handler);


		
	extern inode_t *ext2_load(void *_);
	//ext2_load(&atadev);

	process_t *init = load_elf("/init");
	
	extern void spawn_init(process_t*);
	spawn_init(init);
	for(;;);
}

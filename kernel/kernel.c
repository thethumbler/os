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
#include <devfs.h>
#include <console.h>
#include <device.h>

void map_mem(multiboot_info_t*);

uint8_t *vga = (uint8_t*)0xA0000000000;
void init_video()
{/*
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
	
	TLB_flush();*/
}

void kmain(void)
{
	kernel_end = (uint64_t)heap_addr;
	kernel_heap_ptr = (uint8_t*)((uint64_t)&VMA + kernel_end + heap_size) ;
	UPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x3000);
	KPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x4000);
	PML4 = (uint64_t*)((uint64_t)&VMA + kernel_end);
	*(KPD + 511) = (kernel_end + 0x4000) | 3;
	map_mem(mboot_info);

#if _GFX_
	init_video();
#endif
	
	idt_install();
	isr_install();

	vmem_init();
	
	serial.init();

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

	multiboot_module_t *mod = (multiboot_module_t*)(uint64_t)mboot_info->mods_addr;

	ramdev_private_t ramdev_private = 
		(ramdev_private_t) 
		{ 
			.ptr  = (void*)((uint64_t)&VMA + mod->mod_start),
			.size = (uint32_t)(mod->mod_end - mod->mod_start),
		};
		
 	inode_t ramdisk_inode =
	 	(inode_t) 
 		{
 			.name 	= "ramdisk",
 			.type	= FS_CHRDEV,
 			.fs		= &devfs,
 			.dev	= &ramdev,
 			.p 		= &ramdev_private, 
 		};
 	
 	vfs_create(&dev_root, "/", &ramdisk_inode);

	inode_t *rootfs = initramfs.load(&ramdisk_inode);
	
	vfs_mount_root(rootfs);
	
	irq_install_handler(1, kbd_handler);
	
	devman.init();
	fsman.init();
	
#if _DBG_CON_
	// We should disable debugging by this stage!
	serial.end();
#endif

	process_t *init = load_elf("/bin/init");
		
	extern void spawn_init(process_t*);
	spawn_init(init);
	for(;;);
}

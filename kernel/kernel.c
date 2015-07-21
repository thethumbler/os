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
	serial.init();
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
	
	vfs_mount(vfs_root, "/dev/", &dev_root);
	
	inode_t console_inode = 
		(inode_t)
		{
			.name = "console",
			.type = FS_CHRDEV,
			.dev = &condev,
			.fs  = &devfs,
		};
		
	vfs_create(&dev_root, "/", &console_inode);
	
	tty_master_t ttym = 
		(tty_master_t)
		{
			.cur_tty	= 0,
			//.invoke		= ttym_invoke,
			.console	= &console_inode,
		};
	
	tty_device_t tty_dev = 
		(tty_device_t)
		{
			.id			= 0,
			.master		= &ttym,
			.pos		= 0,
			.virtcon	= get_virtcon(&tty_dev, 80, 25),
			.p			= NULL,
			.buf		= NULL,
			.len		= 0,
		};
	
	inode_t tty_inode = 
		(inode_t)
		{
			.name	= "0",
			.type	= FS_CHRDEV,
			.fs		= &devfs,
			.dev	= &ttydev,
			.p		= &tty_dev,
		};
	
	inode_t tty_dir = 
		(inode_t)
		{
			.name	= "tty",
			.type	= FS_DIR,
			//.fs	= &devfs,
			//.dev	= &ramdev,
		};
		
	vfs_create(&dev_root, "/", &tty_dir);
	vfs_create(&tty_dir, "/", &tty_inode);
	vfs_mount(vfs_root, "/dev/", &dev_root);
	
	virtcon_device_t *virtcon = tty_dev.virtcon;
	
	inode_t *tty0 = vfs_trace_path(vfs_root, "/dev/tty/0");

	char *msg = "tty0\n";
	vfs_write(tty0, 0, strlen(msg), msg);

	irq_install_handler(1, kbd_handler);

	extern inode_t *cur_tty;
	cur_tty = tty0;
	/*
	inode_t sda_inode =
	 	(inode_t) 
 		{
 			.name 	= "sda",
 			.type	= FS_BLKDEV,
 			.fs		= &devfs,
 			.dev	= &atadev,
 			//.p 		= &atadev_private, 
 		};

	vfs_create(&dev_root, "/", &sda_inode);*/
	//vfs_tree(vfs_root);

	//for(;;);
	
	//extern inode_t *ext2_load(void *_);
	//ext2_load(&atadev);

	process_t *init = load_elf("/bin/init");
		
	extern void spawn_init(process_t*);
	spawn_init(init);
	for(;;);
}

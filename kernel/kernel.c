#include <system.h>
#include <multiboot.h>
#include <io.h>
#include <serial.h>
#include <debug.h>
#include <kmem.h>


extern multiboot_info_t *mboot_info;
extern uint32_t kernel_size;
extern uint32_t heap_addr;
extern uint32_t heap_size;
extern uint32_t kernel_heap_size;
extern uint32_t VMA;
//extern uint32_t kend;
uint64_t kernel_end;
//uint8_t *kernel_heap_ptr;

void map_mem(multiboot_info_t*);

void kmain(void)
{
	serial.init();
	//serial.write_str("Hello, World");
	kernel_end = (uint64_t)heap_addr; //(uint64_t)&kend/0x1000*0x1000 + ((uint64_t)&kend%0x1000?0x1000:0x0);
	kernel_heap_ptr = (uint8_t*)((uint64_t)&VMA + kernel_end + heap_size) ;
	KPD = (uint64_t*)((uint64_t)&VMA + kernel_end + 0x4000);
	map_mem(mboot_info);
	vmem_init();
	idt_install();
	isr_install();
	
	
	
	extern void timer();
	//irq_install_handler(0, timer);
	irq_install();
	//asm("sti");
	
	*(char*)(0xB8002) = 'K';
	for(;;);
}

void map_mem(multiboot_info_t *mboot)
{
	debug("Kernel heap starts at 0x%lx\n", kernel_heap_ptr);
	uint32_t total = mboot->mem_lower + mboot->mem_upper;
	debug("Total usable memory %d KiB\n", total);
	multiboot_memory_map_t 
	*mmap = (multiboot_memory_map_t*)(uint64_t)mboot->mmap_addr;
	uint64_t max_mmap = mboot->mmap_addr + mboot->mmap_length;
	
	while( ((uint64_t)mmap) < max_mmap )
	{
		total_mem += mmap->len;
		if(mmap->type==1) total_usable_mem += mmap->len;
		mmap = (multiboot_memory_map_t*)(uint64_t)
				((uint64_t)mmap + mmap->size + sizeof(uint32_t));
	}
	
	debug("Total Mem = %lx\n", total_mem);
	mman.setup(total_mem);
	mmap = (multiboot_memory_map_t*)(uint64_t)mboot->mmap_addr;
	
	while( ((uint64_t)mmap + mmap->size + sizeof(uint32_t)) < max_mmap )
	{
		uint64_t len = mmap->len;
		debug("%lx => %lx : %s\n", mmap->addr, mmap->addr + mmap->len, 
				mmap->type==1?"usable":"unusable");
		if(mmap->type==1) mman.set_usable(mmap->addr, mmap->len);
		mmap = (multiboot_memory_map_t*)(uint64_t)
				((uint64_t)mmap + mmap->size + sizeof(uint32_t));
			
	}
	mman.set_unusable(0, kernel_end + heap_size + kernel_heap_size * 0x1000); 
}

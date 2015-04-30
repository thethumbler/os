#include <system.h>
#include <debug.h>
#include <kmem.h>
#include <multiboot.h>
#include <string.h>

/*
 *
 *	Physical memory manager
 *
 */
 
uint64_t total_mem;
uint64_t total_usable_mem;

uint8_t *bitmap;
uint64_t bitmap_frames_count;	// Must be multible of 8

void *heap_alloc(uint32_t size)
{
	//debug("Allocating %d from heap\n", size);
	void *ptr = kernel_heap_ptr;
	kernel_heap_ptr += size;
	//debug("Kenrel heap moved to 0x%lx\n", kernel_heap_ptr);
	return ptr;
}

static uint64_t bitmap_set(uint64_t addr)
{
	uint64_t ret_addr = addr;
	addr /= 0x1000;
	if( addr > bitmap_frames_count ) return 0;
	bitmap[addr/8] |= 1 << addr%8;
	return ret_addr;
}

static uint64_t bitmap_set_pages(uint64_t addr, uint32_t count)
{
	uint64_t ret_addr = addr;
	addr /= 0x1000;
	while(count--)
	{
		if( addr > bitmap_frames_count ) return 0;
		bitmap[addr/8] |= 1 << addr%8;
		++addr;
	}
	return ret_addr;
}

static uint64_t bitmap_clear(uint64_t addr)
{
	uint64_t ret_addr = addr;
	addr /= 0x1000;
	if( addr > bitmap_frames_count ) return 0;
	bitmap[addr/8] &= ~(1 << addr%8);
	return ret_addr;
}


static uint64_t bitmap_clear_pages(uint64_t addr, uint32_t count)
{
	uint64_t ret_addr = addr;
	addr /= 0x1000;
	while(count--)
	{
		if( addr > bitmap_frames_count ) return 0;
		bitmap[addr/8] &= ~(1 << addr%8);
		++addr;
	}
	return ret_addr;
}

static uint32_t bitmap_check(uint64_t addr)
{
	addr /= 0x1000;
	if( addr > bitmap_frames_count ) return 0;	/* XXX */
	return (bitmap[addr/8] & (1 << (addr%8)));
}

static void bitmap_set_usable(uint64_t addr, uint64_t size)
{
	//debug("Setting memory at address %lx and size %ld as usable\n", addr, size); 
	uint32_t pages_count = size/4096;
	while(pages_count--)
	{
		bitmap_set(addr);
		addr += 0x1000;
	}
}

static void bitmap_set_unusable(uint64_t addr, uint64_t size)
{
	//debug("Setting memory at address %lx and size %ld as unusable\n", addr, size); 
	uint32_t pages_count = size/4096;
	while(pages_count--)
	{
		bitmap_clear(addr);
		addr += 0x1000;
	}
}

static void bitmap_setup(uint64_t mem)
{
	bitmap_frames_count = mem/4096/8*8;
	bitmap = (uint8_t*)heap_alloc(bitmap_frames_count/8);
	memset(bitmap, 0, bitmap_frames_count/8);
}

static uint64_t bitmap_get_frame()
{
	uint64_t i = 0;
	while( i < bitmap_frames_count )
	{
		if(bitmap_check(i * 0x1000))
			return bitmap_clear(i*0x1000);
		++i;
	}
	//debug("Can't find free page frame\n");
}

static uint64_t bitmap_get_frames(uint32_t count)
{
	uint64_t i = 0, k = 0;
	while( i < bitmap_frames_count )
	{
		if( k = i, bitmap_check(i*0x1000) )
			while( i < bitmap_frames_count && bitmap_check(++i*0x1000))
				if( (i-k) == count )
					return bitmap_set_pages(k*0x1000, count);
		++i;
	}
	//debug("Can't find contigouos page frames\n");
}

void bitmap_dump()
{
	//debug("Dump of Bitmap : size = %d B\n", bitmap_frames_count/8);
	uint64_t x = 0;
	uint64_t y = 0;
	while( y < bitmap_frames_count/8/4 )
	{
		//debug("0x%x : ", y * 32 * 0x1000);
		while( x < 4 )
		{
			//debug("%b ", bitmap[4*y + x]);
			++x;
		}
		x = 0;
		++y;
		//debug("\n");
	}
}

mman_t mman = {
	.set = &bitmap_set,
	.set_pages = &bitmap_set_pages,
	.clear = &bitmap_clear,
	.clear_pages = &bitmap_clear_pages,
	.check = &bitmap_check,
	.set_usable = &bitmap_set_usable,
	.set_unusable = &bitmap_set_unusable,
	.get_frame = &bitmap_get_frame,
	.get_frames = &bitmap_get_frames,
	.setup = &bitmap_setup,
	.dump = &bitmap_dump,
};

void map_mem(multiboot_info_t *mboot)
{
	//debug("Kernel heap starts at 0x%lx\n", kernel_heap_ptr);
	uint32_t total = mboot->mem_lower + mboot->mem_upper;
	//debug("Total usable memory %d KiB\n", total);
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
	
	//debug("Total Mem = %lx\n", total_mem);
	mman.setup(total_mem);
	mmap = (multiboot_memory_map_t*)(uint64_t)mboot->mmap_addr;
	
	while( ((uint64_t)mmap + mmap->size + sizeof(uint32_t)) < max_mmap )
	{
		uint64_t len = mmap->len;
		//debug("%lx => %lx : %s\n", mmap->addr, mmap->addr + mmap->len, 
		//		mmap->type==1?"usable":"unusable");
		if(mmap->type==1) mman.set_usable(mmap->addr, mmap->len);
		mmap = (multiboot_memory_map_t*)(uint64_t)
				((uint64_t)mmap + mmap->size + sizeof(uint32_t));
			
	}
	mman.set_unusable(0, kernel_end + heap_size + kernel_heap_size * 0x1000); 
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



/*
 *
 *	Virtual memory manager
 *
 */

extern uint32_t heap_addr;
extern uint32_t VMA;

uint64_t *VMPD;	// = 0xFFFFFFFF80000000;
uint64_t *VMIPT;
uint64_t *nodes_bitmap_table;
uint64_t *nodes_bitmap;


typedef struct vmem_node_t
{
	uint32_t addr : 30;
	uint32_t free : 1;
	uint32_t _ : 1;	// Currently unused
	uint32_t size: 26;
	uint32_t next: 22;
}__attribute__((packed)) vmem_node_t;


#define NODES_BITMAP_TABLE	0xFFFFFFFF80001000
#define NODES_BITMAP 		0xFFFFFFFF80200000
#define NODES_TABLE			0xFFFFFFFF80002000
#define NODES				0xFFFFFFFF80400000
#define ALLOC_START			0xFFFFFFFF81400000

vmem_node_t *head = NODES;

static vmem_node_t *get_node();
static void node_set(uint32_t id);
static void node_clear(uint32_t id);
void *kmalloc(uint32_t size);
void kfree(void *addr);

void dump_nodes()
{
	//debug("Nodes dump\n");
	vmem_node_t *tmp = head;
	do {
		//debug("Node : %lx\n", tmp);
		//debug("   |_ Addr   : %lx\n", ALLOC_START + (uint64_t)tmp->addr);
		//debug("   |_ free ? : %s\n", tmp->free?"yes":"no");
		//debug("   |_ Size   : %d B [ %d KiB ]\n", 
		//	(tmp->size + 1) * 4, (tmp->size + 1) * 4/1024 );
		//debug("   |_ Next   : %lx\n", (vmem_node_t*)NODES + tmp->next );
	} while(tmp->next && (tmp = (vmem_node_t*)NODES + tmp->next));

}

void vmem_init()
{
	//for(;;);	// XXX ... memory in not allocated ... figure out a solution !
	uint64_t* _VMPD = mman.get_frame();
	*(uint64_t*)((uint64_t)&VMA + (uint64_t)_VMPD) = (uint64_t)_VMPD | 3;
	*(uint64_t*)((uint64_t)&VMA + heap_addr + 0x2FF0) = (uint64_t)_VMPD | 3;
	VMPD = (uint64_t)&VMA + (uint64_t)_VMPD;

	nodes_bitmap_table = mman.get_frame();
	*(VMPD + 1) = (uint64_t)nodes_bitmap_table | 3;
	nodes_bitmap_table = NODES_BITMAP_TABLE;
	uint32_t i;
	for(i = 0; i < 64; ++i)
		*(nodes_bitmap_table + i) = mman.get_frame() | 3;

	memset(NODES_BITMAP, -1, 26214);
	
	for(i = 0; i < 8; ++i)
		*(VMPD + 2 + i) = mman.get_frame() | 3;
		
	uint64_t *nodes_table = NODES_TABLE;
	for(i = 0; i < 512; ++i)
		*(nodes_table + i) = mman.get_frame() | 3;
		
	// Setting up initial node
	*(vmem_node_t*)((uint64_t)NODES) = 
		(vmem_node_t)
		{ 
		  .addr = 0,
		  .free = 1, 
		  .size = -1,
		  .next = 0
		};
	node_clear(0);
}

static uint8_t node_check(uint32_t id)
{
	uint8_t *addr = NODES_BITMAP + id/8;
	return (*addr>>(id%8))&1;
}

static void node_set(uint32_t id)
{
	uint8_t *addr = NODES_BITMAP + id/8;
	*addr |= 1 << id%8;
}

static void node_clear(uint32_t id)
{
	uint8_t *addr = NODES_BITMAP + id/8;
	*addr &= ~(1 << id%8);
}

static vmem_node_t *get_node()
{
	uint32_t i = -1; 
	while(!node_check(++i));
	node_clear(i);
	return i; 
}

void map_to_physical(void *ptr, uint32_t size)
{
	//debug("Mapping %d to physical ptr : %lx\n", size, ptr);
	uint32_t tables_count = size/0x200000 + (size%0x200000?1:0);
	uint32_t pages_count = size/0x1000 + (size%0x1000?1:0);
	uint64_t index = ((uint64_t)ptr&0xFFFFFFF)/0x200000;
	uint64_t pindex = ((uint64_t)ptr&0xFFFFF)/0x1000;
	uint64_t *init_table = 0xFFFFFFFF80000000 + index * 0x8;
	uint64_t *init_page = 0xFFFFFFFF80000000 + index * 0x1000 + 0x8 * pindex;
	//debug("Mapping at %x [%d] %lx %lx\n", (uint32_t)index, (uint32_t)pindex, init_table, init_page);
	uint32_t i;
	for(i = 0; i < tables_count; ++i)
	{
		//debug("T %lx : %lx\n", init_table + i, *(init_table + i));
		if(!(*(init_table+i)&1))
			*(init_table+i) = mman.get_frame() | 3;
	}
	for(i = 0; i < pages_count; ++i)
		if(!(*(init_page+i)&1))
			*(init_page+i) = mman.get_frame() | 3;
	TLB_flush();
}

void unmap_from_physical(void *ptr, uint32_t size)
{
	//debug("Unmapping %d B at #%lx\n", size, ptr);
	uint32_t tables_count = size/0x200000;
	tables_count -= tables_count?((uint64_t)ptr%0x20000)?1:0:0;
	
	uint32_t pages_count = size/0x1000;
	pages_count -= pages_count?((uint64_t)ptr%0x1000)?1:0:0;
	
	uint64_t index = ((uint64_t)ptr&0xFFFFFFF)/0x200000;
	uint64_t pindex = ((uint64_t)ptr&0xFFFFF)/0x1000;
	uint64_t *init_table = 0xFFFFFFFF80000000 + index * 0x8 
		+ (tables_count?(((uint64_t)ptr)/0x20000?0x8:0):0);
	uint64_t *init_page = 0xFFFFFFFF80000000 + index * 0x1000 + 0x8 * pindex
		+ (pages_count?(((uint64_t)ptr)/0x1000?0x8:0):0);
		
	//debug("init_table = %lx, init_page = %lx\n", init_table, init_page);

	uint32_t i;
	for(i = 0; i < pages_count; ++i)
		if((*(init_page+i)&1))	// We better make sure it's set
		{
			mman.set(*(init_page+i));	// Reclaim
			*(init_page+i) = 0;
		}

	for(i = 0; i < tables_count; ++i)
		if(!(*(init_table+i)&1))
		{
			mman.set(*(init_table+i));
			*(init_table+i) = 0;
		}
	TLB_flush();
}

void *kmalloc(uint32_t size)
{
	size = size/4*4 + (size%4?4:0);
	vmem_node_t *tmp = head;
	do
	{
		if(	(tmp->size + 1)*4 >= size && tmp->free ) goto found_valid_node;
		if(!tmp->next) return NULL;
		tmp = (vmem_node_t*)NODES + tmp->next;
	}
	while(tmp); 
	return NULL;
	
	found_valid_node:
	////debug("Found a valid node at %lx [%d] \n", tmp, (tmp->size + 1) * 4);
	map_to_physical( ALLOC_START + (uint64_t)tmp->addr, size);
	if( size == (tmp->size + 1) * 4 )
	{
		tmp->free = 0;
		return ALLOC_START + (uint64_t)tmp->addr;
	}
	uint32_t tmp_size = (tmp->size + 1) * 4 - size;
	vmem_node_t *tmp_next = tmp->next;
	tmp->next = get_node();
	vmem_node_t *next_node = (vmem_node_t*)NODES + tmp->next;
	*next_node = 
		(vmem_node_t) {
			.addr = tmp->addr + size,
			.size = tmp_size / 4 - 1,
			.free = 1,
			.next = tmp_next
		};
	tmp->size = size / 4 - 1;
	tmp->free = 0;
	return ALLOC_START + (uint64_t)tmp->addr;
}

void kfree(void *addr)
{
	// let's search for this address
	vmem_node_t *tmp = head;
	while( tmp->next && (ALLOC_START + tmp->addr != addr) ) 
		tmp = (vmem_node_t*)NODES + tmp->next;
	if(ALLOC_START + tmp->addr != addr || 
	( (ALLOC_START + tmp->addr == addr) && tmp->free ))	
		return;	// Trying to free unallocated address 
	
	unmap_from_physical( ALLOC_START + tmp->addr, (tmp->size + 1) * 4);
	if(((vmem_node_t*)NODES + tmp->next)->free)
	{
		// Found reclaimable area .. we must be lucky =D
		vmem_node_t *next = (vmem_node_t*)NODES + tmp->next;
		uint32_t next_id = tmp->next;
		tmp->size += next->size + 1;
		tmp->next = next->next;
		tmp->free = 1;
		node_set(next_id);
	}
	// else not reclaimable area :/
	tmp->free = 1;
}

void __attribute__((always_inline)) TLB_flush()
{
	asm volatile("movq %%cr3, %%rax;movq %%rax, %%cr3":::"rax");
}


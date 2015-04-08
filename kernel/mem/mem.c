#include <system.h>
#include <mem.h>

uint64_t total_mem;
uint64_t total_usable_mem;

uint8_t *bitmap;
uint64_t bitmap_frames_count;	// Must be multible of 8

void *heap_alloc(uint32_t size)
{
	void *ptr = kernel_heap_ptr;
	kernel_heap_ptr += size;
	debug("Kenrel heap moved to 0x%lx\n", kernel_heap_ptr);
	return ptr;
}

static uint8_t bitmap_set(uint64_t addr)
{
	addr /= 0x1000;
	if( addr > bitmap_frames_count ) return 0;
	bitmap[addr/8] |= 1 << addr%8;
	return 1;
}

static uint8_t bitmap_clear(uint64_t addr)
{
	addr /= 0x1000;
	if( addr > bitmap_frames_count ) return 0;
	bitmap[addr/8] &= ~(1 << addr%8);
	return 1;
}

static uint32_t bitmap_check(uint64_t addr)
{
	addr /= 0x1000;
	if( addr > bitmap_frames_count ) return 0;	/* XXX */
	return (bitmap[addr/8] & (1 << (addr%8)));
}

static void bitmap_set_usable(uint64_t addr, uint64_t size)
{
	debug("Setting memory at address %lx and size %ld as usable\n", addr, size); 
	uint32_t pages_count = size/4096;
	while(pages_count--)
	{
		bitmap_set(addr);
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
			return bitmap_clear(i*0x1000), i * 0x1000;
		++i;
	}
	debug("Can't find free page frame\n");
}

static uint64_t bitmap_get_frames(uint32_t count)
{
	uint64_t i = 0, k = 0;
	while( i < bitmap_frames_count )
	{
		if( k = i, bitmap_check(i*0x1000) )
			while( i < bitmap_frames_count && bitmap_check(++i*0x1000))
				if( (i-k) == count )
					return k*0x1000;
		++i;
	}
	debug("Can't find contigouos page frames\n");
}

void bitmap_dump()
{
	debug("Dump of Bitmap : size = %d B\n", bitmap_frames_count/8);
	uint64_t x = 0;
	uint64_t y = 0;
	while( y < bitmap_frames_count/8/4 )
	{
		debug("0x%x : ", y * 32 * 0x1000);
		while( x < 4 )
		{
			debug("%b ", bitmap[4*y + x]);
			++x;
		}
		x = 0;
		++y;
		debug("\n");
	}
}

void *memset(void *addr, uint8_t val, uint32_t size)
{
	uint8_t *_addr = (uint8_t*)addr;
	while(size--) *_addr++ = val;
	return (void*)addr;
}

mman_t mman = {
	.set = &bitmap_set,
	.clear = &bitmap_clear,
	.check = &bitmap_check,
	.set_usable = &bitmap_set_usable,
	.get_frame = &bitmap_get_frame,
	.get_frames = &bitmap_get_frames,
	.setup = &bitmap_setup,
	.dump = &bitmap_dump,
};


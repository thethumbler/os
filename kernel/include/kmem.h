#ifndef _KMEM_H
#define _KMEM_H

#include <system.h>
#include <multiboot.h>

uint64_t *UPD;	// User page directory
uint64_t *PML4;
uint64_t *KPD;	// Kernel page directory
uint8_t *kernel_heap_ptr;

extern uint64_t total_mem;
extern uint64_t total_usable_mem;

extern multiboot_info_t *mboot_info;
extern uint32_t kernel_size;
extern uint32_t heap_addr;
extern uint32_t heap_size;
extern uint32_t kernel_heap_size;
extern uint32_t VMA;
//extern uint32_t kend;
uint64_t kernel_end;
//uint8_t *kernel_heap_ptr;

typedef struct {
	uint64_t (*set) (uint64_t);
	uint64_t (*set_pages) (uint64_t, uint32_t);
	uint64_t (*clear) (uint64_t);
	uint64_t (*clear_pages) (uint64_t, uint32_t);
	uint32_t (*check) (uint64_t);
	void (*set_usable) (uint64_t, uint64_t);
	void (*set_unusable) (uint64_t, uint64_t);
	uint64_t (*get_frame) ();
	uint64_t (*get_frames) (uint32_t);
	void (*setup) (uint64_t);
	void (*dump) ();
} mman_t;

mman_t mman;
void *memset(void*, uint8_t, uint32_t);

void map_mem(multiboot_info_t*);
void vmem_init();
void *kalloc(uint32_t);
void free(void*);

#endif

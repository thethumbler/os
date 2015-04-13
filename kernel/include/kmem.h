#ifndef _KMEM_H
#define _KMEM_H

#include <system.h>

uint64_t *KPD;	// Kernel page directory
uint8_t *kernel_heap_ptr;

extern uint64_t total_mem;
extern uint64_t total_usable_mem;

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

void vmem_init();
void *kalloc(uint32_t);
void free(void*);

#endif

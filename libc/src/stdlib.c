#include <system.h>
#include <stddef.h>
#include <stdint.h>
//#include <stdio.h>		////printf for debugging
#include <stdlib.h>
#include <unistd.h>

typedef struct meta_data
{
	size_t size;
	struct meta_data *next;
	uint8_t	free;
} __attribute__((packed)) md_t;

void *malloc(size_t size)
{
	char *__ = "\0\0\0\0\0\0\0\0";	// Nasty hack
	
	void **heap_start = __;
	
	/* User Dynamic Memory Allocator "malloc"
	 * This function is used for dynamic memory allocation
	 * It uses syscall sbrk() for extending the heap of the program
	 * 
	 * What this function does ?!
	 * Nothing ... just basic memory management for the extra area
	 * It uses first fit alogarithm for finding a free block
	 * It supports splitting and merging blocks
	 */

	if(!*heap_start) {		//initializing

		*heap_start = sbrk(META_SIZE + size);

		if(!*heap_start) return NULL;		//malloc failed

		((md_t*) *heap_start)->size = size;
		((md_t*) *heap_start)->next = NULL;
		((md_t*) *heap_start)->free = 0;

		return *heap_start + META_SIZE;
	}
//printf("--------------------------------------------------\n");
//printf("Entered umalloc\n");
//printf("Size == %u\n",(unsigned) size);

	md_t *prev = NULL;
	md_t *current = *heap_start;
	md_t *next = NULL;

	while(current) {
		if(current->free) {		//if current block is free
//printf("Entered if free\n");
			next = current->next;
			while(next && next->free) {		//Merge blocks
//printf("--------------------\n");
//printf("Entered Merge\n");
//printf("Old Size   == %u\n",(unsigned) current->size);
//printf("Old Next   == 0x%lX\n",(unsigned long) current->next);
				current->size = current->size + META_SIZE + next->size;
				current->next = next = next->next;
//printf("New Size   == %u\n",(unsigned) current->size);
//printf("New Next   == 0x%lX\n",(unsigned long) current->next);
			}

			if(current->size >= size) break;		//found a free block with enough size
		}

		prev = current;
		current = current->next;		//check next block
	}

	if(!current) {		//if no free blocks found ... extend the heap
//printf("--------------------\n");
//printf("Entered heap extension\n");
		prev->next = current = sbrk(META_SIZE + size);

		if(!current) return NULL;		//malloc failed

		current->size = size;
		current->next = NULL;
		//current->free = 0;

		//return (void*) current + META_SIZE;
	}
	else if(current->size >= META_SIZE + size + MIN_TO_SPLIT) {		//splittable free block with enough space found
//printf("--------------------\n");
//printf("Entered split\n");
//printf("current          == 0x%lX\n",(unsigned long)current);
//printf("META_SIZE        == 0x%X\n",(unsigned) META_SIZE);
//printf("Size             == %u\n",(unsigned) size);
			next = (void *) current + META_SIZE + size;
			next->size = current->size - size - META_SIZE;
			next->next = current->next;
			next->free = 1;

			current->size = size;
			current->next = next;
//printf("next should be   == 0x%lX\n",(unsigned long) current + META_SIZE + size);
//printf("next is          == 0x%lX\n",(unsigned long) next);
			//current->free = 0;

			//return (void*) current + META_SIZE;
	}
//printf("--------------------\n");
//printf("Entered return stage\n");
//printf("current       == 0x%lX\n",(unsigned long)current);
//printf("META_SIZE     == 0x%X\n",(unsigned) META_SIZE);
//printf("should return == 0x%lX\n",(unsigned long) (void*) current + META_SIZE);
//printf("--------------------------------------------------\n");
	current->free = 0;
	return (void *) current + META_SIZE;

}

void free(void *ptr)
{
	/* Very simple free
	 * TODO: Free block even if ptr is in the middle of the block
	 */
	((md_t *) (ptr-META_SIZE))->free = 1;
}

void *calloc(size_t num_units,size_t size_unit)
{
	//TODO:Add overflow check
	size_t i = num_units*size_unit;
	uint8_t *ptr, *_ptr;

	ptr = _ptr = malloc(i);
	if(!ptr) return NULL;

	//Doing it byte by byte since malloc isn't aligned
	while(i--) { *_ptr = 0; _ptr++; }

	return ptr;
}

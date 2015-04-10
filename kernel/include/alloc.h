#ifndef _ALLOC_H
#define _ALLOC_H

uint64_t *KPD;	// Kernel page directory

typedef enum { PAGE_NOT_FOUND, TABLE_NOT_FOUND, PAGE_FOUND } check_page_t;

check_page_t check_page(uint64_t);
void *kmalloc(uint32_t);

#endif

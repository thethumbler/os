#ifndef _STDLIB_H
#define _STDLIB_H

#define META_SIZE (2*sizeof(void *) + 1)
#define MIN_TO_SPLIT 1		//min free space required to split the blocks

void *malloc(size_t);
void free(void *);
void *calloc(size_t, size_t);

#endif

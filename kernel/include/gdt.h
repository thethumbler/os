#ifndef _GDT_H
#define _GDT_H

#include <system.h>

typedef struct
{
	uint32_t limit_low 	: 16;
	uint32_t base_low  	: 24;
	uint8_t  type		: 4;
	uint8_t  system		: 1;
	uint8_t	 dpl		: 2;
	uint8_t	 present	: 1;
	uint16_t limit_high	: 4;
	uint8_t  avilable	: 1;
	uint8_t  longmode	: 1;
	uint8_t	 size		: 1;
	uint8_t  gran		: 1;
	uint32_t base_high	: 8;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
	uint16_t limit;
	uint64_t base;
} gdt_ptr_t;


typedef struct
{
	uint32_t limit_low	: 16;
	uint32_t base_low	: 24;
	uint8_t	 type		:  4;
	uint8_t  _0_1		:  1;
	uint8_t  dpl		:  2;
	uint8_t  present	:  1;
	uint8_t  limit_high	:  4;
	uint8_t  avilable	:  1;
	uint8_t  _0_2		:  2;
	uint8_t  gran		:  1;
	uint64_t base_high	: 40;
	uint8_t  _reserved1 :  8;
	uint8_t	 _0_3		:  5;
	uint32_t _reserved2 : 19;
} __attribute__((packed)) tss64_seg_t;

typedef struct
{
	uint32_t _reserved;
	uint32_t rsp0_low;
	uint32_t rsp0_high;
	uint32_t _unused_in_our_kernel[23];
} tss64_t;

#endif

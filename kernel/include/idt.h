#ifndef _IDT_H
#define _IDT_H

#include <system.h>

typedef struct
{
	uint16_t offset_low;
	uint16_t selector;
	uint8_t set_to_0;
	uint8_t flags;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t __reserved__space__not__used;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt[256];
idt_ptr_t idt_ptr;

void idt_set_gate(uint32_t, uint64_t, uint16_t, uint8_t);
void idt_install();

#endif

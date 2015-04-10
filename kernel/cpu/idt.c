#include <system.h>
#include <idt.h>

void idt_set_gate( uint32_t id, uint64_t offset, uint16_t sel, uint8_t flags)
{
	idt[id].offset_low  = (offset >> 0x00) & 0xFFFF;
	idt[id].offset_mid  = (offset >> 0x10) & 0xFFFF;
	idt[id].offset_high = (offset >> 0x20) & 0xFFFFFFFF;

	idt[id].selector = sel;
	
	idt[id].set_to_0 = 0;
	idt[id].flags = flags;
}

void idt_install()
{
	extern void isr80();
	idt_set_gate(80, &isr80, 0x8, 0x8E);
	extern void idt_flush();
	idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
	idt_ptr.base = (uint64_t)&idt;
	idt_flush();
}

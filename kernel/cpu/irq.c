#include <system.h>
#include <debug.h>
#include <isr.h>

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

typedef void (*irq_handler_t)(regs_t*);

static irq_handler_t irq_routines[16] = { NULL };

void irq_install_handler(uint32_t irq, irq_handler_t handler) {
	irq_routines[irq] = handler;
}

void irq_uninstall_handler(uint32_t irq) {
	irq_routines[irq] = 0;
}

void irq_remap(void) {
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
}

void irq_gates(void) {
	idt_set_gate(32, irq0, 0x08, 0x8E);
	idt_set_gate(33, irq1, 0x08, 0x8E);
	idt_set_gate(34, irq2, 0x08, 0x8E);
	idt_set_gate(35, irq3, 0x08, 0x8E);
	idt_set_gate(36, irq4, 0x08, 0x8E);
	idt_set_gate(37, irq5, 0x08, 0x8E);
	idt_set_gate(38, irq6, 0x08, 0x8E);
	idt_set_gate(39, irq7, 0x08, 0x8E);
	idt_set_gate(40, irq8, 0x08, 0x8E);
	idt_set_gate(41, irq9, 0x08, 0x8E);
	idt_set_gate(42, irq10, 0x08, 0x8E);
	idt_set_gate(43, irq11, 0x08, 0x8E);
	idt_set_gate(44, irq12, 0x08, 0x8E);
	idt_set_gate(45, irq13, 0x08, 0x8E);
	idt_set_gate(46, irq14, 0x08, 0x8E);
	idt_set_gate(47, irq15, 0x08, 0x8E);
}

void irq_install(void) {
	irq_remap();
	irq_gates();
}

void irq_ack(uint32_t irq_no) {
	if (irq_no >= 8) {
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
}

extern uint32_t int_num;
void irq_handler(regs_t *regs) {
	debug("Recieved IRQ %x\n", int_num);
	void (*handler)(regs_t *regs);
	if (int_num > 47 || int_num < 32) {
		handler = NULL;
	} else {
		handler = irq_routines[int_num - 32];
	}
	if (handler) {
		handler(regs);
	} else {
		irq_ack(int_num - 32);
	}
}

void timer(regs_t *regs)
{
	debug("Recieved timer\n");
}

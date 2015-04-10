bits 64

extern idt_ptr
global idt_flush
idt_flush:
	mov rax, idt_ptr
	lidt [rax]
	ret

global isr80
isr80:
	mov eax, 0xb8000
	mov [eax], byte 'S'
	;jmp $
	iretq

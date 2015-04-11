bits 64

extern idt_ptr
global idt_flush
idt_flush:
	mov rax, idt_ptr
	lidt [rax]
	ret

; We emulate iretq .... ugly !
%macro iretq 0
	mov rax, qword [rsp]
	add rsp, 40
	jmp rax
%endmacro

%macro pusha 0
	push rax
	push rdx
	push rcx
	push rbx
	push rsp
	push rbp
	push rsi
	push rdi
%endmacro

%macro popa 0
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rcx
	pop rdx
	pop rax
%endmacro

global int_num, err_num
int_num: dw 0
err_num: dw 0

%macro ISR_NOERR 1
	global isr%1
	isr%1:
		cli
		mov [rel err_num], dword 0
		mov [rel int_num], dword %1
		jmp isr_handler
%endmacro

%macro ISR_ERR 1
	global isr%1
	isr%1:
		cli
		pop rax
		mov [rel err_num], rax
		mov [rel int_num], dword %1
		jmp isr_handler
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

isr_handler:
	pusha
	mov rdi, rsp
	extern interrupt
	call interrupt
	popa
	iretq

%macro IRQ 2
	global irq%1
	irq%1:
		cli
		mov [rel err_num], rax
		mov [rel int_num], dword %2
		jmp irq_stub
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
irq_stub:
	pusha
	extern irq_handler
	call irq_handler
	popa
	sti
	iretq

bits 64

extern gdt_ptr
global gdt_flush
gdt_flush:
	mov rax, gdt_ptr
	lgdt [rax]
	ret

extern idt_ptr
global idt_flush
idt_flush:
	mov rax, idt_ptr
	lidt [rax]
	ret
	
global load_tss
load_tss:
	;extern tss_ptr
	ltr [rel tss_ptr]
	ret

tss_ptr:
	dw 0x28

; We emulate iretq .... ugly !
;%macro iretq 0
;	mov rax, qword [rsp]
;	add rsp, 40
;	jmp rax
;%endmacro

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
int_num: dq 0
err_num: dq 0

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


%macro push_context 0
	push rax
	push rdx
	push rcx
	push rbx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro
	
%macro pop_context 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	pop rcx
	pop rdx
	;pop rax ;XXX RAX has to be poped manually
%endmacro

isr_handler:
	push_context
	mov rdi, rsp
	extern interrupt
	call interrupt
	pop_context
	pop rax
	iretq
	
global isr128
isr128:
	push_context
	mov rdi, rsp
	extern syscall_int
	call   syscall_int
	pop_context
	pop rax
	iretq

	
global switch_to_usermode
switch_to_usermode:
	cli
	mov ax, 0x23
	mov ds, ax
	mov rax, 0x7FC0000000	; Stack
	push qword 0x23
	push rax
	pushfq
	pop rax
	bts rax, 9
	push rax
	push qword 0x1B
	push qword rdi
	iretq

global switch_context
switch_context:
	;pop_context
	cli
	mov rax, rsp
	mov rsp, rdi
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	pop rcx
	mov rdx, rsp
	mov rsp, rax
	mov ax, 0x23
	mov ds, ax
	mov rax, [rdx + 4*0x8] ; rsp
	push qword 0x23
	push rax
	mov rax, [rdx + 3*0x8] ; rflags
	bts rax, 9
	push rax
	push qword 0x1B
	mov rax, [rdx + 2*0x8] ; rip
	push qword rax
	mov rax, [rdx + 0x8]	; rax
	mov rdx, [rdx]			; rdx
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
	push_context
	mov rdi, rsp
	extern irq_handler
	call   irq_handler
	pop_context
	pop rax
	iretq

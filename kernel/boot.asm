bits 32

kernel_init_heap equ 4

section .text

global _start

_start:
	jmp real_start
	align 4
	dd 0x1BADB002
	dd 3
	dd -(0x1BADB002 + 3)

real_start:
	mov [mboot_info], ebx
	
	; Disable IRQs
    ;mov al, 0xFF         ; Out 0xFF to 0xA1 and 0x21 to disable all IRQs.
    ;out 0xA1, al
    ;out 0x21, al
	
	extern kstart, kend
	
	; Get heap address
	; Lets check if any modules are loaded
	lea eax, [ ebx + 0x14 ]
	mov eax, [eax]
	or  eax, eax
	jz no_modules_found
	dec eax
	lea ecx, [ ebx + 0x18 ]
	mov ecx, [ecx]
	mov ebx, 0x10
	mul ebx
	add eax, ecx
	add eax, 0x4
	mov eax, [eax]	; Now eax has the end address of the last module
	jmp found_modules
	
	no_modules_found:
	mov eax, kend
	found_modules:
	mov ebx, 0x1000
	div ebx
	mov ecx, edx
	mul ebx
	or ecx, ecx
	jz .skip
	add eax, 0x1000
	.skip:
	mov [heap_addr], eax

	; Get bootloader size in pages
	xor edx, edx
	mov eax, kstart
	mov ebx, 0x1000
	div ebx
	or  edx, edx
	jz .skip2
	inc eax
	.skip2:
	mov [boot_size], eax
	xor edx, edx
	mov ebx, 0x200
	div ebx
	or edx, edx
	jz ._skip2
	inc eax
	._skip2:
	mov [boot_size_ptables], eax
	
	; Get kernel size in pages
	xor edx, edx
	mov eax, [heap_addr]	; Allocate up to heap address
	mov ebx, 0x1000
	div ebx
	or edx, edx
	jz .skip3
	inc eax
	.skip3:
	mov ecx, kernel_init_heap
	mov [kernel_heap_size], ecx
	add eax, ecx
	mov [kernel_size], eax
	xor edx, edx
	mov ebx, 0x200
	div ebx
	or edx, edx
	jz ._skip3
	inc eax
	._skip3:
	mov [kernel_size_ptables], eax

	; clear boot paging buffer
	mov eax, 5	; PML4, 2 PDPTs, 2 PDs
	add eax, [boot_size_ptables]
	add eax, [kernel_size_ptables]
	mov ebx, 0x1000
	mul ebx
	mov [heap_size], eax
	
	mov ecx, eax
	xor eax, eax
	mov edi, [heap_addr]
	rep stosb
	
	; Setup PML4 with 2 PDPTs
	mov edi, [heap_addr]
	; Setup first PDPT
	lea eax, [edi + 0x1000]
	or eax, 3
	mov [edi], eax
	; Setup second PDPT
	add eax, 0x1000
	add edi, 0x0FF8
	mov [edi], eax
	
	; Setup first PDPT with first PD
	mov edi, [heap_addr]
	lea eax, [edi + 0x3003]
	add edi, 0x1000
	mov [edi], eax
	
	; Setup second PDPT with second PD
	mov edi, [heap_addr]
	lea eax, [edi + 0x4003]
	add edi, 0x2FF8
	mov [edi], eax
	
	; Setup first PD with required page tables for boot
	mov edi, [heap_addr]
	lea eax, [edi + 0x5003]
	mov ecx, [boot_size_ptables]
	add edi, 0x3000
	lea esi, [eax - 0x3]
	mov [boot_ptable_addr], esi
	.loop_first_pd:
		mov [edi], eax
		add eax, 0x1000
		add edi, 8
		dec ecx
		or ecx, ecx
		jnz .loop_first_pd
		
	; Setup second PD with required page tables for kernel
	mov edi, [heap_addr]
	mov eax, [boot_size_ptables]
	mov ebx, 0x1000
	mul ebx
	lea ebx, [edi + 0x5003]
	add edi, eax
	add eax, ebx
	
	mov ecx, [kernel_size_ptables]
	add edi, 0x3000
	lea esi, [eax - 0x3]
	mov [kernel_ptable_addr], esi
	.loop_second_pd:
		mov [edi], eax
		add eax, 0x1000
		add edi, 8
		dec ecx
		or ecx, ecx
		jnz .loop_second_pd
	
	; Setup boot page tables
	mov edi, [boot_ptable_addr]
	mov eax, 3
	mov ecx, [boot_size_ptables]
	.boot_ptable_loop:
		push ecx
		mov ecx, 0x200
		.loop_page_table:
			mov [edi], eax
			add eax, 0x1000
			add edi, 8
			dec ecx
			or  ecx, ecx
			jnz .loop_page_table
		pop ecx
		dec ecx
		or  ecx, ecx
		jnz .boot_ptable_loop
	
	; Setup kernel page tables
	mov edi, [kernel_ptable_addr]
	mov eax, 3
	mov ecx, [kernel_size_ptables]
	.kernel_ptable_loop:
		push ecx
		mov ecx, 0x200
		.kernel_page_table:
			mov [edi], eax
			add eax, 0x1000
			add edi, 8
			dec ecx
			or  ecx, ecx
			jnz .kernel_page_table
		pop ecx
		dec ecx
		or  ecx, ecx
		jnz .kernel_ptable_loop
		
    mov eax, 10100000b                ; Set the PAE and PGE bit.
    mov cr4, eax
 
    mov edx, [heap_addr]			  ; Point CR3 at the PML4.
    mov cr3, edx



    mov ecx, 0xC0000080               ; Read from the EFER MSR. 
    rdmsr    
 
    or eax, 0x00000100                ; Set the LME bit.
    wrmsr
 
    mov ebx, cr0                      ; Activate long mode -
    or ebx,0x80000001                 ; - by enabling paging and protection simultaneously.
    mov cr0, ebx                    

    lgdt [GDT.Pointer]                ; Load GDT.Pointer defined below.
 
    jmp 0x8:longmode             ; Load CS with 64 bit segment and flush the instruction cache

	jmp $
	
	
GDT:
    dq 0x0000000000000000	; Null
    dq 0x0020980000000000	; Kernel Code
    dq 0x0000900000000000	; Kerned Data
	dq 0x
	dq 0x
	
ALIGN 4
    dw 0
.Pointer:
    dw $ - GDT - 1
    dd GDT

bits 64
longmode:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
    
	extern kmain
	call kmain
	jmp $
	

global mboot_info
mboot_info: dq 0
global heap_addr
heap_addr: dd 0
global boot_size
boot_size: dd 0
boot_size_ptables: dd 0
boot_ptable_addr: dd 0
global kernel_size
kernel_size: dd 0
kernel_size_ptables: dd 0
kernel_ptable_addr: dd 0
global kernel_heap_size
kernel_heap_size: dd 0
global heap_size
heap_size: dd 0

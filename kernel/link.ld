OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {

	PMA = 0x100000;
	. = PMA;
	
	.boot.text ALIGN(0x1000) : {
		boot.o(.text)
		/*boot.o(.rodata*)*/
	}
	
	.boot.data ALIGN(0x1000) : {
		boot.o(.data)
	}	
	
	.boot.bss ALIGN(0x1000) : {
		boot.o(.bss)
	}
	
	VMA = 0xFFFFFFFFC0000000;
	. += VMA;
	
	kstart = . - VMA;
	
	.text ALIGN(0x1000) : AT(ADDR(.text) - VMA) {
		*(EXCLUDE_FILE(boot.o).text)
		*(.rodata*)
	}
	
	kstart_ = ADDR(.text);
	.data ALIGN(0x1000) : AT(ADDR(.data) - VMA) {
		*(EXCLUDE_FILE(*boot.o).data)
	}
	
	.bss ALIGN(0x1000) : AT(ADDR(.bss) - VMA) {
		*(EXCLUDE_FILE(*boot.o).bss)
	}
	
	
	kend = . - VMA;
}

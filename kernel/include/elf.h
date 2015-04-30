#ifndef _ELF_H
#define _ELF_H

typedef struct
{
	uint8_t  magic[4];
	uint8_t  class;
	uint8_t  endian;
	uint8_t  elf_version;
	uint8_t  abi;
	uint8_t  abi_version;
	uint8_t  pad[7];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} elf_hdr_t;

typedef struct
{
	uint32_t name;
	uint32_t type;
	uint64_t flags;
	uint64_t addr;
	uint64_t off;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t addralign;
	uint64_t entsize;
} elf_section_hdr_t;

#endif

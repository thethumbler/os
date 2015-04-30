#include <system.h>
#include <process.h>
#include <vfs.h>
#include <debug.h>
#include <elf.h>
#include <kmem.h>

extern process_t *current_process;

typedef struct
{
	file_t *file;
	uint64_t entry;
	uint32_t text_off;
	uint32_t text_size;
	uint64_t text_addr;
	uint32_t data_off;
	uint32_t data_size;
	uint64_t data_addr;
} elf_t;

elf_t *parse_elf(file_t *file)
{
	elf_t *ret = kmalloc(sizeof(elf_t));
	elf_hdr_t *hdr = file->buf;
	elf_section_hdr_t *text = file->buf + hdr->shoff + hdr->shentsize;
	elf_section_hdr_t *data = file->buf + hdr->shoff + 2 * hdr->shentsize;
	*ret = 
		(elf_t)
		{
			.file = file,
			.entry = hdr->entry,
			.text_off  = text->off,
			.text_size = text->size,
			.text_addr = text->addr,
			.data_off  = data->off,
			.data_size = data->size,
			.data_addr = data->addr,		
		};
	return ret;
}

uint64_t switch_pdpt(uint64_t pdpt)
{
	uint64_t ret = *PML4;
	*PML4 = pdpt | 7;
	TLB_flush();
	return ret;
}

void map_mem_user(uint64_t pdpt, uint64_t *ptr, uint32_t size)
{
	uint64_t cur_pdpt = switch_pdpt(pdpt);
	
	uint32_t pages = size/0x1000 + (size%0x1000?1:0);
	uint32_t tbls = pages/0x200 + (pages%0x200?1:0);
	uint32_t pds = tbls/0x200 + (tbls%0x200?1:0);

	uint32_t pindex = ((uint64_t)ptr) / 0x1000;
	uint32_t tindex = pindex/0x200;
	uint32_t dindex = tindex/0x200;
	
	//debug("Index [%d]\n", index);
	
	uint64_t
		*init_pd   = 0x7FFFFFF000 + 0x8 * dindex - 0x8, 
		*init_tbl  = 0x7FFFE00000 + 0x8 * tindex - 0x8,
		*init_page = 0x7FC0000000 + 0x8 * pindex - 0x8;
	
	//debug("ipd  %lx\nitbl %lx\nip   %lx\n", init_pd + 1, init_tbl + 1, init_page + 1);

	while(pds--)
		if(!(*++init_pd&1)) *init_pd   = mman.get_frame() | 7;	
	while(tbls--)
		if(!(*++init_tbl&1)) *init_tbl = mman.get_frame() | 7;
	while(pages--)
		if(!(*++init_page)) *init_page = mman.get_frame() | 7;
	switch_pdpt(cur_pdpt);
}

void unmap_mem_user(uint64_t pdpt, uint64_t *ptr, uint32_t size)
{
	uint64_t cur_pdpt = switch_pdpt(pdpt);
	
	uint32_t pages = size/0x1000 + (size%0x1000?1:0);
	uint32_t tbls = pages/0x200 + (pages%0x200?1:0);
	uint32_t pds = tbls/0x200 + (tbls%0x200?1:0);

	uint32_t pindex = ((uint64_t)ptr) / 0x1000;
	uint32_t tindex = pindex/0x200;
	uint32_t dindex = tindex/0x200;
	
	//debug("Index [%d]\n", index);
	
	uint64_t
		*init_pd   = 0x7FFFFFF000 + 0x8 * dindex - 0x8, 
		*init_tbl  = 0x7FFFE00000 + 0x8 * tindex - 0x8,
		*init_page = 0x7FC0000000 + 0x8 * pindex - 0x8;
	
	//debug("ipd  %lx\nitbl %lx\nip   %lx\n", init_pd + 1, init_tbl + 1, init_page + 1);

	while(pages--)
		if((*++init_page&1))
		{
			debug(" #%lx -> #%lx\n", init_page, *init_page);
			mman.set(*init_page);
			*init_page = 0;
		}
	while(tbls--)
		if((*++init_tbl&1))
		{
			mman.set(*init_tbl);
			*init_tbl = 0;
		}

	while(pds--)
		if((*++init_pd&1)) 
		{
			mman.set(*init_pd);
			*init_pd = 0;
		}
			
	switch_pdpt(cur_pdpt);
}

void exit_process(process_t *p)
{
	debug("Exitinig process %s\n", p->name);
	
	memset(0, 0, p->size * 0x1000);
	unmap_mem_user(p->pdpt, 0, p->size * 0x1000);
	unmap_mem_user(p->pdpt, 0x7FC0000000 - 0x8000, 0x8000);
	
	//for(;;);
	
	if(p->parent->status == WAITING_CHILD) 
	{
		p->parent->status = READY;
		//schedule_process(p->parent);
	}
	
	//mman.set(p->pdpt);
	extern void	deschedule_process(process_t*);
	deschedule_process(current_process);
	kfree(p->name);
	kfree(p);
	//switch_pdpt(cur_pdpt);
	TLB_flush();
}

uint64_t get_pdpt()
{
	//debug("PML4 %lx\n", PML4);
	uint64_t _pdpt = *PML4;
	//debug("_PDPT %lx\n", _pdpt);
	uint64_t pdpt = mman.get_frame();
	extern uint64_t kernel_end;
	*PML4 = ((uint64_t)kernel_end) | 3;
	*(uint64_t*)(0x8) = pdpt | 3;
	memset(0x1000, 0, 0x1000 - 0x8);
	*(uint64_t*)(0x1FF8) = pdpt | 3;
	*PML4 = _pdpt;
	TLB_flush();
	return pdpt;
}

uint64_t get_pid()
{
	static uint64_t pid = 0;
	return ++pid;
}

process_t *load_elf(char *filename)
{	
	file_t *file = vfs_fopen(filename, NULL);
	elf_t  *elf  = parse_elf(file);

	process_t *proc = kmalloc(sizeof(process_t));	
	proc->pdpt = get_pdpt();
	proc->name = strdup(filename);
	proc->pid  = get_pid();
	
	uint32_t size = elf->text_size + elf->data_size;
	proc->size = size/0x1000 + (size%0x1000?1:0);
	proc->heap = proc->size;
	
	proc->fds.len = 0;
	proc->fds.max_len = 5;
	proc->fds.ent = kmalloc( 5 * sizeof(inode_t*));

	map_mem_user(proc->pdpt, 0x0, size );
	map_mem_user(proc->pdpt, 0x7FC0000000 - 0x8000, 0x8000);	// Stack
	
	uint64_t cur_pdpt = switch_pdpt(proc->pdpt);

	// let's load the file at 0x0
	uint8_t *load = 0x0;
	uint32_t i;
	for(i = 0; i < size; ++i)
		*(load+i) = elf->file->buf[elf->text_off + i];
	
	kfree(file->buf);
	kfree(file);
	kfree(elf);

	proc->stat.rip = 0;
	proc->stat.rsp = 0x7FC0000000;

	switch_pdpt(cur_pdpt);
	return proc;
	//debug("Starting process %s [%d]\n", filename, size);

	//extern process_t *process_queue;
	//schedule_process(current_process);
}

void switch_process(process_t *p)
{
	debug("Switching process\n");
	debug("RIP  %lx\n", p->stat.rip);
	switch_pdpt(p->pdpt);
	extern void switch_context(stat_t*);
	switch_context(&p->stat);
}

void copy_process_stat(process_t *dest, process_t *src)
{
	// TODO : Optimzie this to memcpy()
	dest->stat.rax = src->stat.rax;
	dest->stat.rbx = src->stat.rbx;
	dest->stat.rcx = src->stat.rcx;
	dest->stat.rdx = src->stat.rdx;
	dest->stat.rsi = src->stat.rsi;
	dest->stat.rdi = src->stat.rdi;
	dest->stat.rsp = src->stat.rsp;
	dest->stat.rbp = src->stat.rbp;
	dest->stat.r8  = src->stat.r8;
	dest->stat.r9  = src->stat.r9;
	dest->stat.r10 = src->stat.r10;
	dest->stat.r11 = src->stat.r11;
	dest->stat.r12 = src->stat.r12;
	dest->stat.r13 = src->stat.r13;
	dest->stat.r14 = src->stat.r14;
	dest->stat.r15 = src->stat.r15;
	dest->stat.rip = src->stat.rip;
	dest->stat.rflags = src->stat.rflags;
}

void fork_process(process_t *p)
{
	debug("Forking procss [%s]\n", p->name);
	process_t *new_process = kmalloc(sizeof(process_t));
	memcpy(new_process, p, sizeof(process_t));
	new_process->name = strcat(p->name, " {fork}");
	new_process->parent = p;
	new_process->pdpt = get_pdpt();
	new_process->size = p->size;
	new_process->pid = get_pid();
	new_process->fds = p->fds;
	new_process->status = READY;
	
	map_mem_user(new_process->pdpt, 0x0, p->size);
	map_mem_user(new_process->pdpt, 0x7FC0000000 - 0x8000, 0x8000);	// Stack
	
	uint8_t *buf = kmalloc( (p->size + 1) * 0x1000 );
	uint8_t *stack = kmalloc( 0x9000 );

	uint64_t i;
	// Copying heap
	for(i = 0; i < p->size * 0x1000; ++i)
		*(buf+i) = *(uint8_t*)i;
	// Copying stack	
	for(i = 0; i < 0x8000; ++i)
		*(stack+i) = *(uint8_t*)(0x7FC0000000 - 0x8000 + i);	

	switch_pdpt(new_process->pdpt);
	
	// Restoring heap
	for(i = 0; i < p->size * 0x1000; ++i)
		*(uint8_t*)i = *(buf+i);
	// Restoring stack	
	for(i = 0; i < 0x8000; ++i)
		*(uint8_t*)(0x7FC0000000 - 0x8000 + i) = *(stack+i);	
		
		
	kfree(buf);
	kfree(stack);

	// Copying process state
	copy_process_stat(new_process, p);
	
	current_process->stat.rax = new_process->pid;
	new_process->stat.rax = 0;

	schedule_process(new_process);
	kernel_idle();
	//switch_process(current_process);
	//switch_pdpt(current_process->pdpt);
}

void exec_process(uint8_t *path)
{
	file_t *file = vfs_fopen(path, NULL);
	elf_t  *elf  = parse_elf(file);

	//process_t *proc = kmalloc(sizeof(process_t));	
	//proc->pdpt = get_pdpt();
	kfree(current_process->name);
	current_process->name = strdup(path);
	//proc->pid  = get_pid();
	
	uint32_t size = elf->text_size + elf->data_size;
	current_process->size = size/0x1000 + (size%0x1000?1:0);
	current_process->heap = current_process->size;
	
	// TODO : unmap extra allocated virtual address space
	map_mem_user(current_process->pdpt, 0x0, size );
	map_mem_user(current_process->pdpt, 0x7FC0000000 - 0x8000, 0x8000);	// Stack
	
	uint64_t cur_pdpt = switch_pdpt(current_process->pdpt);

	// let's load the file at 0x0
	uint8_t *load = 0x0;
	uint32_t i;
	for(i = 0; i < size; ++i)
		*(load+i) = elf->file->buf[elf->text_off + i];
	
	kfree(file->buf);
	kfree(file);
	kfree(elf);

	current_process->stat.rip = 0;
	current_process->stat.rsp = 0x7FC0000000;

	switch_pdpt(cur_pdpt);
	kernel_idle();
}

uint32_t get_fd(process_t *proc)
{
	uint32_t i;
	for(i = 0; i < proc->fds.len; ++i)
	{
		if(!proc->fds.ent[i]) return i;
	}
	return proc->fds.len++;
}

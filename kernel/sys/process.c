#include <system.h>
#include <process.h>
#include <vfs.h>
#include <debug.h>
#include <elf.h>
#include <kmem.h>
#include <fpu.h>
#include <signal.h>
#include <string.h>

extern process_t *current_process;

typedef struct
{
	uint8_t  *buf;
	uint64_t entry;
	uint32_t text_off;
	uint32_t text_size;
	uint64_t text_addr;
	uint32_t data_off;
	uint32_t data_size;
	uint64_t data_addr;
} elf_t;

elf_t *parse_elf(inode_t *file)
{
	uint8_t *buf = kmalloc(file->size);
	file->fs->read(file, 0, file->size, buf);
	
	elf_t *ret = kmalloc(sizeof(elf_t));
	elf_hdr_t *hdr = (elf_hdr_t*)buf;
	elf_section_hdr_t *text = (elf_section_hdr_t*)(buf + hdr->shoff + hdr->shentsize);
	elf_section_hdr_t *data = (elf_section_hdr_t*)(buf + hdr->shoff + 2 * hdr->shentsize);
	*ret = 
		(elf_t)
		{
			.buf = buf,
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
		*init_pd   = (uint64_t*)(0x7FFFFFF000 + 0x8 * dindex - 0x8), 
		*init_tbl  = (uint64_t*)(0x7FFFE00000 + 0x8 * tindex - 0x8),
		*init_page = (uint64_t*)(0x7FC0000000 + 0x8 * pindex - 0x8);
	
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
	memset(ptr, 0, size);
	
	uint32_t pages = size/0x1000 + (size%0x1000?1:0);
	uint32_t tbls = pages/0x200 + (pages%0x200?1:0);
	uint32_t pds = tbls/0x200 + (tbls%0x200?1:0);

	uint32_t pindex = ((uint64_t)ptr) / 0x1000;
	uint32_t tindex = pindex/0x200;
	uint32_t dindex = tindex/0x200;
	
	//debug("Index [%d]\n", index);
	
	uint64_t
		*init_pd   = (uint64_t*)(0x7FFFFFF000 + 0x8 * dindex - 0x8),
		*init_tbl  = (uint64_t*)(0x7FFFE00000 + 0x8 * tindex - 0x8),
		*init_page = (uint64_t*)(0x7FC0000000 + 0x8 * pindex - 0x8);
	
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
	
	unmap_mem_user(p->pdpt, 0, p->size * 0x1000);
	unmap_mem_user(p->pdpt, 
		(uint64_t*)(USER_STACK - USER_STACK_SIZE), USER_STACK_SIZE);
	
	//for(;;);
	
	if(p->parent)
		signal_send(p->parent, SIGCHLD);

	//mman.set(p->pdpt);
	extern void	deschedule_process(process_t*);
	deschedule_process(p);
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
	memset((void*)0x1000, 0, 0x1000 - 0x8);
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
	inode_t *file = vfs_trace_path(vfs_root, filename);
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
	proc->fds.ent = kmalloc(5 * sizeof(process_file_t));

	map_mem_user(proc->pdpt, 0x0, size ); // Heap
	map_mem_user(proc->pdpt, 
		(uint64_t*)(USER_STACK - USER_STACK_SIZE), USER_STACK_SIZE); // Stack
	
	uint64_t cur_pdpt = switch_pdpt(proc->pdpt);

	// let's load the file at 0x0
	uint8_t *load = 0x0;
	uint32_t i;
	for(i = 0; i < size; ++i)
		*(load+i) = elf->buf[elf->text_off + i];
	
	kfree(elf->buf);
	kfree(elf);

	proc->stat.rip = 0;
	proc->stat.rsp = USER_STACK;

	switch_pdpt(cur_pdpt);
	return proc;
}

void switch_process(process_t *p)
{
	if(current_process->fstat)
		save_fpu();
		
	if(!p->fstat)
		disable_fpu();
	
	debug("Switching process\n");
	debug("RIP  %lx\n", p->stat.rip);
	switch_pdpt(p->pdpt);
	if(p->sigqueue) // Any pending signals !?
	{
		void *stack = (void*)p->stat.rsp;
		stack -= sizeof(stat_t);
		memcpy(stack, &p->stat, sizeof(stat_t));
		stack -= sizeof(uint64_t);
		*(uint64_t*)stack = -1;
		p->stat.rsp = (uint64_t)stack;
		signal_queue_t *q = p->sigqueue;
		p->stat.rip = (uint64_t)p->handlers[q->signum].sa_handler;
		p->sigqueue = q->next;
		kfree(q);
	}
	switch_context(&p->stat);
}

void copy_process_stat(process_t *dest, process_t *src)
{
	memcpy(&dest->stat, &src->stat, sizeof(stat_t));
}

void fork_process(process_t *p)
{
	debug("Forking procss [%s]\n", p->name);
	process_t *new_process = kmalloc(sizeof(process_t));
	memcpy(new_process, p, sizeof(process_t));
	new_process->name = strcat(p->name, " (fork)");
	new_process->parent = p;
	new_process->pdpt = get_pdpt();

	debug("%lx\n", new_process->pdpt);
	
	new_process->size = p->size;
	new_process->pid = get_pid();
	new_process->fds = p->fds;
	new_process->fds.ent = kmalloc(new_process->fds.max_len * sizeof(process_file_t*));
	memcpy(new_process->fds.ent, p->fds.ent, p->fds.max_len * sizeof(process_file_t*));
	new_process->status = READY;
	
	map_mem_user(new_process->pdpt, 0x0, p->size); // Heap
	map_mem_user(new_process->pdpt, 
		(uint64_t*)(USER_STACK - USER_STACK_SIZE), USER_STACK_SIZE); // Stack
	
	uint8_t *buf = kmalloc( (p->size + 1) * 0x1000 );
	uint8_t *stack = kmalloc( USER_STACK_SIZE + 0x1000 );

	uint64_t i;
	// Copying heap
	for(i = 0; i < p->size * 0x1000; ++i)
		*(buf+i) = *(uint8_t*)i;
	// Copying stack	
	for(i = 0; i < 0x8000; ++i)
		*(stack+i) = *(uint8_t*)(USER_STACK - USER_STACK_SIZE + i);	
	
	switch_pdpt(new_process->pdpt);
	
	// Restoring heap
	for(i = 0; i < p->size * 0x1000; ++i)
		*(uint8_t*)i = *(buf+i);
	// Restoring stack	
	for(i = 0; i < USER_STACK_SIZE; ++i)
		*(uint8_t*)(USER_STACK - USER_STACK_SIZE + i) = *(stack+i);	
		
	kfree(buf);
	kfree(stack);

	// Copying process state
	copy_process_stat(new_process, p);
	
	current_process->stat.rax = new_process->pid;
	new_process->stat.rax = 0;

	schedule_process(new_process);
	kernel_idle();
}

void exec_process(uint8_t *path)
{
	inode_t *file = vfs_trace_path(vfs_root, path);
	if(!file) return;
	elf_t   *elf  = parse_elf(file);

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
	map_mem_user(current_process->pdpt, 
		(uint64_t*)(USER_STACK - USER_STACK_SIZE), USER_STACK_SIZE); // Stack
	
	uint64_t cur_pdpt = switch_pdpt(current_process->pdpt);

	// let's load the file at 0x0
	uint8_t *load = 0x0;
	uint32_t i;
	for(i = 0; i < size; ++i)
		*(load+i) = elf->buf[elf->text_off + i];
	
	kfree(elf->buf);
	kfree(elf);

	current_process->stat.rip = 0;
	current_process->stat.rsp = USER_STACK;

	switch_pdpt(cur_pdpt);
	kernel_idle();
}

uint32_t get_fd(process_t *proc)
{
	uint32_t i;
	for(i = 0; i < proc->fds.len; ++i)
	{
		if(!proc->fds.ent[i].inode) return i;
	}
	return proc->fds.len++;
}

uint32_t validate(process_t *proc, void *ptr)
{
	if( (uint64_t)ptr <= proc->heap * 0x1000 || 
		((uint64_t)ptr <= USER_STACK && (uint64_t)ptr >= (USER_STACK - USER_STACK_SIZE)))
			return 1;
	return 0;
}

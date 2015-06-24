#include <system.h>
#include <process.h>
#include <scheduler.h>
#include <debug.h>
#include <isr.h>
#include <sys/types.h>

typedef struct 
{
	uint32_t count;
	process_t *head;
	process_t *tail;
} process_queue_t;

process_queue_t process_queue;
process_t *current_process;

void dump_processes()
{
	debug("\n");
	process_t *p = process_queue.head;
	while(p)
	{
		debug("#%lx [%s]\n", p, p->name);
		p = p->next;
	}
	debug("\n");
}

void spawn_init(process_t *init)
{
	init->status = READY;
	init->prev = NULL;
	init->next = NULL;
	process_queue.head = init;
	process_queue.tail	= init;
	process_queue.count = 1;
	current_process = init;
	
	init->fstat = NULL;
	//schedule_process(init);
	switch_process(init);
	//kernel_idle();
}

uint8_t kidle = 0;

void schedule(regs_t *regs)
{
	extern void irq_ack(uint32_t);
	irq_ack(0);
	static uint32_t x = 0;
	//if(x++%32) return;
	
	debug("Scheduling\n");	
	dump_processes();
	if(!kidle)
	{
		current_process->stat.rax = regs->rax;
		current_process->stat.rdx = regs->rdx;
		current_process->stat.rcx = regs->rcx;
		current_process->stat.rbx = regs->rbx;
		current_process->stat.rsp = regs->rsp;
		current_process->stat.rbp = regs->rbp;
		current_process->stat.rsi = regs->rsi;
		current_process->stat.rdi = regs->rdi;
		current_process->stat.r8  = regs->r8;
		current_process->stat.r9  = regs->r9;
		current_process->stat.r10 = regs->r10;
		current_process->stat.r11 = regs->r11;
		current_process->stat.r12 = regs->r12;
		current_process->stat.r13 = regs->r13;
		current_process->stat.r14 = regs->r14;
		current_process->stat.r15 = regs->r15;
		current_process->stat.rip = regs->rip;
		current_process->stat.rflags = regs->rflags;
	} else kidle = 0;
	
	process_t *p = current_process?current_process->next:NULL;
	if(p)
	{
		while(p && p->status != READY) p = p->next;
		// Now we either got a ready process or the last process
		if(p && p->status == READY)
		{
			debug("Found next %lx [%s]\n", p, p->name);
			switch_process(current_process = p);
		}
	}
	
	// Otherwise we should just loop
	if(process_queue.head)
	{
		p = process_queue.head;
		while(p && p->status != READY) p = p->next;
		// Now we either got a ready process or the last process
		if(p && p->status == READY)
		{
			debug("Loop %lx [%s]\n", p, p->name);
			switch_process(current_process = p);
		}
	}
	
	// Otherwise we got nothing so we should just idle
	kernel_idle();
}

process_t *process_by_pid(pid_t pid)
{
	process_t *p = process_queue.head;
	while(p && p->pid != pid) p = p->next;
	return p;
}

void schedule_process(process_t *p)
{
	debug("Added [%s] to queue\n", p->name);
	p->next = NULL;
	p->prev = process_queue.tail;
	process_queue.count++;
	process_queue.tail->next = p;
	process_queue.tail = p;
}


void deschedule_process(process_t *p)
{
	if(p->prev)
	{
		if(p->next)
			p->prev->next = p->next;
		else if(process_queue.tail == p)
		{
			p->prev->next = NULL;
			process_queue.tail = p->prev;
		}
	}
	else if (process_queue.head == p)
	{
		process_queue.head = p->next;
		p->next->prev = NULL;
	}
	else 
	{
		// Where the hell did you get this process from !!?
	}
}

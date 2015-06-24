#include <system.h>
#include <syscall.h>
#include <process.h>
#include <debug.h>
#include <vfs.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

void kernel_idle()
{
	extern uint8_t kidle;
	kidle = 1;
	for(;;) asm("sti; hlt;");
}

SYS(sys_exit)	// 0
{
	debug("Exiting\n");
	exit_process(current_process);
	kernel_idle();
}

SYS(sys_open)	// rbx path, rcx flags
{
	if(!validate(current_process, (void*)rbx))
	{
		current_process->stat.rax = -1;
		kernel_idle();
	}
	uint32_t fd = get_fd(current_process);
	debug("Openning file %s [%s:%d]\n", rbx, current_process->name, fd);
	current_process->fds.ent[fd].inode = vfs_trace_path(vfs_root, (uint8_t*)rbx);
	current_process->fds.ent[fd].offset = 0;
	current_process->stat.rax = fd;
	kernel_idle();
}

SYS(sys_read)	// rbx fd, rcx buf, rdx len
{
	if( rbx > current_process->fds.len			||
		!current_process->fds.ent[rbx].inode	||
		!validate(current_process, (void*)rcx))
	{
		current_process->stat.rax = -1;
		kernel_idle();
	}
		
	debug("Reading fd:%d #%lx, [%d]\n", rbx, rcx, rdx);
	inode_t *inode = current_process->fds.ent[rbx].inode;
	debug("inode %lx\n", inode);
	uint32_t offset = current_process->fds.ent[rbx].offset;
	vfs_read(inode, offset, rdx, (void*)rcx);
	kernel_idle();
}

SYS(sys_write)	// rbx fd, rcx buf, rdx len
{
	if( rbx > current_process->fds.len			||
		!current_process->fds.ent[rbx].inode	||
		!validate(current_process, (void*)rcx))
	{
		current_process->stat.rax = -1;
		kernel_idle();
	}
			
	inode_t *inode = current_process->fds.ent[rbx].inode;
	debug("Wrtiting to inode %lx\n", inode);
	vfs_write(inode, (void*)rcx, rdx);
	kernel_idle();
}

SYS(sys_close)	// rbx fd
{
	if(rbx > current_process->fds.len) 
	{
		current_process->stat.rax = -1;
		kernel_idle();
	}
	
	current_process->fds.ent[rbx].inode = NULL;
	current_process->fds.ent[rbx].offset = 0;
	current_process->stat.rax = 0;
	
	kernel_idle();
}

SYS(sys_fork)
{
	extern void fork_process(process_t*);
	fork_process(current_process);
}

SYS(sys_execv)	// rbx path
{
	extern void exec_process(uint8_t*);
	uint8_t *p = strdup((uint8_t*)rbx);
	debug("execv %s\n", p);
	exec_process(p);
	current_process->stat.rax = -1;
	kernel_idle();
}

SYS(sys_sbrk)	// rbx size
{
	debug("Allocating %d B for process %s\n", rbx, current_process->name);
	uint64_t ret = current_process->heap;
	if(current_process->size - current_process->heap > rbx)
	{
		current_process->heap += rbx;
		current_process->stat.rax = ret;
		kernel_idle();
	}
	
	uint64_t req_size = rbx - (current_process->size - current_process->heap);
	uint64_t req_size_pages = req_size + (req_size%0x1000?0x1000:0);
	map_mem_user(current_process->pdpt, current_process->size, req_size_pages);
	current_process->size += req_size_pages;
	current_process->heap += req_size;
	current_process->stat.rax = ret;
	kernel_idle();
}

SYS(sys_getpid)
{
	current_process->stat.rax = current_process->pid;
	kernel_idle();
}

SYS(sys_wait)	// rbx status
{
	debug("Waiting for childern @%s\n", current_process->name);
	current_process->status = WAITING;
	//deschedule_process(current_process->pid);
	kernel_idle();
}

SYS(sys_print)
{
	debug("&%lx\n", rbx);
	//debug("%s\n", rbx);
	for(;;);
}

SYS(sys_yield)
{
	debug("Yielding\n");
	kernel_idle();
}

SYS(sys_kill)
{
	debug("Signal\n");
	signal_send_by_pid((pid_t)rbx, (signal_num_t)rcx);
	kernel_idle();
}

SYS(sys_signal)	// rbx signum, rcx signal pointer
{
	if(!validate(current_process, (void*)rcx))
	{
		current_process->stat.rax = -1;
		kernel_idle();
	}
	
	debug("Registering signal handler %d->%lx\n", rbx, rcx);
	void *old_handler = &current_process->handlers[(signal_num_t)rbx];
	current_process->handlers[(signal_num_t)rbx].sa_handler = (void*)rcx;
	current_process->stat.rax = (uint64_t)old_handler;
	kernel_idle();
}

void *sys_calls[] = 
{
	sys_exit,
	sys_open,
	sys_read,
	sys_write,
	sys_close,
	sys_fork,
	sys_execv,
	sys_sbrk,
	sys_getpid,
	sys_wait,
	//sys_print,
	//sys_yield,
	sys_kill,
	sys_signal,
};


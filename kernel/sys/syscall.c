#include <system.h>
#include <syscall.h>
#include <process.h>
#include <debug.h>
#include <vfs.h>

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
	uint32_t fd = get_fd(current_process);
	current_process->fds.ent[fd] = vfs_trace_path(vfs_root, rbx);
	current_process->stat.rax = fd;
	kernel_idle();
}

SYS(sys_read)	// rbx fd, rcx buf, rdx len
{
	debug("FS %s\n", current_process->fds.ent[rbx]->fs->name);
	debug("Reading fd:%lx #%lx, [%lx]\n", rbx, rcx, rdx);
	inode_t *inode = current_process->fds.ent[rbx];
	vfs_read(inode, (void*)rcx, rdx);
	kernel_idle();
}

SYS(sys_write)	// rbx fd, rcx buf, rdx len
{
	inode_t *inode = current_process->fds.ent[rbx];
	vfs_write(inode, rcx, rdx);
	kernel_idle();
}

SYS(sys_close)	// rbx fd
{
	current_process->fds.ent[rbx] = NULL;
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
	uint8_t *p = strdup(rbx);
	debug("SS %s [%d]\n", p, strlen(p));
	exec_process(p);
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
	debug("PID %d\n", current_process->pid);
	kernel_idle();
}

SYS(sys_wait)	// rbx status
{
	debug("Waiting for childern @%s\n", current_process->name);
	current_process->status = WAITING_CHILD;
	//deschedule_process(current_process->pid);
	kernel_idle();
}

SYS(sys_print)
{
	debug("&%lx\n", rbx);
	debug("%s\n", rbx);
	for(;;);
}

SYS(sys_yield)
{
	debug("Yielding\n");
	kernel_idle();
}

void* sys_calls[] = 
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
	sys_print,
	//sys_yield,
};


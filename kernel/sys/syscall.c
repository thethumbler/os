#include <system.h>
#include <syscall.h>
#include <process.h>
#include <debug.h>
#include <vfs.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <pit.h>
#include <kmem.h>

void kernel_idle()
{
	// FIXME : This should minimize I/O wait time but for processes invoking
	// 		   many consequent syscalls this will keep other processes starving
	if(current_process && current_process->status == READY) 
		switch_process(current_process);

	extern uint8_t kidle;
	kidle = 1;
	for(;;) 
	{
		schedule();
		asm("sti; hlt; cli;");
	}
}


#define RETURN(val) \
	{	\
		current_process->stat.rax = val; \
		kernel_idle(); \
	} \

SYS(sys_exit)	// 0
{
	debug("Exiting\n");
	exit_process(current_process);
	//kernel_idle(); // We should not idle here
	extern uint8_t kidle;
	kidle = 1;
	schedule();
}

SYS(sys_open)	// rbx path, rcx flags
{
	if(!validate(current_process, (void*)rbx)) RETURN(-1);
	
	uint32_t fd = get_fd(current_process);
	debug("Openning file %s [%s:%d]\n", rbx, current_process->name, fd);
	inode_t *i = vfs_trace_path(vfs_root, (uint8_t*)rbx);
	if(i)
	{
		if(i->type == FS_MOUNTPOINT)
		{
			debug("Openning mount point\n");
			i = ((vfs_mountpoint_t*)i->p)->inode;
		}
		
		current_process->fds.ent[fd].inode = i;
		current_process->fds.ent[fd].offset = 0;
		current_process->stat.rax = fd;
	}
	else
		current_process->stat.rax = -1;
		
	kernel_idle();
}

SYS(sys_read)	// rbx fd, rcx buf, rdx len
{
	if( rbx > current_process->fds.len			||
		!current_process->fds.ent[rbx].inode	||
		!validate(current_process, (void*)rcx))	RETURN(-1);

	debug("Reading fd:%d #%lx, [%d]\n", rbx, rcx, rdx);
	inode_t *inode = current_process->fds.ent[rbx].inode;
	debug("inode %lx\n", inode);
	uint32_t offset = current_process->fds.ent[rbx].offset;
	uint32_t size = vfs_read(inode, offset, rdx, (void*)rcx);
	current_process->fds.ent[rbx].offset += size;
	RETURN(size);
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
			
	inode_t *inode  = current_process->fds.ent[rbx].inode;
	uint64_t offset = current_process->fds.ent[rbx].offset;
	debug("Wrtiting to inode %lx\n", inode);
	
	int len = vfs_write(inode, offset, rdx, (void*)rcx);
	RETURN(len);
}

SYS(sys_lseek)	// rbx fd, rcx offset, rdx whence
{
	if( rbx > current_process->fds.len			||
		!current_process->fds.ent[rbx].inode)	RETURN(-1);
	
	switch(rdx)
	{
		case 0:	//SEEK_SET
			current_process->fds.ent[rbx].offset = rcx;
			break;
		case 1: //SEEK_CUR
			current_process->fds.ent[rbx].offset += rcx;
			break;
		case 2: //SEEK_END
			current_process->fds.ent[rbx].offset = current_process->fds.ent[rbx].inode->size;
			break;
		default:
			current_process->stat.rax = -1;
			kernel_idle();
	}
	RETURN(current_process->fds.ent[rbx].offset);
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
	RETURN(0);
}

SYS(sys_fork)
{
	fork_process(current_process);
}

SYS(sys_execve)	// rbx path, rcx argv, rdx env
{
	if(rbx)
	{
		uint8_t *p = strdup((uint8_t*)rbx);
		debug("execve(%s, %lx, %lx)\n", p, rcx, rdx);
		exec_process(p, (uint8_t**)rcx, (uint8_t**)rdx);
	}
	RETURN(-1);
}

SYS(sys_sbrk)	// rbx size
{
	debug("Allocating %d B for process %s\n", rbx, current_process->name);
	if(!rbx) RETURN(current_process->heap);
	
	//if(current_process->name[0] == 'm') for(;;);
	uint64_t ret = current_process->heap;
	
	if(current_process->size * 0x1000 - current_process->heap > rbx)
	{
		current_process->heap += rbx;
		RETURN(ret);
	}
	
	uint32_t req_size = rbx - (current_process->size * 0x1000 - current_process->heap);
	uint32_t req_size_pages = req_size/0x1000 + (req_size%0x1000?1:0);

#if _LAZYALLOC_
	map_mem_user(current_process->pdpt, 
		(uint64_t*)(current_process->size * 0x1000), req_size_pages * 0x1000);
#endif

	current_process->size += req_size_pages;
	current_process->heap += req_size;
	RETURN(ret);
}

SYS(sys_getpid)
{
	RETURN(current_process->pid);
}

SYS(sys_wait)	// rbx status
{
	debug("Waiting for childern @%s\n", current_process->name);
	current_process->status = WAITING;
	//deschedule_process(current_process->pid);
	kernel_idle();
}

SYS(sys_kill)
{
	debug("Signal\n");
	signal_send_by_pid((pid_t)rbx, (signal_num_t)rcx);
	RETURN(0);	//TODO Actuall checking for signal success
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
	RETURN((uint64_t)old_handler);
}

SYS(sys_usleep)	// rbx time in us
{
	debug("uSleep [%d us] on process %s\n", rbx, current_process->name);
	current_process->wait_us = rbx;
	current_process->ticks   = ticks;
	current_process->sub_ticks = sub_ticks;
	current_process->status = WAITING;
	kernel_idle();
}

SYS(sys_getatt)	// rbx attribute, rcx buf
{
	switch(rbx)
	{
		case 0:	// get args length
			current_process->stat.rax = current_process->argslen;
			break;
		case 1:	// get args into buffer
			if(!validate(current_process, (void*)rcx))
			{
				current_process->stat.rax = -1;
				kernel_idle();
			}
			memcpy((void*)rcx, current_process->args, current_process->argslen);
			kernel_idle();
			break;
		case 2:	// get envs length
			current_process->stat.rax = current_process->envslen;
			break;
		case 3:	// get envs into buffer
			if(!validate(current_process, (void*)rcx))
			{
				current_process->stat.rax = -1;
				kernel_idle();
			}
			memcpy((void*)rcx, current_process->envs, current_process->envslen);
			kernel_idle();
			break;
		default:
			current_process->stat.rax = -1;
	}
	kernel_idle();
}

SYS(sys_readdir)	// rbx fd, rcx index, rdx dirent
{	
	if(	 rbx > current_process->fds.len 		||
		 !current_process->fds.ent[rbx].inode	||
		 !validate(current_process, (void*)rdx)
	  );

	inode_t *inode = current_process->fds.ent[rbx].inode;

	dentry_t *dentry = inode->list;
	if(!dentry) { current_process->stat.rax = -1; kernel_idle(); }
	
	uint32_t index = rcx;
	if(index >= dentry->count) { current_process->stat.rax = -1; kernel_idle(); }

	inode_t *i = dentry->head;
	while(i && i->next && index--) i = i->next;

	if(i->name)
	{
		struct dirent *dir = (struct dirent *)rdx;
		memcpy(dir->d_name, i->name, strlen(i->name) + 1);
		current_process->stat.rax = 0;
	} else
		current_process->stat.rax = -1;
	kernel_idle();
}

SYS(sys_getcwd)	// rbx buf, rcx size
{
	if(!validate(current_process, (void*)rbx) || rcx < strlen(current_process->cwd))
	{
		current_process->stat.rax = 0;	// NULL
		kernel_idle();
	}
	
	memcpy(rbx, current_process->cwd, strlen(current_process->cwd));
	RETURN(rbx);
}

SYS(sys_chdir)	//rbx path
{
	if(!validate(current_process, (void*)rbx) || !vfs_trace_path(vfs_root, (uint8_t*)rbx))
		RETURN(-1);
	
	if(current_process->cwd) kfree(current_process->cwd);
	if(((uint8_t*)rbx)[strlen((uint8_t*)rbx)-1] == '/')
	{
		current_process->cwd = (uint8_t*)kmalloc(strlen((uint8_t*)rbx));
		memcpy(current_process->cwd, rbx, strlen((uint8_t*)rbx) + 1);
	} else
	{
		uint32_t len = strlen((uint8_t*)rbx);
		current_process->cwd = (uint8_t*)kmalloc(len + 2);
		memcpy(current_process->cwd, rbx, len + 1);
		current_process->cwd[len] = '/';
		current_process->cwd[len + 1] = '\0';
	}
	RETURN(0);
}

SYS(sys_mount)	// rbx filesystem, rcx dst, rdx src
{
	if(	!validate(current_process, (void*)rbx) 	||
		!validate(current_process, (void*)rcx) 	||
		!vfs_trace_path(vfs_root, (uint8_t*)rcx)||
		!(rdx ? validate(current_process, (void*)rdx) : 1)
		) RETURN(-1);

	fs_t *fs = fsman.getfs((uint8_t*)rbx);
	if(!fs) RETURN(-2);

	inode_t *src = rdx ? vfs_trace_path(vfs_root, (uint8_t*)rdx) : NULL;
	inode_t *dst = vfs_trace_path(vfs_root, (uint8_t*)rcx);
	if(!dst) RETURN(-3);

	RETURN(fs->mount(dst, src));
}

void *sys_calls[] = 
{
	sys_exit,
	sys_open,
	sys_read,
	sys_write,
	sys_close,
	sys_fork,
	sys_execve,
	sys_sbrk,
	sys_getpid,
	sys_wait,
	sys_kill,
	sys_signal,
	sys_usleep,
	sys_getatt,
	sys_lseek,
	sys_readdir,
	sys_getcwd,
	sys_chdir,
	sys_mount,
};


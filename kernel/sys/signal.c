#include <system.h>
#include <signal.h>
#include <process.h>
#include <vfs.h>
#include <string.h>
#include <sys/types.h>
#include <kmem.h>

int signal_default_action[] =
{
	IGN,	//0
	TRM,	//SIGHUP
	TRM,	//SIGINT
	TRM,	//SIGQUIT
	TRM,	//SIGILL
	TRM,	//SIGTRAP
	TRM,	//SIGABRT
	TRM,	//SIGBUS
	TRM,	//SIGFPE
	TRM,	//SIGKILL
	IGN,	//SIGUSR1
	TRM,	//SIGSEGV
	IGN,	//SIGUSR2
	TRM,	//SIGPIPE
	TRM,	//SIGALRM
	IGN,	//__UNUSED__
	CNT,	//SIGCHLD 	IGN XXX
	CNT,	//SIGCONT
	STP,	//SIGSTOP
	STP,	//SIGTSTP
	STP,	//SIGTTIN
	STP,	//SIGTTOU
};

char *signal_names[] = 
{
	"SIGHUP",
	"SIGINT",	
	"SIGQUIT",	
	"SIGILL",	
	"SIGTRAP",	
	"SIGABRT",	
	"SIGBUS",	
	"SIGFPE",	
	"SIGKILL",	
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"__UNUSED__",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
};

void signal_send(process_t *p, signal_num_t signal)
{	
	inode_t *p_stdout = p->fds.len > 2 ? p->fds.ent[1].inode : NULL;
	
	uint8_t *msg;
	
	if(p->handlers[signal].sa_handler)
	{
		if((uint64_t)p->handlers[signal].sa_handler == 1)	// Ignore
		{
			debug("Ignoring signal\n");
			return;
		}
		else if((uint64_t)p->handlers[signal].sa_handler == -1)	// Error ( terminate )
		{
			if(p_stdout) 
			{
				msg = strcat(strcat("Process terminated [", signal_names[signal - 1]), "]\n");
				vfs_write(p_stdout, 0, strlen(msg), msg);
			}
			exit_process(p);
			return;
		}
		debug("Queuing signal\n");
		if(!p->sigqueue) 
		{
			signal_queue_t *q = p->sigqueue = kmalloc(sizeof(signal_queue_t));
			q->signum = signal;
			q->next = NULL;
		}
		else
		{
			signal_queue_t *q = p->sigqueue;
			while(q->next) q = q->next;
			q->next = kmalloc(sizeof(signal_queue_t));
			q = q->next;
			q->signum = signal;
		}
	}
	
	else
	{
		int action = signal_default_action[signal];
		switch(action)
		{
			case IGN: break;
			case TRM:
				if(p_stdout) 
				{
					msg = strcat(strcat("Process terminated [", signal_names[signal - 1]), "]\n");
					vfs_write(p_stdout, 0, strlen(msg), msg);
				}
				exit_process(p);
				break;
			case STP:
				p->status = WAITING;
				break;
			case CNT:
				p->status = READY;
				break;
		}
	}
}

void signal_send_by_pid(pid_t pid, signal_num_t signal)
{
		process_t *p = process_by_pid(pid);
		if(!p) return;
		signal_send(p, signal);
}

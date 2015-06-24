#include <system.h>
#include <process.h>
#include <scheduler.h>
#include <debug.h>
#include <kmem.h>

process_t *last_fpu_proc = NULL;	// The last process that used the FPU
uint8_t fpu_state = 0;

uint8_t fstat[512] __attribute__((aligned(16)));

void save_fpu()
{
	debug("Saving FPU [%s]\n", current_process->name);
	asm("fxsave %0"::"m"(fstat));
	memcpy(current_process->fstat, &fstat, 512);
}

void restore_fpu()
{
	debug("Restoring FPU [%s]\n", current_process->name);
	memcpy(&fstat, current_process->fstat, 512);
	asm("fxrstor %0"::"m"(fstat));
}

void trap_fpu()
{
	debug("Traping FPU [%s]\n", current_process->name);
	enable_fpu();
	if(last_fpu_proc == current_process)
		return;

	last_fpu_proc = current_process;
	if(!current_process->fstat)
	{
		asm("fninit;");
		current_process->fstat = kmalloc(512);
		return;
	}
	
	restore_fpu();
}

void enable_fpu()
{
	if(fpu_state) return; 
	fpu_state = 1;
	//debug("Enabling FPU [%s]\n", current_process->name);
	/* Enable SSE */
	/* Sets CR4.OSFXSR & CR4.OSXMMEXCPT */
	asm("clts; \
		 movq %cr4, %rax; \
		 or   $0x600, %rax; \
		 movq %rax, %cr4;"
		);	 
}

void disable_fpu()
{
	if(!fpu_state) return;
	fpu_state = 0;
	debug("Disabling FPU [%s]\n", current_process->name);
	/* Disable all FPU & SSE instructions */
	/* Sets CR0.EM ( Emulation bit ) so all FPU instructions trap to kernel */
	asm("movq %cr0, %rax; \
		 and  $~0x4, %rax; \
		 movq %rax, %cr0;"
		);
}

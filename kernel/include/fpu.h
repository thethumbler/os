#ifndef _FPU_H
#define _FPU_H

#include <system.h>
void save_fpu();
void restore_fpu();
void trap_fpu();
void enable_fpu();
void disable_fpu();

#endif

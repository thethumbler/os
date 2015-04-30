#ifndef _KBD_H
#define _KBD_H

#include <system.h>
#include <isr.h>

void kbd_load(void*);
void kbd_handler(regs_t*);

#endif

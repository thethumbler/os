#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <config.h>
#include <utypes.h>
#include <debug.h>

#define NULL ((void*)0)
#define MIN(a, b) ((a<b)?a:b)
#define MAX(a, b) ((a>b)?a:b)

#if _DEBUG_ > 0
#define assert(level, var, msg) \
	do{ \
		if(!var) \
			debug("%s @%s[%d] : '%s' %s\n", __FILE__, __FUNCTION__, \
					__LINE__, #var, msg); \
		if(level) for(;;); \
	}while(0);
#else 
#define assert(level, var, msg)
#endif

#endif

#ifndef _DEBUG_H
#define _DEBUG_H

#if _DEBUG_ > 0
	void debug(uint8_t *, ...);
#else
	#define debug(...)
#endif

#endif

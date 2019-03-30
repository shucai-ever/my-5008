#ifndef _PI5008_BOOT_DEBUG
#define _PI5008_BOOT_DEBUG
#include <stdio.h>
#include "uart.h"

#ifdef DEBUG_PRINT
#define dbg(x...)	{ printf("[%s:%d] ", __FILE__, __LINE__); \
								printf(x); }
#else
#define dbg(x...)	(void)(0)
#endif


#endif

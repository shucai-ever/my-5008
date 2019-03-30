#ifndef _PI5008_DEBUG_H
#define _PI5008_DEBUG_H
#include <stdio.h>
#include "type.h"
#include "task_uartcon.h"

/* log information en/disable */
/* ex : [DEBG] func:line num: format */
#define LOG_PRINT_MORE_INFO                (0)

////////////////////////////////////////////////////////////////////////////////
// LOG Print level 
#define LOG_LVL_NONE		 		0
#define LOG_LVL_CRITICAL			1
#define LOG_LVL_WARNING				2
#define LOG_LVL_NOTICE				3
#define LOG_LVL_LOG					4
#define LOG_LVL_DEBUG				5

extern unsigned char log_run_level; //defined on debug.c. is possible define on user module.

extern const char * log_level_strings []; //const defined on debug.c

#define LOG_SHOULD_I( level ) ( level <= LOG_BUILD_LEVEL && level <= log_run_level && LOG_BUILD_LEVEL != LOG_LVL_NONE)

#if LOG_PRINT_MORE_INFO
#define LOG(level, fmt, arg...) do {   \
    if ( LOG_SHOULD_I(level) ) { \
    	AppUARTCon_DebugPrint("[%s] %s:%d: " fmt, log_level_strings[level], __FUNCTION__,__LINE__, ##arg); \
    } \
} while(0)

#define LOG_ISR(level, fmt, arg...) do {   \
    if ( LOG_SHOULD_I(level) ) { \
    	AppUARTCon_DebugPrintFromISR("[%s] %s:%d: " fmt, log_level_strings[level], __FUNCTION__,__LINE__, ##arg); \
    } \
} while(0)

#else
#define LOG(level, fmt, arg...) do {   \
    if ( LOG_SHOULD_I(level) ) { \
		AppUARTCon_DebugPrint(fmt, ##arg); \
    } \
} while(0)

#define LOG_ISR(level, fmt, arg...) do {   \
    if ( LOG_SHOULD_I(level) ) { \
		AppUARTCon_DebugPrintFromISR(fmt, ##arg); \
    } \
} while(0)

#endif

#define LOG_DEBUG( fmt, arg... )     LOG( LOG_LVL_DEBUG, fmt, ##arg )
#define LOG_LOG( fmt, arg... )       LOG( LOG_LVL_LOG, fmt,##arg )
#define LOG_NOTICE( fmt,arg... )     LOG( LOG_LVL_NOTICE, fmt, ##arg )
#define LOG_WARNING( fmt, arg... )   LOG( LOG_LVL_WARNING, fmt, ##arg )
#define LOG_CRITICAL( fmt, arg... )  LOG( LOG_LVL_CRITICAL, fmt, ##arg )

#define LOG_DEBUG_ISR( fmt, arg... )     LOG_ISR( LOG_LVL_DEBUG, fmt, ##arg )
#define LOG_LOG_ISR( fmt, arg... )       LOG_ISR( LOG_LVL_LOG, fmt,##arg )
#define LOG_NOTICE_ISR( fmt,arg... )     LOG_ISR( LOG_LVL_NOTICE, fmt, ##arg )
#define LOG_WARNING_ISR( fmt, arg... )   LOG_ISR( LOG_LVL_WARNING, fmt, ##arg )
#define LOG_CRITICAL_ISR( fmt, arg... )  LOG_ISR( LOG_LVL_CRITICAL, fmt, ##arg )
// TODO
// LOG_ISR

#define ASSERT(cond)								\
{										\
	if (!(cond)){								\
		printf("Failed assertion in %s:\n"				\
				"%s at %s\n"					\
				"line %d\n"					\
				"RA=%lx\n",					\
				__func__,					\
				#cond,						\
				__FILE__,					\
				__LINE__,					\
				(unsigned long)__builtin_return_address(0));	\
		while (1)							\
			;							\
	}									\
}

#define PANIC(args)		\
{				\
	printf("ERROR:"args);	\
	while (1) ;		\
}

static inline void dump_mem(const void *mem, int count)
{
	const unsigned char *p = mem;
	int i = 0;

	for(i = 0; i < count; i++){

		if( i % 16 == 0)
			printf("\n");

		printf("%02x ", p[i]);
	}
}

/* help to trace back */
static inline void dump_stack(void)
{
        unsigned long *stack;
        unsigned long addr;

	__asm__ __volatile__ ("\tori\t%0, $sp, #0\n" : "=r" (stack));
	printf("Call Trace:\n");
	addr = *stack;
        while (addr) {
                addr = *stack++;
                printf("[<%08lx>] ", addr);
//                print_symbol("%s\n", addr);
	}
	printf("\n");
	return;
}


void print_hex(uint8 *buf, uint32 size);
void print_hexw(uint32 *buf, uint32 size);


#endif

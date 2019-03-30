#ifndef __PI5008_CONFIG_H__
#define __PI5008_CONFIG_H__

/*
 * Select Platform
 */
#define CONFIG_PLAT_PI5008 1

/*
 * Platform Option
 */
#define VECTOR_BASE 0x20100000

#define VECTOR_NUMINTRS 32
#define NO_EXTERNAL_INT_CTL 1
//#define XIP_MODE 1
#define LOAD_MODE 1

#undef CONFIG_HW_PRIO_SUPPORT

/*
 * Cache Option
 */
#if (!defined(__NDS32_ISA_V3M__)  && defined(CONFIG_CACHE_SUPPORT))
#define CONFIG_CPU_ICACHE_ENABLE 1
#define CONFIG_CPU_DCACHE_ENABLE 1
//#define CONFIG_CPU_DCACHE_WRITETHROUGH 1
#endif

#undef CONFIG_CHECK_RANGE_ALIGNMENT
#undef CONFIG_CACHE_L2
#undef CONFIG_FULL_ASSOC

/*
 * ZOL Options (do ZOL supporting when CPU supports ZOL)
 */
#define	CFG_HWZOL
#define CONFIG_HWZOL

/*
 * App config
 */
#define IRQ_STACK_SIZE            5120		/* IRQ stack size */
#define CONFIG_MINIMAL_STACK_SIZE 0x1000

#include "pi5008.h"

#endif /* __PI5008_CONFIG_H__ */

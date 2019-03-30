#ifndef __PVIRX_H__
#define __PVIRX_H__

#include "pvirx_support.h"
#include "pvirx_table.h"
#include "pvirx_drvcommon.h"
#include "pvirx_user_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds32_intrinsic.h>
#include "type.h"
#include "error.h"
#include "nds32.h"

#ifndef NULL
#define NULL  		(0)
#endif
#undef TRUE
#define TRUE  		(1)
#undef FALSE
#define FALSE  		(0)
#undef ENABLE
#define ENABLE 		(1)
#undef DISABLE
#define DISABLE		(0)

#undef START
#define START           (1)
#undef STOP
#define STOP            (0)
#undef HIGH
#define HIGH            (1)
#undef LOW
#define LOW             (0)

#define COMMA ,

#ifndef _VER_PVIRX
#define _VER_PVIRX "1_000"
#endif //_VER_PVIRX

#define _SET_BIT(bit, pData) (*pData|=(1<<bit))
#define _CLEAR_BIT(bit, pData) (*pData&=~(1<<bit))
#define _TEST_BIT(bit, pData) ((*pData&(1<<bit))?1:0)
#define WAKE_UP_INTERRUPTIBLE(pData) (*pData=1)
#define DO_DIV(x,y) ((uint64_t)x/(uint64_t)y)
#define SPIN_LOCK_INIT(pData) (*pData=0)
#define SPIN_LOCK_IRQSAVE(pData, flag) {while(*pData==1);*pData=1;}
#define SPIN_UNLOCK_IRQRESTORE(pData, flag) (*pData=0)
typedef volatile int SPINLOCK_T;
typedef	volatile int WAITQHEAD_T;
#define ZALLOC(pMem) malloc(pMem)
#define FREE(pMem) free(pMem)

// port architecture
#ifndef  mdelay //No use OS function. because pvirx driver executed before OS schedule.
#ifdef FPGA_ASIC_TOP 
#define mdelay(x) {unsigned int count=x*10000;while(count-->0){asm volatile ("nop");}}
#else
#define mdelay(x) {unsigned int count=x*10000;while(count-->0){asm volatile ("nop");}}
#endif // FPGA_ASIC_TOP 
#endif

/* define value */
#define POLLIN          0x0001
#define POLLPRI         0x0002
#define POLLOUT         0x0004
#define POLLERR         0x0008
#define POLLHUP         0x0010
#define POLLNVAL        0x0020
/* The rest seem to be more-or-less nonstandard. Check them! */
#define POLLRDNORM      0x0040
#define POLLRDBAND      0x0080
#define POLLWRNORM      0x0100
#define POLLWRBAND      0x0200
#define POLLMSG         0x0400
#define POLLREMOVE      0x1000
#define POLLRDHUP       0x2000

#define MAX_CNT_TUNN_CHROMALOCK              (10)                              

////////////////////////////////////////////////////////////////////////
typedef struct {
	uint8_t			u8Initialized;

	uint16_t		chipID;
	uint8_t			u8RevID;

	WAITQHEAD_T 		wqPoll;

	_stPVIRX_Isr		stPVIRX_Isr;
	_stPrRxMode 		stPrRxMode;
#ifdef SUPPORT_PVIRX_UTC
	_stUTCCmd		stUTCRecvCmd;
#endif // SUPPORT_PVIRX_UTC
#ifdef SUPPORT_PVIRX_CEQ
#endif // SUPPORT_PVIRX_CEQ
	//////////////////////////////////////////////
}_PviRxDrvHost;
extern _PviRxDrvHost *gpPviRxDrvHost[MAX_PVIRX_CHANCNT];
extern uint8_t gbPviRxSelSD960H;

//////////////////////////////////////////////////////////////////

PP_U32 PVIRX_poll(PP_S32 chipInx);
PP_RESULT_E PVIRX_proc(void);

extern _PviRxDrvHost gPviRxDrvHost[MAX_PVIRX_CHANCNT];

#define PrPrint(string, args) {printf("[PVI_RX] ");printf(string,args); printf("\r");}
#define PrError(string, args) {printf("%c[0;31;47m [PVI_RX] ### ERR ### :",27);printf(string,args);printf("%c[0m 0;31m\r",27);}
//#define PrPrint(string, args)
//#define PrError(string, args)
#ifdef SUPPORT_PVIRX_DBG_PRINT
#define PrDbg(string, args) {printf("[PVI_RX]dbg> ");printf(string,args);printf("\r");}
#else
#define PrDbg(string, args)
#endif
#define PrPrintString(string) {printf("[PVI_RX] ");printf(string); printf("\r");}
#define PrErrorString(string) {printf("%c[0;31;47m [PVI_RX] ### ERR ### :",27);printf(string);printf("%c[0m 0;31m\r",27);}
//#define PrPrintString(string)
//#define PrErrorString(string)
#ifdef SUPPORT_PVIRX_DBG_PRINT
#define PrDbgString(string) {printf("[PVI_RX]dbg> ");printf(string);printf("\r");}
#else
#define PrDbgString(string)
#endif

#endif /* __PVIRX_H__ */



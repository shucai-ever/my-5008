#ifndef __PVITX_H__
#define __PVITX_H__

#include "pvirx_support.h"
#include "pvitx_table.h"
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

#ifndef _VER_PVITX
#define _VER_PVITX "1_000"
#endif //_VER_PVITX

#define _SET_BIT(bit, pData) (*pData|=(1<<bit))
#define _CLEAR_BIT(bit, pData) (*pData&=~(1<<bit))
#define _TEST_BIT(bit, pData) ((*pData&(1<<bit))?1:0)
#define WAKE_UP_INTERRUPTIBLE(pData) (*pData=1)
#define DO_DIV(x,y) ((uint64_t)x/(uint64_t)y)
#define SPIN_LOCK_INIT(pData) (*pData=0)
#define SPIN_LOCK_IRQSAVE(pData, flag) {while(*pData==1);*pData=1;}
#define SPIN_UNLOCK_IRQRESTORE(pData, flag) (*pData=0)

// port architecture
#ifndef  mdelay
#ifdef FPGA_ASIC_TOP 
#define mdelay(x) {unsigned int count=x*40000;while(count-->0){asm volatile ("nop");}}
#else
#define mdelay(x) {unsigned int count=x*40000;while(count-->0){asm volatile ("nop");}}
#endif // FPGA_ASIC_TOP 
#endif


extern int gPviTxType;
extern int gPviTxResol;

#define PtPrint(string, args) {printf("[PVI_TX] ");printf(string,args); printf("\r");}
#define PtError(string, args) {printf("%c[0;31;47m [PVI_TX] ### ERR ### :",27);printf(string,args);printf("%c[0m 0;31m\r",27);}
//#define PtPrint(string, args)
//#define PtError(string, args)
#ifdef SUPPORT_PVITX_DBG_PRINT
#define  PtDbg(string, args) {printf("[PVI_TX]dbg> ");printf(string,args);printf("\r");}
//#define PtDbg(string, args)
#else
#define  PtDbg(string, args)
#endif
#define  PtPrintString(string) {printf("[PVI_TX] ");printf(string); printf("\r");}
#define  PtErrorString(string) {printf("%c[0;31;47m [PVI_TX] ### ERR ### :",27);printf(string);printf("%c[0m 0;31m\r",27);}
//#define PtPrintString(string)
//#define PtErrorString(string)
#ifdef SUPPORT_PVITX_DBG_PRINT
#define  PtDbgString(string) {printf("[PVI_TX]dbg> ");printf(string);printf("\r");}
#else
#define  PtDbgString(string)
#endif

////////////////////////////////////////////////////////////////////////
typedef struct {
	PP_U8			u8Initialized;
	
	PP_U16		u16ChipID;
	PP_U8			u8RevID;

	PP_S32 			pviTxType;
	PP_S32 			pviTxResol;
	//////////////////////////////////////////////
}_PviTxDrvHost;
extern _PviTxDrvHost *gpPviTxDrvHost;

//////////////////////////////////////////////////////////////////

PP_RESULT_E PVITX_proc(void);

#endif /* __PVITX_H__ */



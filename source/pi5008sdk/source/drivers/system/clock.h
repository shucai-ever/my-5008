
#ifndef _SYSTEM_CLOCK_H_
#define _SYSTEM_CLOCK_H_
#include "type.h"

void ClockInit(void);
uint32 GetFPLLFreq(void);
uint32 GetMPLLFreq(int num);
uint32 GetCPUFreq(void);
uint32 GetDDRAXIFreq(void);
uint32 GetAPBFreq(void);
uint32 GetTimerFreq(void);
uint32 GetWDTFreq(void);
uint32 GetUARTFreq(void);

extern unsigned int u32CPUClk;
extern unsigned int u32DDRClk;
extern unsigned int u32APBClk;
extern unsigned int u32TimerClk;
extern unsigned int u32WDTClk;
extern unsigned int u32UARTClk;
#endif /* _SYSTEM_CLOCK_H_ */

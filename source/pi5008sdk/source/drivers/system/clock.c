#include "system.h"
#include "type.h"
#include "clock.h"


unsigned int u32CPUClk;
unsigned int u32DDRClk;
unsigned int u32APBClk;
unsigned int u32TimerClk;
unsigned int u32WDTClk;
unsigned int u32UARTClk;

#if !defined(FPGA_ASIC_TOP)

void ClockInit(void)
{
	u32CPUClk = GetCPUFreq();
	u32DDRClk = GetDDRAXIFreq() * 2;
	u32APBClk = GetAPBFreq();
	u32TimerClk = GetTimerFreq();
	u32WDTClk = GetWDTFreq();
	u32UARTClk = GetUARTFreq();
}

uint32 GetFPLLFreq(void)
{
	uint32 m,n;
	uint32 clk;
	vuint32 reg;

	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x50);

	m = ((reg >> 7) & 1) + 1;
	n = reg & 0x7f;

	clk = (XIN * n) / m;

	return clk;
}

uint32 GetMPLLFreq(int num)
{
	uint32 m,n;
	uint32 clk;
	vuint32 reg;

	if(num < 0 || num > 1) return 0;

	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x54 + num*4);

	m = ((reg >> 26) & 3) + 1;
	n = ((reg >> 20) & 0x3f);

	clk = (XIN * n) / m;

	return clk;
}

uint32 GetCPUFreq(void)
{
	vuint32 reg;
	sint32 div;
	uint32 out_clk;
	uint32 src_clk;

	//core cpu
	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x00);

	if((reg&0x8) == 0){
		out_clk = XIN;
	}else{
		src_clk = GetFPLLFreq();
		div = (reg & 0x7);
		if(div < 5){
			div = div + 2;
		}else if(div == 5){
			div = 8;
		}else if(div == 6){
			div = 1;
			src_clk = GetMPLLFreq(1);
		}else if(div == 7){
			div = 16;
			src_clk = XIN;
		}

		out_clk = src_clk / div;
	}

	return out_clk;
}

uint32 GetDDRAXIFreq(void)
{
	vuint32 reg;
	sint32 div;
	uint32 out_clk;
	uint32 src_clk;

	//core cpu
	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x00);

	if(((reg>>19)&1) == 0){
		out_clk = XIN;
	}else{
		src_clk = GetFPLLFreq();
		div = ((reg>>16) & 0x7);
		if(div < 5){
			div = div + 4;
		}else if(div == 5){
			div = 16;
		}else if(div == 6){
			div = 1;
			src_clk = GetMPLLFreq(1);
		}else if(div == 7){
			div = 16;
			src_clk = XIN;
		}
		out_clk = src_clk / div;

	}

	return out_clk;
}

uint32 GetAPBFreq(void)
{
	vuint32 reg;
	sint32 div;
	uint32 out_clk;
	uint32 src_clk;

	//core cpu
	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x00);

	if(((reg>>27)&1) == 0){
		out_clk = XIN;
	}else{
		src_clk = GetFPLLFreq();
		div = ((reg>>24) & 0x7);

		if(div == 0){
			div = 4;
			src_clk = GetMPLLFreq(1);
		}else if(div == 1){
			div = 8;
		}else if(div == 2){
			div = 16;
		}else if(div == 3){
			div = 20;
		}else if(div == 4){
			div = 32;
		}else if(div == 5){
			div = 16;
			src_clk = XIN;
		}

		out_clk = src_clk / div;
	}

	return out_clk;
}

uint32 GetTimerFreq(void)
{
	vuint32 reg;
	sint32 div;
	uint32 out_clk;
	uint32 src_clk;

	//core cpu
	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x04);

	if(((reg>>11)&1) == 0){
		out_clk = XIN;
	}else{
		src_clk = GetFPLLFreq();
		div = ((reg>>8) & 0x7);
		if(div == 0){
			div = 2;
			src_clk = XIN;
		}else if(div < 3){
			div = div*4;
			src_clk = XIN;
		}else if(div == 3){
			div = 128;
		}else if(div == 4){
			div = 16;
			src_clk = XIN;
		}
		out_clk = src_clk / div;

	}

	return out_clk;
}

uint32 GetWDTFreq(void)
{
	vuint32 reg;
	uint32 out_clk;

	//core cpu
	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x04);

	if(((reg>>19)&1) == 0){
		out_clk = XIN;
	}else{
		out_clk = GetTimerFreq();
	}

	return out_clk;
}


uint32 GetUARTFreq(void)
{
	vuint32 reg;
	sint32 div;
	uint32 out_clk;
	uint32 src_clk;

	//core cpu
	reg = *(vuint32 *)(SCU_BASE_ADDR + 0x04);

	if(((reg>>27)&1) == 0){
		out_clk = XIN;
	}else{
		src_clk = GetFPLLFreq();
		div = ((reg>>24) & 0x7);
		if(div == 0){
			div = 128;
		}else if(div == 1){
			div = 16;
			out_clk = XIN;

		}
		out_clk = src_clk / div;

	}

	return out_clk;
}


#endif

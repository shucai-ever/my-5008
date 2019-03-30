/*
 * i2s_api.c
 *
 *  Created on: 2017. 9. 25.
 *      Author: ihkong
 */

#include <stdio.h>
#include <unistd.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "i2s.h"
#include "audio_api.h"
#include "pinmux.h"
#include "proc.h"

static uint32 old_pinmux[4];
static uint32 old_io[2];

//PROC irq
extern PP_RESULT_E I2S_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stI2S_PROC_irqs[] = {
	    { .fn = I2S_PROC_IRQ,	.irqNum = IRQ_I2S_VECTOR,    .next = (void*)0 },
};
//PROC device
extern PP_RESULT_E I2S_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stI2S_PROC_devices[] = {
	    { .pName = "I2S",   .fn = I2S_PROC_DEVICE,    .next = (void*)0 },
};

static void audio_isr_handler(void)
{
	//LOG_DEBUG("audio callback\n");
	// Todo

}
static void audio_set_pinmux(uint32 isMaster)
{
	// backup pinmux
	old_pinmux[0] = get_pinmux(0, 4);	// I2S_MCLK
	old_pinmux[1] = get_pinmux(0, 8);	// I2S_OUT_CLK
	old_pinmux[2] = get_pinmux(0, 10);	// I2S_OUT_SYN
	old_pinmux[3] = get_pinmux(0, 12);	// I2S_OUT_DAT

	// backup io type
	old_io[0] = *(vuint32 *)(MISC_BASE_ADDR + 0);
	old_io[1] = *(vuint32 *)(MISC_BASE_ADDR + 4);

	// set pinmux
	set_pinmux(0, 4, PINMUX_0_I2SM_CLK);
	set_pinmux(0, 8, PINMUX_0_I2S_OUT_CLK);
	set_pinmux(0, 10, PINMUX_0_I2S_OUT_SYN);
	set_pinmux(0, 12, PINMUX_0_I2S_OUT_DAT);

	// set io type
	*(vuint32 *)(MISC_BASE_ADDR + 0) = ((old_io[0] & 0xff00ffff) | 0x10000);	// MCLK -> input
	if(isMaster){	// Master
		*(vuint32 *)(MISC_BASE_ADDR + 4) = ((old_io[1] & 0xff000000) | 0x20202);	// DATA, Bit Clk, L/R Clk  -> output
	}else{			// Slave
		*(vuint32 *)(MISC_BASE_ADDR + 4) = ((old_io[1] & 0xff000000) | 0x20101);	// DATA -> out, Bit Clk, L/R Clk  -> input
	}

}

static void audio_restore_pinmux(void)
{
	set_pinmux(0, 4, old_pinmux[0]);
	set_pinmux(0, 8, old_pinmux[1]);
	set_pinmux(0, 10, old_pinmux[2]);
	set_pinmux(0, 12, old_pinmux[3]);

	*(vuint32 *)(MISC_BASE_ADDR + 0) = old_io[0];
	*(vuint32 *)(MISC_BASE_ADDR + 4) = old_io[1];
}

PP_RESULT_E PPAPI_AUDIO_Initialize(PP_I2S_INIT_S IN *pstInit)
{
	PP_RESULT_E ret = eSUCCESS;

	audio_set_pinmux(pstInit->u32IsMaster);
	if((ret = PPDRV_I2S_Initialize(pstInit)) != eSUCCESS)return ret;
	PPDRV_I2S_SetISR(audio_isr_handler);

    //register proc infomation.
	SYS_PROC_addIrq(stI2S_PROC_irqs);
	SYS_PROC_addDevice(stI2S_PROC_devices);

	return ret;
}

PP_RESULT_E PPAPI_AUDIO_Play(PP_U8 IN *pu8Buf, PP_U32 IN u32Size)
{
	PP_RESULT_E enRet = eSUCCESS;

	// Todo: Not implemented yet

	return enRet;
}

PP_VOID PPAPI_AUDIO_Stop(PP_VOID)
{

	// Todo: Not implemented yet

}

PP_VOID PPAPI_AUDIO_Pause(PP_VOID)
{

	// Todo: Not implemented yet

}

PP_VOID PPAPI_AUDIO_Resume(PP_VOID)
{

	// Todo: Not implemented yet

}

PP_AUDIO_STATUS_E PPAPI_AUDIO_GetPlayStatus(PP_VOID)
{
	PP_AUDIO_STATUS_E enRet = eSTATUS_STOP;

	// Todo: Not implemented yet

	return enRet;
}




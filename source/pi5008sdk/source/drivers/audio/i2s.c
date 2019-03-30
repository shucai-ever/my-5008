/*
 * i2s.c
 *
 *  Created on: 2017. 4. 6.
 *      Author: ihkong
 */

#include <stdio.h>

#include "osal.h"
#include "i2s.h"
#include "type.h"
#include "utils.h"
#include "debug.h"
#include "pi5008.h"
#include "interrupt.h"
#include "proc.h"

/* I2S interface control register - I2SENCON */
#define DAI_TXENA					(0x1)
#define DAT_RXENA					(0x1<<4)

/* I2S IRQ Pending register */
#define IRQ_PND_OVER_RX				(1<<6)
#define IRQ_PND_UNDER_TX			(1<<2)

/* I2S IRQ Enable register */
#define IRQ_EN_RX					(1<<6)
#define IRQ_EN_TX					(1<<2)

typedef struct ppI2S_REG_S
{
	vuint32	DAICON; 			// 0x000 - Control register
	vuint32 DAIENCON;			// 0x004 - Channel enable register
	vuint32 DAITXBASE;			// 0x008 - Tx buffer base address register
	vuint32 DAITXWPT;			// 0x00c - Tx buffer write pointer
	vuint32	DAITXRPT;			// 0x010 - Tx buffer read pointer
	vuint32	DAIRXWPT;			// 0x014 - Rx buffer write pointer
	vuint32	DAIRXRPT;			// 0x018 - Rx buffer read pointer
	vuint32	RSV[12];
	vuint32	DAIBUFMARG;			// 0x04c - buffer overflow/under-run margin register
	vuint32	DAIRXBASE;			// 0x050 - Rx buffer base address register
	vuint32	RSV1[3];
	vuint32	DAIIRQPND;			// 0x060 - Interrupt pending register
	vuint32	DAIIRQEN;			// 0x064 - Interrupt enable register
	vuint32	DAIMCUCLKDIV;		// 0x068 - MCU clock divider
	vuint32	DAIDCNTEND;			// 0x06c - Delay count end

} PP_I2S_REG_S;

#define I2S_CTRL_REG			((PP_I2S_REG_S*)			(I2S_BASE_ADDR))

static I2S_ISR_CALLBACK gI2S_ISR;
static PP_I2S_REG_S *gpI2S = I2S_CTRL_REG;
static uint8 *gpPCMBuff;

static uint32 gu32I2SIntcCnt = 0;

ISR(i2s_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpI2S->DAIIRQPND;

	//LOG_DEBUG("rp: 0x%x, wp: 0x%x\n", (uint32)gpI2S->DAITXRPT, (uint32)gpI2S->DAITXWPT);
	if(reg & IRQ_PND_UNDER_TX){
		if(gI2S_ISR)gI2S_ISR();

		//LOG_DEBUG("*(volatile unsigned int *)0x%08x |= 0x%x	// ISR\n", (unsigned int)&gpI2S->DAIIRQPND, IRQ_PND_UNDER_TX);
		gpI2S->DAIIRQPND |= IRQ_PND_UNDER_TX;

	}

	gu32I2SIntcCnt++;

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

PP_RESULT_E PPDRV_I2S_Initialize(PP_I2S_INIT_S IN *pstInit)
{
	PP_RESULT_E ret = eSUCCESS;

    OSAL_register_isr(IRQ_I2S_VECTOR, i2s_isr, NULL);
    INTC_irq_clean(IRQ_I2S_VECTOR);
    INTC_irq_enable(IRQ_I2S_VECTOR);

    gI2S_ISR = NULL;

    //LOG_DEBUG("*(volatile unsigned int *)0x%08x = 0x%x // reset DAI\n", (unsigned int)&gpI2S->DAICON, 1);
    gpI2S->DAICON = 1;	// reset DAI

    gpI2S->DAICON = ( ( (pstInit->u32IsMaster ? 0 : 1) << 1 )	|		// external clock - slave
					( (pstInit->enSampleFreqRatio & 3) << 2) |			// RFS
    				( (pstInit->enDataFormat & 3) << 4 ) |				// BFS
					( (pstInit->enMLBFirst & 1) << 9 ) |				// MLF
					( (pstInit->enBufferSize & 3) << 12 ) |				// SIZE
					( (pstInit->enDataFormat & 3) << 14 ) );			// DFS

    gpPCMBuff = OSAL_malloc( (1<<(12 + pstInit->enBufferSize)) );
    if(!gpPCMBuff){
    	LOG_CRITICAL("I2S malloc fail\n");
    	ret = eERROR_FAILURE;
    	goto END_FT;
    }
    //LOG_DEBUG("*(volatile unsigned int *)0x%08x = 0x%x // set txbase\n", (unsigned int)&gpI2S->DAITXBASE, (uint32)gpPCMBuff);
    gpI2S->DAITXBASE = (uint32)gpPCMBuff;

    //LOG_DEBUG("*(volatile unsigned int *)0x%08x = 0x%x // set txwpt\n", (unsigned int)&gpI2S->DAITXWPT, 0);
    gpI2S->DAITXWPT = 0;

    //LOG_DEBUG("*(volatile unsigned int *)0x%08x = 0x%x // set bufmarg\n", (unsigned int)&gpI2S->DAIBUFMARG, (uint32)utilPutBits( gpI2S->DAIBUFMARG, 0, 16, pInit->UnderMargin));
    gpI2S->DAIBUFMARG = (uint32)utilPutBits( gpI2S->DAIBUFMARG, 0, 16, pstInit->u32UnderMargin );

END_FT:
	return ret;
}

PP_VOID PPDRV_I2S_SetISR(I2S_ISR_CALLBACK IN cbISR)
{
	gI2S_ISR = cbISR;
}

PP_VOID PPDRV_I2S_WriteEnable(PP_S32 IN s32Enable)
{
	if(s32Enable){

		gpI2S->DAIIRQPND |= IRQ_PND_UNDER_TX;	// clear irq pending
		gpI2S->DAIENCON |= DAI_TXENA;			// tx enable
		gpI2S->DAIIRQEN |= IRQ_EN_TX;			// tx irq enable
	}else{
		gpI2S->DAIIRQEN &= (~IRQ_EN_TX);			// tx irq disable
		gpI2S->DAIENCON &= (~DAI_TXENA);			// tx disable
	}
}

PP_VOID PPDRV_I2S_ResetBufPtr(PP_VOID)
{
	gpI2S->DAITXWPT = 0;
	gpI2S->DAIRXRPT = 0;
}

PP_VOID PPDRV_I2S_WriteBuf(PP_U8 IN *pu8Buf, PP_U32 IN u32Size)
{
	uint32 tog;
	uint32 wp;
	uint32 buf_size;
	sint32 i;
	uint8 *base_addr;

	buf_size = ( 1 << (((gpI2S->DAICON >> 12) & 3) + 12) );

	base_addr = (uint8 *)gpI2S->DAITXBASE;
	base_addr = (uint8 *)ADDR_NON_CACHEABLE((uint32)base_addr);

	wp = gpI2S->DAITXWPT;
	tog = (wp & (1<<31));
	wp &= (~(1<<31));

	for(i=0;i<u32Size;i++){
		*(base_addr + wp) = pu8Buf[i];
		wp++;
		if(wp >= buf_size){
			//LOG_DEBUG("wp: 0x%x, buf size: 0x%x\n", wp, buf_size);
			wp = 0;
			tog ^= (1<<31);
		}

	}

	gpI2S->DAITXWPT = (wp | tog);

}

PP_S32 PPDRV_I2S_GetWriteBuffAvailSize(PP_VOID)
{
	sint32 ret = 1;
	uint32 rp;
	uint32 wp;
	uint32 buf_size;

	rp = gpI2S->DAITXRPT;
	wp = gpI2S->DAITXWPT;
	buf_size = ( 1 << (((gpI2S->DAICON >> 12) & 3) + 12) );

	if( (rp & (1<<31)) == (wp & (1<<31)) ){
		if(wp >= rp)
			ret = buf_size - (wp - rp);
		else
			ret = -1;	//
	}else{
		if(rp >= wp)
			ret = rp - wp;
		else
			ret = -1;
	}

	return ret;

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PP_RESULT_E I2S_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT, INTERVAL_TIME)

	PRINT_PROC_IRQ("I2S", IRQ_I2S_VECTOR, gu32I2SIntcCnt);

	return(eSUCCESS);
}

extern struct proc_device_struct stI2S_PROC_devices[];
PP_RESULT_E I2S_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	vuint32 reg;

	if( (argc) && (strcmp(argv[0], stI2S_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}

	printf("\n%s Device Info -------------\n", stI2S_PROC_devices[0].pName);

	/* TODO add device info printf */

	reg = gpI2S->DAICON;
	LOG_DEBUG("Master/Slave   : %s\n", ((reg >> 1)&1) ? "Slave" : "Master");
	LOG_DEBUG("MCLK freq      : %s\n", ((reg >> 2)&3) == 0 ? "256fs" : (((reg >> 2)&3) == 1 ? "384fs" : (((reg >> 2)&3) == 2 ? "512fs" : "768fs")) );
	LOG_DEBUG("Data bits      : %s\n", ((reg >> 4)&3) == 0 ? "16bit" : (((reg >> 4)&3) == 2) ? "32bit" : "unknown"  );
	LOG_DEBUG("LR polarity    : %s\n", ((reg >> 8)&1) == 0 ? "Left ch high" : "Right ch high");
	LOG_DEBUG("MLF            : %s\n", ((reg >> 9)&1) == 0 ? "MSB first" : "LSB first");
	LOG_DEBUG("Buffer size    : %s\n", ((reg >> 12)&3) == 0 ? "4KB" : (((reg >> 12)&3) == 1 ? "8KB" : (((reg >> 12)&3) == 2 ? "16KB" : "32KB")) );
	LOG_DEBUG("Tx enable      : %d\n", (gpI2S->DAIENCON&1));
	LOG_DEBUG("Buffer base    : 0x%x\n", gpI2S->DAITXBASE);
	LOG_DEBUG("Tx wp          : 0x%x\n", gpI2S->DAITXWPT);
	LOG_DEBUG("Tx rp          : 0x%x\n", gpI2S->DAITXRPT);
	LOG_DEBUG("Tx under margin: 0x%x\n", gpI2S->DAIBUFMARG & 0xffff);
	LOG_DEBUG("IRQ enable     : %d\n", (gpI2S->DAIIRQEN >> 2)&1);


	return(eSUCCESS);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



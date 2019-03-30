
#include "osal.h"
#include "system.h"
#include "pi5008.h"
#include "interrupt.h"
#include "utils.h"
#include "gpio.h"


typedef struct tagGPIO_REG_T
{
	vuint32 id;								/* 0x000 		- ID and revision register */
	vuint32 rsv0[3];						/* 0x004 ~ 0x00c	- reserved */
	vuint32 cfg;							/* 0x010			- Configuration register */
	vuint32 rsv1[3];						/* 0x014 ~ 0x01c	- reserved */
	vuint32 din;							/* 0x020			- Channel data in register */
	vuint32 dout;							/* 0x024			- Channel data out register */
	vuint32 dir;							/* 0x028			- Channel direction register */
	vuint32 dout_clr;						/* 0x02c			- Channel data out clear register */
	vuint32 dout_set;						/* 0x030			- Channel data out set register */
	vuint32 rsv2[3];						/* 0x034 ~ 0x03c	- reserved */
	vuint32 pull_enable;					/* 0x040			- Channel pull enable register */
	vuint32 pull_type;						/* 0x044			- Channel pull type register */
	vuint32 rsv3[2];						/* 0x048 ~ 0x04c	- reserved */
	vuint32 irq_enable;						/* 0x050			- Interrupt enable register */
	vuint32 irq_mode0;						/* 0x054			- Interrupt mode0 register */
	vuint32 irq_mode1;						/* 0x058			- Interrupt mode1 register */
	vuint32 irq_mode2;						/* 0x05c			- Interrupt mode2 register */
	vuint32 irq_mode3;						/* 0x060			- Interrupt mode3 register */
	vuint32 irq_status;						/* 0x064			- Interrupt status register */
	vuint32 rsv4[2];						/* 0x068 ~ 0x06c	- reserved */
	vuint32 debounce_en;					/* 0x070			- Debounce enable register*/
	vuint32 debounce_ctrl;					/* 0x074			- Debounce control register */
	vuint32 rsv5[2];						/* 0x078 ~ 0x07c	- reserved */
}GPIO_REG_T;

#define GPIO0_CTRL_REG			((GPIO_REG_T*)			(GPIO0_BASE_ADDR))
#define GPIO1_CTRL_REG			((GPIO_REG_T*)			(GPIO1_BASE_ADDR))

// interrupt
#define GPIO_INTERRUPT_DISABLE  (0)
#define GPIO_INTERRUPT_ENABLE   (1)


static GPIO_REG_T *gpGPIO[] = {
	GPIO0_CTRL_REG,
	GPIO1_CTRL_REG,
};

/* each pin has isr function */
static GPIO_ISR_CALLBACK gGPIO_ISR[MAX_GPIO_DEV_NUM][MAX_GPIO_PIN_NUM];

ISR(gpio_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	sint32 i,j;
	uint8 dev = 0;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	for(j=0; j<MAX_GPIO_DEV_NUM; j++)
    {
        if(IRQ_GPIO_STATUS_MASK(j) == FLAG_SET)
        {
            dev = j;
		    reg = gpGPIO[dev]->irq_status;
		    for(i=0; i<32; i++)
            {
			    if((reg >> i) & 1)
                {
				    if(gGPIO_ISR[dev][i])
                        gGPIO_ISR[dev][i](i);
			    }
		    }
		    //LOG_DEBUG_ISR("gpio irq status: 0x%x, 0x%x\n", (unsigned int)gpGPIO[0]->irq_status, (unsigned int)gpGPIO[1]->irq_status);
		    gpGPIO[dev]->irq_status = reg;
	    }	
    }

    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}


PP_VOID PPDRV_GPIO_Initialize(PP_VOID)
{
    GPIO_ISR_CALLBACK *old = NULL;
    uint32 i, j;

    OSAL_register_isr(IRQ_GPIO_VECTOR, gpio_isr, old);
    INTC_irq_clean(IRQ_GPIO_VECTOR); 
    INTC_irq_enable(IRQ_GPIO_VECTOR);

    /* Initialize GPIO pin & callback func */
	for(i=0; i<MAX_GPIO_DEV_NUM; i++)
    {
        // set default irq disable
	    gpGPIO[i]->irq_enable = (vuint32)GPIO_INTERRUPT_DISABLE;

	    // set default dir in
	    gpGPIO[i]->dir = (vuint32)eDIR_IN;

	    // set default irq mode - dual edge
	    gpGPIO[i]->irq_mode0 = 0x77777777;	// ch0 ~ 7
	    gpGPIO[i]->irq_mode1 = 0x77777777;	// ch8 ~ 15
	    gpGPIO[i]->irq_mode2 = 0x77777777;	// ch16 ~ 23
	    gpGPIO[i]->irq_mode3 = 0x77777777;	// ch24 ~ 31

	    gpGPIO[i]->debounce_ctrl |= (1<<31);	// pclk

        for(j=0; j<MAX_GPIO_PIN_NUM; j++)
        {
            gGPIO_ISR[i][j] = NULL;
        }
    }
}

PP_VOID PPDRV_GPIO_SetIntrMode(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_GPIO_INTR_MODE_E IN enMode)
{
    uint32 shiftCount;

    shiftCount = (s32Ch % 8) * 4;
    
    if(s32Ch < 8)
    {
        /* pin 0 ~ 7 interrupt mode */
    	gpGPIO[s32Dev]->irq_mode0 &= ~((0x7) << (shiftCount));
    	gpGPIO[s32Dev]->irq_mode0 |= ((enMode) << (shiftCount));
    }
    else if(s32Ch < 16)
    {
    	gpGPIO[s32Dev]->irq_mode1 &= ~((0x7) << (shiftCount));
    	gpGPIO[s32Dev]->irq_mode1 |= ((enMode) << (shiftCount));
    }
    else if(s32Ch < 24)
    {
    	gpGPIO[s32Dev]->irq_mode2 &= ~((0x7) << (shiftCount));
    	gpGPIO[s32Dev]->irq_mode2 |= ((enMode) << (shiftCount));
    }
    else if(s32Ch < 32)
    {
    	gpGPIO[s32Dev]->irq_mode3 &= ~((0x7) << (shiftCount));
    	gpGPIO[s32Dev]->irq_mode3 |= ((enMode) << (shiftCount));
    }
    else
    {
        /* Unknown pin_num */
    }
}


PP_VOID PPDRV_GPIO_SetDir(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_GPIO_DIR_E IN enDir, PP_S32 s32OutVal)
{
	if(eDIR_OUT == enDir){
		gpGPIO[s32Dev]->dout = utilPutBit(gpGPIO[s32Dev]->dout, s32Ch, s32OutVal);
	}
	gpGPIO[s32Dev]->dir = utilPutBit(gpGPIO[s32Dev]->dir, s32Ch, enDir);
}

PP_GPIO_DIR_E PPDRV_GPIO_GetDir(PP_S32 IN s32Dev, PP_S32 IN s32Ch)
{
	return ((gpGPIO[s32Dev]->dir >> s32Ch ) & 1);
}

PP_VOID PPDRV_GPIO_SetIRQEnable(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_S32 IN s32Enable)
{
	if(s32Enable)
		gpGPIO[s32Dev]->irq_enable = utilSetBit(gpGPIO[s32Dev]->irq_enable, s32Ch);
		
	else
		gpGPIO[s32Dev]->irq_enable = utilClearBit(gpGPIO[s32Dev]->irq_enable, s32Ch);
}

PP_S32 PPDRV_GPIO_GetIRQEnable(PP_S32 IN s32Dev, PP_S32 IN s32Ch)
{
	return ((gpGPIO[s32Dev]->irq_enable >> s32Ch) & 1);
}

PP_RESULT_E PPDRV_GPIO_SetValue(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_S32 IN s32Value)
{
	
	if(PPDRV_GPIO_GetDir(s32Dev, s32Ch) != eDIR_OUT)
		return eERROR_FAILURE;

	if(s32Value)
		gpGPIO[s32Dev]->dout = utilSetBit(gpGPIO[s32Dev]->dout, s32Ch);
	else
		gpGPIO[s32Dev]->dout = utilClearBit(gpGPIO[s32Dev]->dout, s32Ch);

	return eSUCCESS;
		
}


PP_S32 PPDRV_GPIO_GetOutValue(PP_S32 IN s32Dev, PP_S32 IN s32Ch)
{
	if(PPDRV_GPIO_GetDir(s32Dev, s32Ch) != eDIR_OUT)
		return -1;

	return ((gpGPIO[s32Dev]->dout >> s32Ch) & 1);

}


PP_S32 PPDRV_GPIO_GetValue(PP_S32 IN s32Dev, PP_S32 IN s32Ch)
{
	if(PPDRV_GPIO_GetDir(s32Dev, s32Ch) != eDIR_IN)
		return -1;

	return ((gpGPIO[s32Dev]->din >> s32Ch) & 1);
}

PP_VOID PPDRV_GPIO_SetISR(PP_S32 IN s32Dev, PP_S32 IN s32Ch, GPIO_ISR_CALLBACK IN cbISR)
{
	gGPIO_ISR[s32Dev][s32Ch] = cbISR;
}


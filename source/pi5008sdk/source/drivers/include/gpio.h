#ifndef _PI5008_GPIO_H
#define _PI5008_GPIO_H

#include "system.h"
#include "type.h"
#include "error.h"

// GPIO Device
#define GPIO_DEV_0      (0) 
#define GPIO_DEV_1      (1)


#define MAX_GPIO_DEV_NUM		2
#define MAX_GPIO_PIN_NUM        32

typedef enum ppGPIO_DIR_E
{
	eDIR_IN = 0,
	eDIR_OUT,
}PP_GPIO_DIR_E;

typedef enum ppGPIO_INTR_MODE_E
{
    eHIGH_LEVEL = 2,
    eLOW_LEVEL,
    eNEGATIVE_EDGE = 5,
    ePOSITIVE_EDGE,
    eDUAL_EDGE,
}PP_GPIO_INTR_MODE_E;


#if 0
typedef void (*GPIO_ISR) (sint32 num);

void gpio_init(void);
void gpio_interrupt_mode(sint32 pin_dev, sint32 pin_num, uint8 mode);
void gpio_set_dir(sint32 dev, sint32 pin, sint32 dir, sint32 out_val);
sint32 gpio_get_dir(sint32 dev, sint32 pin);
void gpio_set_irq_en(sint32 dev, sint32 pin, sint32 enable);
sint32 gpio_get_irq_en(sint32 dev, sint32 pin);
sint32 gpio_set_value(sint32 dev, sint32 pin, sint32 value);
sint32 gpio_get_out_value(sint32 dev, sint32 pin);
sint32 gpio_get_value(sint32 dev, sint32 pin);
void gpio_set_isr(sint32 dev, sint32 pin, GPIO_ISR isr);

void gpio_set_debounce_prescale(sint32 dev, uint8 prescale);
void gpio_set_debounce_en(sint32 dev, sint32 pin, uint32 enable);
#else

typedef PP_VOID (*GPIO_ISR_CALLBACK) (PP_S32 s32Num);

PP_VOID PPDRV_GPIO_Initialize(PP_VOID);
PP_VOID PPDRV_GPIO_SetISR(PP_S32 IN s32Dev, PP_S32 IN s32Ch, GPIO_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_GPIO_SetIntrMode(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_GPIO_INTR_MODE_E IN enMode);
PP_VOID PPDRV_GPIO_SetDir(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_GPIO_DIR_E IN enDir, PP_S32 s32OutVal);
PP_GPIO_DIR_E PPDRV_GPIO_GetDir(PP_S32 IN s32Dev, PP_S32 IN s32Ch);
PP_VOID PPDRV_GPIO_SetIRQEnable(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_S32 IN s32Enable);
PP_S32 PPDRV_GPIO_GetIRQEnable(PP_S32 IN s32Dev, PP_S32 IN s32Ch);
PP_RESULT_E PPDRV_GPIO_SetValue(PP_S32 IN s32Dev, PP_S32 IN s32Ch, PP_S32 IN s32Value);
PP_S32 PPDRV_GPIO_GetOutValue(PP_S32 IN s32Dev, PP_S32 IN s32Ch);
PP_S32 PPDRV_GPIO_GetValue(PP_S32 IN s32Dev, PP_S32 IN s32Ch);
#endif


#endif

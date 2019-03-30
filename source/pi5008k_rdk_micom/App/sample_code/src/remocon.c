/*
 * (C) COPYRIGHT 2011 CRZ
 *
 * File Name : remocon.c
 * 
 * History 
 *
 * when         who       what, where, why
 *--------------------------------------------------------------
 * 04/04/2012   sikim     Initial release
 */

/* includes */

#include "hw_config.h"

/* Defines */

//#define IR_RX_TIME_VAL_START (32400*2)
//#define IR_RX_TIME_VAL_0     (3960*2)
//#define IR_RX_TIME_VAL_1     (7920*2)


#define IR_RX_TIME_VAL_START (14800)
#define IR_RX_TIME_VAL_0     (12500)
#define IR_RX_TIME_VAL_1     (24210)



#define IR_RX_TIME_VAL_GUARD 500

#define IR_RX_1KEY_COUNT     32
#define IR_RX_ABNORMAL_KEY   0xFFFF

// _SEND_VAL define generation macro

#define IR_KEY_GEN(def_data) def_data##_SEND_VAL
#define IR_KEY_PRINT(def_data) printf("%s\n", #def_data)

// IR KEY Received Value

#define IR_KEY_TV_POWER  0x40BE629D

#define IR_KEY_TV_NUM_1  0xE0E020DF
#define IR_KEY_TV_NUM_2  0xE0E0A05F
#define IR_KEY_TV_NUM_3  0xE0E0609F
#define IR_KEY_TV_NUM_4  0xE0E010EF
#define IR_KEY_TV_NUM_5  0xE0E0906F
#define IR_KEY_TV_NUM_6  0xE0E050AF
#define IR_KEY_TV_NUM_7  0xE0E030CF
#define IR_KEY_TV_NUM_8  0xE0E0B04F
#define IR_KEY_TV_NUM_9  0xE0E0708F
#define IR_KEY_TV_NUM_0  0xE0E08877

#define IR_KEY_TV_MEM_DEL       0xE0E018E7
#define IR_KEY_TV_SLEEP_RESERV  0xE0E0C03F
#define IR_KEY_TV_CONFIG_MEMORY 0xE0E09867
#define IR_KEY_TV_AUTO_CHANNEL  0xE0E05CA3

#define IR_KEY_TV_VOL_PLUS      0xE0E0E01F
#define IR_KEY_TV_VOL_MINUS     0xE0E0D02F
#define IR_KEY_TV_CHANNEL_UP    0xE0E048B7
#define IR_KEY_TV_CHANNEL_DOWN  0xE0E008F7
#define IR_KEY_TV_SELECT_LIST   0xE0E058A7

#define IR_KEY_TV_MUTE     0xE0E0F00F
#define IR_KEY_TV_CONTROL  0xE0E0B847

#define IR_KEY_TV_TV_VIDEO 0xE0E0807F
#define IR_KEY_TV_TV_CABLE 0xE0E0E41B

#define IR_KEY_VIDEO_POWER   0x040440BF
#define IR_KEY_VIDEO_TV_VCR  0x0404807F

#define IR_KEY_VIDEO_REWIND       0x040418E7
#define IR_KEY_VIDEO_PLAY         0x04049867
#define IR_KEY_VIDEO_FASTFORWARD  0x040458A7

#define IR_KEY_VIDEO_VOL_UP       0x0404E01F
#define IR_KEY_VIDEO_VOL_DOWN     0x0404D02F
#define IR_KEY_VIDEO_REC_STOP     0x0404A857
#define IR_KEY_VIDEO_REC_PAUSE    0x04046897
#define IR_KEY_VIDEO_CH_UP        0x040448B7
#define IR_KEY_VIDEO_CH_DOWN      0x040408F7

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
void remocon_temp(void)
{

}

/* Static Functions */

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
static bool check_time_is_start(uint16_t diffVal)
{
    if((diffVal > (IR_RX_TIME_VAL_START - IR_RX_TIME_VAL_GUARD))
        && (diffVal < (IR_RX_TIME_VAL_START + IR_RX_TIME_VAL_GUARD)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
static uint32_t check_time_value(uint16_t diffVal)
{
    if((diffVal > (IR_RX_TIME_VAL_0 - IR_RX_TIME_VAL_GUARD))
        && (diffVal < (IR_RX_TIME_VAL_0 + IR_RX_TIME_VAL_GUARD)))
    {
        return 0;
    }
    else if((diffVal > (IR_RX_TIME_VAL_1 - IR_RX_TIME_VAL_GUARD))
            && (diffVal < (IR_RX_TIME_VAL_1 + IR_RX_TIME_VAL_GUARD)))
    {
        return 1;
    }
    else
    {
        return IR_RX_ABNORMAL_KEY;
    }
}

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
static uint32_t get_current_ir_key_value(void)
{
    uint32_t currKeyVal = 0, i, currBit;
    uint16_t diffVal;

    for(i = 0; i < IR_RX_1KEY_COUNT; i ++)
    {
        Delay(3);

        if(TRUE == IrRxQueue_Is_Empty())
        {
            // printf("[ERROR] Queue empty, index: %d\n", i);
            return IR_RX_ABNORMAL_KEY;
        }

        diffVal = IrRxQueue_DeQueue();
        currBit = check_time_value(diffVal);
        // printf("IR-RX, Diff: %d\n", diffVal);

        if(IR_RX_ABNORMAL_KEY == currBit)
        {
            // printf("[ERROR] currBit: %d\n", currBit);        
            return IR_RX_ABNORMAL_KEY;
        }
        
        currKeyVal = currKeyVal | (currBit << (IR_RX_1KEY_COUNT - 1 - i));
    }

    //printf("currKeyVal: %08X\n", currKeyVal);

    return currKeyVal;
}

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
static void send_current_key_value(uint32_t currKeyVal)
{
    switch(currKeyVal)
    {
    case IR_KEY_TV_POWER:
        IR_KEY_PRINT(IR_KEY_TV_POWER);  break;

    case IR_KEY_TV_NUM_1:
        IR_KEY_PRINT(IR_KEY_TV_NUM_1);   break;
    case IR_KEY_TV_NUM_2:
        IR_KEY_PRINT(IR_KEY_TV_NUM_2);   break;
    case IR_KEY_TV_NUM_3:
        IR_KEY_PRINT(IR_KEY_TV_NUM_3);   break;
    case IR_KEY_TV_NUM_4:
        IR_KEY_PRINT(IR_KEY_TV_NUM_4);   break;
    case IR_KEY_TV_NUM_5:
        IR_KEY_PRINT(IR_KEY_TV_NUM_5);   break;
    case IR_KEY_TV_NUM_6:
        IR_KEY_PRINT(IR_KEY_TV_NUM_6);   break;
    case IR_KEY_TV_NUM_7:
        IR_KEY_PRINT(IR_KEY_TV_NUM_7);   break;
    case IR_KEY_TV_NUM_8:
        IR_KEY_PRINT(IR_KEY_TV_NUM_8);   break;
    case IR_KEY_TV_NUM_9:
        IR_KEY_PRINT(IR_KEY_TV_NUM_9);   break;
    case IR_KEY_TV_NUM_0:
        IR_KEY_PRINT(IR_KEY_TV_NUM_0);   break;
    
    case IR_KEY_TV_MEM_DEL:
        IR_KEY_PRINT(IR_KEY_TV_MEM_DEL);  break;
    case IR_KEY_TV_SLEEP_RESERV:
        IR_KEY_PRINT(IR_KEY_TV_SLEEP_RESERV);  break;
    case IR_KEY_TV_CONFIG_MEMORY:
        IR_KEY_PRINT(IR_KEY_TV_CONFIG_MEMORY);  break;
    case IR_KEY_TV_AUTO_CHANNEL:
        IR_KEY_PRINT(IR_KEY_TV_AUTO_CHANNEL);  break;
    
    case IR_KEY_TV_VOL_PLUS:
        IR_KEY_PRINT(IR_KEY_TV_VOL_PLUS);  break;
    case IR_KEY_TV_VOL_MINUS:
        IR_KEY_PRINT(IR_KEY_TV_VOL_MINUS);  break;
    case IR_KEY_TV_CHANNEL_UP:
        IR_KEY_PRINT(IR_KEY_TV_CHANNEL_UP);  break;
    case IR_KEY_TV_CHANNEL_DOWN:
        IR_KEY_PRINT(IR_KEY_TV_CHANNEL_DOWN);  break;
    case IR_KEY_TV_SELECT_LIST:
        IR_KEY_PRINT(IR_KEY_TV_SELECT_LIST);  break;

    case IR_KEY_TV_MUTE:
        IR_KEY_PRINT(IR_KEY_TV_MUTE);  break;
    case IR_KEY_TV_CONTROL:
        IR_KEY_PRINT(IR_KEY_TV_CONTROL);  break;
    case IR_KEY_TV_TV_VIDEO:
        IR_KEY_PRINT(IR_KEY_TV_TV_VIDEO);  break;
    case IR_KEY_TV_TV_CABLE:
        IR_KEY_PRINT(IR_KEY_TV_TV_CABLE);  break;

    case IR_KEY_VIDEO_POWER:
        IR_KEY_PRINT(IR_KEY_VIDEO_POWER);  break;
    case IR_KEY_VIDEO_TV_VCR:
        IR_KEY_PRINT(IR_KEY_VIDEO_TV_VCR);  break;
    
    case IR_KEY_VIDEO_REWIND:
        IR_KEY_PRINT(IR_KEY_VIDEO_REWIND);  break;
    case IR_KEY_VIDEO_PLAY:
        IR_KEY_PRINT(IR_KEY_VIDEO_PLAY);  break;
    case IR_KEY_VIDEO_FASTFORWARD:
        IR_KEY_PRINT(IR_KEY_VIDEO_FASTFORWARD);  break;

    case IR_KEY_VIDEO_VOL_UP:
        IR_KEY_PRINT(IR_KEY_VIDEO_VOL_UP);  break;
    case IR_KEY_VIDEO_VOL_DOWN:
        IR_KEY_PRINT(IR_KEY_VIDEO_VOL_DOWN);  break;
    case IR_KEY_VIDEO_REC_STOP:
        IR_KEY_PRINT(IR_KEY_VIDEO_REC_STOP);  break;
    case IR_KEY_VIDEO_REC_PAUSE:
        IR_KEY_PRINT(IR_KEY_VIDEO_REC_PAUSE);  break;
    case IR_KEY_VIDEO_CH_UP:
        IR_KEY_PRINT(IR_KEY_VIDEO_CH_UP);  break;
    case IR_KEY_VIDEO_CH_DOWN:
        IR_KEY_PRINT(IR_KEY_VIDEO_CH_DOWN);  break;

    default:
        break;
    }
}

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
uint32_t g_old_KeyVal = 0;

void remocon_test(void)
{
    uint32_t currKeyVal;
    uint16_t diffVal;
    if(FALSE == IrRxQueue_Is_Empty())    
    {        
        diffVal = IrRxQueue_DeQueue();    
        if(check_time_is_start(diffVal))
        {
            Delay(10);
            currKeyVal = get_current_ir_key_value();    
            send_current_key_value(currKeyVal); 
            return;
        }
        else
        {
            // printf("[ERROR] Invalid Call Process_IR_RX\n");
            return;
        }        
    }
}

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
void remocon_initialzie(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;    
    NVIC_InitTypeDef NVIC_InitStructure;    
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;    

    /******************** Clock Enable ********************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        
    /******************** GPIO configure ********************/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);   

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);    

    /******************** Fuction configure ********************/
    /* Configure EXTI Line to generate an interrupt */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);    

    /* Enable the EXTI4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the TIM4 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* Prescaler configuration */
    TIM_PrescalerConfig(TIM4, 9, TIM_PSCReloadMode_Immediate);

    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);    
    
}




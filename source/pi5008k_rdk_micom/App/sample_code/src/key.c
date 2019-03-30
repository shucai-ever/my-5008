/*
 * (C) COPYRIGHT 2009 CRZ
 *
 * File Name : key.c
 * Author    : POOH
 * Version   : V1.0
 * Date      : 08/20/2009
 */

/* Includes */

#include "hw_config.h"

/* functions */

/*
void LED_On_Red (void);
void LED_Off_Red (void);
void LED_On_Yellow (void);
void LED_Off_Yellow (void);
void LED_On_Blue (void);
void LED_Off_Blue (void);
void LED_On_All (void);
void LED_Off_All (void);
*/

void KEY_Initialize (void)
{
    /******************** Clock Enable ********************/
    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    
    
    /******************** GPIO configure ********************/
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Configure gpio as input : Button WKUP & USER */
    GPIO_InitStructure.GPIO_Pin  = GPIO_KEY1_PIN | GPIO_KEY2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIO_KEY, &GPIO_InitStructure);    

    /******************** Fuction configure ********************/
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Configure gpio as input : Button Left-WKUP */
    /* Connect EXTI Line to gpio pin */
    GPIO_EXTILineConfig(GPIO_PORTSOURCE_KEY, GPIO_PINSOURCE_KEY1);

    /* Configure EXTI Line to generate an interrupt */
    EXTI_InitStructure.EXTI_Line    = GPIO_EXTI_Line_KEY1;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Configure gpio as input : Button Right-USER */
    /* Connect EXTI Line to gpio pin */
    GPIO_EXTILineConfig(GPIO_PORTSOURCE_KEY, GPIO_PINSOURCE_KEY2);

    /* Configure EXTI Line to generate an interrupt */  
    EXTI_InitStructure.EXTI_Line    = GPIO_EXTI_Line_KEY2;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);      
    
    /******************** Interrupt configure ********************/
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the EXTI1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);    
  
}

void KEY_Test (void)
{
    uint32_t i = 0;

    LED_Off_All();

    while(1)
    {
        delay_100_milli_second();

        if((i++ & 0x1) == 0x0)
        {
            LED_On_Blue();
        }
        else
        {
            LED_Off_Blue();
        }

        if(GPIO_ReadInputDataBit(GPIO_KEY, GPIO_KEY1_PIN) == Bit_SET)
        {
            LED_On_Red();
        }
        else
        {
            LED_Off_Red();
        }

        if(GPIO_ReadInputDataBit(GPIO_KEY, GPIO_KEY2_PIN) == Bit_SET)
        {
            LED_On_Yellow();
        }
        else
        {
            LED_Off_Yellow();
        }
    }
}


/*
 * (C) COPYRIGHT 2009 CRZ
 *
 * File Name : seven_segment.c
 * Author    : POOH
 * Version   : V1.0
 * Date      : 08/20/2009
 */

/* Includes */

#include "hw_config.h"

/* functions */

#define GPIO_7_SEG_POWER_PIN    GPIO_Pin_8
#define GPIO_7_SEG_A_PIN        GPIO_Pin_0
#define GPIO_7_SEG_B_PIN        GPIO_Pin_1
#define GPIO_7_SEG_C_PIN        GPIO_Pin_2
#define GPIO_7_SEG_D_PIN        GPIO_Pin_3
#define GPIO_7_SEG_E_PIN        GPIO_Pin_4
#define GPIO_7_SEG_F_PIN        GPIO_Pin_5
#define GPIO_7_SEG_G_PIN        GPIO_Pin_7
#define GPIO_7_SEG_DP_PIN       GPIO_Pin_6

#define GPIO_7_SEG              GPIOC

void PowerOn_7_SEG(void)
{
    GPIO_7_SEG->BRR |= GPIO_7_SEG_POWER_PIN;
}

void PowerOff_7_SEG(void)
{
    GPIO_7_SEG->BSRR |= GPIO_7_SEG_POWER_PIN;
}

void On_7_SEG_OnePin(uint16_t pinNum)
{
    GPIO_7_SEG->BRR |= pinNum;
}

void Off_7_SEG_OnePin(uint16_t pinNum)
{
    GPIO_7_SEG->BSRR |= pinNum;
}

void On_7_SEG_AllPin(void)
{
    On_7_SEG_OnePin
        (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
         | GPIO_7_SEG_D_PIN | GPIO_7_SEG_E_PIN | GPIO_7_SEG_F_PIN
         | GPIO_7_SEG_G_PIN | GPIO_7_SEG_DP_PIN);
}

void Off_7_SEG_AllPin(void)
{
    Off_7_SEG_OnePin
        (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
         | GPIO_7_SEG_D_PIN | GPIO_7_SEG_E_PIN | GPIO_7_SEG_F_PIN
         | GPIO_7_SEG_G_PIN | GPIO_7_SEG_DP_PIN);
}

void OutPut_7_SEG_Number(unsigned int num)
{
    Off_7_SEG_AllPin();

    switch(num)
    {
    case 0:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
            | GPIO_7_SEG_D_PIN | GPIO_7_SEG_E_PIN | GPIO_7_SEG_F_PIN);
        break;

    case 1:
        On_7_SEG_OnePin
            (GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN);
        break;

    case 2:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_D_PIN
            | GPIO_7_SEG_E_PIN | GPIO_7_SEG_G_PIN);
        break;

    case 3:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
            | GPIO_7_SEG_D_PIN | GPIO_7_SEG_G_PIN);
        break;

    case 4:
        On_7_SEG_OnePin
            (GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
            | GPIO_7_SEG_F_PIN | GPIO_7_SEG_G_PIN);
        break;

    case 5:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_C_PIN | GPIO_7_SEG_D_PIN
            | GPIO_7_SEG_F_PIN | GPIO_7_SEG_G_PIN);
        break;

    case 6:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_C_PIN | GPIO_7_SEG_D_PIN
            | GPIO_7_SEG_E_PIN | GPIO_7_SEG_F_PIN | GPIO_7_SEG_G_PIN);
        break;

    case 7:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
            | GPIO_7_SEG_F_PIN);
        break;

    case 8:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
            | GPIO_7_SEG_D_PIN | GPIO_7_SEG_E_PIN | GPIO_7_SEG_F_PIN
            | GPIO_7_SEG_G_PIN);
        break;

    case 9:
        On_7_SEG_OnePin
            (GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN | GPIO_7_SEG_C_PIN
            | GPIO_7_SEG_D_PIN | GPIO_7_SEG_F_PIN | GPIO_7_SEG_G_PIN);
        break;
    }
}

void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct);

void Seven_Segment_Initilaze(void)
{
    /******************** Clock Enable ********************/
    /* Enable GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);    
    
    /******************** GPIO configure ********************/
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Configure gpio as output : LED1, LED2, LED3 */
    GPIO_InitStructure.GPIO_Pin
        = GPIO_7_SEG_POWER_PIN | GPIO_7_SEG_A_PIN | GPIO_7_SEG_B_PIN
          | GPIO_7_SEG_C_PIN | GPIO_7_SEG_D_PIN | GPIO_7_SEG_E_PIN
          | GPIO_7_SEG_F_PIN | GPIO_7_SEG_G_PIN | GPIO_7_SEG_DP_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_7_SEG, &GPIO_InitStructure);    
}

void Seven_Segment_Test(void)
{
    int i;
    
    printf("Seven_Segment_Test\n");

    PowerOn_7_SEG();

    delay_1_second();
    Off_7_SEG_AllPin();

    On_7_SEG_OnePin(GPIO_7_SEG_A_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_B_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_C_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_D_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_E_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_F_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_G_PIN);
    delay_1_second();
    On_7_SEG_OnePin(GPIO_7_SEG_DP_PIN);
    delay_1_second();

    for(i=0; i<10; i++)
    {
        OutPut_7_SEG_Number(i);
        delay_1_second();
    }
}


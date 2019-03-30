/*
 * (C) COPYRIGHT 2009 CRZ
 *
 * File Name : hw_config.h
 * Author    : POOH
 * Version   : V1.0
 * Date      : 08/12/2009
 */

#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* includes */

#include "stm32f10x.h"
#include "stdio.h"
#include "debug.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "usb_desc.h"

/* global variables */

extern bool g_TestProcessState;
extern __IO uint16_t BoardDetectValue;

/* defines */

#define GPIO_LED             GPIOB
#define GPIO_KEY             GPIOA
#define GPIO_USART           GPIOA

#define GPIO_USB_DISCONNECT        GPIOA
#define GPIO_7_SEG              GPIOC
#define GPIO_LED1_PIN        GPIO_Pin_9 /* RED */
#define GPIO_LED2_PIN        GPIO_Pin_5 /* YELLOW */ 
#define GPIO_LED3_PIN        GPIO_Pin_8 /* BLUE */

#define GPIO_KEY1_PIN        GPIO_Pin_0 /* LEFT_WKUP */
#define GPIO_KEY2_PIN        GPIO_Pin_1 /* RIGHT_USER */

#define GPIO_USART_Rx_Pin    GPIO_Pin_10
#define GPIO_USART_Tx_Pin    GPIO_Pin_9

#define GPIO_USB_DISCONNECT_PIN    GPIO_Pin_8


#define GPIO_7_SEG_POWER_PIN    GPIO_Pin_8
#define GPIO_7_SEG_A_PIN        GPIO_Pin_0
#define GPIO_7_SEG_B_PIN        GPIO_Pin_1
#define GPIO_7_SEG_C_PIN        GPIO_Pin_2
#define GPIO_7_SEG_D_PIN        GPIO_Pin_3
#define GPIO_7_SEG_E_PIN        GPIO_Pin_4
#define GPIO_7_SEG_F_PIN        GPIO_Pin_5
#define GPIO_7_SEG_G_PIN        GPIO_Pin_7
#define GPIO_7_SEG_DP_PIN       GPIO_Pin_6

#define GPIO_PORTSOURCE_KEY        GPIO_PortSourceGPIOA
#define GPIO_PINSOURCE_KEY1        GPIO_PinSource0
#define GPIO_PINSOURCE_KEY2        GPIO_PinSource1
#define GPIO_EXTI_Line_KEY1        EXTI_Line0
#define GPIO_EXTI_Line_KEY2        EXTI_Line1

/* functions */

void LED_On_Red (void);
void LED_Off_Red (void);
void LED_Toggle_Red (void);
void LED_On_Yellow (void);
void LED_Off_Yellow (void);
void LED_Toggle_Yellow (void);
void LED_On_Blue (void);
void LED_Off_Blue (void);
void LED_Toggle_Blue (void);
void LED_On_All (void);
void LED_Off_All (void);
void LED_Initialize(void);

void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void USB_Initialize (void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void EXTI_Configuration(void);
void USART1_Init(void);
void USART2_Init(void);
void USART3_Init(void);
void Get_SerialNum(void);
uint8_t USART_GetCharacter(USART_TypeDef *  usart_p);

void LED_Test (void);
void KEY_Test (void);
void KEY_Initialize (void);

void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);

void delay_1_second(void);
void delay_100_milli_second(void);

void Seven_Segment_Test(void);
void Seven_Segment_Initilaze(void);

void board_detect(void);
void board_detect_initialzie(void);

void UartQueue_Initialize(void);
void IrRxQueue_Initialize(void);
bool UartQueue_COM_Is_Empty(void);
void UartQueue_COM_EnQueue(uint16_t data);
uint16_t UartQueue_COM_DeQueue(void);
bool UartQueue_Debug_Is_Empty(void);
void UartQueue_Debug_EnQueue(uint16_t data);
uint16_t UartQueue_Debug_DeQueue(void);
bool IrRxQueue_Is_Empty(void);
void IrRxQueue_EnQueue(uint16_t data);
uint16_t IrRxQueue_DeQueue(void);

void remocon_initialzie(void);
void remocon_test(void);

#endif  /*__HW_CONFIG_H */


//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2008  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name			:  BOARD.H
// Description			:  This file contains typedefine for the driver files	
// Ref. Docment			: 
// Revision History 	:


#ifndef		__BOARD_H__  
#define		__BOARD_H__  


#include	"..\common\type.h"

// -----------------------------------------------------------------------------
// Include files
// -----------------------------------------------------------------------------
#ifndef __STM32F10x_GPIO_H
#include	"stm32f10x_gpio.h"
#endif


// -----------------------------------------------------------------------------
// Struct/Union Types and define
// -----------------------------------------------------------------------------

#if !defined (USE_STM32100B_EVAL) && !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL) &&  !defined (USE_STM32100E_EVAL) 
 //#define USE_STM32100B_EVAL
//#define USE_STM3210B_EVAL	// 
#define USE_STM3210E_EVAL
//#define USE_STM32100E_EVAL
// #define USE_STM3210C_EVAL 
#endif

#define	__USE_I2C_DMA_MODE__		0

// -----------------------------------------------------------------------------
//	Definition for I2C port
// -----------------------------------------------------------------------------

typedef enum
{
	I2C_1 = 0,
	I2C_2 = 1,
	I2C_no
}I2C_BusID;

typedef enum
{
	I2C_OK = 0,
	I2C_NOT_FREE,
	I2C_NOT_START,
	I2C_HOST_NACK,
	I2C_NO_DATA,
	I2C_TIME_OUT
}I2C_ErrorCode;

#define	NACK				1
#define	ACK					0

#define COMn                             2

typedef enum
{
	COM1 = 0,
	COM2,
}COM_TypeDef;

#define USB_USART		COM1

/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM3210B_EVAL
	#define	USB_DISCONNECT				GPIOD  
	#define	USB_DISCONNECT_PIN			GPIO_Pin_9
	#define	RCC_APB2Periph_GPIO_DISCONNECT	RCC_APB2Periph_GPIOD
	#define	EVAL_COM1_IRQn				USART1_IRQn
#elif defined (USE_STM3210E_EVAL)
	#define	USB_DISCONNECT				GPIOC  
	#define	USB_DISCONNECT_PIN			GPIO_Pin_14
	#define	RCC_APB2Periph_GPIO_DISCONNECT	RCC_APB2Periph_GPIOC
	#define	EVAL_COM1_IRQn				USART1_IRQn
#elif defined (USE_STM3210C_EVAL)
	#define	USB_DISCONNECT				0  
	#define	USB_DISCONNECT_PIN			0
	#define	RCC_APB2Periph_GPIO_DISCONNECT	0
	#define	EVAL_COM1_IRQn				USART2_IRQn
#endif /* USE_STM3210B_EVAL */


// -----------------------------------------------------------------------------
//	GPIOs
// -----------------------------------------------------------------------------

/////////////////////////////////
// Global
/////////////////////////////////

#define	PI5008K_POWER_RESET(x)	GPIO_WriteBit(GPIOA, GPIO_Pin_5, (x)? Bit_SET : Bit_RESET)

// -----------------------------------------------------------------------------
// External Variables declaration
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Exported function Prototype
// -----------------------------------------------------------------------------
bool USART_Config(void);

#endif	/* __BOARD_H__ */

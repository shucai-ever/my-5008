//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2005  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	INIT.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include <stdio.h>
#include "stm32f10x.h"
#include "..\Common\board.h"
// ----------------------------------------------------------------------
// Struct/Union Types and define
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Global functions
// ----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//
//		USART
//
//////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------

static void Set_USART1(void)
{	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);

	/* Enable UART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USART configuration */
//	USART_InitStructure.USART_BaudRate = 256000;		// for XRC
//	USART_InitStructure.USART_BaudRate = 460800;	// for debug print
//	USART_InitStructure.USART_BaudRate = 230400;	// for debug print
	USART_InitStructure.USART_BaudRate = 115200;	// for debug print
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);



	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}  
static void Set_USART2(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);

	/* Enable UART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USART configuration */
//	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_BaudRate = 115200;		// for XRC
//	USART_InitStructure.USART_BaudRate = 460800;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}  

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (BYTE) ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){}
	return ch;
}

//////////////////////////////////////////////////////////////////////////
//
//		GPIO
//
//////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------
static void Set_GPIO_Port(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;    
    NVIC_InitTypeDef NVIC_InitStructure;    

    /******************** Clock Enable ********************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |	\
		RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO	\
		, ENABLE);

	/* for can tx and usb_disconnect pin */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = \
		 GPIO_Pin_8|GPIO_Pin_12;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* for can rx and KEY test */
	//pa1 - key
	//pa11 - can rx
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = \
		GPIO_Pin_1 |   GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


        









	/* START 5DIR_KEY GPIO PIN */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =  \
		GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; 	//PC13= sw1, PC14 = sw2, PC15 = sw3
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;  //PA1 = sw4
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;  //PB1 = sw5
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* END 5DIR_KEY GPIO PIN SET */





	/* for LED GPIO PIN */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =  \
		GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9; //LED
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	
	/* for AD KEY TEST */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  //ADKEY
	GPIO_Init(GPIOB, &GPIO_InitStructure);








	/* for test */

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	 
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);    

	/* Configure EXTI Line to generate an interrupt */
	EXTI_InitStructure.EXTI_Line	= EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);    

	/* Enable the EXTI4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);



}


//////////////////////////////////////////////////////////////////////////
//
//		Timer
//
//////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------
void Set_Timer(void)		
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;    
    NVIC_InitTypeDef NVIC_InitStructure;    
    
#if 0 //300ms timer3
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Prescaler = 65535;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseStructure.TIM_Period = 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_Cmd(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
#endif

	// 10us timer4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	/* Enable the TIM4 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 358;
	TIM_TimeBaseStructure.TIM_Prescaler = 2;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);


	/* TIM4 enable counter */
	TIM_Cmd(TIM4, ENABLE);    
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE); // interrupt enable

}

//--------------------------------------------------------------------------------------------------------------------------
BYTE outdata = 0;
void Check_Timer(void)
{
	outdata = !outdata;
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, (outdata)?Bit_SET : Bit_RESET);
}

//--------------------------------------------------------------------------------------------------------------------------
static VWORD count_10ms;
void TimeService_10ms(void)
{
	count_10ms++;
	count_10ms %= 1000;
}

//--------------------------------------------------------------------------------------------------------------------------
WORD CalTimer(WORD SetupCnt)
{
	if(SetupCnt > count_10ms)
		return (0xffff-(SetupCnt - count_10ms));
	else
		return (count_10ms - SetupCnt);
}

//--------------------------------------------------------------------------------------------------------------------------
//#define	GetTimer()		count_10ms
WORD GetTimer(void)
{
	return count_10ms;
}

//--------------------------------------------------------------------------------------------------------------------------
static __IO DWORD TimingDelay;
void Delay_ms(WORD nTime)
{
	TimingDelay = (DWORD)nTime;
	if (SysTick_Config(SystemCoreClock / 1000))	// 1ms
	{ while (1);}
	while(TimingDelay != 0);
}

//--------------------------------------------------------------------------------------------------------------------------
void Delay_10us(WORD nTime)
{
	TimingDelay = (DWORD)nTime;
	if (SysTick_Config(SystemCoreClock / 100000))	// 10us
	{ while (1);}
	while(TimingDelay != 0);
}

//--------------------------------------------------------------------------------------------------------------------------
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00) TimingDelay--;
}


//////////////////////////////////////////////////////////////////////////
//
//		I2C Bus
//
//////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------
static void Set_I2C_Port(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
#if __USE_I2C_DMA_MODE__ == 1
	NVIC_InitTypeDef NVIC_InitStructure;  
#endif

	// Enable IOE_I2C and IOE_I2C_PORT & Alternate Function clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2, ENABLE);
		
	// I2C1 SCL and SDA pins configuration
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// I2C2 SCL and SDA pins configuration
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// I2C configuration
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;

	/* I2C1 Peripheral Enable */
	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);

	/* I2C2 Peripheral Enable */
	I2C_Cmd(I2C2, ENABLE);
	I2C_Init(I2C2, &I2C_InitStructure);


#if __USE_I2C_DMA_MODE__ == 1

	/* Configure and enable I2C DMA TX Channel interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure and enable I2C DMA RX Channel interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_Init(&NVIC_InitStructure);  

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = ((DWORD)(I2C1_BASE+0x10));	/* This parameter will be configured durig communication */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (DWORD)0;	/* This parameter will be configured durig communication */
	DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)0;	/* This parameter will be configured durig communication */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	/* This parameter will be configured durig communication */
	DMA_InitStructure.DMA_BufferSize = 0xFFFF;				/* This parameter will be configured durig communication */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
	
#endif


}

void EXTI0_Config(void)
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
		
	/* Enable GPIOA clock */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PB.00 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Connect EXTI0 Line to PB.00 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);

	/* Configure EXTI0 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Set_Remocon(void)
{
	
	
	GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;    
    NVIC_InitTypeDef NVIC_InitStructure;    

	/* for ir rx pin pa0 */
    /******************** GPIO configure ********************/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);   

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);    

    /******************** Fuction configure ********************/
    /* Configure EXTI Line to generate an interrupt */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);    

    /* Enable the EXTI4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
}

void Set_5Dir_Sw(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;

	/* for 5dir key */
	/******************** GPIO configure ********************/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);   
	
	/* for 5dir key */
	/******************** GPIO configure ********************/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);   
	GPIO_Init(GPIOB, &GPIO_InitStructure);   

    
}

//--------------------------------------------------------------------------------------------------------------------------
void uComOnChipInitial(void)
{


//	__disable_irq();  
	Set_USART1();
	Set_USART2();

	Set_GPIO_Port();
	Set_Timer();

	Set_I2C_Port();

	Set_Remocon();
	Set_5Dir_Sw();
	//EXTI0_Config();
	
	Set_Can_Init();

	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET); //Led1 Off
	Delay_ms(200);	// delay 200ms
	GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET); //Led2 Off
	Delay_ms(200);	// delay 200ms
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET); //Led3 Off

//	__enable_irq();
}

/*  FILE_END_HERE */


/*
 * (C) COPYRIGHT 2009 CRZ
 *
 * File Name : hw_config.c
 * Author    : POOH
 * Version   : V1.0
 * Date      : 08/12/2009
 */

/* includes */

#include "hw_config.h"

/* defines */

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* global variables */

uint32_t ADC_ConvertedValueX = 0;
uint32_t ADC_ConvertedValueX_1 = 0;
//static volatile uint32_t TimingDelay;

/* functions */

void delay_1_second(void)
{
    Delay(1000);
}

void delay_100_milli_second(void)
{
    Delay(100);
}

/*
 * Name   : USART1_Init
 * Input  : None
 * Output : None
 * Return : None
 */
void USART1_Init(void)
{
    /******************** Clock Enable ********************/
    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    

    /* Enable USART1 clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);        

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
    
    /******************** GPIO configure ********************/
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USARTx_Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_USART_Tx_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIO_USART, &GPIO_InitStructure);

    /* Configure USARTx_Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = GPIO_USART_Rx_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIO_USART, &GPIO_InitStructure);    
    
    /******************** Fuction configure ********************/
    
    USART_InitTypeDef USART_InitStructure;

    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     - BaudRate = 115200 baud  
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate   = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl
                                         = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USARTx */ 
    USART_Init(USART1, &USART_InitStructure);

    /* Enable the USART1 */
    USART_Cmd(USART1, ENABLE);
}

/*
 * Name   : USART2_Init
 * Input  : None
 * Output : None
 * Return : None
 */
void USART2_Init(void)
{
    /******************** Clock Enable ********************/
    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    

    /* Enable USART3 clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
    
    /******************** GPIO configure ********************/
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USARTx_Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USARTx_Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);    
    
    /******************** Fuction configure ********************/
    
    USART_InitTypeDef USART_InitStructure;

    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     - BaudRate = 115200 baud  
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate   = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl
                                         = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USARTx */ 
    USART_Init(USART2, &USART_InitStructure);

    /* Enable the USART2 */
    USART_Cmd(USART2, ENABLE);
}

/*
 * Name   : USART3_Init
 * Input  : None
 * Output : None
 * Return : None
 */
void USART3_Init(void)
{
    /******************** Clock Enable ********************/
    /* Enable GPIOB clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 

    /* Enable USART3 clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
    
    /******************** GPIO configure ********************/
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USARTx_Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USARTx_Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);    
    
    /******************** Fuction configure ********************/
    
    USART_InitTypeDef USART_InitStructure;

    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     - BaudRate = 9600 baud  
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate   = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl
                                         = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USARTx */ 
    USART_Init(USART3, &USART_InitStructure);

    /* Enable the USART3 */
    USART_Cmd(USART3, ENABLE);
}


/*
 * Name   : RCC_Configuration
 * Input  : None
 * Output : None
 * Return : None
 */
void RCC_Configuration(void)
{
#if 0
    SystemInit();
#endif
#if 1
    ErrorStatus HSEStartUpStatus;

    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1); 

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1); 

        /* ADCCLK = PCLK2/4 */
        RCC_ADCCLKConfig(RCC_PCLK2_Div6); 

        /* PLLCLK = 12MHz * 6 = 72 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);

        /* Enable PLL */ 
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }    
#endif
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{ 
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */ 
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */ 
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

}

/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz).
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_USBClock(void)
{
    /* Select USBCLK source */
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

    /* Enable USB clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode.
* Description    : Power-off system clocks and power while entering suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
    /* Set the device state to suspend */
    bDeviceState = SUSPENDED;

#if 0
    /* Clear EXTI Line18 pending bit */
    EXTI_ClearITPendingBit(GPIO_EXTI_Line_KEY1);
#endif

    /* Request to enter STOP mode with regulator in low power mode */
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode.
* Description    : Restores system clocks and power while exiting suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
    DEVICE_INFO *pInfo = &Device_Info;

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    RCC_WaitForHSEStartUp();

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {}

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x08)
    {}

    /* Set the device state to the correct state */
    if (pInfo->Current_Configuration != 0)
    {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
    else 
    {
        bDeviceState = ATTACHED;
    }
}

/*******************************************************************************
* Function Name  : USB_Interrupts_Config.
* Description    : Configures the USB interrupts.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Interrupts_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Enable the USB interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USB Wake-up interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Enable the DMA1 Channel1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : USB_Cable_Config.
* Description    : Software Connection/Disconnection of USB Cable.
* Input          : NewState: new state.
* Output         : None.
* Return         : None
*******************************************************************************/
void USB_Cable_Config (FunctionalState NewState)
{ 
    if (NewState != DISABLE)
    {
        GPIO_ResetBits(GPIO_USB_DISCONNECT, GPIO_USB_DISCONNECT_PIN);
    }
    else
    {
        GPIO_SetBits(GPIO_USB_DISCONNECT, GPIO_USB_DISCONNECT_PIN);
    }
}

/*******************************************************************************
* Function Name  : USB_Initialize.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None
*******************************************************************************/
void USB_Initialize (void)
{ 
  
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;    

    /******************** Clock Enable ********************/
    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    /******************** GPIO configure ********************/
    /* USB_DISCONNECT used as USB pull-up */
    GPIO_InitStructure.GPIO_Pin   = GPIO_USB_DISCONNECT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(GPIO_USB_DISCONNECT, &GPIO_InitStructure);  
    GPIO_SetBits(GPIO_USB_DISCONNECT, GPIO_USB_DISCONNECT_PIN);
    
    /* USB Wakeup from suspend event */
    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line    = EXTI_Line18; // USB resume from suspend mode
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;    
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
* Function Name : EXTI_Configuration.
* Description   : Configure the EXTI lines for Key and Tamper push buttons.
* Input         : None.
* Output        : None.
* Return value  : The direction value.
*******************************************************************************/
void EXTI_Configuration(void)
{ 
    EXTI_InitTypeDef EXTI_InitStructure;

    /* USB Wakeup from suspend event */
    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line    = EXTI_Line18; // USB resume from suspend mode
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;
  
  Device_Serial0 = *(uint32_t*)(0x1FFFF7E8);
  Device_Serial1 = *(uint32_t*)(0x1FFFF7EC);
  Device_Serial2 = *(uint32_t*)(0x1FFFF7F0);
  
  if(Device_Serial0 != 0)
  {
     CustomHID_StringSerial[2] = (uint8_t)(Device_Serial0 & 0x000000FF);  
     CustomHID_StringSerial[4] = (uint8_t)((Device_Serial0 & 0x0000FF00) >> 8);
     CustomHID_StringSerial[6] = (uint8_t)((Device_Serial0 & 0x00FF0000) >> 16);
     CustomHID_StringSerial[8] = (uint8_t)((Device_Serial0 & 0xFF000000) >> 24);  

     CustomHID_StringSerial[10] = (uint8_t)(Device_Serial1 & 0x000000FF);  
     CustomHID_StringSerial[12] = (uint8_t)((Device_Serial1 & 0x0000FF00) >> 8);
     CustomHID_StringSerial[14] = (uint8_t)((Device_Serial1 & 0x00FF0000) >> 16);
     CustomHID_StringSerial[16] = (uint8_t)((Device_Serial1 & 0xFF000000) >> 24); 

     CustomHID_StringSerial[18] = (uint8_t)(Device_Serial2 & 0x000000FF);  
     CustomHID_StringSerial[20] = (uint8_t)((Device_Serial2 & 0x0000FF00) >> 8);
     CustomHID_StringSerial[22] = (uint8_t)((Device_Serial2 & 0x00FF0000) >> 16);
     CustomHID_StringSerial[24] = (uint8_t)((Device_Serial2 & 0xFF000000) >> 24); 
  }   
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
 #if 0
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}
#endif
/*
 * Name   : putchar
 * Input  : None
 * Output : None
 * Return : None
 */
 #if 0
int fputc(int ch, FILE *f)
{
    /* Write a character to the USART */  
    if( ch == '\n') {
        USART_SendData(USART1, '\r');
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, '\n');
    }else {
        USART_SendData(USART1, (u8) ch);
    }

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

    return ch;
}
uint8_t USART_GetCharacter(USART_TypeDef *  usart_p)
{
    uint8_t data;

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(usart_p, USART_FLAG_RXNE) == RESET);

    /* Write a character to the USART */
    data = USART_ReceiveData(usart_p);

    USART_SendData(usart_p, data);
    while(USART_GetFlagStatus(usart_p, USART_FLAG_TXE) == RESET);

    if( data == '\r' )  return (int)('\n');
    else                return(data);
}
#endif


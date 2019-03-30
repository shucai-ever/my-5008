#include "pi5008k_can.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define __CAN1_USED__
/* #define __CAN2_USED__*/

#ifdef  __CAN1_USED__
  #define CANx                       CAN1
  #define GPIO_CAN                   GPIO_CAN1
  #define GPIO_Remapping_CAN         GPIO_Remapping_CAN1
  #define GPIO_CAN                   GPIO_CAN1
  #define GPIO_Pin_CAN_RX            GPIO_Pin_CAN1_RX
  #define GPIO_Pin_CAN_TX            GPIO_Pin_CAN1_TX
#else /*__CAN2_USED__*/
  #define CANx                       CAN2
  #define GPIO_CAN                   GPIO_CAN2
  #define GPIO_Remapping_CAN             GPIO_Remap_CAN2
  #define GPIO_CAN                   GPIO_CAN2
  #define GPIO_Pin_CAN_RX            GPIO_Pin_CAN2_RX
  #define GPIO_Pin_CAN_TX            GPIO_Pin_CAN2_TX
#endif  /* __CAN1_USED__ */

#define KEY_PRESSED     0x01
#define KEY_NOT_PRESSED 0x00

CAN_InitTypeDef        CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
CanTxMsg TxMessage;
uint8_t KeyNumber = 0x0;

extern CanRxMsg RxMessage;

/**
  * @brief  Configures the NVIC for CAN.
  * @param  None
  * @retval None
  */
void CAN_NVIC_Config(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}



/**
  * @brief  Configures the CAN.
  * @param  None
  * @retval None
  */
void CAN_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* GPIO clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  /* Configure CAN pin: RX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure CAN pin: TX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  //GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);
  
  /* CANx Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
  
  
  /* CAN register init */
  CAN_DeInit(CAN1);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  
  /* CAN Baudrate */
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;

	  

#if 1 // 125K   
	  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	  CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	  CAN_InitStructure.CAN_Prescaler = 32;   
#endif
#if 0 // 250K
	  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	  CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	  CAN_InitStructure.CAN_Prescaler = 16;   
#endif   
#if 0 // 500K
	  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	  CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	  CAN_InitStructure.CAN_Prescaler = 8;	 
#endif  
#if 0 // 1MBps   
	CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	CAN_InitStructure.CAN_Prescaler = 4;
#endif

  
  CAN_Init(CAN1, &CAN_InitStructure);

  /* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
  /* Transmit */
  TxMessage.StdId = 0x321;
  TxMessage.ExtId = 0x01;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 1;
}






void Set_Can_Init()
{
	CAN_NVIC_Config();
	/* CAN configuration */
	CAN_Config();
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
	


}
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
  UART1_PRINTF("CAN1_RX0_IRQHandler\r\n");
  if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1))
  {
	  UART1_PRINTF("RxMessage.Data[0] =0x%2x \r\n" ,RxMessage.Data[0]);
    KeyNumber = RxMessage.Data[0];
  }
}

void CAN1_RX0_IRQHandler(void)
{
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
  UART1_PRINTF("CAN1_RX1_IRQHandler\r\n");
  if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1))
  {
	  UART1_PRINTF("RxMessage.Data[0] =0x%2x \r\n" ,RxMessage.Data[0]);
    KeyNumber = RxMessage.Data[0];
  }
}



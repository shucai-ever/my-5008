#ifndef DRV_CAN_H
#define DRV_CAN_H

#include "stm32f10x.h"

  #define RCC_APB2Periph_GPIO_CAN1    RCC_APB2Periph_GPIOA
  #define GPIO_Remapping_CAN1         GPIO_Remap1_CAN1
  #define GPIO_CAN1                   GPIOA
  #define GPIO_Pin_CAN1_RX            GPIO_Pin_11
  #define GPIO_Pin_CAN1_TX            GPIO_Pin_12


void CAN_NVIC_Config(void);
void CAN_Config(void);
void Set_Can_Init();


#endif

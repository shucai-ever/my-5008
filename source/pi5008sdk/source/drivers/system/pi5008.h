#ifndef PI5008_HEADER
#define PI5008_HEADER

#ifndef __ASSEMBLER__
#include <inttypes.h>
#include <nds32_intrinsic.h>
#endif
#include "system.h"

/*****************************************************************************
 * System clock
 ****************************************************************************/
#define KHz                     (1000)
#define MHz                     (1000000)
//#define MB_CPUCLK               (PCLK)
//#define MB_HCLK                 (MB_CPUCLK)
//#define MB_PCLK                 (MB_CPUCLK)

/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#define IRQ_0_VECTOR            (0)
#define IRQ_1_VECTOR            (1)
#define IRQ_2_VECTOR            (2)
#define IRQ_D10_VECTOR          (3)
#define IRQ_SWI_VECTOR          (4)
#define IRQ_SPI_FLASH_VECTOR    (5) // SPI - serial flash controller - NOT QSPI
#define IRQ_SPI_VECTOR          (6) // SPI com
#define IRQ_DMA_VECTOR          (7)
#define IRQ_WDT_VECTOR          (8)
#define IRQ_TIMER0_VECTOR       (9)
#define IRQ_TIMER1_VECTOR       (10)
#define IRQ_GPIO_VECTOR         (11)
#define IRQ_I2S_VECTOR          (12)
#define IRQ_UART0_VECTOR        (13)
#define IRQ_UART1_VECTOR        (14)
#define IRQ_UART2_VECTOR        (15)
#define IRQ_I2CM_VECTOR         (16)
#define IRQ_17_VECTOR           (17)
#define IRQ_18_VECTOR           (18)
#define IRQ_DU_VECTOR           (19)
#define IRQ_SVM_VECTOR          (20)
#define IRQ_VPU0_VECTOR         (21)
#define IRQ_VPU1_VECTOR         (22)
#define IRQ_EDGE_VECTOR         (23)
#define IRQ_24_VECTOR           (24)

/* IRQ priority level */
#define IRQ_PRIORITY_HIGHEST    (0U)
#define IRQ_PRIORITY_HIGH       (1U)
#define IRQ_PRIORITY_LOW        (2U)
#define IRQ_PRIORITY_LOWEST     (3U)

/* IRQ priority */
/* priority : TIMER > ... > SWI */
#define IRQ_0_PRI               (IRQ_PRIORITY_LOW)  // 0
#define IRQ_1_PRI               (IRQ_PRIORITY_LOW)  // 1
#define IRQ_2_PRI               (IRQ_PRIORITY_LOW)  // 2
#define IRQ_D10_PRI             (IRQ_PRIORITY_LOW)  // 3
#define IRQ_SWI_PRI             (IRQ_PRIORITY_LOW)  // 4
#define IRQ_SPI_MEMC_PRI        (IRQ_PRIORITY_LOW)  // 5
#define IRQ_SPI_PRI             (IRQ_PRIORITY_LOW)  // 6
#define IRQ_DMA_PRI             (IRQ_PRIORITY_LOW)  // 7
#define IRQ_WDT_PRI             (IRQ_PRIORITY_LOW)  // 8
#define IRQ_TIMER0_PRI          (IRQ_PRIORITY_LOW)  // 9
#define IRQ_TIMER1_PRI          (IRQ_PRIORITY_LOW)  // 10
#define IRQ_GPIO_PRI            (IRQ_PRIORITY_LOW)	// 11
#define IRQ_I2S_PRI             (IRQ_PRIORITY_LOW)	// 12
#define IRQ_UART0_PRI           (IRQ_PRIORITY_LOW)	// 13
#define IRQ_UART1_PRI           (IRQ_PRIORITY_LOW)	// 14
#define IRQ_UART2_PRI           (IRQ_PRIORITY_LOW)	// 15
#define IRQ_I2C_PRI             (IRQ_PRIORITY_LOW)	// 16
#define IRQ_17_PRI              (IRQ_PRIORITY_LOW)	// 17
#define IRQ_18_PRI              (IRQ_PRIORITY_LOW)	// 18
#define IRQ_DU_PRI              (IRQ_PRIORITY_LOW)	// 19
#define IRQ_SVM_PRI             (IRQ_PRIORITY_LOW)	// 20
#define IRQ_VPU0_PRI            (IRQ_PRIORITY_LOW)	// 21
#define IRQ_VPU1_PRI            (IRQ_PRIORITY_LOW)	// 22
#define IRQ_EDGE_PRI            (IRQ_PRIORITY_LOW)	// 23
#define IRQ_24_PRI              (IRQ_PRIORITY_HIGH)  	// 24

#define PRI1_DEFAULT      ((IRQ_0_PRI)             <<  0U) \
                        | ((IRQ_1_PRI)             <<  2U) \
                        | ((IRQ_2_PRI)             <<  4U) \
                        | ((IRQ_D10_PRI)           <<  6U) \
                        | ((IRQ_SWI_PRI)           <<  8U) \
                        | ((IRQ_SPI_MEMC_PRI)      << 10U) \
                        | ((IRQ_SPI_PRI)           << 12U) \
                        | ((IRQ_DMA_PRI)           << 14U) \
                        | ((IRQ_WDT_PRI)           << 16U) \
                        | ((IRQ_TIMER0_PRI)        << 18U) \
                        | ((IRQ_TIMER1_PRI)        << 20U) \
                        | ((IRQ_GPIO_PRI)          << 22U) \
                        | ((IRQ_I2S_PRI)           << 24U) \
                        | ((IRQ_UART0_PRI)         << 26U) \
                        | ((IRQ_UART1_PRI)         << 28U) \
                        | ((IRQ_UART2_PRI)         << 30U)

#define PRI2_DEFAULT      ((IRQ_I2C_PRI)           <<  0U) \
                        | ((IRQ_17_PRI)            <<  2U) \
                        | ((IRQ_18_PRI)            <<  4U) \
                        | ((IRQ_DU_PRI)            <<  6U) \
                        | ((IRQ_SVM_PRI)           <<  8U) \
                        | ((IRQ_VPU0_PRI)          << 10U) \
                        | ((IRQ_VPU1_PRI)          << 12U) \
                        | ((IRQ_EDGE_PRI)          << 14U) \
                        | ((IRQ_24_PRI)      	   << 16U)


/* The system tick IRQ for OS */
#define IRQ_SYS_TICK_VECTOR     IRQ_TIMER_VECTOR


#define ISR(name, vecId)            void name (int vecId)

/* get IRQ status mask */
#define IRQ_SPI_STATUS_MASK(dev)    ( (GetRegValue(SPI_IRQ_STATUS_ADDR) >> (dev)) & ((unsigned int)1U) )
#define IRQ_GPIO_STATUS_MASK(dev)   ( (GetRegValue(GPIO_IRQ_STATUS_ADDR) >> (dev)) & ((unsigned int)1U) )
#define IRQ_I2C_STATUS_MASK(dev)    ( (GetRegValue(I2C_IRQ_STATUS_ADDR) >> (dev)) & ((unsigned int)1U) )
#endif	/* PI5008_H */

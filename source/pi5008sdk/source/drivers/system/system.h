/* system.h */
#ifndef SYSTEM_H
#define SYSTEM_H

#include "config/board_config.h"

//FPGA test
#define SYSTEM_BUS_DW

#define XIN				27000000

/*
 *	          CPU   CPU_AXI	     APB    DDR_CLK   DDR
 *DDR_513	256.5    128.25	  64.125	  256.5	 1026
 *DDR_400	256.5	96.1875	48.09375	192.375	769.5
 */
//Select DDR speeed 513|400

#if defined(FPGA_ASIC_TOP)
	#define CPU_CLK		(74250000U)
	#define PCLK		(37125000U)
	#define UART_CLK	(27000000U)
	#define TIMER_CLK	(6750000U)		// Timer - Use External clock
	//#define WDT_CLK		PCLK			// Watchdog - Use APB clock
	#define WDT_CLK		(6750000U)		// Watchdog - Use External clock
#else	// #if defined(FPGA_ASIC_TOP)
	#define CPU_CLK		(249750000U)
	#define PCLK		(62437500U)
	#define UART_CLK	(27000000U)
	#define TIMER_CLK	(6750000U)		// Timer - Use External clock
	#define WDT_CLK		(6750000U)		// Watchdog - Use External clock
#endif

/* System Memory Map */
#define FLASH_BASE_ADDR             (0x00000000UL) /* On-chip Boot Memory */
#define SRAM_BASE_ADDR              (0x00010000UL) /* On-chip SRAM Memory */
#define SPI_MEM_BASE_ADDR           (0x10000000UL) /* SPI Flash Memory */
#define DRAM_MEM_BASE_ADDR          (0x20000000UL) /* DRAM Memory */

#define SCU_BASE_ADDR               (0xF0000000UL) /* SCU */
#define MISC_BASE_ADDR              (0xF0100000UL) /* MISC */
#define GPIO0_BASE_ADDR             (0xF0200000UL) /* GPIO0 (KEY, SW(LED)) */
#if defined(SYSTEM_BUS_DW)
#define GPIO1_BASE_ADDR             (0xF0201000UL) /* GPIO1 (MICOM) */
#define GPIO_IRQ_STATUS_ADDR        (0xF0210000UL) /* GPIO_IRQ_ST */
#endif
#define SUB_INTR0_BASE_ADDR    		(0xf0300000UL)
#define SUB_INTR1_BASE_ADDR    		(0xf0301000UL)
#define SUB_INTR2_BASE_ADDR    		(0xf0302000UL)
#define SUB_INTR3_BASE_ADDR    		(0xf0303000UL)

#define WDT_BASE_ADDR               (0xF0400000UL) /* WDT */
#define DDRC_BASE_ADDR              (0xF0500000UL) /* DDRC */
#define ISP0_BASE_ADDR              (0xF0600000UL) /* ISP0 */
#define ISP1_BASE_ADDR              (0xF0604000UL) /* ISP1 */
#define ISP2_BASE_ADDR              (0xF0608000UL) /* ISP2 */
#define ISP3_BASE_ADDR              (0xF060c000UL) /* ISP3 */

#define PVIRX0_BASE_ADDR            (0xF0700000UL) /* PVIRX0 */
#define PVIRX1_BASE_ADDR            (0xF0701000UL) /* PVIRX0 */
#define PVIRX2_BASE_ADDR            (0xF0702000UL) /* PVIRX0 */
#define PVIRX3_BASE_ADDR            (0xF0703000UL) /* PVIRX0 */

#define SVM_BASE_ADDR               (0xF0800000UL) /* SVM */
#define DU_BASE_ADDR                (0xF0900000UL) /* DU */
#define PVITX_BASE_ADDR                (0xF0901800UL) /* PVITX */

#define ADC_BASE_ADDR               (0xF0A00000UL) /* ADCC */
#define QSPI_BASE_ADDR              (0xF0B00000UL) /* QSPI Memory Controller (Read Only) */

#define TIMER0_BASE_ADDR            (0xF0C00000UL) /* TIMER 0 (4ch) */
#if defined(SYSTEM_BUS_DW)
#define TIMER1_BASE_ADDR            (0xF0C01000UL) /* TIMER 1 (4ch) */
#define TIMER_IRQ_STATUS_ADDR       (0xF0C10000UL) /* TIMER_IRQ_ST */
#endif
#define UART0_BASE_ADDR             (0xF0D00000UL) /* UART0 (PC DEBUG) */
#if defined(SYSTEM_BUS_DW)
#define UART1_BASE_ADDR             (0xF0D01000UL) /* UART1 (MICOM) */
#define UART2_BASE_ADDR             (0xF0D02000UL) /* UART2 */
#define UART_IRQ_STATUS_ADDR        (0xF0D10000UL) /* UART_IRQ_ST */
#endif
#define MIPI_BASE_ADDR              (0xF0E00000UL) /* MIPI */
#define VIN_BASE_ADDR				(0xF0F00000UL) /* Video Input */
#define I2CS_BASE_ADDR              (0xF1000000UL) /* I2CS (for picaso) */
#define I2CM0_BASE_ADDR             (0xF1100000UL) /* I2CMS 0 (Periboard, Parallel 0UL) */
#if defined(SYSTEM_BUS_DW)
#define I2CM1_BASE_ADDR             (0xF1101000UL) /* I2CMS 1 (OV9716, Parallel 1) */
#define I2CM2_BASE_ADDR             (0xF1102000UL) /* I2CMS 2 (Sensor 2, Parallel 2) */
#define I2CM3_BASE_ADDR             (0xF1103000UL) /* I2CMS 3 (Sensor 3, Parallel 3) */
#define I2C_IRQ_STATUS_ADDR         (0xF1110000UL) /* I2C_IRQ_ST */
#endif
#if defined(SYSTEM_BUS_DW)
#define SPI_FLASH_BASE_ADDR         (0xF0B01000UL) /* SPI serial IF 0 (for erase, program) */
#define SPI0_BASE_ADDR              (0xF1200000UL) /* SPI serial IF 1 (SD Card) */
#define SPI1_BASE_ADDR              (0xF1201000UL) /* SPI serial IF 2 (MICOM) */
#define SPI2_BASE_ADDR              (0xF1202000UL) /* SPI serial IF 3 (I2S) */
#define SPI_IRQ_STATUS_ADDR         (0xF1210000UL) /* SPI_IRQ_ST */
#else
#define SPI0_BASE_ADDR              (0xF1300000UL) /* SPI serial IF 0 (for erase, program) */
#endif

#if defined(SYSTEM_BUS_DW)
#define I2S_BASE_ADDR               (0xF1300000UL) /* I2S */
#endif

#define VPU_BASE_ADDR               (0xF1400000UL) /* VPU (Vaccel) */
#define VPU_DMA_MON_BASE_ADDR       (0xF1400600UL) /* VPU (Vaccel) */
#define VPU_HAMMING_BASE_ADDR       (0xF1410000UL) /* VPU (Vaccel) */
#define VPU_CONFIG_BASE_ADDR        (0xF1420000UL) /* VPU (Vaccel) */
#define VPU_INTC_BASE_ADDR          (0xF1421000UL) /* VPU (Vaccel) */
#define VPU_FILTER_BASE_ADDR        (0xF1422000UL) /* VPU (Vaccel) */
#define VPU_DEBUG_BASE_ADDR         (0xF1427020UL) /* VPU (Vaccel) */
#define CAN_BASE_ADDR               (0xF1440000UL) /* Edge Detector */
#define MBOX_BASE_ADDR				(0xF1500000UL) /* Mail Box */
//#define DMA_BASE_ADDR               (0xF2000000UL) /* DMA Register */
#define DMA_BASE_ADDR               (0xF1600000UL) /* DMA Register */

/* DMA Request Table */
#if 0
#define UART0_DMA_RX_REQ		0
#define UART0_DMA_TX_REQ		1
#define I2C_DMA_REQ				2
#define SPI0_DMA_RX_REQ			3
#define SPI0_DMA_TX_REQ			4
#define SPI1_DMA_RX_REQ			(-1)
#define SPI1_DMA_TX_REQ			(-1)
#define SPI2_DMA_RX_REQ			(-1)
#define SPI2_DMA_TX_REQ			(-1)
#define SPI3_DMA_RX_REQ			(-1)
#define SPI3_DMA_TX_REQ			(-1)
#define SPI_FLASH_DMA_RX_REQ	(-1)
#define SPI_FLASH_DMA_TX_REQ	6
#else
#define UART0_DMA_RX_REQ		0
#define UART0_DMA_TX_REQ		1
#define UART1_DMA_RX_REQ		2
#define UART1_DMA_TX_REQ		3
#define SPI_FLASH_DMA_RX_REQ	(-1)
#define SPI_FLASH_DMA_TX_REQ	4
#define SPI0_DMA_RX_REQ			5
#define SPI0_DMA_TX_REQ			6
#define SPI1_DMA_RX_REQ			7
#define SPI1_DMA_TX_REQ			8
#define SPI2_DMA_RX_REQ			9
#define SPI2_DMA_TX_REQ			10
#define SPI3_DMA_RX_REQ			(-1)
#define SPI3_DMA_TX_REQ			(-1)
#define I2C0_DMA_REQ			11
#define I2C1_DMA_REQ			12
#define I2C2_DMA_REQ			13
#define UART2_DMA_RX_REQ		14
#define UART2_DMA_TX_REQ		15
#endif



/* Low-level port I/O */
#define GetRegValue(reg)		(*((volatile unsigned int *)(reg)))
#define SetRegValue(reg, data)	((*((volatile unsigned int *)(reg))) = (unsigned int)(data))
#define BITCLR(reg, mask)		((*((volatile unsigned int *)(reg))) &= ~(unsigned int)(mask))
#define BITSET(reg, mask)		((*((volatile unsigned int *)(reg))) |= (unsigned int)(mask))

#define LOOPDELAY_10USEC(x)  { \
    int i; \
    volatile unsigned int rdata = 0; \
    for(i=0;i<(6*(x));i++) { \
        rdata += *(volatile unsigned int *) 0xf0000000; \
        rdata += *(volatile unsigned int *) 0xf0000000; \
        rdata += *(volatile unsigned int *) 0xf0000000; \
        rdata += *(volatile unsigned int *) 0xf0000000; \
    } \
    i = (int)rdata; \
}

#define LOOPDELAY_MSEC(x)  { \
    int i; \
    volatile unsigned int rdata = 0; \
    for(i=0;i<(600*(x));i++) { \
        rdata += *(volatile unsigned int *) 0xf0000000; \
        rdata += *(volatile unsigned int *) 0xf0000000; \
        rdata += *(volatile unsigned int *) 0xf0000000; \
        rdata += *(volatile unsigned int *) 0xf0000000; \
    } \
    i = (int)rdata; \
}


#endif /* SYSTEM_H */

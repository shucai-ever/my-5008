/* system.h */
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;


#define SYSTEM_BUS_DW

//==================================================
#define SPI_MEM_BASE_ADDR		(0x10000000)
#define DRAM_MEM_BASE_ADDR		(0x20000000)

#define SCU_BASE_ADDR			(0xf0000000)
#define MISC_BASE_ADDR			(0xf0100000)

#define GPIO0_BASE_ADDR			(0xf0200000)
#if defined(SYSTEM_BUS_DW)
#define GPIO1_BASE_ADDR			(0xf0201000)
#endif

#define WDT_BASE_ADDR			(0xf0400000)
#define DDRC_BASE_ADDR			(0xf0500000)
#define ISP_BASE_ADDR			(0xf0600000)
#define ISP_SRAM_BASE_ADDR		(0xf0700000)
#define SVM_BASE_ADDR			(0xf0800000)

#define DU_BASE_ADDR			(0xf0900000)
#define ADC_BASE_ADDR			(0xf0a00000)
#define QSPI_BASE_ADDR			(0xf0b00000)	//qspi memory controller

#define TIMER0_BASE_ADDR		(0xf0c00000)
#if defined(SYSTEM_BUS_DW)
#define TIMER1_BASE_ADDR		(0xf0c01000)
#define TIMER_IRQ_STATUS_ADDR	(0xf0c10000)
#endif

#define UART0_BASE_ADDR			(0xf0d00000)
#if defined(SYSTEM_BUS_DW)
#define UART1_BASE_ADDR			(0xf0d01000)
#define UART_IRQ_STATUS_ADDR	(0xf0d10000)
#endif

#define MIPI_BASE_ADDR			(0xf0e00000)

#define I2CS_BASE_ADDR			(0xf1000000)	//for picaso
#define I2CM0_BASE_ADDR			(0xf1100000)	//sensor
#if defined(SYSTEM_BUS_DW)
#define I2CM1_BASE_ADDR			(0xf1101000)
#define I2CM2_BASE_ADDR			(0xf1102000)
#define I2CM3_BASE_ADDR			(0xf1103000)
#define I2CM_IRQ_STATUS_ADDR	(0xf1110000)
#endif

#if defined(SYSTEM_BUS_DW)
#define SPI_FLASH_BASE_ADDR		(0xf0b01000)
#define SPI0_BASE_ADDR			(0xf1200000)
#define SPI1_BASE_ADDR			(0xf1201000)
#define SPI2_BASE_ADDR			(0xf1202000)
#define SPI_IRQ_STATUS_ADDR		(0xf1210000)
#else
#define SPI0_BASE_ADDR			(0xf1300000)
#endif

#if defined(SYSTEM_BUS_DW)
#define I2S_BASE_ADDR			(0xf1300000)
#endif

#define DMA_BASE_ADDR			(0xf2000000)
#define VPU_BASE_ADDR			(0xf1400000)

//===================================================
    
#define DDR_MEM_BASE	DRAM_MEM_BASE_ADDR
#define DDR2_MODE		0			
#define DDR3_MODE		0			
#define DDR_REG_BASE	0xf0500000

/* Low-level port I/O */
#define GetRegValue(reg)		(*((volatile unsigned int *)(reg)))
#define SetRegValue(reg, data)	((*((volatile unsigned int *)(reg))) = (unsigned int)(data))
#define BITCLR(reg, mask)		((*((volatile unsigned int *)(reg))) &= ~(unsigned int)(mask))
#define BITSET(reg, mask)		((*((volatile unsigned int *)(reg))) |= (unsigned int)(mask))

#define LOOPDELAY_10USEC(x)  \
{ \
    int i; \
    volatile unsigned int rdata; \
    for(i=0;i<(15*x);i++) { \
        rdata = *(volatile unsigned int *) 0xf0000000; \
        *(volatile unsigned int *) 0xf0000000 = rdata; \
    } \
}

#define LOOPDELAY_MSEC(x)  \
{ \
    int cnt; \
    for(cnt=0;cnt<x;cnt++) { \
        LOOPDELAY_10USEC(100); \
    } \
}

typedef void (*handler)(void);


#endif//__SYSTEM_H__

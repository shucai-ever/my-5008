/*
 * pinmux.h
 *
 *  Created on: 2017. 8. 17.
 *      Author: ihkong
 */

#ifndef PI5008_PINMUX_H_
#define PI5008_PINMUX_H_

// FUNCTION 0
#define PINMUX_0_GPIO0_0		(0<<0)
#define PINMUX_0_I2CS_SCL		(1<<0)

#define PINMUX_0_GPIO0_1		(0<<2)
#define PINMUX_0_I2CS_SDA		(1<<2)

#define PINMUX_0_GPIO0_2		(0<<4)
#define PINMUX_0_PWM_4			(1<<4)
#define PINMUX_0_I2SM_CLK		(2<<4)

#define PINMUX_0_GPIO0_3		(0<<6)
#define PINMUX_0_PWM_5			(1<<6)

#define PINMUX_0_GPIO0_4		(0<<8)
#define PINMUX_0_HD_PTZ_0		(1<<8)
#define PINMUX_0_UTC_MON_0		(2<<8)
#define PINMUX_0_I2S_OUT_CLK	(3<<8)

#define PINMUX_0_GPIO0_5		(0<<10)
#define PINMUX_0_HD_PTZ_1		(1<<10)
#define PINMUX_0_UTC_MON_1		(2<<10)
#define PINMUX_0_I2S_OUT_SYN	(3<<10)

#define PINMUX_0_GPIO0_6		(0<<12)
#define PINMUX_0_HD_PTZ_2		(1<<12)
#define PINMUX_0_UTC_MON_2		(2<<12)
#define PINMUX_0_I2S_OUT_DAT	(3<<12)

#define PINMUX_0_GPIO0_7		(0<<14)
#define PINMUX_0_HD_PTZ_3		(1<<14)
#define PINMUX_0_GENLOCK_P		(2<<14)

#define PINMUX_0_I2C0_DAT		(0<<16)
#define PINMUX_0_GPIO0_8		(1<<16)
#define PINMUX_0_MIPI_I2C_DAT	(2<<16)

#define PINMUX_0_I2C0_CLK		(0<<18)
#define PINMUX_0_GPIO0_9		(1<<18)
#define PINMUX_0_MIPI_I2C_CLK	(2<<18)

#define PINMUX_0_RO0_DATA_0		(0<<20)
#define PINMUX_0_GPIO0_10		(1<<20)
#define PINMUX_0_QO0_DATA_0		(2<<20)

#define PINMUX_0_RO0_DATA_1		(0<<22)
#define PINMUX_0_GPIO0_11		(1<<22)
#define PINMUX_0_QO0_DATA_1		(2<<22)

#define PINMUX_0_RO0_DATA_2		(0<<24)
#define PINMUX_0_GPIO0_12		(1<<24)
#define PINMUX_0_QO0_DATA_2		(2<<24)

#define PINMUX_0_RO0_DATA_3		(0<<26)
#define PINMUX_0_GPIO0_13		(1<<26)
#define PINMUX_0_QO0_DATA_3		(2<<26)

#define PINMUX_0_RO0_DATA_4		(0<<28)
#define PINMUX_0_GPIO0_14		(1<<28)
#define PINMUX_0_QO0_DATA_4		(2<<28)

#define PINMUX_0_RO0_DATA_5		(0<<30)
#define PINMUX_0_GPIO0_15		(1<<30)
#define PINMUX_0_QO0_DATA_5		(2<<30)

// FUNCTION 1
#define PINMUX_1_RO0_DATA_6		(0<<0)
#define PINMUX_1_GPIO0_16		(1<<0)
#define PINMUX_1_QO0_DATA_6		(2<<0)

#define PINMUX_1_RO0_DATA_7		(0<<2)
#define PINMUX_1_GPIO0_17		(1<<2)
#define PINMUX_1_QO0_DATA_7		(2<<2)

#define PINMUX_1_RO1_DATA_0		(0<<4)
#define PINMUX_1_GPIO0_18		(1<<4)
#define PINMUX_1_QO1_DATA_0		(2<<4)

#define PINMUX_1_RO1_DATA_1		(0<<6)
#define PINMUX_1_GPIO0_19		(1<<6)
#define PINMUX_1_QO1_DATA_1		(2<<6)

#define PINMUX_1_RO1_DATA_2		(0<<8)
#define PINMUX_1_GPIO0_20		(1<<8)
#define PINMUX_1_QO1_DATA_2		(2<<8)

#define PINMUX_1_RO1_DATA_3		(0<<10)
#define PINMUX_1_GPIO0_21		(1<<10)
#define PINMUX_1_QO1_DATA_3		(2<<10)

#define PINMUX_1_RO1_DATA_4		(0<<12)
#define PINMUX_1_GPIO0_22		(1<<12)
#define PINMUX_1_QO1_DATA_4		(2<<12)

#define PINMUX_1_RO1_DATA_5		(0<<14)
#define PINMUX_1_GPIO0_23		(1<<14)
#define PINMUX_1_QO1_DATA_5		(2<<14)

#define PINMUX_1_RO1_DATA_6		(0<<16)
#define PINMUX_1_GPIO0_24		(1<<16)
#define PINMUX_1_QO1_DATA_6		(2<<16)

#define PINMUX_1_RO1_DATA_7		(0<<18)
#define PINMUX_1_GPIO0_25		(1<<18)
#define PINMUX_1_QO1_DATA_7		(2<<18)

#define PINMUX_1_SPI0_SS0		(0<<20)
#define PINMUX_1_GPIO0_26		(1<<20)

#define PINMUX_1_SPI0_SS1		(0<<22)
#define PINMUX_1_GPIO0_27		(1<<22)

#define PINMUX_1_SPI0_SCK		(0<<24)
#define PINMUX_1_GPIO0_28		(1<<24)

#define PINMUX_1_SPI1_SS		(0<<26)
#define PINMUX_1_GPIO0_29		(1<<26)

#define PINMUX_1_SPI1_SCK		(0<<28)
#define PINMUX_1_GPIO0_30		(1<<28)

#define PINMUX_1_SPI1_MOSI		(0<<30)
#define PINMUX_1_GPIO0_31		(1<<30)

// FUNCTION 2
#define PINMUX_2_SPI1MISO		(0<<0)
#define PINMUX_2_GPIO1_0		(1<<0)

#define PINMUX_2_QSPI_SS		(0<<2)
#define PINMUX_2_GPIO1_1		(1<<2)

#define PINMUX_2_VI0_D_0		(0<<4)
#define PINMUX_2_GPIO1_2		(1<<4)
#define PINMUX_2_I2CS_SCL		(2<<4)

#define PINMUX_2_VI0_D_1		(0<<6)
#define PINMUX_2_GPIO1_3		(1<<6)
#define PINMUX_2_I2CS_SDA		(2<<6)

#define PINMUX_2_VI0_D_2		(0<<8)
#define PINMUX_2_GPIO1_4		(1<<8)
#define PINMUX_2_PWM_4			(2<<8)

#define PINMUX_2_VI0_D_3		(0<<10)
#define PINMUX_2_GPIO1_5		(1<<10)
#define PINMUX_2_PWM_5			(2<<10)

#define PINMUX_2_VI0_D_4		(0<<12)
#define PINMUX_2_GPIO1_6		(1<<12)
#define PINMUX_2_HD_PTZ_0		(2<<12)
#define PINMUX_2_UTC_MON_0		(2<<12)

#define PINMUX_2_VI0_D_5		(0<<14)
#define PINMUX_2_GPIO1_7		(1<<14)
#define PINMUX_2_HD_PTZ_1		(2<<14)
#define PINMUX_2_UTC_MON_1		(2<<14)

#define PINMUX_2_VI0_D_6		(0<<16)
#define PINMUX_2_GPIO1_8		(1<<16)
#define PINMUX_2_HD_PTZ_2		(2<<16)
#define PINMUX_2_UTC_MON_2		(2<<16)

#define PINMUX_2_VI0_D_7		(0<<18)
#define PINMUX_2_GPIO1_9		(1<<18)
#define PINMUX_2_HD_PTZ_3		(2<<18)

#define PINMUX_2_VI0_D_8		(0<<20)
#define PINMUX_2_GPIO1_10		(1<<20)
#define PINMUX_2_I2S_OUT_CLK	(2<<20)

#define PINMUX_2_VI0_D_9		(0<<22)
#define PINMUX_2_GPIO1_11		(1<<22)
#define PINMUX_2_I2S_OUT_SYN	(2<<22)

#define PINMUX_2_VI0_D_10		(0<<24)
#define PINMUX_2_GPIO1_12		(1<<24)
#define PINMUX_2_I2S_OUT_DAT	(2<<24)

#define PINMUX_2_VI0_D_11		(0<<26)
#define PINMUX_2_GPIO1_13		(1<<26)

#define PINMUX_2_VI0_HSYNC		(0<<28)
#define PINMUX_2_GPIO1_14		(1<<28)

#define PINMUX_2_VI0_VSYNC		(0<<30)
#define PINMUX_2_GPIO1_15		(1<<30)
#define PINMUX_2_GEN_LOCK_P		(1<<30)

// FUNCTION 3
#define PINMUX_3_VI0_PCLK		(0<<0)

#define PINMUX_3_VI1_D_0		(0<<2)
#define PINMUX_3_GPIO1_16		(1<<2)
#define PINMUX_3_HD_PTZ_0		(2<<2)
#define PINMUX_3_UTC_MON_0		(3<<2)

#define PINMUX_3_VI1_D_1		(0<<4)
#define PINMUX_3_GPIO1_17		(1<<4)
#define PINMUX_3_HD_PTZ_1		(2<<4)
#define PINMUX_3_UTC_MON_1		(3<<4)

#define PINMUX_3_VI1_D_2		(0<<6)
#define PINMUX_3_GPIO1_18		(1<<6)
#define PINMUX_3_HD_PTZ_2		(2<<6)
#define PINMUX_3_UTC_MON_2		(3<<6)

#define PINMUX_3_VI1_D_3		(0<<8)
#define PINMUX_3_GPIO1_19		(1<<8)
#define PINMUX_3_HD_PTZ_3		(2<<8)

#define PINMUX_3_VI1_D_4		(0<<10)
#define PINMUX_3_GPIO1_20		(1<<10)
#define PINMUX_3_I2CS_SCL		(2<<10)

#define PINMUX_3_VI1_D_5		(0<<12)
#define PINMUX_3_GPIO1_21		(1<<12)
#define PINMUX_3_I2CS_SDA		(2<<12)

#define PINMUX_3_VI1_D_6		(0<<14)
#define PINMUX_3_GPIO1_22		(1<<14)
#define PINMUX_3_PWM_4			(2<<14)

#define PINMUX_3_VI1_D_7		(0<<16)
#define PINMUX_3_GPIO1_23		(1<<16)
#define PINMUX_3_PWM_5			(2<<16)

#define PINMUX_3_VI1_D_8		(0<<18)
#define PINMUX_3_GPIO1_24		(1<<18)
#define PINMUX_3_I2S_OUT_CLK	(2<<18)

#define PINMUX_3_VI1_D_9		(0<<20)
#define PINMUX_3_GPIO1_25		(1<<20)
#define PINMUX_3_I2S_OUT_SYN	(2<<20)

#define PINMUX_3_VI1_D_10		(0<<22)
#define PINMUX_3_GPIO1_26		(1<<22)
#define PINMUX_3_I2S_OUT_DAT	(2<<22)

#define PINMUX_3_VI1_D_11		(0<<24)
#define PINMUX_3_GPIO1_27		(1<<24)

#define PINMUX_3_VI1_HSYNC		(0<<26)
#define PINMUX_3_GPIO1_28		(1<<26)

#define PINMUX_3_VI1_VSYNC		(0<<28)
#define PINMUX_3_GPIO1_29		(1<<28)
#define PINMUX_3_GEN_LOCK_P		(2<<28)

#define PINMUX_3_VI1_PCLK		(0<<30)

// FUNCTION 4
#define PINMUX_4_VODATA			(0<<0)
#define PINMUX_4_TEST_MODE		(1<<0)

#define PINMUX_4_UART1_RX		(0<<2)
#define PINMUX_4_HD_PTZ_0		(1<<2)
#define PINMUX_4_I2S_OUT_CLK	(2<<2)

#define PINMUX_4_UART1_TX		(0<<4)
#define PINMUX_4_HD_PTZ_1		(1<<4)
#define PINMUX_4_I2S_OUT_SYN	(2<<4)

#define PINMUX_4_UART2_RX		(0<<6)
#define PINMUX_4_GPIO1_30		(1<<6)
#define PINMUX_4_HD_PTZ_2		(2<<6)
#define PINMUX_4_I2S_OUT_DAT	(3<<6)

#define PINMUX_4_UART2_TX		(0<<8)
#define PINMUX_4_GPIO1_31		(1<<8)
#define PINMUX_4_HD_PTZ_3		(2<<8)


typedef struct ppPIN_MUX_S
{
	PP_U32 u32Group;
	PP_U32 u32BitPos;
	PP_U32 u32Val;
}PP_PIN_MUX_S;

void init_pinmux(void);
uint32 get_pinmux(uint32 group, uint32 shift);
void set_pinmux(uint32 group, uint32 shift, uint32 pinsel);
void clear_pinmux(uint32 group, uint32 shift);

void init_pad_io(void);

void PPDRV_SYSTEM_SetVOPadIO(PP_BOOL bOut);

extern PP_PIN_MUX_S gstGPIOPinMux[2][32];

#endif

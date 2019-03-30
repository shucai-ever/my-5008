/*
 * pinmux.c
 *
 *  Created on: 2017. 8. 17.
 *      Author: ihkong
 */

#include <stdio.h>
#include <unistd.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "debug.h"
#include "board_config.h"
#include "system.h"
#include "pinmux.h"



PP_PIN_MUX_S gstGPIOPinMux[2][32] = {
	// GPIO DEV0
	{
		{ 0, 0, PINMUX_0_GPIO0_0}, 		// GPIO DEV:0 Ch:0
		{ 0, 2, PINMUX_0_GPIO0_1}, 		// GPIO DEV:0 Ch:1
		{ 0, 4, PINMUX_0_GPIO0_2}, 		// GPIO DEV:0 Ch:2
		{ 0, 6, PINMUX_0_GPIO0_3}, 		// GPIO DEV:0 Ch:3
		{ 0, 8, PINMUX_0_GPIO0_4}, 		// GPIO DEV:0 Ch:4
		{ 0, 10, PINMUX_0_GPIO0_5}, 	// GPIO DEV:0 Ch:5
		{ 0, 12, PINMUX_0_GPIO0_6}, 	// GPIO DEV:0 Ch:6
		{ 0, 14, PINMUX_0_GPIO0_7}, 	// GPIO DEV:0 Ch:7
		{ 0, 16, PINMUX_0_GPIO0_8}, 	// GPIO DEV:0 Ch:8
		{ 0, 18, PINMUX_0_GPIO0_9}, 	// GPIO DEV:0 Ch:9
		{ 0, 20, PINMUX_0_GPIO0_10},	// GPIO DEV:0 Ch:10
		{ 0, 22, PINMUX_0_GPIO0_11},	// GPIO DEV:0 Ch:11
		{ 0, 24, PINMUX_0_GPIO0_12},	// GPIO DEV:0 Ch:12
		{ 0, 26, PINMUX_0_GPIO0_13},	// GPIO DEV:0 Ch:13
		{ 0, 28, PINMUX_0_GPIO0_14},	// GPIO DEV:0 Ch:14
		{ 0, 30, PINMUX_0_GPIO0_15},	// GPIO DEV:0 Ch:15
		{ 1, 0, PINMUX_1_GPIO0_16},		// GPIO DEV:0 Ch:16
		{ 1, 2, PINMUX_1_GPIO0_17},		// GPIO DEV:0 Ch:17
		{ 1, 4, PINMUX_1_GPIO0_18},		// GPIO DEV:0 Ch:18
		{ 1, 6, PINMUX_1_GPIO0_19},		// GPIO DEV:0 Ch:19
		{ 1, 8, PINMUX_1_GPIO0_20},		// GPIO DEV:0 Ch:20
		{ 1, 10, PINMUX_1_GPIO0_21},	// GPIO DEV:0 Ch:21
		{ 1, 12, PINMUX_1_GPIO0_22},	// GPIO DEV:0 Ch:22
		{ 1, 14, PINMUX_1_GPIO0_23},	// GPIO DEV:0 Ch:23
		{ 1, 16, PINMUX_1_GPIO0_24},	// GPIO DEV:0 Ch:24
		{ 1, 18, PINMUX_1_GPIO0_25},	// GPIO DEV:0 Ch:25
		{ 1, 20, PINMUX_1_GPIO0_26},	// GPIO DEV:0 Ch:26
		{ 1, 22, PINMUX_1_GPIO0_27},	// GPIO DEV:0 Ch:27
		{ 1, 24, PINMUX_1_GPIO0_28},	// GPIO DEV:0 Ch:28
		{ 1, 26, PINMUX_1_GPIO0_29},	// GPIO DEV:0 Ch:29
		{ 1, 28, PINMUX_1_GPIO0_30},	// GPIO DEV:0 Ch:30
		{ 1, 30, PINMUX_1_GPIO0_31},	// GPIO DEV:0 Ch:31
	},
	// GPIO DEV1
	{
		{ 2, 0, PINMUX_2_GPIO1_0}, 		// GPIO DEV:1 Ch:0
		{ 2, 2, PINMUX_2_GPIO1_1}, 		// GPIO DEV:1 Ch:1
		{ 2, 4, PINMUX_2_GPIO1_2}, 		// GPIO DEV:1 Ch:2
		{ 2, 6, PINMUX_2_GPIO1_3}, 		// GPIO DEV:1 Ch:3
		{ 2, 8, PINMUX_2_GPIO1_4}, 		// GPIO DEV:1 Ch:4
		{ 2, 10, PINMUX_2_GPIO1_5}, 	// GPIO DEV:1 Ch:5
		{ 2, 12, PINMUX_2_GPIO1_6}, 	// GPIO DEV:1 Ch:6
		{ 2, 14, PINMUX_2_GPIO1_7}, 	// GPIO DEV:1 Ch:7
		{ 2, 16, PINMUX_2_GPIO1_8}, 	// GPIO DEV:1 Ch:8
		{ 2, 18, PINMUX_2_GPIO1_9}, 	// GPIO DEV:1 Ch:9
		{ 2, 20, PINMUX_2_GPIO1_10},	// GPIO DEV:1 Ch:10
		{ 2, 22, PINMUX_2_GPIO1_11},	// GPIO DEV:1 Ch:11
		{ 2, 24, PINMUX_2_GPIO1_12},	// GPIO DEV:1 Ch:12
		{ 2, 26, PINMUX_2_GPIO1_13},	// GPIO DEV:1 Ch:13
		{ 2, 28, PINMUX_2_GPIO1_14},	// GPIO DEV:1 Ch:14
		{ 2, 30, PINMUX_2_GPIO1_15},	// GPIO DEV:1 Ch:15
		{ 3, 2, PINMUX_3_GPIO1_16},		// GPIO DEV:1 Ch:16
		{ 3, 4, PINMUX_3_GPIO1_17},		// GPIO DEV:1 Ch:17
		{ 3, 6, PINMUX_3_GPIO1_18},		// GPIO DEV:1 Ch:18
		{ 3, 8, PINMUX_3_GPIO1_19},		// GPIO DEV:1 Ch:19
		{ 3, 10, PINMUX_3_GPIO1_20},		// GPIO DEV:1 Ch:20
		{ 3, 12, PINMUX_3_GPIO1_21},	// GPIO DEV:1 Ch:21
		{ 3, 14, PINMUX_3_GPIO1_22},	// GPIO DEV:1 Ch:22
		{ 3, 16, PINMUX_3_GPIO1_23},	// GPIO DEV:1 Ch:23
		{ 3, 18, PINMUX_3_GPIO1_24},	// GPIO DEV:1 Ch:24
		{ 3, 20, PINMUX_3_GPIO1_25},	// GPIO DEV:1 Ch:25
		{ 3, 22, PINMUX_3_GPIO1_26},	// GPIO DEV:1 Ch:26
		{ 3, 24, PINMUX_3_GPIO1_27},	// GPIO DEV:1 Ch:27
		{ 3, 26, PINMUX_3_GPIO1_28},	// GPIO DEV:1 Ch:28
		{ 3, 28, PINMUX_3_GPIO1_29},	// GPIO DEV:1 Ch:29
		{ 4, 6, PINMUX_4_GPIO1_30},	// GPIO DEV:1 Ch:30
		{ 4, 8, PINMUX_4_GPIO1_31},	// GPIO DEV:1 Ch:31
	},

};

#ifdef FPGA_ASIC_TOP
void init_pinmux(void)
{/*{{{*/
	vuint32 pinmux0;
	vuint32 pinmux1;
	vuint32 pinmux2;
	vuint32 pinmux3;
	vuint32 pinmux4;

	pinmux0 = (
			// 0
			PINMUX_0_GPIO0_0		|
			//PINMUX_0_I2CS_SCL		|

			// 2
			PINMUX_0_GPIO0_1		|
			//PINMUX_0_I2CS_SDA		|

			// 4
			PINMUX_0_GPIO0_2		|
			//PINMUX_0_PWM_4			|
			//PINMUX_0_I2SM_CLK		|

			// 6
			PINMUX_0_GPIO0_3		|
			//PINMUX_0_PWM_5			|

			// 8
			//PINMUX_0_GPIO0_4		|
			PINMUX_0_HD_PTZ_0		|
			//PINMUX_0_UTC_MON_0		|
			//PINMUX_0_I2S_OUT_CLK	|

			// 10
			//PINMUX_0_GPIO0_5		|
			PINMUX_0_HD_PTZ_1		|
			//PINMUX_0_UTC_MON_1		|
			//PINMUX_0_I2S_OUT_SYN	|

			// 12
			//PINMUX_0_GPIO0_6		|
			PINMUX_0_HD_PTZ_2		|
			//PINMUX_0_UTC_MON_2		|
			//PINMUX_0_I2S_OUT_DAT	|

			// 14
			//PINMUX_0_GPIO0_7		|
			PINMUX_0_HD_PTZ_3		|
			//PINMUX_0_GENLOCK_P		|

			// 16
			PINMUX_0_I2C0_DAT		|
			//PINMUX_0_GPIO0_8		|
			//PINMUX_0_MIPI_I2C_DAT	|

			// 18
			PINMUX_0_I2C0_CLK		|
			//PINMUX_0_GPIO0_9		|
			//PINMUX_0_MIPI_I2C_CLK	|

			// 20
			PINMUX_0_RO0_DATA_0		|
			//PINMUX_0_GPIO0_10		|

			// 22
			PINMUX_0_RO0_DATA_1		|
			//PINMUX_0_GPIO0_11		|

			// 24
			PINMUX_0_RO0_DATA_2		|
			//PINMUX_0_GPIO0_12		|

			// 26
			PINMUX_0_RO0_DATA_3		|
			//PINMUX_0_GPIO0_13		|

			// 28
			PINMUX_0_RO0_DATA_4		|
			//PINMUX_0_GPIO0_14		|

			// 30
			PINMUX_0_RO0_DATA_5
			//PINMUX_0_GPIO0_15
		);


	pinmux1 = (
			// 0
			PINMUX_1_RO0_DATA_6		|
			//PINMUX_1_GPIO0_16		|

			// 2
			PINMUX_1_RO0_DATA_7		|
			//PINMUX_1_GPIO0_17		|

			// 4
			PINMUX_1_RO1_DATA_0		|
			//PINMUX_1_GPIO0_18		|

			// 6
			PINMUX_1_RO1_DATA_1		|
			//PINMUX_1_GPIO0_19		|

			// 8
			PINMUX_1_RO1_DATA_2		|
			//PINMUX_1_GPIO0_20		|

			// 10
			PINMUX_1_RO1_DATA_3		|
			//PINMUX_1_GPIO0_21		|

			// 12
			PINMUX_1_RO1_DATA_4		|
			//PINMUX_1_GPIO0_22		|

			// 14
			PINMUX_1_RO1_DATA_5		|
			//PINMUX_1_GPIO0_23		|

			// 16
			PINMUX_1_RO1_DATA_6		|
			//PINMUX_1_GPIO0_24		|

			// 18
			PINMUX_1_RO1_DATA_7		|
			//PINMUX_1_GPIO0_25		|

			// 20
			PINMUX_1_SPI0_SS0		|
			//PINMUX_1_GPIO0_26		|

			// 22
			PINMUX_1_SPI0_SS1		|
			//PINMUX_1_GPIO0_27		|

			// 24
			PINMUX_1_SPI0_SCK		|
			//PINMUX_1_GPIO0_28		|

			// 26
			PINMUX_1_SPI1_SS		|
			//PINMUX_1_GPIO0_29		|

			// 28
			PINMUX_1_SPI1_SCK		|
			//PINMUX_1_GPIO0_30		|

			// 30
			PINMUX_1_SPI1_MOSI
			//PINMUX_1_GPIO0_31
		);


	pinmux2 = (
			// 0
			PINMUX_2_SPI1MISO		|
			//PINMUX_2_GPIO1_0		|

			// 2
			PINMUX_2_QSPI_SS		|
			//PINMUX_2_GPIO1_1		|

			// 4
			PINMUX_2_VI0_D_0		|
			//PINMUX_2_GPIO1_2		|
			//PINMUX_2_I2CS_SCL		|

			// 6
			PINMUX_2_VI0_D_1		|
			//PINMUX_2_GPIO1_3		|
			//PINMUX_2_I2CS_SDA		|

			// 8
			PINMUX_2_VI0_D_2		|
			//PINMUX_2_GPIO1_4		|
			//PINMUX_2_PWM_4			|

			// 10
			PINMUX_2_VI0_D_3		|
			//PINMUX_2_GPIO1_5		|
			//PINMUX_2_PWM_5			|

			// 12
			PINMUX_2_VI0_D_4		|
			//PINMUX_2_GPIO1_6		|
			//PINMUX_2_HD_PTZ_0		|
			//PINMUX_2_UTC_MON_0		|

			// 14
			PINMUX_2_VI0_D_5		|
			//PINMUX_2_GPIO1_7		|
			//PINMUX_2_HD_PTZ_1		|
			//PINMUX_2_UTC_MON_1		|

			// 16
			PINMUX_2_VI0_D_6		|
			//PINMUX_2_GPIO1_8		|
			//PINMUX_2_HD_PTZ_2_		|
			//PINMUX_2_UTC_MON_2		|

			// 18
			PINMUX_2_VI0_D_7		|
			//PINMUX_2_GPIO1_9		|
			//PINMUX_2_HD_PTZ_3_		|

			// 20
			PINMUX_2_VI0_D_8		|
			//PINMUX_2_GPIO1_10		|
			//PINMUX_2_I2S_OUT_CLK	|

			// 22
			PINMUX_2_VI0_D_9		|
			//PINMUX_2_GPIO1_11		|
			//PINMUX_2_I2S_OUT_SYN	|

			// 24
			PINMUX_2_VI0_D_10		|
			//PINMUX_2_GPIO1_12		|
			//PINMUX_2_I2S_OUT_DAT_	|

			// 26
			PINMUX_2_VI0_D_11		|
			//PINMUX_2_GPIO1_13		|

			// 28
			PINMUX_2_VI0_HSYNC		|
			//PINMUX_2_GPIO1_14		|

			// 30
			PINMUX_2_VI0_VSYNC
			//PINMUX_2_GPIO1_15
			//PINMUX_2_GEN_LOCK_P
		);

	pinmux3 = (
			// 0
			PINMUX_3_VI0_PCLK		|
			//PINMUX_3_GPIO1_16		|

			// 2
			PINMUX_3_VI1_D_0		|
			//PINMUX_3_GPIO1_17		|
			//PINMUX_3_HD_PTZ_0		|
			//PINMUX_3_UTC_MON_0		|

			// 4
			PINMUX_3_VI1_D_1		|
			//PINMUX_3_GPIO1_18		|
			//PINMUX_3_HD_PTZ_1		|
			//PINMUX_3_UTC_MON_1		|

			// 6
			PINMUX_3_VI1_D_2		|
			//PINMUX_3_GPIO1_19		|
			//PINMUX_3_HD_PTZ_2		|
			//PINMUX_3_UTC_MON_2		|

			// 8
			PINMUX_3_VI1_D_3		|
			//PINMUX_3_GPIO1_20		|
			//PINMUX_3_HD_PTZ_3		|

			// 10
			PINMUX_3_VI1_D_4		|
			//PINMUX_3_GPIO1_21		|
			//PINMUX_3_I2CS_SCL		|

			// 12
			PINMUX_3_VI1_D_5		|
			//PINMUX_3_GPIO1_22		|
			//PINMUX_3_I2CS_SDA		|

			// 14
			PINMUX_3_VI1_D_6		|
			//PINMUX_3_GPIO1_23		|
			//PINMUX_3_PWM_4			|

			// 16
			PINMUX_3_VI1_D_7		|
			//PINMUX_3_GPIO1_24		|
			//PINMUX_3_PWM_5			|

			// 18
			PINMUX_3_VI1_D_8		|
			//PINMUX_3_GPIO1_25		|
			//PINMUX_3_I2S_OUT_CLK	|

			// 20
			PINMUX_3_VI1_D_9		|
			//PINMUX_3_GPIO1_26		|
			//PINMUX_3_I2S_OUT_SYN	|

			// 22
			PINMUX_3_VI1_D_10		|
			//PINMUX_3_GPIO1_27		|
			//PINMUX_3_I2S_OUT_DAT	|

			// 24
			PINMUX_3_VI1_D_11		|
			//PINMUX_3_GPIO1_28		|

			// 26
			PINMUX_3_VI1_HSYNC		|
			//PINMUX_3_GPIO1_29		|

			// 28
			PINMUX_3_VI1_VSYNC		|
			//PINMUX_3_GPIO1_30		|
			//PINMUX_3_GEN_LOCK_P		|

			// 30
			PINMUX_3_VI1_PCLK
			//PINMUX_3_GPIO1_31
		);


	pinmux4 = (
			// 0
			PINMUX_4_VODATA			|
			//PINMUX_4_TEST_MODE		|

			// 2
			PINMUX_4_UART1_RX		|
			//PINMUX_4_HD_PTZ_0		|
			//PINMUX_4_I2S_OUT_CLK	|

			// 4
			PINMUX_4_UART1_TX		|
			//PINMUX_4_HD_PTZ_1		|
			//PINMUX_4_I2S_OUT_SYN	|

			// 6
			PINMUX_4_UART2_RX		|
			//PINMUX_4_HD_PTZ_2		|
			//PINMUX_4_I2S_OUT_DAT	|

			// 8
			PINMUX_4_UART2_TX
			//PINMUX_4_HD_PTZ_3
		);


	*(vuint32 *)(MISC_BASE_ADDR + 0x58) = pinmux0;
	*(vuint32 *)(MISC_BASE_ADDR + 0x5c) = pinmux1;
	*(vuint32 *)(MISC_BASE_ADDR + 0x60) = pinmux2;
	*(vuint32 *)(MISC_BASE_ADDR + 0x64) = pinmux3;
	*(vuint32 *)(MISC_BASE_ADDR + 0x68) = pinmux4;

}/*}}}*/
#else    
void init_pinmux(void)
{/*{{{*/
	vuint32 pinmux0;
	vuint32 pinmux1;
	vuint32 pinmux2;
	vuint32 pinmux3;
	vuint32 pinmux4;

	pinmux0 = (//10101010 10100000 00111111 00100101
			// 0
			//PINMUX_0_GPIO0_0		|
			PINMUX_0_I2CS_SCL		|

			// 2
			//PINMUX_0_GPIO0_1		|
			PINMUX_0_I2CS_SDA		|

			// 4
			//PINMUX_0_GPIO0_2		|
			//PINMUX_0_PWM_4			|
			PINMUX_0_I2SM_CLK		|

			// 6
			PINMUX_0_GPIO0_3		|
			//PINMUX_0_PWM_5			|

			// 8
			//PINMUX_0_GPIO0_4		|
			//PINMUX_0_HD_PTZ_0		|
			//PINMUX_0_UTC_MON_0		|
			PINMUX_0_I2S_OUT_CLK	|

			// 10
			//PINMUX_0_GPIO0_5		|
			//PINMUX_0_HD_PTZ_1		|
			//PINMUX_0_UTC_MON_1		|
			PINMUX_0_I2S_OUT_SYN	|

			// 12
			//PINMUX_0_GPIO0_6		|
			//PINMUX_0_HD_PTZ_2		|
			//PINMUX_0_UTC_MON_2		|
			PINMUX_0_I2S_OUT_DAT	|

			// 14
			PINMUX_0_GPIO0_7		|
			//PINMUX_0_HD_PTZ_3		|
			//PINMUX_0_GENLOCK_P		|

			// 16
			PINMUX_0_I2C0_DAT		|
			//PINMUX_0_GPIO0_8		|
			//PINMUX_0_MIPI_I2C_DAT	|

			// 18
			PINMUX_0_I2C0_CLK		|
			//PINMUX_0_GPIO0_9		|
			//PINMUX_0_MIPI_I2C_CLK	|

			// 20
			//PINMUX_0_RO0_DATA_0		|
			//PINMUX_0_GPIO0_10		|
			PINMUX_0_QO0_DATA_0		|

			// 22
			//PINMUX_0_RO0_DATA_1		|
			//PINMUX_0_GPIO0_11		|
			PINMUX_0_QO0_DATA_1		|

			// 24
			//PINMUX_0_RO0_DATA_2		|
			//PINMUX_0_GPIO0_12		|
			PINMUX_0_QO0_DATA_2		|

			// 26
			//PINMUX_0_RO0_DATA_3		|
			//PINMUX_0_GPIO0_13		|
			PINMUX_0_QO0_DATA_3		|

			// 28
			//PINMUX_0_RO0_DATA_4		|
			//PINMUX_0_GPIO0_14		|
			PINMUX_0_QO0_DATA_4		|

			// 30
			//PINMUX_0_RO0_DATA_5     |
			//PINMUX_0_GPIO0_15     |
			PINMUX_0_QO0_DATA_5   |
		        
		    0
		);


	pinmux1 = (//00000000 00001010 10101010 10101010
			// 0
			//PINMUX_1_RO0_DATA_6		|
			//PINMUX_1_GPIO0_16		|
			PINMUX_1_QO0_DATA_6		|

			// 2
			//PINMUX_1_RO0_DATA_7		|
			//PINMUX_1_GPIO0_17		|
			PINMUX_1_QO0_DATA_7		|

			// 4
			//PINMUX_1_RO1_DATA_0		|
			//PINMUX_1_GPIO0_18		|
			PINMUX_1_QO1_DATA_0		|

			// 6
			//PINMUX_1_RO1_DATA_1		|
			//PINMUX_1_GPIO0_19		|
			PINMUX_1_QO1_DATA_1		|

			// 8
			//PINMUX_1_RO1_DATA_2		|
			//PINMUX_1_GPIO0_20		|
			PINMUX_1_QO1_DATA_2		|

			// 10
			//PINMUX_1_RO1_DATA_3		|
			//PINMUX_1_GPIO0_21		|
			PINMUX_1_QO1_DATA_3		|

			// 12
			//PINMUX_1_RO1_DATA_4		|
			//PINMUX_1_GPIO0_22		|
			PINMUX_1_QO1_DATA_4		|

			// 14
			//PINMUX_1_RO1_DATA_5		|
			//PINMUX_1_GPIO0_23		|
			PINMUX_1_QO1_DATA_5		|

			// 16
			//PINMUX_1_RO1_DATA_6		|
			//PINMUX_1_GPIO0_24		|
			PINMUX_1_QO1_DATA_6		|

			// 18
			//PINMUX_1_RO1_DATA_7		|
			//PINMUX_1_GPIO0_25		|
			PINMUX_1_QO1_DATA_7		|

			// 20
			PINMUX_1_SPI0_SS0		|
			//PINMUX_1_GPIO0_26		|

			// 22
			PINMUX_1_SPI0_SS1		|
			//PINMUX_1_GPIO0_27		|

			// 24
			PINMUX_1_SPI0_SCK		|
			//PINMUX_1_GPIO0_28		|

			// 26
			PINMUX_1_SPI1_SS		|
			//PINMUX_1_GPIO0_29		|

			// 28
			PINMUX_1_SPI1_SCK		|
			//PINMUX_1_GPIO0_30		|

			// 30
			PINMUX_1_SPI1_MOSI      |
			//PINMUX_1_GPIO0_31     |
		        
		    0
		);

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
	pinmux2 = (//00000000 00000000 00000000 00000000
			// 0
			PINMUX_2_SPI1MISO		|
			//PINMUX_2_GPIO1_0		|

			// 2
			PINMUX_2_QSPI_SS		|
			//PINMUX_2_GPIO1_1		|

			// 4
			PINMUX_2_VI0_D_0		|
			//PINMUX_2_GPIO1_2		|
			//PINMUX_2_I2CS_SCL		|

			// 6
			PINMUX_2_VI0_D_1		|
			//PINMUX_2_GPIO1_3		|
			//PINMUX_2_I2CS_SDA		|

			// 8
			PINMUX_2_VI0_D_2		|
			//PINMUX_2_GPIO1_4		|
			//PINMUX_2_PWM_4			|

			// 10
			PINMUX_2_VI0_D_3		|
			//PINMUX_2_GPIO1_5		|
			//PINMUX_2_PWM_5			|

			// 12
			PINMUX_2_VI0_D_4		|
			//PINMUX_2_GPIO1_6		|
			//PINMUX_2_HD_PTZ_0		|
			//PINMUX_2_UTC_MON_0		|

			// 14
			PINMUX_2_VI0_D_5		|
			//PINMUX_2_GPIO1_7		|
			//PINMUX_2_HD_PTZ_1		|
			//PINMUX_2_UTC_MON_1		|

			// 16
			PINMUX_2_VI0_D_6		|
			//PINMUX_2_GPIO1_8		|
			//PINMUX_2_HD_PTZ_2_		|
			//PINMUX_2_UTC_MON_2		|

			// 18
			PINMUX_2_VI0_D_7		|
			//PINMUX_2_GPIO1_9		|
			//PINMUX_2_HD_PTZ_3_		|

			// 20
			PINMUX_2_VI0_D_8		|
			//PINMUX_2_GPIO1_10		|
			//PINMUX_2_I2S_OUT_CLK	|

			// 22
			PINMUX_2_VI0_D_9		|
			//PINMUX_2_GPIO1_11		|
			//PINMUX_2_I2S_OUT_SYN	|

			// 24
			PINMUX_2_VI0_D_10		|
			//PINMUX_2_GPIO1_12		|
			//PINMUX_2_I2S_OUT_DAT_	|

			// 26
			PINMUX_2_VI0_D_11		|
			//PINMUX_2_GPIO1_13		|

			// 28
			PINMUX_2_VI0_HSYNC		|
			//PINMUX_2_GPIO1_14		|

			// 30
			PINMUX_2_VI0_VSYNC      |
			//PINMUX_2_GPIO1_15     |
			//PINMUX_2_GEN_LOCK_P   |
                
            0
		);
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
	pinmux2 = (
			// 0
			PINMUX_2_SPI1MISO		|
			//PINMUX_2_GPIO1_0		|

			// 2
			PINMUX_2_QSPI_SS		|
			//PINMUX_2_GPIO1_1		|

			// 4
			//PINMUX_2_VI0_D_0		|
			//PINMUX_2_GPIO1_2		|
			//PINMUX_2_I2CS_SCL		|

			// 6
			//PINMUX_2_VI0_D_1		|
			//PINMUX_2_GPIO1_3		|
			//PINMUX_2_I2CS_SDA		|

			// 8
			//PINMUX_2_VI0_D_2		|
			//PINMUX_2_GPIO1_4		|
			//PINMUX_2_PWM_4			|

			// 10
			//PINMUX_2_VI0_D_3		|
			//PINMUX_2_GPIO1_5		|
			//PINMUX_2_PWM_5			|

			// 12
			//PINMUX_2_VI0_D_4		|
			//PINMUX_2_GPIO1_6		|
			//PINMUX_2_HD_PTZ_0		|
			//PINMUX_2_UTC_MON_0		|

			// 14
			//PINMUX_2_VI0_D_5		|
			//PINMUX_2_GPIO1_7		|
			//PINMUX_2_HD_PTZ_1		|
			//PINMUX_2_UTC_MON_1		|

			// 16
			//PINMUX_2_VI0_D_6		|
			//PINMUX_2_GPIO1_8		|
			//PINMUX_2_HD_PTZ_2_		|
			//PINMUX_2_UTC_MON_2		|

			// 18
			//PINMUX_2_VI0_D_7		|
			//PINMUX_2_GPIO1_9		|
			//PINMUX_2_HD_PTZ_3_		|

			// 20
			//PINMUX_2_VI0_D_8		|
			//PINMUX_2_GPIO1_10		|
			//PINMUX_2_I2S_OUT_CLK	|

			// 22
			//PINMUX_2_VI0_D_9		|
			//PINMUX_2_GPIO1_11		|
			//PINMUX_2_I2S_OUT_SYN	|

			// 24
			//PINMUX_2_VI0_D_10		|
			//PINMUX_2_GPIO1_12		|
			//PINMUX_2_I2S_OUT_DAT_	|

			// 26
			//PINMUX_2_VI0_D_11		|
			//PINMUX_2_GPIO1_13		|

			// 28
			//PINMUX_2_VI0_HSYNC		|
			//PINMUX_2_GPIO1_14		|

			// 30
			//PINMUX_2_VI0_VSYNC    |
			//PINMUX_2_GPIO1_15     |
			//PINMUX_2_GEN_LOCK_P   |
                
            0
		);
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL)
	pinmux2 = (
			// 0
			PINMUX_2_SPI1MISO		|
			//PINMUX_2_GPIO1_0		|

			// 2
			PINMUX_2_QSPI_SS		|
			//PINMUX_2_GPIO1_1		|

			// 4
			PINMUX_2_VI0_D_0		|
			//PINMUX_2_GPIO1_2		|
			//PINMUX_2_I2CS_SCL		|

			// 6
			PINMUX_2_VI0_D_1		|
			//PINMUX_2_GPIO1_3		|
			//PINMUX_2_I2CS_SDA		|

			// 8
			PINMUX_2_VI0_D_2		|
			//PINMUX_2_GPIO1_4		|
			//PINMUX_2_PWM_4			|

			// 10
			PINMUX_2_VI0_D_3		|
			//PINMUX_2_GPIO1_5		|
			//PINMUX_2_PWM_5			|

			// 12
			PINMUX_2_VI0_D_4		|
			//PINMUX_2_GPIO1_6		|
			//PINMUX_2_HD_PTZ_0		|
			//PINMUX_2_UTC_MON_0		|

			// 14
			PINMUX_2_VI0_D_5		|
			//PINMUX_2_GPIO1_7		|
			//PINMUX_2_HD_PTZ_1		|
			//PINMUX_2_UTC_MON_1		|

			// 16
			PINMUX_2_VI0_D_6		|
			//PINMUX_2_GPIO1_8		|
			//PINMUX_2_HD_PTZ_2_		|
			//PINMUX_2_UTC_MON_2		|

			// 18
			PINMUX_2_VI0_D_7		|
			//PINMUX_2_GPIO1_9		|
			//PINMUX_2_HD_PTZ_3_		|

			// 20
			PINMUX_2_VI0_D_8		|
			//PINMUX_2_GPIO1_10		|
			//PINMUX_2_I2S_OUT_CLK	|

			// 22
			PINMUX_2_VI0_D_9		|
			//PINMUX_2_GPIO1_11		|
			//PINMUX_2_I2S_OUT_SYN	|

			// 24
			PINMUX_2_VI0_D_10		|
			//PINMUX_2_GPIO1_12		|
			//PINMUX_2_I2S_OUT_DAT_	|

			// 26
			PINMUX_2_VI0_D_11		|
			//PINMUX_2_GPIO1_13		|

			// 28
			PINMUX_2_VI0_HSYNC		|
			//PINMUX_2_GPIO1_14		|

			// 30
			PINMUX_2_VI0_VSYNC      |
			//PINMUX_2_GPIO1_15     |
			//PINMUX_2_GEN_LOCK_P   |
                
            0
		);
#else
	pinmux2 = (
			// 0
			PINMUX_2_SPI1MISO		|
			//PINMUX_2_GPIO1_0		|

			// 2
			PINMUX_2_QSPI_SS		|
			//PINMUX_2_GPIO1_1		|

			// 4
			PINMUX_2_VI0_D_0		|
			//PINMUX_2_GPIO1_2		|
			//PINMUX_2_I2CS_SCL		|

			// 6
			PINMUX_2_VI0_D_1		|
			//PINMUX_2_GPIO1_3		|
			//PINMUX_2_I2CS_SDA		|

			// 8
			PINMUX_2_VI0_D_2		|
			//PINMUX_2_GPIO1_4		|
			//PINMUX_2_PWM_4			|

			// 10
			PINMUX_2_VI0_D_3		|
			//PINMUX_2_GPIO1_5		|
			//PINMUX_2_PWM_5			|

			// 12
			PINMUX_2_VI0_D_4		|
			//PINMUX_2_GPIO1_6		|
			//PINMUX_2_HD_PTZ_0		|
			//PINMUX_2_UTC_MON_0		|

			// 14
			PINMUX_2_VI0_D_5		|
			//PINMUX_2_GPIO1_7		|
			//PINMUX_2_HD_PTZ_1		|
			//PINMUX_2_UTC_MON_1		|

			// 16
			PINMUX_2_VI0_D_6		|
			//PINMUX_2_GPIO1_8		|
			//PINMUX_2_HD_PTZ_2_		|
			//PINMUX_2_UTC_MON_2		|

			// 18
			PINMUX_2_VI0_D_7		|
			//PINMUX_2_GPIO1_9		|
			//PINMUX_2_HD_PTZ_3_		|

			// 20
			PINMUX_2_VI0_D_8		|
			//PINMUX_2_GPIO1_10		|
			//PINMUX_2_I2S_OUT_CLK	|

			// 22
			PINMUX_2_VI0_D_9		|
			//PINMUX_2_GPIO1_11		|
			//PINMUX_2_I2S_OUT_SYN	|

			// 24
			PINMUX_2_VI0_D_10		|
			//PINMUX_2_GPIO1_12		|
			//PINMUX_2_I2S_OUT_DAT_	|

			// 26
			PINMUX_2_VI0_D_11		|
			//PINMUX_2_GPIO1_13		|

			// 28
			PINMUX_2_VI0_HSYNC		|
			//PINMUX_2_GPIO1_14		|

			// 30
			PINMUX_2_VI0_VSYNC      |
			//PINMUX_2_GPIO1_15     |
			//PINMUX_2_GEN_LOCK_P   |
                
            0
		);
#endif

#if (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1)
	pinmux2 = (
			// 0
			PINMUX_2_SPI1MISO		|
			//PINMUX_2_GPIO1_0		|

			// 2
			PINMUX_2_QSPI_SS		|
			//PINMUX_2_GPIO1_1		|

			// 4
			PINMUX_2_VI0_D_0		|
			//PINMUX_2_GPIO1_2		|
			//PINMUX_2_I2CS_SCL		|

			// 6
			PINMUX_2_VI0_D_1		|
			//PINMUX_2_GPIO1_3		|
			//PINMUX_2_I2CS_SDA		|

			// 8
			PINMUX_2_VI0_D_2		|
			//PINMUX_2_GPIO1_4		|
			//PINMUX_2_PWM_4			|

			// 10
			PINMUX_2_VI0_D_3		|
			//PINMUX_2_GPIO1_5		|
			//PINMUX_2_PWM_5			|

			// 12
			PINMUX_2_VI0_D_4		|
			//PINMUX_2_GPIO1_6		|
			//PINMUX_2_HD_PTZ_0		|
			//PINMUX_2_UTC_MON_0		|

			// 14
			PINMUX_2_VI0_D_5		|
			//PINMUX_2_GPIO1_7		|
			//PINMUX_2_HD_PTZ_1		|
			//PINMUX_2_UTC_MON_1		|

			// 16
			PINMUX_2_VI0_D_6		|
			//PINMUX_2_GPIO1_8		|
			//PINMUX_2_HD_PTZ_2_		|
			//PINMUX_2_UTC_MON_2		|

			// 18
			PINMUX_2_VI0_D_7		|
			//PINMUX_2_GPIO1_9		|
			//PINMUX_2_HD_PTZ_3_		|

			// 20
			PINMUX_2_VI0_D_8		|
			//PINMUX_2_GPIO1_10		|
			//PINMUX_2_I2S_OUT_CLK	|

			// 22
			PINMUX_2_VI0_D_9		|
			//PINMUX_2_GPIO1_11		|
			//PINMUX_2_I2S_OUT_SYN	|

			// 24
			PINMUX_2_VI0_D_10		|
			//PINMUX_2_GPIO1_12		|
			//PINMUX_2_I2S_OUT_DAT_	|

			// 26
			PINMUX_2_VI0_D_11		|
			//PINMUX_2_GPIO1_13		|

			// 28
			PINMUX_2_VI0_HSYNC		|
			//PINMUX_2_GPIO1_14		|

			// 30
			PINMUX_2_VI0_VSYNC      |
			//PINMUX_2_GPIO1_15     |
			//PINMUX_2_GEN_LOCK_P   |
                
            0
		);
#endif //(VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1)

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
	pinmux3 = (//00000000 00000000 00000010 10101000
			// 0
			PINMUX_3_VI0_PCLK		|

			// 2
			//PINMUX_3_VI1_D_0		|
			//PINMUX_3_GPIO1_16		|
			PINMUX_3_HD_PTZ_0		|
			//PINMUX_3_UTC_MON_0		|

			// 4
			//PINMUX_3_VI1_D_1		|
			//PINMUX_3_GPIO1_17		|
			PINMUX_3_HD_PTZ_1		|
			//PINMUX_3_UTC_MON_1		|

			// 6
			//PINMUX_3_VI1_D_2		|
			//PINMUX_3_GPIO1_18		|
			PINMUX_3_HD_PTZ_2		|
			//PINMUX_3_UTC_MON_2		|

			// 8
			//PINMUX_3_VI1_D_3		|
			//PINMUX_3_GPIO1_19		|
			PINMUX_3_HD_PTZ_3		|

			// 10
			PINMUX_3_VI1_D_4		|
			//PINMUX_3_GPIO1_20		|
			//PINMUX_3_I2CS_SCL		|
			     
			// 12
			PINMUX_3_VI1_D_5		|
			//PINMUX_3_GPIO1_21		|
			//PINMUX_3_I2CS_SDA		|

			// 14
			PINMUX_3_VI1_D_6		|
			//PINMUX_3_GPIO1_22		|
			//PINMUX_3_PWM_4			|

			// 16
			PINMUX_3_VI1_D_7		|
			//PINMUX_3_GPIO1_23		|
			//PINMUX_3_PWM_5			|

			// 18
			PINMUX_3_VI1_D_8		|
			//PINMUX_3_GPIO1_24		|
			//PINMUX_3_I2S_OUT_CLK	|

			// 20
			PINMUX_3_VI1_D_9		|
			//PINMUX_3_GPIO1_25		|
			//PINMUX_3_I2S_OUT_SYN	|

			// 22
			PINMUX_3_VI1_D_10		|
			//PINMUX_3_GPIO1_26		|
			//PINMUX_3_I2S_OUT_DAT	|

			// 24
			PINMUX_3_VI1_D_11		|
			//PINMUX_3_GPIO1_27		|

			// 26
			PINMUX_3_VI1_HSYNC		|
			//PINMUX_3_GPIO1_28		|

			// 28
			PINMUX_3_VI1_VSYNC		|
			//PINMUX_3_GPIO1_29		|
			//PINMUX_3_GEN_LOCK_P		|

			// 30
			PINMUX_3_VI1_PCLK       |
                
            0
		);
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
	pinmux3 = (
			// 0
			PINMUX_3_VI0_PCLK		|

			// 2
			//PINMUX_3_VI1_D_0		|
			//PINMUX_3_GPIO1_16		|
			//PINMUX_3_HD_PTZ_0		|
			//PINMUX_3_UTC_MON_0		|

			// 4
			//PINMUX_3_VI1_D_1		|
			//PINMUX_3_GPIO1_17		|
			//PINMUX_3_HD_PTZ_1		|
			//PINMUX_3_UTC_MON_1		|

			// 6
			//PINMUX_3_VI1_D_2		|
			//PINMUX_3_GPIO1_18		|
			//PINMUX_3_HD_PTZ_2		|
			//PINMUX_3_UTC_MON_2		|

			// 8
			//PINMUX_3_VI1_D_3		|
			//PINMUX_3_GPIO1_19		|
			//PINMUX_3_HD_PTZ_3		|

			// 10
			//PINMUX_3_VI1_D_4		|
			//PINMUX_3_GPIO1_20		|
			//PINMUX_3_I2CS_SCL		|
			     
			// 12
			//PINMUX_3_VI1_D_5		|
			//PINMUX_3_GPIO1_21		|
			//PINMUX_3_I2CS_SDA		|

			// 14
			PINMUX_3_VI1_D_6		|
			//PINMUX_3_GPIO1_22		|
			//PINMUX_3_PWM_4			|

			// 16
			PINMUX_3_VI1_D_7		|
			//PINMUX_3_GPIO1_23		|
			//PINMUX_3_PWM_5			|

			// 18
			PINMUX_3_VI1_D_8		|
			//PINMUX_3_GPIO1_24		|
			//PINMUX_3_I2S_OUT_CLK	|

			// 20
			PINMUX_3_VI1_D_9		|
			//PINMUX_3_GPIO1_25		|
			//PINMUX_3_I2S_OUT_SYN	|

			// 22
			PINMUX_3_VI1_D_10		|
			//PINMUX_3_GPIO1_26		|
			//PINMUX_3_I2S_OUT_DAT	|

			// 24
			PINMUX_3_VI1_D_11		|
			//PINMUX_3_GPIO1_27		|

			// 26
			PINMUX_3_VI1_HSYNC		|
			//PINMUX_3_GPIO1_28		|

			// 28
			PINMUX_3_VI1_VSYNC		|
			//PINMUX_3_GPIO1_29		|
			//PINMUX_3_GEN_LOCK_P		|

			// 30
			PINMUX_3_VI1_PCLK       |
                
            0
		);
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL)
	pinmux3 = (
			// 0
			PINMUX_3_VI0_PCLK		|

			// 2
			PINMUX_3_VI1_D_0		|
			//PINMUX_3_GPIO1_16		|
			//PINMUX_3_HD_PTZ_0		|
			//PINMUX_3_UTC_MON_0		|

			// 4
			PINMUX_3_VI1_D_1		|
			//PINMUX_3_GPIO1_17		|
			//PINMUX_3_HD_PTZ_1		|
			//PINMUX_3_UTC_MON_1		|

			// 6
			PINMUX_3_VI1_D_2		|
			//PINMUX_3_GPIO1_18		|
			//PINMUX_3_HD_PTZ_2		|
			//PINMUX_3_UTC_MON_2		|

			// 8
			PINMUX_3_VI1_D_3		|
			//PINMUX_3_GPIO1_19		|
			//PINMUX_3_HD_PTZ_3		|

			// 10
			PINMUX_3_VI1_D_4		|
			//PINMUX_3_GPIO1_20		|
			//PINMUX_3_I2CS_SCL		|
			     
			// 12
			PINMUX_3_VI1_D_5		|
			//PINMUX_3_GPIO1_21		|
			//PINMUX_3_I2CS_SDA		|

			// 14
			PINMUX_3_VI1_D_6		|
			//PINMUX_3_GPIO1_22		|
			//PINMUX_3_PWM_4			|

			// 16
			PINMUX_3_VI1_D_7		|
			//PINMUX_3_GPIO1_23		|
			//PINMUX_3_PWM_5			|

			// 18
			PINMUX_3_VI1_D_8		|
			//PINMUX_3_GPIO1_24		|
			//PINMUX_3_I2S_OUT_CLK	|

			// 20
			PINMUX_3_VI1_D_9		|
			//PINMUX_3_GPIO1_25		|
			//PINMUX_3_I2S_OUT_SYN	|

			// 22
			PINMUX_3_VI1_D_10		|
			//PINMUX_3_GPIO1_26		|
			//PINMUX_3_I2S_OUT_DAT	|

			// 24
			PINMUX_3_VI1_D_11		|
			//PINMUX_3_GPIO1_27		|

			// 26
			PINMUX_3_VI1_HSYNC		|
			//PINMUX_3_GPIO1_28		|

			// 28
			PINMUX_3_VI1_VSYNC		|
			//PINMUX_3_GPIO1_29		|
			//PINMUX_3_GEN_LOCK_P		|

			// 30
			PINMUX_3_VI1_PCLK       |
                
            0
		);
#else
	pinmux3 = (
			// 0
			PINMUX_3_VI0_PCLK		|

			// 2
			PINMUX_3_VI1_D_0		|
			//PINMUX_3_GPIO1_16		|
			//PINMUX_3_HD_PTZ_0		|
			//PINMUX_3_UTC_MON_0		|

			// 4
			PINMUX_3_VI1_D_1		|
			//PINMUX_3_GPIO1_17		|
			//PINMUX_3_HD_PTZ_1		|
			//PINMUX_3_UTC_MON_1		|

			// 6
			PINMUX_3_VI1_D_2		|
			//PINMUX_3_GPIO1_18		|
			//PINMUX_3_HD_PTZ_2		|
			//PINMUX_3_UTC_MON_2		|

			// 8
			PINMUX_3_VI1_D_3		|
			//PINMUX_3_GPIO1_19		|
			//PINMUX_3_HD_PTZ_3		|

			// 10
			//PINMUX_3_VI1_D_4		|
			//PINMUX_3_GPIO1_20		|
			PINMUX_3_I2CS_SCL		|
			     
			// 12
			//PINMUX_3_VI1_D_5		|
			//PINMUX_3_GPIO1_21		|
			PINMUX_3_I2CS_SDA		|

			// 14
			PINMUX_3_VI1_D_6		|
			//PINMUX_3_GPIO1_22		|
			//PINMUX_3_PWM_4			|

			// 16
			PINMUX_3_VI1_D_7		|
			//PINMUX_3_GPIO1_23		|
			//PINMUX_3_PWM_5			|

			// 18
			PINMUX_3_VI1_D_8		|
			//PINMUX_3_GPIO1_24		|
			//PINMUX_3_I2S_OUT_CLK	|

			// 20
			PINMUX_3_VI1_D_9		|
			//PINMUX_3_GPIO1_25		|
			//PINMUX_3_I2S_OUT_SYN	|

			// 22
			PINMUX_3_VI1_D_10		|
			//PINMUX_3_GPIO1_26		|
			//PINMUX_3_I2S_OUT_DAT	|

			// 24
			PINMUX_3_VI1_D_11		|
			//PINMUX_3_GPIO1_27		|

			// 26
			PINMUX_3_VI1_HSYNC		|
			//PINMUX_3_GPIO1_28		|

			// 28
			PINMUX_3_VI1_VSYNC		|
			//PINMUX_3_GPIO1_29		|
			//PINMUX_3_GEN_LOCK_P		|

			// 30
			PINMUX_3_VI1_PCLK       |
                
            0
		);
#endif

#if (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1)
	pinmux3 = (
			// 0
			PINMUX_3_VI0_PCLK		|

			// 2
			PINMUX_3_VI1_D_0		|
			//PINMUX_3_GPIO1_16		|
			//PINMUX_3_HD_PTZ_0		|
			//PINMUX_3_UTC_MON_0		|

			// 4
			PINMUX_3_VI1_D_1		|
			//PINMUX_3_GPIO1_17		|
			//PINMUX_3_HD_PTZ_1		|
			//PINMUX_3_UTC_MON_1		|

			// 6
			PINMUX_3_VI1_D_2		|
			//PINMUX_3_GPIO1_18		|
			//PINMUX_3_HD_PTZ_2		|
			//PINMUX_3_UTC_MON_2		|

			// 8
			PINMUX_3_VI1_D_3		|
			//PINMUX_3_GPIO1_19		|
			//PINMUX_3_HD_PTZ_3		|

			// 10
			PINMUX_3_VI1_D_4		|
			//PINMUX_3_GPIO1_20		|
			//PINMUX_3_I2CS_SCL		|
			     
			// 12
			PINMUX_3_VI1_D_5		|
			//PINMUX_3_GPIO1_21		|
			//PINMUX_3_I2CS_SDA		|

			// 14
			PINMUX_3_VI1_D_6		|
			//PINMUX_3_GPIO1_22		|
			//PINMUX_3_PWM_4			|

			// 16
			PINMUX_3_VI1_D_7		|
			//PINMUX_3_GPIO1_23		|
			//PINMUX_3_PWM_5			|

			// 18
			PINMUX_3_VI1_D_8		|
			//PINMUX_3_GPIO1_24		|
			//PINMUX_3_I2S_OUT_CLK	|

			// 20
			PINMUX_3_VI1_D_9		|
			//PINMUX_3_GPIO1_25		|
			//PINMUX_3_I2S_OUT_SYN	|

			// 22
			PINMUX_3_VI1_D_10		|
			//PINMUX_3_GPIO1_26		|
			//PINMUX_3_I2S_OUT_DAT	|

			// 24
			PINMUX_3_VI1_D_11		|
			//PINMUX_3_GPIO1_27		|

			// 26
			PINMUX_3_VI1_HSYNC		|
			//PINMUX_3_GPIO1_28		|

			// 28
			PINMUX_3_VI1_VSYNC		|
			//PINMUX_3_GPIO1_29		|
			//PINMUX_3_GEN_LOCK_P		|

			// 30
			PINMUX_3_VI1_PCLK       |
                
            0
		);
#endif //(VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1)

	pinmux4 = (//00000000 00000000 00000001 01000000
			// 0
			PINMUX_4_VODATA			|
			//PINMUX_4_TEST_MODE		|

			// 2
			PINMUX_4_UART1_RX		|
			//PINMUX_4_HD_PTZ_0		|
			//PINMUX_4_I2S_OUT_CLK	|

			// 4
			PINMUX_4_UART1_TX		|
			//PINMUX_4_HD_PTZ_1		|
			//PINMUX_4_I2S_OUT_SYN	|

			// 6
			PINMUX_4_UART2_RX		|
			PINMUX_4_GPIO1_30		|
			//PINMUX_4_HD_PTZ_2		|
			//PINMUX_4_I2S_OUT_DAT	|

			// 8
			PINMUX_4_UART2_TX       |
			PINMUX_4_GPIO1_31		|
			//PINMUX_4_HD_PTZ_3     |

			0
		);

	//MISC_BASE_ADDR = 0x0100 0000UL
	*(vuint32 *)(MISC_BASE_ADDR + 0x58) = pinmux0;
	*(vuint32 *)(MISC_BASE_ADDR + 0x5c) = pinmux1;
	*(vuint32 *)(MISC_BASE_ADDR + 0x60) = pinmux2;
	*(vuint32 *)(MISC_BASE_ADDR + 0x64) = pinmux3;
	*(vuint32 *)(MISC_BASE_ADDR + 0x68) = pinmux4;

}/*}}}*/
#endif // FPGA_ASIC_TOP

uint32 get_pinmux(uint32 group, uint32 shift)
{
	uint32 reg;

	reg = *(vuint32 *)(MISC_BASE_ADDR + 0x58 + group*4);

	reg &= (3<<shift);

	return reg;
}

void set_pinmux(uint32 group, uint32 shift, uint32 pinsel)
{
	uint32 reg;

	reg = *(vuint32 *)(MISC_BASE_ADDR + 0x58 + group*4);

	reg &= (~(3<<shift));
	reg |= pinsel;

	*(vuint32 *)(MISC_BASE_ADDR + 0x58 + group*4) = reg;
}

void clear_pinmux(uint32 group, uint32 shift)
{
	uint32 reg;

	reg = *(vuint32 *)(MISC_BASE_ADDR + 0x58 + group*4);

	reg &= (~(3<<shift));

	*(vuint32 *)(MISC_BASE_ADDR + 0x58 + group*4) = reg;

}



#ifdef FPGA_ASIC_TOP
void init_pad_io(void)
{
	*(vuint32 *)(MISC_BASE_ADDR + 0x04) = 0x02020202; //PTZ0~3 output direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x4c) = 0x00020202;
	*(vuint32 *)(MISC_BASE_ADDR + 0x50) = 0x02020202;

	*(vuint32 *)(MISC_BASE_ADDR + 0x70) = 0x00104078;
}
#else
void init_pad_io(void)
{
	*(vuint32 *)(MISC_BASE_ADDR)		= 0x01010101;
	*(vuint32 *)(MISC_BASE_ADDR + 0x10) = 0x01020302; //SDCARD sck in/out
	*(vuint32 *)(MISC_BASE_ADDR + 0x18) = 0x03030302; //QSPI sck in/out
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
	*(vuint32 *)(MISC_BASE_ADDR + 0x3C) = 0x02020202; //PTZ0~3 output direction
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
	//*(vuint32 *)(MISC_BASE_ADDR + 0x2C) = 0x02020202; //VI0_D0~3 output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x30) = 0x02020202; //VI0_D4~7 output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x34) = 0x02020202; //VI0_8~11 output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x38) = 0x00020202; //VI0_H/V/PCLK output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x3C) = 0x02020202; //VI1_D0~3 output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x40) = 0x02020202; //VI1_D4~7 output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x44) = 0x02020202; //VI1_D8~11 output direction
	//*(vuint32 *)(MISC_BASE_ADDR + 0x48) = 0x00020202; //VI1_H/V/PCLK output direction
    *(vuint32 *)(MISC_BASE_ADDR + 0x2C) = 0x01010101; //VI0_D0~3 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x30) = 0x01010101; //VI0_D4~7 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x34) = 0x01010101; //VI0_8~11 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x38) = 0x00010101; //VI0_H/V/PCLK input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x3C) = 0x01010101; //VI1_D0~3 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x40) = 0x01010101; //VI1_D4~7 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x44) = 0x01010101; //VI1_D8~11 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x48) = 0x00010101; //VI1_H/V/PCLK input direction
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL)
	*(vuint32 *)(MISC_BASE_ADDR + 0x2C) = 0x01010101; //VI0_D0~3 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x30) = 0x01010101; //VI0_D4~7 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x34) = 0x01010101; //VI0_8~11 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x38) = 0x00010101; //VI0_H/V/PCLK input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x3C) = 0x01010101; //VI1_D0~3 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x40) = 0x01010101; //VI1_D4~7 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x44) = 0x01010101; //VI1_D8~11 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x48) = 0x00010101; //VI1_H/V/PCLK input direction
#else
	*(vuint32 *)(MISC_BASE_ADDR + 0x3C) = 0x01010101; //PTZ0~3 input direction
#endif

#if (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1)
	*(vuint32 *)(MISC_BASE_ADDR + 0x2C) = 0x01010101; //VI0_D0~3 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x30) = 0x01010101; //VI0_D4~7 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x34) = 0x01010101; //VI0_8~11 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x38) = 0x00010101; //VI0_H/V/PCLK input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x3C) = 0x01010101; //VI1_D0~3 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x40) = 0x01010101; //VI1_D4~7 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x44) = 0x01010101; //VI1_D8~11 input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x48) = 0x00010101; //VI1_H/V/PCLK input direction
	*(vuint32 *)(MISC_BASE_ADDR + 0x70) = 0x00104078;
#endif //(VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1)

    PPDRV_SYSTEM_SetVOPadIO(0); //0:input, else:output
}
#endif // FPGA_ASIC_TOP

void PPDRV_SYSTEM_SetVOPadIO(PP_BOOL bOut)
{
    if(bOut)
    {
#if ( ((BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT) )
        //*(vuint32 *)(MISC_BASE_ADDR + 0x4c) = 0x00010102; //VODATA 8bit output
        *(vuint32 *)(MISC_BASE_ADDR + 0x4c) = 0x00010202; //VODATA 8bit output
#elif ( ((BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC16_EMB) || ((BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC16_EXT) )
        *(vuint32 *)(MISC_BASE_ADDR + 0x4c) = 0x00010202; //VODATA 16bit output
#elif ((BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_RGB24)
        *(vuint32 *)(MISC_BASE_ADDR + 0x4c) = 0x00020202; //VODATA 24bit output
#else
# error "Invalid DU video signal format"
#endif
        *(vuint32 *)(MISC_BASE_ADDR + 0x50) = 0x02020202; //VO H/V/Clk/En enable.
    }
    else
    {
        *(vuint32 *)(MISC_BASE_ADDR + 0x4c) = 0x00010101; //VODATA 8bit output
        *(vuint32 *)(MISC_BASE_ADDR + 0x50) = 0x01010101; //VO H/V/Clk/En enable.
    }
}


/*
 * asic_init.c
 *
 *  Created on: 2017. 6. 29.
 *      Author: ihkong
 */

void asic_init(void)
{

#if	( (BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P )
// FHD
    *(volatile unsigned int *) 0xf0000050    = 0x00000101; // fpll_con
    *(volatile unsigned int *) 0xf0000054    = 0x041f3210; // mpll0_con
    *(volatile unsigned int *) 0xf0000058    = 0x041f3210; // mpll1_con

    // pll selection[7:0], [7:4] reserved, [3] = XIN or mux out select, [2:0] = clock select
    *(volatile unsigned int *) 0xf0000000    = 0x0A080808; // apb, ddr_axi, cpu_axi, cpu
    *(volatile unsigned int *) 0xf0000004    = 0x00080808; // uart, wdt, timer, qspi
    *(volatile unsigned int *) 0xf0000008    = 0x00000008; // gpio, svm, i2s, gadc
    *(volatile unsigned int *) 0xf000000C    = 0x00000000; // pvi_rx3, rx2, rx1, rx0
    *(volatile unsigned int *) 0xf0000010    = 0x00000000; // vadc3, vadc2, vadc1, vadc0
    *(volatile unsigned int *) 0xf0000014    = 0x00000000; // isp3, isp2, isp1, isp0
    *(volatile unsigned int *) 0xf0000018    = 0x00000000; // pvi_tx, du, vpu, vin
    *(volatile unsigned int *) 0xf000001C    = 0x00000000; // Rev, recfmt, vdac, pvi_tx_fifo
    *(volatile unsigned int *) 0xf0000020    = 0x00000000; // Rev, recfmt_2x_mph_1, 2x_mph_0, recfmt_2x
    *(volatile unsigned int *) 0xf0000024    = 0x00000000; // disfmt_2x_mph, Rev, disfmt, Rev
    *(volatile unsigned int *) 0xf0000028    = 0x00000000; // mipi_pixel, mipi_lane, rxbyteclkhs, Rev
    *(volatile unsigned int *) 0xf000002C    = 0x08080000; // ddr_postclk_2x, postclk, ctclk, mipi_cci // fpga
    *(volatile unsigned int *) 0xf000002C    = 0x08080800; // ddr_postclk_2x, postclk, ctclk, mipi_cci // asic
    *(volatile unsigned int *) 0xf0000030    = 0x00000000; // vdck1, vdck0, dmclk, tck
    *(volatile unsigned int *) 0xf0000034    = 0x00000000; // polarity
    *(volatile unsigned int *) 0xf0000038    = 0x00000000; // Rev
    *(volatile unsigned int *) 0xf000003C    = 0x00000000; // Rev
    *(volatile unsigned int *) 0xf0000040    = 0xffffffff; // sw rst
    *(volatile unsigned int *) 0xf0000044    = 0xffffffff; // sw rst
    *(volatile unsigned int *) 0xf0000048    = 0xffffffff; // sw rst
    *(volatile unsigned int *) 0xf000004C    = 0; // Rev
    *(volatile unsigned int *) 0xf000005C    = 0; // vdac_con
    *(volatile unsigned int *) 0xf0000060    = 0; // bgr_con

    // Pad control (each register control 8bit only)
    // Common Control Signals from MISC, [6]=PIN2, [5]=PIN1, [4]=PD, [3]=PU, [2]=SMT, [1]=OE, [0]=IE
    *(volatile unsigned int *) 0xf0100000    = 0x01; // IOC_TEST
    *(volatile unsigned int *) 0xf0100004    = 0x01; // IOC_RSTB
    *(volatile unsigned int *) 0xf0100008    = 0x01; // IOC_OPT
    *(volatile unsigned int *) 0xf010000C    = 0x01; // IOC_BOOT_MD
    *(volatile unsigned int *) 0xf0100010    = 0x00; // IOC_GPIO_0
    *(volatile unsigned int *) 0xf0100014    = 0x00; // IOC_GPIO_1
    *(volatile unsigned int *) 0xf0100018    = 0x00; // IOC_GPIO_2
    *(volatile unsigned int *) 0xf010001C    = 0x00; // IOC_GPIO_3
    *(volatile unsigned int *) 0xf0100020    = 0x00; // IOC_GPIO_4
    *(volatile unsigned int *) 0xf0100024    = 0x00; // IOC_GPIO_5
    *(volatile unsigned int *) 0xf0100028    = 0x00; // IOC_GPIO_6
    *(volatile unsigned int *) 0xf010002C    = 0x00; // IOC_GPIO_7
    *(volatile unsigned int *) 0xf0100030    = 0x02; // IOC_I2C0_DAT
    *(volatile unsigned int *) 0xf0100034    = 0x02; // IOC_I2C0_CLK
    *(volatile unsigned int *) 0xf0100038    = 0x02; // IOC_I2C1_DAT
    *(volatile unsigned int *) 0xf010003C    = 0x02; // IOC_I2C1_CLK
    *(volatile unsigned int *) 0xf0100040    = 0x02; // IOC_I2C2_DAT
    *(volatile unsigned int *) 0xf0100044    = 0x02; // IOC_I2C2_CLK
    *(volatile unsigned int *) 0xf0100048    = 0x02; // IOC_SPI0_SS0
    *(volatile unsigned int *) 0xf010004C    = 0x02; // IOC_SPI0_SS1
    *(volatile unsigned int *) 0xf0100050    = 0x02; // IOC_SPI0_SLK
    *(volatile unsigned int *) 0xf0100054    = 0x02; // IOC_SPI0_MOSI
    *(volatile unsigned int *) 0xf0100058    = 0x01; // IOC_SPI0_MISO
    *(volatile unsigned int *) 0xf010005C    = 0x02; // IOC_SPI1_SS
    *(volatile unsigned int *) 0xf0100060    = 0x02; // IOC_SPI1_SLK
    *(volatile unsigned int *) 0xf0100064    = 0x02; // IOC_SPI1_MOSI
    *(volatile unsigned int *) 0xf0100068    = 0x01; // IOC_SPI1_MISO
    *(volatile unsigned int *) 0xf010006C    = 0x03; // IOC_QSPI_SS
    *(volatile unsigned int *) 0xf0100070    = 0x02; // IOC_QSPI_SCK
    *(volatile unsigned int *) 0xf0100074    = 0x03; // IOC_QSPI_D0
    *(volatile unsigned int *) 0xf0100078    = 0x03; // IOC_QSPI_D1
    *(volatile unsigned int *) 0xf010007C    = 0x03; // IOC_QSPI_D2
    *(volatile unsigned int *) 0xf0100080    = 0x03; // IOC_QSPI_D3
    *(volatile unsigned int *) 0xf0100084    = 0x01; // IOC_I2S_OUT_CLK
    *(volatile unsigned int *) 0xf0100088    = 0x01; // IOC_I2S_OUT_SYN
    *(volatile unsigned int *) 0xf010008C    = 0x02; // IOC_I2S_OUT_DAT
    *(volatile unsigned int *) 0xf0100090    = 0x01; // IOC_UART0_RX
    *(volatile unsigned int *) 0xf0100094    = 0x02; // IOC_UART0_TX
    *(volatile unsigned int *) 0xf0100098    = 0x01; // IOC_UART1_RX
    *(volatile unsigned int *) 0xf010009C    = 0x02; // IOC_UART1_TX
    *(volatile unsigned int *) 0xf01000A0    = 0x01; // IOC_UART2_RX
    *(volatile unsigned int *) 0xf01000A4    = 0x02; // IOC_UART2_TX
    *(volatile unsigned int *) 0xf01000A8    = 0x01; // IOC_TCK
    *(volatile unsigned int *) 0xf01000AC    = 0x02; // IOC_TDO
    *(volatile unsigned int *) 0xf01000B0    = 0x01; // IOC_TDI
    *(volatile unsigned int *) 0xf01000B4    = 0x01; // IOC_TMS
    *(volatile unsigned int *) 0xf01000B8    = 0x01; // IOC_TRSTn
    *(volatile unsigned int *) 0xf01000BC    = 0x01; // IOC_SRSTn
    *(volatile unsigned int *) 0xf01000C0    = 0x01; // IOC_VI0_D
    *(volatile unsigned int *) 0xf01000C4    = 0x01; // IOC_VI0_HSYNC
    *(volatile unsigned int *) 0xf01000C8    = 0x01; // IOC_VI0_VSYNC
    *(volatile unsigned int *) 0xf01000CC    = 0x01; // IOC_VI0_PCLK
    *(volatile unsigned int *) 0xf01000D0    = 0x02; // IOC_VI1_D_0
    *(volatile unsigned int *) 0xf01000D4    = 0x02; // IOC_VI1_D_1
    *(volatile unsigned int *) 0xf01000D8    = 0x02; // IOC_VI1_D_2
    *(volatile unsigned int *) 0xf01000DC    = 0x02; // IOC_VI1_D_3
    *(volatile unsigned int *) 0xf01000E0    = 0x01; // IOC_VI1_D_4
    *(volatile unsigned int *) 0xf01000E4    = 0x01; // IOC_VI1_D_5
    *(volatile unsigned int *) 0xf01000E8    = 0x01; // IOC_VI1_D_6
    *(volatile unsigned int *) 0xf01000EC    = 0x01; // IOC_VI1_D_7
    *(volatile unsigned int *) 0xf01000F0    = 0x01; // IOC_VI1_D_8
    *(volatile unsigned int *) 0xf01000F4    = 0x01; // IOC_VI1_D_9
    *(volatile unsigned int *) 0xf01000F8    = 0x01; // IOC_VI1_D_10
    *(volatile unsigned int *) 0xf01000FC    = 0x01; // IOC_VI1_D_11
    *(volatile unsigned int *) 0xf0100100    = 0x01; // IOC_VI1_HSYNC
    *(volatile unsigned int *) 0xf0100104    = 0x01; // IOC_VI1_VSYNC
    *(volatile unsigned int *) 0xf0100108    = 0x01; // IOC_VI1_PCLK
    *(volatile unsigned int *) 0xf010010C    = 0x02; // IOC_VODATA
    *(volatile unsigned int *) 0xf0100110    = 0x02; // IOC_VOHSYNC
    *(volatile unsigned int *) 0xf0100114    = 0x02; // IOC_VOVSYNC
    *(volatile unsigned int *) 0xf0100118    = 0x02; // IOC_VOEN
    *(volatile unsigned int *) 0xf010011C    = 0x02; // IOC_VOCLK
    *(volatile unsigned int *) 0xf0100120    = 0x02; // IOC_RO0_DATA
    *(volatile unsigned int *) 0xf0100124    = 0x02; // IOC_RO0_CLK
    *(volatile unsigned int *) 0xf0100128    = 0x02; // IOC_RO1_DATA
    *(volatile unsigned int *) 0xf010012C    = 0x02; // IOC_RO1_CLK
#else
// HD
    *(volatile unsigned int *) 0xf0000050    = 0x00000101; // fpll_con
    *(volatile unsigned int *) 0xf0000054    = 0x041f3210; // mpll0_con
    *(volatile unsigned int *) 0xf0000058    = 0x041f3210; // mpll1_con

    // pll selection[7:0], [7:4] reserved, [3] = XIN or mux out select, [2:0] = clock select
    *(volatile unsigned int *) 0xf0000000    = 0x09090808; // apb, ddr_axi, cpu_axi, cpu
    *(volatile unsigned int *) 0xf0000004    = 0x0008080A; // uart, wdt, timer, qspi
    *(volatile unsigned int *) 0xf0000008    = 0x00050008; // gpio, svm, i2s, gadc
    *(volatile unsigned int *) 0xf000000C    = 0x00000000; // pvi_rx3, rx2, rx1, rx0
    *(volatile unsigned int *) 0xf0000010    = 0x00000000; // vadc3, vadc2, vadc1, vadc0
    *(volatile unsigned int *) 0xf0000014    = 0x02020202; // isp3, isp2, isp1, isp0
    *(volatile unsigned int *) 0xf0000018    = 0x00020202; // pvi_tx, du, vpu, vin
    *(volatile unsigned int *) 0xf000001C    = 0x00000000; // Rev, recfmt, vdac, pvi_tx_fifo
    *(volatile unsigned int *) 0xf0000020    = 0x00000000; // Rev, recfmt_2x_mph_1, 2x_mph_0, recfmt_2x
    *(volatile unsigned int *) 0xf0000024    = 0x00000200; // disfmt_2x_mph, Rev, disfmt, Rev
    *(volatile unsigned int *) 0xf0000028    = 0x00000000; // mipi_pixel, mipi_lane, rxbyteclkhs, Rev
    *(volatile unsigned int *) 0xf000002C    = 0x08080800; // ddr_postclk_2x, postclk, ctclk, mipi_cci
    *(volatile unsigned int *) 0xf0000030    = 0x00000000; // vdck1, vdck0, dmclk, tck
    *(volatile unsigned int *) 0xf0000034    = 0x00000000; // polarity
    *(volatile unsigned int *) 0xf0000038    = 0x00000000; // Rev
    *(volatile unsigned int *) 0xf000003C    = 0x00000000; // Rev
    *(volatile unsigned int *) 0xf0000040    = 0xffffffff; // sw rst
    *(volatile unsigned int *) 0xf0000044    = 0xffffffff; // sw rst
    *(volatile unsigned int *) 0xf0000048    = 0xffffffff; // sw rst
    *(volatile unsigned int *) 0xf000004C    = 0; // Rev
    *(volatile unsigned int *) 0xf000005C    = 0; // vdac_con
    *(volatile unsigned int *) 0xf0000060    = 0; // bgr_con

    // Pad control (each register control 8bit only)
    // Common Control Signals from MISC, [6]=PIN2, [5]=PIN1, [4]=PD, [3]=PU, [2]=SMT, [1]=OE, [0]=IE
    *(volatile unsigned int *) 0xf0100000    = 0x01; // IOC_TEST
    *(volatile unsigned int *) 0xf0100004    = 0x01; // IOC_RSTB
    *(volatile unsigned int *) 0xf0100008    = 0x01; // IOC_OPT
    *(volatile unsigned int *) 0xf010000C    = 0x01; // IOC_BOOT_MD
    *(volatile unsigned int *) 0xf0100010    = 0x00; // IOC_GPIO_0
    *(volatile unsigned int *) 0xf0100014    = 0x00; // IOC_GPIO_1
    *(volatile unsigned int *) 0xf0100018    = 0x00; // IOC_GPIO_2
    *(volatile unsigned int *) 0xf010001C    = 0x00; // IOC_GPIO_3
    *(volatile unsigned int *) 0xf0100020    = 0x00; // IOC_GPIO_4
    *(volatile unsigned int *) 0xf0100024    = 0x00; // IOC_GPIO_5
    *(volatile unsigned int *) 0xf0100028    = 0x00; // IOC_GPIO_6
    *(volatile unsigned int *) 0xf010002C    = 0x00; // IOC_GPIO_7
    *(volatile unsigned int *) 0xf0100030    = 0x02; // IOC_I2C0_DAT
    *(volatile unsigned int *) 0xf0100034    = 0x02; // IOC_I2C0_CLK
    *(volatile unsigned int *) 0xf0100038    = 0x02; // IOC_I2C1_DAT
    *(volatile unsigned int *) 0xf010003C    = 0x02; // IOC_I2C1_CLK
    *(volatile unsigned int *) 0xf0100040    = 0x02; // IOC_I2C2_DAT
    *(volatile unsigned int *) 0xf0100044    = 0x02; // IOC_I2C2_CLK
    *(volatile unsigned int *) 0xf0100048    = 0x02; // IOC_SPI0_SS0
    *(volatile unsigned int *) 0xf010004C    = 0x02; // IOC_SPI0_SS1
    *(volatile unsigned int *) 0xf0100050    = 0x02; // IOC_SPI0_SLK
    *(volatile unsigned int *) 0xf0100054    = 0x02; // IOC_SPI0_MOSI
    *(volatile unsigned int *) 0xf0100058    = 0x01; // IOC_SPI0_MISO
    *(volatile unsigned int *) 0xf010005C    = 0x02; // IOC_SPI1_SS
    *(volatile unsigned int *) 0xf0100060    = 0x02; // IOC_SPI1_SLK
    *(volatile unsigned int *) 0xf0100064    = 0x02; // IOC_SPI1_MOSI
    *(volatile unsigned int *) 0xf0100068    = 0x01; // IOC_SPI1_MISO
    *(volatile unsigned int *) 0xf010006C    = 0x03; // IOC_QSPI_SS
    *(volatile unsigned int *) 0xf0100070    = 0x02; // IOC_QSPI_SCK
    *(volatile unsigned int *) 0xf0100074    = 0x03; // IOC_QSPI_D0
    *(volatile unsigned int *) 0xf0100078    = 0x03; // IOC_QSPI_D1
    *(volatile unsigned int *) 0xf010007C    = 0x03; // IOC_QSPI_D2
    *(volatile unsigned int *) 0xf0100080    = 0x03; // IOC_QSPI_D3
    *(volatile unsigned int *) 0xf0100084    = 0x01; // IOC_I2S_OUT_CLK
    *(volatile unsigned int *) 0xf0100088    = 0x01; // IOC_I2S_OUT_SYN
    *(volatile unsigned int *) 0xf010008C    = 0x02; // IOC_I2S_OUT_DAT
    *(volatile unsigned int *) 0xf0100090    = 0x01; // IOC_UART0_RX
    *(volatile unsigned int *) 0xf0100094    = 0x02; // IOC_UART0_TX
    *(volatile unsigned int *) 0xf0100098    = 0x01; // IOC_UART1_RX
    *(volatile unsigned int *) 0xf010009C    = 0x02; // IOC_UART1_TX
    *(volatile unsigned int *) 0xf01000A0    = 0x01; // IOC_UART2_RX
    *(volatile unsigned int *) 0xf01000A4    = 0x02; // IOC_UART2_TX
    *(volatile unsigned int *) 0xf01000A8    = 0x01; // IOC_TCK
    *(volatile unsigned int *) 0xf01000AC    = 0x02; // IOC_TDO
    *(volatile unsigned int *) 0xf01000B0    = 0x01; // IOC_TDI
    *(volatile unsigned int *) 0xf01000B4    = 0x01; // IOC_TMS
    *(volatile unsigned int *) 0xf01000B8    = 0x01; // IOC_TRSTn
    *(volatile unsigned int *) 0xf01000BC    = 0x01; // IOC_SRSTn
    *(volatile unsigned int *) 0xf01000C0    = 0x01; // IOC_VI0_D
    *(volatile unsigned int *) 0xf01000C4    = 0x01; // IOC_VI0_HSYNC
    *(volatile unsigned int *) 0xf01000C8    = 0x01; // IOC_VI0_VSYNC
    *(volatile unsigned int *) 0xf01000CC    = 0x01; // IOC_VI0_PCLK
    *(volatile unsigned int *) 0xf01000D0    = 0x02; // IOC_VI1_D_0
    *(volatile unsigned int *) 0xf01000D4    = 0x02; // IOC_VI1_D_1
    *(volatile unsigned int *) 0xf01000D8    = 0x02; // IOC_VI1_D_2
    *(volatile unsigned int *) 0xf01000DC    = 0x02; // IOC_VI1_D_3
    *(volatile unsigned int *) 0xf01000E0    = 0x01; // IOC_VI1_D_4
    *(volatile unsigned int *) 0xf01000E4    = 0x01; // IOC_VI1_D_5
    *(volatile unsigned int *) 0xf01000E8    = 0x01; // IOC_VI1_D_6
    *(volatile unsigned int *) 0xf01000EC    = 0x01; // IOC_VI1_D_7
    *(volatile unsigned int *) 0xf01000F0    = 0x01; // IOC_VI1_D_8
    *(volatile unsigned int *) 0xf01000F4    = 0x01; // IOC_VI1_D_9
    *(volatile unsigned int *) 0xf01000F8    = 0x01; // IOC_VI1_D_10
    *(volatile unsigned int *) 0xf01000FC    = 0x01; // IOC_VI1_D_11
    *(volatile unsigned int *) 0xf0100100    = 0x01; // IOC_VI1_HSYNC
    *(volatile unsigned int *) 0xf0100104    = 0x01; // IOC_VI1_VSYNC
    *(volatile unsigned int *) 0xf0100108    = 0x01; // IOC_VI1_PCLK
    *(volatile unsigned int *) 0xf010010C    = 0x02; // IOC_VODATA
    *(volatile unsigned int *) 0xf0100110    = 0x02; // IOC_VOHSYNC
    *(volatile unsigned int *) 0xf0100114    = 0x02; // IOC_VOVSYNC
    *(volatile unsigned int *) 0xf0100118    = 0x02; // IOC_VOEN
    *(volatile unsigned int *) 0xf010011C    = 0x02; // IOC_VOCLK
    *(volatile unsigned int *) 0xf0100120    = 0x02; // IOC_RO0_DATA
    *(volatile unsigned int *) 0xf0100124    = 0x02; // IOC_RO0_CLK
    *(volatile unsigned int *) 0xf0100128    = 0x02; // IOC_RO1_DATA
    *(volatile unsigned int *) 0xf010012C    = 0x02; // IOC_RO1_CLK
#endif
}

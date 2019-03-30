#include <stdio.h>
#include "system.h"

void asic_init(void)
{
#if (DDR_RESOL_INOUT_OPER == HD720pToHD720p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (720p) input, HD (720p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting

#if defined(CPU_OVER_CLOCK)
		*(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x255f3210; // MPLL1 283.5MHz
        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
        // APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 283.5M(MPLL1)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080E; // APB, DDR_axi, CPU_axi, CPU
        // UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
        *(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
        // SVM : 112.05M(FPLL/10), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00050001; // Rev, SVM, Rev, Gadc

#else	// #if defined(CPU_OVER_CLOCK)

#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
        //*(volatile unsigned int *) 0xf0000050 = 0x000002B9; // FPLL 769.5MHz
        //*(volatile unsigned int *) 0xf0000050 = 0x000002B8; // FPLL 756MHz
        //*(volatile unsigned int *) 0xf0000050 = 0x000002B7; // FPLL 742.5MHz
        //*(volatile unsigned int *) 0xf0000050 = 0x000002B2; // FPLL 675MHz
        //*(volatile unsigned int *) 0xf0000050 = 0x000002AC; // FPLL 594MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        //*(volatile unsigned int *) 0xf0000058 = 0x296f3210; // MPLL1 198MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x24ff3210; // MPLL1 202.5MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x208f3210; // MPLL1 216MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x299f3210; // MPLL1 225MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x252f3210; // MPLL1 243MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x2e5f3210; // MPLL1 249.75MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x253f3210; // MPLL1 256.5MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x29df3210; // MPLL1 261MHz
        //*(volatile unsigned int *) 0xf0000058 = 0x2e7f3210; // MPLL1 263.25MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
        //*(volatile unsigned int *) 0xf0000000 = 0x0A08080e; // APB, DDR_axi, CPU_axi, CPU --> MPLL1
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 124.875M(FPLL/4), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00050001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU -> FPLL/4
        //*(volatile unsigned int *) 0xf0000000 = 0x0A080809; // APB, DDR_axi, CPU_axi, CPU -> FPLL/3
        //*(volatile unsigned int *) 0xf0000000 = 0x0A08080e; // APB, DDR_axi, CPU_axi, CPU -> MPLL1
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 99.562M(FPLL/4), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00040001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

#endif 	// #if defined(CPU_OVER_CLOCK)

	// PVI Rx : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 37.125M(mpll0/8)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x18081808; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 37.125M(mpll0/8), VPU : 37.125M(mpll0/8), VIN : 37.125M(mpll0/8) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00020404; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 74.25M(MPLL0/4), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x02000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 37.125M(mpll0/8), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x02000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 74.25M(mpll0/4), Quad_2x : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000202; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 37.125M(mpll0/8), lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x04000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == FHD1080pToFHD1080p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //FHD (1080p) input, FHD (1080p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00050001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00040001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x06040604; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 74.25M(mpll0/4), VPU : 74.25M(mpll0/4), VIN : 74.25M(mpll0/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00000202; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 148.5M(MPLL0/2), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x00000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 74.25M(mpll0/4), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x00000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 148.5M(mpll0/2), Quad_2x : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000000; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 74.25M(mpll0/4), lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x02000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == SD720hToSD720h)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SD 720H input, SD 720H output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 74.25M(mpll0/4), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00080001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 74.25M(mpll0/4), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00080001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif


	// PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : disable
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x02000200; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 27M(MPLL1/8), DU : 13.5M(mpll1/16), VPU : 13.5M(mpll1/16), VIN : 13.5M(mpll1/16) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x03050707; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 27M(MPLL1/8), RECfmt : 108M(mpll1/2), VDAC,PVI_TxFifo : 27M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x05010000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 13.5M(mpll1/16), RECfmt_2x : 216M(mpll1)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x05010101; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 27M(mpll1/8), Quad_2x : 27M(mpll1/8)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000505; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD960pToHD720p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (960p) input, HD (720p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 124.875M(FPLL/4), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00040001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 99.562M(FPLL/4), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00040001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 72.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x06040604; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 37.125M(mpll0/8), VPU : 74.25M(mpll0/4), VIN : 74.25M(mpll0/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00020202; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 74.25M(MPLL0/4), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x02000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 37.125M(mpll0/8), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x02000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 74.25M(mpll0/4), Quad_2x : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000202; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD960p47MToHD960p94M) 
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (960p) input, HD (960p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        //*(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x24ef3210; // MPLL0 189MHz
        *(volatile unsigned int *) 0xf0000058 = 0x2A1f3210; // MPLL1 297MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        //*(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	//*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        //*(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll1/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00070001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 47.25(mpll0/4)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x06040604; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : disable, DU : 47.25M(mpll0/4), VPU : 47.25M(mpll0/4), VIN : 47.25M(mpll0/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x80000202; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : disable, RECfmt : 94.5M(mpll0/2), VDAC,PVI_TxFifo : disable
        *(volatile unsigned int *) 0xf000001C = 0x80008080 | 0x00000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 47.25M(mpll0/4), RECfmt_2x : 189M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x00000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 94.5M(mpll0/2), Quad_2x : 94.5M(mpll0/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000000; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 47.25M, lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x02000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD960p74MToHD960p94M) 
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (960p) input, HD (960p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        //*(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x24ef3210; // MPLL0 189MHz
        *(volatile unsigned int *) 0xf0000058 = 0x2A1f3210; // MPLL1 297MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        //*(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	//*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        //*(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll1/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00070001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : 148.5M(mpll1/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x01010101; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
    // ISP : 74.25(mpll1/4)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x07050705; // ISP3, ISP2, ISP1, ISP0
    // PVI_TxRd : disable, DU : 47.25M(mpll0/4), VPU : 74.25M(mpll1/4), VIN : 74.25M(mpll1/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x80000303; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : disable, RECfmt : 94.5M(mpll0/2), VDAC,PVI_TxFifo : disable
        *(volatile unsigned int *) 0xf000001C = 0x80008080 | 0x00000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo
	// QUAD : 47.25M(mpll0/4), RECfmt_2x : 189M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x00000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 94.5M(mpll0/2), Quad_2x : 94.5M(mpll0/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000000; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 74.25M, lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x03000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/
        
#elif (DDR_RESOL_INOUT_OPER == HD960p54MToHD960p94M) 
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (960p) input, HD (960p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        //*(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x298f3210; // MPLL0 216MHz
        *(volatile unsigned int *) 0xf0000058 = 0x24ef3210; // MPLL1 189MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        //*(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	//*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        //*(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 128.25M(fpll1/6), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00020001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : unused
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : unused
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
    // ISP : 54M(mpll0/4)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x04040404; // ISP3, ISP2, ISP1, ISP0
    // PVI_TxRd : disable, DU : 47.25M(mpll1/4), VPU : 54M(mpll0/4), VIN : 54M(mpll0/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x80010202; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : disable, RECfmt : 94.5M(mpll1/2), VDAC,PVI_TxFifo : disable
        *(volatile unsigned int *) 0xf000001C = 0x80008080 | 0x00010000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo
	// QUAD : 47.25M(mpll1/4), RECfmt_2x : 189M(mpll1)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x01010101; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 94.5M(mpll1/2), Quad_2x : 94.5M(mpll1/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x01000101; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 54M, lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x02000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == FHD1080pToHD720p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //FHD (1080p) input, HD (720p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x06040604; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 37.125M(mpll0/8), VPU : 74.25M(mpll0/4), VIN : 74.25M(mpll0/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00020202; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 74.25M(MPLL0/4), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x02000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 37.125M(mpll0/8), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x02000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 74.25M(mpll0/4), Quad_2x : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000202; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 74.25M(mpll0/4), lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x02000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == SD720hToHD720p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SD (720p) input, HD (720p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : disable
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x02000200; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 37.125M(mpll0/8), VPU : 13.5M(mpll1/16), VIN : 13.5M(mpll1/16) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00020707; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 74.25M(MPLL0/4), RECfmt : 108M(mpll1/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x02010000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 37.125M(mpll0/8), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x02000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 74.25M(mpll0/4), Quad_2x : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000202; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == SD960hToHD720p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SD (960p) input, HD (720p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x2A0f3210; // MPLL1 288MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : disable
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x02000200; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 37.125M(mpll0/8), VPU : 18.0M(mpll1/16), VIN : 18.0M(mpll1/16) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00020707; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 74.25M(MPLL0/4), RECfmt : 144M(mpll1/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x02010000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 37.125M(mpll0/8), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x02000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 74.25M(mpll0/4), Quad_2x : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000202; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD720pToFHD1080p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (720p) input, FHD (1080p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 37.125M(mpll0/8)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x18081808; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 74.25M(mpll0/4), VPU : 37.125M(mpll0/8), VIN : 37.125M(mpll0/8) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00000404; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 148.5M(MPLL0/2), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x00000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 74.25M(mpll0/4), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x00000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 148.5M(mpll0/2), Quad_2x : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000000; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 37.125M(mpll0/8), lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x04000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD960pToFHD1080p) //|| (DDR_RESOL_INOUT_OPER == HD960pToHD960p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (960p) input, FHD (1080p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00050001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00040001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif



	// PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : disable
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x02000200; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 74.25M(mpll0/4), VPU : 74.25M(mpll0/4), VIN : 74.25M(mpll0/4) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00000202; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 148.5M(MPLL0/2), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x00000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 74.25M(mpll0/4), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x00000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 148.5M(mpll0/2), Quad_2x : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000000; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == SD720hToFHD1080p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SD (720p) input, FHD (1080p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif



	// PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : disable
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x02000200; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 74.25M(mpll0/4), VPU : 13.5M(mpll1/16), VIN : 13.5M(mpll1/16) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00000707; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 148.5M(MPLL1/8), RECfmt : 108M(mpll1/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x00010000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 74.25M(mpll0/4), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x00000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 148.5M(mpll0/2), Quad_2x : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000000; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == SD960hToFHD1080p)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SD (960p) input, FHD (1080p) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A0f3210; // MPLL0 288MHz
        *(volatile unsigned int *) 0xf0000058 = 0x2A1f3210; // MPLL1 297MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll1/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00070001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll1/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00070001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI : 148.5M(mpll1/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x01010101; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x01010101; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : disable
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x02000200; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL1/2), DU : 74.25M(mpll1/4), VPU : 18.0M(mpll0/16), VIN : 18.0M(mpll0/16) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x01010606; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 148.5M(MPLL1/2), RECfmt : 144M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x01000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 74.25M(mpll1/4), RECfmt_2x : 297M(mpll1)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x01000001; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 148.5M(mpll1/2), Quad_2x : 148.5M(mpll1/2)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000101; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : disable, lane : disable, Rxbyteclkhs : disable
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x00000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD720pToWSVGA) //WSVGA:1024x600p60
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //HD (720p) input, HD (1024) output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
	// APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
	// APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
	// UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
	*(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
	// SVM : 148.5M(mpll0/2), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00060001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif

	// PVI Rx : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
	// VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
	// ISP : 37.125M(mpll0/8)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x18081808; // ISP3, ISP2, ISP1, ISP0
	// PVI_TxRd : 148.5M(MPLL0/2), DU : 54M(mpll1/4), VPU : 37.125M(mpll0/8), VIN : 37.125M(mpll0/8) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x00010404; // PVI_TxRd, DU, VPU, Vin
	// PVI_TxWr : 148.5M(MPLL0/2), RECfmt : 148.5M(mpll0/2), VDAC,PVI_TxFifo : 148.5M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x00000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

	// QUAD : 37.125M(mpll0/8), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x02000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
	// DISfmt : 108M(mpll1/2), Quad_2x : 74.25M(mpll0/4)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000102; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

	// MIPI pixel : 37.125M(mpll0/8), lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x04000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#elif (DDR_RESOL_INOUT_OPER == HD720pToSD720h)
    {/*{{{*/
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SD 720H input, SD 720H output
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FPLL, MPLL0,1 Setting
#if defined(DDR_SPEED_500M)
        *(volatile unsigned int *) 0xf0000050 = 0x000002CA; // FPLL 999MHz
#elif defined(DDR_SPEED_400M) // 769M
        *(volatile unsigned int *) 0xf0000050 = 0x000002BB; // FPLL 796.5MHz
#else
#error "unkonw ddr speed"
#endif
        *(volatile unsigned int *) 0xf0000054 = 0x2A1f3210; // MPLL0 297MHz
        *(volatile unsigned int *) 0xf0000058 = 0x298f3210; // MPLL1 216MHz

        LOOPDELAY_10USEC(10);

        // IP-Block Clock Setting
#if defined(DDR_SPEED_500M)
           // APB : 62.4375M(FPLL/16), DDR_axi : 249.75M(FPLL/4), CPU_axi : 124.88M(FPLL/8), CPU : 249.75M(FPLL/4)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
           // UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 142.71M(FPLL/7)
           *(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090B; // UART, WDT, Timer, QSPI
           // SVM : 74.25M(mpll0/4), GADC : 49.95M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00080001; // Rev, SVM, Rev, Gadc
#elif defined(DDR_SPEED_400M)
           // APB : 48M(FPLL/16), DDR_axi : 192.375M(FPLL/4), CPU_axi : 48M(FPLL/16), CPU : 256.5M(FPLL/3)
        *(volatile unsigned int *) 0xf0000000 = 0x0A08080A; // APB, DDR_axi, CPU_axi, CPU
           // UART : 27M(XIN), WDT : 6.75M(TIMER), TIMER : 6.75M(XIN/4), QSPI : 128.25M(FPLL/6)
           *(volatile unsigned int *) 0xf0000004 = 0x00000000 | 0x0008090A; // UART, WDT, Timer, QSPI
           // SVM : 74.25M(mpll0/4), GADC : 38.45M(FPLL/20)
        *(volatile unsigned int *) 0xf0000008 = 0x00800080 | 0x00080001; // Rev, SVM, Rev, Gadc
#else
#error "unkonw ddr speed"
#endif


           // PVI : 148.5M(mpll0/2)
        *(volatile unsigned int *) 0xf000000C = 0x80808080 | 0x00000000; // PVI_Rx3, Rx2, Rx1, Rx0
           // VADC : 148.5M(PVI RX)
        *(volatile unsigned int *) 0xf0000010 = 0x80808080 | 0x00000000; // Vadc3, Vadc2, Vadc1, Vadc0
           // ISP : 37.125Mhz(mppl0/8)
        *(volatile unsigned int *) 0xf0000014 = 0x80808080 | 0x18081808; // ISP3, ISP2, ISP1, ISP0
           // PVI_TxRd : 27M(MPLL1/8), DU : 13.5M(mpll1/16), VPU : 37.125M(mpll0/8), VIN : 37.125M(mpll0/8) 
        *(volatile unsigned int *) 0xf0000018 = 0x80808080 | 0x03050404; // PVI_TxRd, DU, VPU, Vin
           // PVI_TxWr : 27M(MPLL1/8), RECfmt : 148M(mpll0/2), VDAC,PVI_TxFifo : 27M(PVI_TxRd)
        *(volatile unsigned int *) 0xf000001C = 0x80808080 | 0x05000000; // PVI_TxWr, Recfmt, Vdac, PVI_TxFifo

           // QUAD : 13.5M(mpll1/16), RECfmt_2x : 297M(mpll0)
        *(volatile unsigned int *) 0xf0000020 = 0x80808080 | 0x05000000; // Quad, Recfmt_2x_mph_1, 2x_mph_0, Recfmt_2x
           // DISfmt : 27M(mpll1/8), Quad_2x : 27M(mpll1/8)
        *(volatile unsigned int *) 0xf0000024 = 0x80808080 | 0x00000505; // Disfmt_2x_mph, Rev, Disfmt, Quad_2x

           // MIPI pixel : 37.125M(mpll0/8), lane : Rxbyteclkhs/2, Rxbyteclkhs : from phy
        *(volatile unsigned int *) 0xf0000028 = 0x80808000 | 0x04000000; // MIPI_pixel, MIPI_lane, Rxbyteclkhs, Rev
        *(volatile unsigned int *) 0xf000002C = 0x80808080 | 0x00000000; // DDR_postclk_2x, postclk, ctclk, mipi_cci
        *(volatile unsigned int *) 0xf0000030 = 0x80808000 | 0x00000000; // Parallel Video input clk1, clk0, DMCLK, TCK
        *(volatile unsigned int *) 0xf0000034 = 0x00000000 | 0x000000CC; // clock poriarity isp3210,pvirx3210
    }/*}}}*/

#endif

    return;
}

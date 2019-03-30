#include <stdio.h>
#include "system.h"

#define CLK_ENNUM_QSPI          (1<<0)
#define CLK_ENNUM_TIMERS        (1<<1)
#define CLK_ENNUM_WDT           (1<<2)
#define CLK_ENNUM_UART          (1<<3)
#define CLK_ENNUM_GADC          (1<<4)
#define CLK_ENNUM_SVM           (1<<5)
#define CLK_ENNUM_PVIRX         (1<<6)
#define CLK_ENNUM_ISP           (1<<7)
#define CLK_ENNUM_VIN           (1<<8)
#define CLK_ENNUM_VPU           (1<<9)
#define CLK_ENNUM_DU            (1<<10)
#define CLK_ENNUM_PVITX         (1<<11)
#define CLK_ENNUM_RO            (1<<12)
#define CLK_ENNUM_QUAD          (1<<13)
#define CLK_ENNUM_DOUT          (1<<14)
#define CLK_ENNUM_MIPI          (1<<15)
#define CLK_ENNUM_DDR           (1<<16)
#define CLK_ENNUM_JTAG          (1<<17)
#define CLK_ENNUM_I2S           (1<<18)
#define CLK_ENNUM_PARAVIN       (1<<19)

void clock_powerdown(const unsigned int u32PowerUpBit, const unsigned int u32PowerDownBit)
{

    /* power up bit */
    {/*{{{*/
        if(u32PowerUpBit & CLK_ENNUM_QSPI)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00000080;  //Qspi 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_TIMERS)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00008000;  //timers 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_WDT)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00800000;  //wdt 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_UART)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x80000000;  //uart 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_GADC)
        {
            *(volatile unsigned int *)0xf0000008 &= (unsigned int)~0x00000080;  //gadc 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_SVM)
        {
            *(volatile unsigned int *)0xf0000008 &= (unsigned int)~0x00800000;  //svm 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_PVIRX)
        {
            *(volatile unsigned int *)0xf000000C &= (unsigned int)~0x80808080;  //pvirx 0:en, 1:dis
            *(volatile unsigned int *)0xf0000010 &= (unsigned int)~0x80808080;  //vadc 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_ISP)
        {
            *(volatile unsigned int *)0xf0000014 &= (unsigned int)~0x80808080;  //isp 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_VIN)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00000080;  //vin 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_VPU)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00008000;  //vpu 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_DU)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00800000;  //du 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_PVITX)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x80000000;  //pvitx rd 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x80000000;  //pvitx wr 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00000080;  //pvitx fifo 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00008000;  //pvitx vdac 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_RO)
        {
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00800000;  //recfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00000080;  //recfmt_2x_clock 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00008000;  //recfmt_2x_mph0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00800000;  //recfmt_2x_mph1 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_QUAD)
        {
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x80000000;  //quad 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x00000080;  //quad_2x 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_DOUT)
        {
            *(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x00008000;  //disfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x80000000;  //disfmt_2x 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_MIPI)
        {
            *(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x00008000;  //rxbyteclkhs 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x00800000;  //mipi_lane 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x80000000;  //mipi_pixel 0:en, 1:dis
            *(volatile unsigned int *)0xf000002C &= (unsigned int)~0x00000080;  //mipi cci 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_DDR)
        {
            *(volatile unsigned int *)0xf000002C &= (unsigned int)~0x80808000;  //ddr 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_JTAG)
        {
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00000080;  //jtag 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_I2S)
        {
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00008000;  //i2S 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_PARAVIN)
        {
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00800000;  //par0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x80000000;  //par1 0:en, 1:dis
        }
    }/*}}}*/
    /* power down bit */
    {/*{{{*/
        if(u32PowerDownBit & CLK_ENNUM_QSPI)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00000080;  //Qspi 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_TIMERS)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00008000;  //timers 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_WDT)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00800000;  //wdt 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_UART)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x80000000;  //uart 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_GADC)
        {
            *(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00000080;  //gadc 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_SVM)
        {
            *(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00800000;  //svm 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_PVIRX)
        {
            *(volatile unsigned int *)0xf000000C |= (unsigned int)0x80808080;  //pvirx 0:en, 1:dis
            *(volatile unsigned int *)0xf0000010 |= (unsigned int)0x80808080;  //vadc 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_ISP)
        {
            *(volatile unsigned int *)0xf0000014 |= (unsigned int)0x80808080;  //isp 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_VIN)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00000080;  //vin 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_VPU)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00008000;  //vpu 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_DU)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00800000;  //du 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_PVITX)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x80000000;  //pvitx rd 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x80000000;  //pvitx wr 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x00000080;  //pvitx fifo 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x00008000;  //pvitx vdac 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_RO)
        {
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x00800000;  //recfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00000080;  //recfmt_2x_clock 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00008000;  //recfmt_2x_mph0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00800000;  //recfmt_2x_mph1 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_QUAD)
        {
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x80000000;  //quad 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 |= (unsigned int)0x00000080;  //quad_2x 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_DOUT)
        {
            *(volatile unsigned int *)0xf0000024 |= (unsigned int)0x00008000;  //disfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 |= (unsigned int)0x80000000;  //disfmt_2x 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_MIPI)
        {
            *(volatile unsigned int *)0xf0000028 |= (unsigned int)0x00008000;  //rxbyteclkhs 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 |= (unsigned int)0x00800000;  //mipi_lane 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 |= (unsigned int)0x80000000;  //mipi_pixel 0:en, 1:dis
            *(volatile unsigned int *)0xf000002C |= (unsigned int)0x00000080;  //mipi cci 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_DDR)
        {
            *(volatile unsigned int *)0xf000002C |= (unsigned int)0x80808000;  //ddr 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_JTAG)
        {
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00000080;  //jtag 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_I2S)
        {
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00008000;  //i2S 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_PARAVIN)
        {
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00800000;  //par0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x80000000;  //par1 0:en, 1:dis
        }
    }/*}}}*/

    return;
}

void SetClockPD(void)
{
    unsigned int u32PowerAllBit = 0;
    unsigned int u32PowerUpBit = 0;
    unsigned int u32PowerDownBit = 0;

    u32PowerAllBit = 
        CLK_ENNUM_QSPI          |
        CLK_ENNUM_TIMERS        |
        CLK_ENNUM_WDT           |
        CLK_ENNUM_UART          |
        CLK_ENNUM_GADC          |
        CLK_ENNUM_SVM           |
        CLK_ENNUM_PVIRX         |
        CLK_ENNUM_ISP           |
        CLK_ENNUM_VIN           |
        CLK_ENNUM_VPU           |
        CLK_ENNUM_DU            |
        CLK_ENNUM_PVITX         |
        CLK_ENNUM_RO            |
        CLK_ENNUM_QUAD          |
        CLK_ENNUM_DOUT          |
        CLK_ENNUM_MIPI          |
        CLK_ENNUM_DDR           |
        CLK_ENNUM_JTAG          |
        CLK_ENNUM_I2S           |
        CLK_ENNUM_PARAVIN       ;

    u32PowerUpBit = 
        CLK_ENNUM_QSPI          |
        CLK_ENNUM_TIMERS        |
        CLK_ENNUM_WDT           |
        CLK_ENNUM_UART          |
        //CLK_ENNUM_GADC          |
        //CLK_ENNUM_SVM           |
        //CLK_ENNUM_PVIRX         |
        //CLK_ENNUM_ISP           |
        //CLK_ENNUM_VIN           |
        //CLK_ENNUM_VPU           |
        //CLK_ENNUM_DU            |
        //CLK_ENNUM_PVITX         |
        //CLK_ENNUM_RO            |
        //CLK_ENNUM_QUAD          |
        //CLK_ENNUM_DOUT          |
        //CLK_ENNUM_MIPI          |
        CLK_ENNUM_DDR           |
        CLK_ENNUM_JTAG          |
        //CLK_ENNUM_I2S           |
        //CLK_ENNUM_PARAVIN       |
        0;

    u32PowerDownBit = u32PowerAllBit & (unsigned int)~u32PowerUpBit;

    clock_powerdown(u32PowerUpBit, u32PowerDownBit);
}

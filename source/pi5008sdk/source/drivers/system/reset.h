#ifndef _PI5008_RESET_H_
#define _PI5008_RESET_H_

///////////////// PI5008 Clock PowerDown control /////////////////////////////
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

void clock_powerdown(const unsigned int u32PowerUpBit, const unsigned int u32PowerDownBit);
void SetClockPD(void);

#endif /* _PI5008_RESET_H_ */

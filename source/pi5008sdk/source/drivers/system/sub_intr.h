#ifndef _SUB_INTR_H_
#define _SUB_INTR_H_

#include "system.h"
//#include "type.h"

typedef struct {
	unsigned int control			;	// 0x00
	unsigned int pend				;	// 0x04
	unsigned int mode				;	// 0x08
	unsigned int mask				;	// 0x0C
	unsigned int level				;	// 0x10
	unsigned int pri_islv[4]		;	// 0x14 ~ 0x20
	unsigned int pri_policy			;	// 0x24
	unsigned int cid_islv[4]		;	// 0x28 ~ 0x34
	unsigned int cid_imst			;	// 0x38
	unsigned int i_isprB			;	// 0x3C
	unsigned int i_ispc				;	// 0x40
	unsigned int pri_fslv[4]		;	// 0x44 ~ 0x50
	unsigned int fslv_policy		;	// 0x54
	unsigned int cid_fslv[4]		;	// 0x58 ~ 0x64
	unsigned int cid_fmst			;	// 0x68
	unsigned int f_isprB			;	// 0x6C
	unsigned int f_ispc				;	// 0x70
	unsigned int polarity			;	// 0x74
	unsigned int irq_vec_base		;	// 0x78
	unsigned int fiq_vec_base		;	// 0x7C
	unsigned int vec_base_addr[32]	;	// 0x80	// ~0xFC
} INTR_REG_T;

#define SUB_INTR0_CTRL_REG			((INTR_REG_T*)			(SUB_INTR0_BASE_ADDR))
#define SUB_INTR1_CTRL_REG			((INTR_REG_T*)			(SUB_INTR1_BASE_ADDR))
#define SUB_INTR2_CTRL_REG			((INTR_REG_T*)			(SUB_INTR2_BASE_ADDR))
#define SUB_INTR3_CTRL_REG			((INTR_REG_T*)			(SUB_INTR3_BASE_ADDR))

/* Define Interrupt Source */
/* INTR0 */
#define IRQ_PVI_RX_3		( 3)
#define IRQ_PVI_RX_2		( 2)
#define IRQ_PVI_RX_1		( 1)
#define IRQ_PVI_RX_0		( 0)

/* INTR1 */
#define IRQ_GADC1_DIAG			  ( 23)
#define IRQ_GADC0_DIAG			  ( 22)
#define IRQ_FREEZE_DU_CH3		  ( 21)
#define IRQ_FREEZE_DU_CH2		  ( 20)
#define IRQ_FREEZE_DU_CH1		  ( 19)
#define IRQ_FREEZE_DU_CH0		  ( 18)
#define IRQ_LOSS_SYNC_CH4		  ( 17)
#define IRQ_LOSS_SYNC_CH3		  ( 16)
#define IRQ_LOSS_SYNC_CH2		  ( 15)
#define IRQ_LOSS_SYNC_CH1		  ( 14)
#define IRQ_LOSS_SYNC_CH0		  ( 13)
#define IRQ_LOSS_FMT_CH4		  ( 12)
#define IRQ_LOSS_FMT_CH3		  ( 11)
#define IRQ_LOSS_FMT_CH2		  ( 10)
#define IRQ_LOSS_FMT_CH1		  ( 9)
#define IRQ_LOSS_FMT_CH0		  ( 8)
#define IRQ_GEN_SYNC			  ( 7)
#define IRQ_GEN_VREFLOSS		  ( 6)
#define IRQ_CRC_CH4			  ( 5)
#define IRQ_CRC_CH3			  ( 4)
#define IRQ_CRC_CH2			  ( 3)
#define IRQ_CRC_CH1			  ( 2)
#define IRQ_CRC_CH0			  ( 1)
#define IRQ_CRC_QUAD			  ( 0)

/* INTR2 */
#define IRQ_ISP_3			  ( 3)
#define IRQ_ISP_2			  ( 2)
#define IRQ_ISP_1			  ( 1)
#define IRQ_ISP_0			  ( 0)

/* INTR3 */
#define IRQ_VSYNC_DU			  ( 7)
#define IRQ_VSYNC_SVM			  ( 6)
#define IRQ_VSYNC_VIN4			  ( 5)
#define IRQ_VSYNC_VIN3			  ( 4)
#define IRQ_VSYNC_VIN2			  ( 3)
#define IRQ_VSYNC_VIN1			  ( 2)
#define IRQ_VSYNC_VIN0			  ( 1)
#define IRQ_VSYNC_QUAD			  ( 0)

#define INTR_MODE_FIQ		1
#define INTR_MODE_IRQ		0

#define ALL_IRQ_SUBINTR0   ((INTR_MODE_IRQ << IRQ_PVI_RX_0		) | \
                   (INTR_MODE_IRQ << IRQ_PVI_RX_1		) | \
                   (INTR_MODE_IRQ << IRQ_PVI_RX_2		) | \
                   (INTR_MODE_IRQ << IRQ_PVI_RX_3		) )

#define ALL_IRQ_SUBINTR1   ((INTR_MODE_IRQ << IRQ_GADC1_DIAG		) | \
                   (INTR_MODE_IRQ << IRQ_GADC0_DIAG		) | \
                   (INTR_MODE_IRQ << IRQ_FREEZE_DU_CH3		) | \
                   (INTR_MODE_IRQ << IRQ_FREEZE_DU_CH2		) | \
                   (INTR_MODE_IRQ << IRQ_FREEZE_DU_CH1		) | \
                   (INTR_MODE_IRQ << IRQ_FREEZE_DU_CH0		) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_SYNC_CH3		) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_SYNC_CH2		) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_SYNC_CH1		) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_SYNC_CH0		) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_FMT_CH3			) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_FMT_CH2			) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_FMT_CH1			) | \
                   (INTR_MODE_IRQ << IRQ_LOSS_FMT_CH0			) | \
                   (INTR_MODE_IRQ << IRQ_GEN_SYNC			) | \
                   (INTR_MODE_IRQ << IRQ_GEN_VREFLOSS			) | \
                   (INTR_MODE_IRQ << IRQ_CRC_CH0			) | \
                   (INTR_MODE_IRQ << IRQ_CRC_CH1			) | \
                   (INTR_MODE_IRQ << IRQ_CRC_CH2			) | \
                   (INTR_MODE_IRQ << IRQ_CRC_CH3			) | \
                   (INTR_MODE_IRQ << IRQ_CRC_QUAD			) )

#define ALL_IRQ_SUBINTR2   ( (INTR_MODE_IRQ << IRQ_ISP_0			) | \
                   (INTR_MODE_IRQ << IRQ_ISP_1			) | \
                   (INTR_MODE_IRQ << IRQ_ISP_2			) | \
                   (INTR_MODE_IRQ << IRQ_ISP_3			) )

#define ALL_IRQ_SUBINTR3   ((INTR_MODE_IRQ << IRQ_VSYNC_QUAD		) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_VIN0		) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_VIN1		) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_VIN2		) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_VIN3			) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_VIN4			) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_SVM			) | \
                   (INTR_MODE_IRQ << IRQ_VSYNC_DU			) )

#define INTR_CONTROL_VAM_BIT		4		// Vector Address Mode, 1:Address Register moe, 0:Table base
#define INTR_CONTROL_GMASK_BIT		3		// Global Mask, 1:mask, 0:unmask
#define INTR_CONTROL_VECTORED_BIT	2		// Vectored, 1:Vectored, 0:non-Vectored
#define INTR_CONTROL_nENBIRQ_BIT	1		// IRQ enable bit, active low
#define INTR_CONTROL_nENBFIQ_BIT	0		// FIQ enable bit, active low

#define INTR_MASK	(1)
#define INTR_UNMASK	(0)

#define INTR_LEVEL	(1)
#define INTR_EDGE	(0)

#define INTR_FALLING_EDGE	1
#define INTR_RISING_EDGE	0

extern INTR_REG_T	*gpSUBINTC[];

/* Define Function */
void sub_intc_control (const int ctlNum, int en);
void sub_intc_enable(const int ctlNum);
void sub_intc_disable(const int ctlNum);
unsigned int get_sub_intr_src (const int ctlNum);
void clr_sub_intr_src (const int ctlNum, unsigned int src);
void initSUBIntr(const int ctlNum);

void SUB_INTR_Initialize(void);

PP_VOID PPDRV_VSYNC_GetCurrentCnt(PP_U32 *pu32VsyncCnt);

#endif		// #ifndef _SUB_INTR_H_

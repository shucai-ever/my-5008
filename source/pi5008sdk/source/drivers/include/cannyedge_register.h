/**
 * \file
 *
 * \brief	EDGE Register Map
 *
 * Copyright (c) 2017 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __CANNYEDGE_REGISTER_H__
#define __CANNYEDGE_REGISTER_H__

#include "type.h"
#include "system.h"

/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppCANNYEDGE_REG_S
{
	vuint32 vu32Enable;					//0x000	// [2] capture_done
												// [1] capture_en
												// [0] canny_en
	vuint32 vu32Ctrl;					//0x004	// [11] field_sel
												// [10] field_end
												// [9] wrap_init
												// [8] wrap_en
												// [6:4] ch_sel
												// [2] ver_dnscale
												// [1] hor_dnscale
												// [0] fixed_th_en
	vuint32 vu32Size;					//0x008	// [26:16] width_m1
												// [10:0] height_m1
	vuint32 vu32Pcnt;					//0x00c	// [20:0] total_pcnt
	vuint32 vu32Thbnd;					//0x010	// [15:8] th_low_bound
												// [7:0] th_hight_bound
	vuint32 vu32Roi;					//0x014	// [26:16] roi_end_line
												// [10:0] roi_start_line
	vuint32 vu32Wbase;					//0x018	// [31:0] w_base_address
	vuint32 vu32Wline;					//0x01c	// [12:0] wrap_wline
	vuint32 vu32Cline;					//0x020	// [28:16] write_current_line
	vuint32 vu32Caddr;					//0x024	// [31:0] current_write_address
	vuint32 vu32Paddr;					//0x028	// [31:0] previous_write_address
	vuint32 vu32Thval;					//0x02c	// [31:24] th_low_fixed_val
												// [23:16] th_high_fixed_val
												// [15:8] th_low_init_val
												// [7:0] th_hight_init_val
	vuint32 _reserved_30[4];			//0x030 ~ 0x03c
	vuint32 vu32Dma;					//0x040	// [2] dma_done
												// [1] dma_start
												// [0] dma_en
	vuint32 vu32Dmabase;				//0x044	// [31:0] r_base_address
	vuint32 vu32Dmabytes;				//0x048	// [31:0] total_bytes
}PP_CANNYEDGE_REG_S;


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#define CANNYEDGE_REG ((PP_CANNYEDGE_REG_S*)(CAN_BASE_ADDR + 0x0000))

#endif //__CANNYEDGE_REGISTER_H__

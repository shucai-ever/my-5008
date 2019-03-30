/* du_register.h */
#ifndef __DU_REGISTER_H__
#define __DU_REGISTER_H__

#include "type.h"
#include "system.h"

#ifdef __cplusplus
EXTERN "C" {
#endif


// version : DU4_RegisterMap_20170915.xlsx
/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
//----------------------------------------------------------------------------
//	du register
//----------------------------------------------------------------------------
typedef volatile struct	ppDU_CTRL_REG_S {
	PP_U32	ctrl;			//0x0000	// [31:28] reserved
										// [27:24] yuv_to_bto_sel 
										// [23:20] yuv_to_osd_sel
										// [19:16] reserved
										// [15:12] reserved
										// [11:8] yuv_from_isp_sel
										// [7:4] bt_to_mvi_sel
										// [3:0] bt_to_port_sel 
	PP_U32	ctrl1;			// 0x0004	// [31:9] reserved
										// [8] pvi_tx_sel
										// [7:6] quad2pvi_async_enable 
										// [5:4] osd2pvi_async_enable
										// [3:1] reserved
										// [0] bto_chroma_swap
	PP_U32	reserved_0x8;	// 0x0008	// [31:0] reserved
	PP_U32	reserved_0xc;	// 0x000c	// [31:0] reserved
	PP_U32	state0;			// 0x0010	// DEBUG only
										// [31:14] reserved 
										// [13] bt_from_bto_hsync 
										// [12] bt_from_bto_vsync 
										// [11] pvi_irq_flag
										// [10] reserved
										// [9] osd_tg_irq
										// [8] osd_bus_irq
										// [7] svm_hsync monitor
										// [6] svm_vsync monitor
										// [5] isp_hsync monitor
										// [4] isp_vsync monitor
										// [3] osd_hsync monitor
										// [2] osd_vsync monitor
										// [1] digital_bto_hsync monitor
										// [0] digital_bto_vsync monitor 
	PP_U32	state1;			// 0x0014	// [31:0] reserved
	PP_U32	state2;			// 0x0018	// [31:0] reserved
	PP_U32	version;		// 0x001c	// [31:0] du4_version
	PP_U32	reserved [512-8];
} PP_DU_CTRL_REG_S;

//----------------------------------------------------------------------------
//  bto register
//----------------------------------------------------------------------------
typedef volatile struct ppDU_BTO_REG_S {
	PP_U32	ctrl0;			// 0x0800	// [31:28] reserved
										// [27] bt1120_vmask_md_ana
										// [26] bt1120_vmask_md_dig
										// [25:24] dis_hd_csc_md
										// [23:20] bto_hav_del
										// [19] sd_prg_md
										// [18] outfmt_cbcr_inv
										// [17] sd_p2i_en
										// [16] clpf
										// [15] dis_async_chg_en
										// [14] dis_async_en
										// [13] outfmt_hav_vblk
										// [12] bto_de_md
										// [11:10] bayer_out_md
										// [9:8] bto_dout_md
										// [7] sd_pal_ntb
										// [6] sd_960h_md
										// [5] sd_outfmt_md
										// [4] sd_fld_pol
										// [3] outfmt_16bit
										// [2] outfmt_bt656
										// [1] outfmt_yc_inv
										// [0] bt1120_limit_digital
	PP_U32	ctrl1;			// 0x0804	// [31:24] reserved
										// [23] bayer_bit_width
										// [22] pat_en
										// [21:16] dis_clko_phase
										// [15:12] pat_md
										// [11:8] pat_sel
										// [7:4] bto_vsync_delay, for debug purpose only
										// [3:1] byte_order 
										// [0] bt1120_lim_ana
	PP_U32	ctrl2;			// 0x0808	// [31:27] reserved
										// [26]	dis_dig_sat_en
										// [25]	dis_dig_brt_en
										// [24]	dis_dig_cont_en
										// [23:16] dis_dig_sat
										// [15:8] dis_dig_brt
										// [7:0] dis_dig_cont
	PP_U32	reserved_0xc;	// 0x080c	// [31:0] reserved
	PP_U32	state;			// 0x0810	// DEBUG only 
										// [31:6] reserved
										// [5] yuvi_hsync, HSYNC of YUV input to BTO block
										// [4] yuvi_vsync, VSYNC of YUV input to BTO block
										// [3] bto_fsync, captured at the negative edge of bto_vsync
										// [2] bto_fsync, FSYNC/DE from BTO block
										// [1] bto_hsync, HSYNC from BTO block
										// [0] bto_vsync, VSYNC from BTO block 
	PP_U32	reserved [512-5];
} PP_DU_BTO_REG_S;

//----------------------------------------------------------------------------
//  bto crc register
//----------------------------------------------------------------------------
typedef volatile struct ppBTO_CRC_REG_S {
	PP_U32	ctrl0;				//0x00	// [31:10] reserved
										// [9:8] FR_OS
										// [7] FLD_SEL
										// [6] FLD_MD
										// [5] ROI_MD
										// [4] ROI_EN
										// [3:1] FR_PERIOD
										// [0] BIST_EN
	PP_U32	ctrl1;				//0x04	// [31:24] reserved
										// [23:0] REF_DATA
 	PP_U32	ctrl2;				//0x08	// [31:27] reserved
 										// [26:16] VSTRT 
 										// [15:11] reserved
 										// [10:0] HSTRT 
	PP_U32	ctrl3;				//0x0c	// [31:27] reserved
										// [26:16] VEND
										// [15:11] reserved
										// [10:0] HEND
	PP_U32	reserved_0x10;		//0x10	// [31:0] reserved
	PP_U32	reserved_0x14;		//0x14	// [31:0] reserved
	PP_U32	reserved_0x18;		//0x18	// [31:0] reserved
	PP_U32	reserved_0x1c;		//0x1c	// [31:0] reserved
	PP_U32	acc0;				//0x20	// [31:24] reserved
										// [23:0] CRC_SUM_DATA_ACC0
	PP_U32	acc1;				//0x24	// [31:24] reserved
										// [23:0] CRC_SUM_DATA_ACC1
	PP_U32	acc2;				//0x28	// [31:24] reserved
										// [23:0] CRC_SUM_DATA_ACC2
	PP_U32	acc3;				//0x2c	// [31:24] reserved
										// [23:0] CRC_SUM_DATA_ACC3
	PP_U32	state;				//0x30	// [31:7] reserved
										// [6:4] CRC_BIST_DONE_ACC
										// [3:1] reserved
										// [0] CRC_BIST_DONE
	PP_U32	reserved[128-13];
} PP_BTO_CRC_REG_S;

typedef volatile struct ppDU_BTO_CRC_REG_S {
	PP_BTO_CRC_REG_S	area0;		//0x2800
	PP_BTO_CRC_REG_S	area1;		//0x2a00
	PP_BTO_CRC_REG_S	area2;		//0x2c00
	PP_BTO_CRC_REG_S	area3;		//0x2e00
} PP_DU_BTO_CRC_REG_S;

//----------------------------------------------------------------------------
//  bus test module
//----------------------------------------------------------------------------
typedef volatile struct ppDU_BUS_TEST_S {
	PP_U32 ctrl0;				// 0x3000	// [31:1] reserved
											// [0] BIST_EN
	PP_U32 reserved_0x04;		// 0x3004	// [31:0] reserved
	PP_U32 reserved_0x08;		// 0x3008	// [31:0] reserved
	PP_U32 reserved_0x0c;		// 0x300c	// [31:0] reserved
	PP_U32 crc0;				// 0x3010	// [31:0] CRC128_DATA[31:0]
	PP_U32 crc1;				// 0x3014	// [31:0] CRC128_DATA[63:32]
	PP_U32 crc2;				// 0x3018	// [31:0] CRC128_DATA[95:64]
	PP_U32 crc3;				// 0x301c	// [31:0] CRC128_DATA[127:96]
	PP_U32 reserved[512-8];
 } PP_DU_BUS_TEST_S;

//----------------------------------------------------------------------------
//  osd register
//----------------------------------------------------------------------------
typedef volatile struct ppOSD_CTRL_REG_S {
	PP_U32	reg_blocked;	//0x4000	// [31:1] reserved
										// [0] reg_blocked
	PP_U32	src_sel;		//0x4004	// [31:24] reserved
										// [23:20] mix_rle4_in
										// [19:16] mix_rle3_in
										// [15:12] mix_rle2_in
										// [11:8] mix_rle1_in
										// [7:4] mix_rle0_in
										// [3:0] osd_output
	PP_U32	reserved_0x08;	//0x4008	// [31:0] reserved
	PP_U32	bus_bw;			//0x400c	// [31:24] rle3_bw
										// [23:16] rle2_bw
										// [15:8] rle1_bw
										// [7:0] rle0_bw 
	PP_U32	state0;			//0x4010	// [31:4] reserved
										// [3] input hsync to OSDTG
										// [2] input vsync to OSDTG
										// [1] output hsync from OSDTG
										// [0] output vsync from OSDTG 
	PP_U32	state1;			//0x4014	// [31:9] reserved
										// [8:4] exception from RLE layer 4~0
										// [3] reserved
										// [2] input to OSDTG blocked
										// [1] reserved
										// [0] OSDTG queue full
	PP_U32	frame_cnt;		//0x4018	// [31:0] frame_counter
	PP_U32	reserved_0x1c;	//0x401c	// [31:0] reserved
	PP_U32	bus_bw1;		//0x4020	// [31:8] reserved
										// [7:0] rle4_bw
 	PP_U32	reserved[64-9];
} PP_OSD_CTRL_REG_S;

typedef volatile struct ppOSD_TG_REG_S {
	PP_U32	ctrl;			// 0x4500	// [31:4] reserved
										// [3] use_posedge
										// [2] ivsync_inv
										// [1] ovsync_inv
										// [0] tg_enable
	PP_U32	reserved[64-1];
} PP_OSD_TG_REG_S;

typedef volatile struct ppOSD_MONITOR0_REG_S {
	// ch0 : RLE0 read request
	// ch1 : RLE1 read request
	// ch2 : RLE2 read request
	// ch3 : RLE3 read request
	PP_U32	ctrl;			// 0x5000	// [31:8] reserved
										// [7] clear_ch3
										// [6] clear_ch2
										// [5] clear_ch1
										// [4] clear_ch0
										// [3] enable_ch3
										// [2] enable_ch2
										// [1] enable_ch1
										// [0] enable_ch0
	PP_U32	reserved_0x04;	// 0x5004	// [31:0] reserved
	PP_U32	reserved_0x08;	// 0x5008	// [31:0] reserved
	PP_U32	reserved_0x0c;	// 0x500c	// [31:0] reserved
	PP_U32	reserved_0x10;	// 0x5010	// [31:0] reserved
	PP_U32	reserved_0x14;	// 0x5014	// [31:0] reserved
	PP_U32	reserved_0x18;	// 0x5018	// [31:0] reserved
	PP_U32	reserved_0x1c;	// 0x501c	// [31:0] reserved
	PP_U32	ch0_total;		// 0x5020	// [31:0] ch0_total
	PP_U32	ch0_word;		// 0x5024	// [31:0] ch0_word
	PP_U32	ch1_total;		// 0x5028	// [31:0] ch1_total
	PP_U32	ch1_word;		// 0x502c	// [31:0] ch1_word
	PP_U32	ch2_total;		// 0x5030	// [31:0] ch2_total
	PP_U32	ch2_word;		// 0x5034	// [31:0] ch2_word
	PP_U32	ch3_total;		// 0x5038	// [31:0] ch3_total
	PP_U32	ch3_word;		// 0x503c	// [31:0] ch3_word
	PP_U32	reserved[64-16];
} PP_OSD_MONITOR0_REG_S;

typedef volatile struct ppOSD_MONITOR1_REG_S {
	// ch4 : RLE4 read request
	// ch5 : reserved
	// ch6 : reserved
	// ch7 : RLE0~RLE4 added read request
	PP_U32	ctrl;			// 0x5400	// [31:8] reserved
										// [7] clear_ch3
										// [6] clear_ch2
										// [5] clear_ch1
										// [4] clear_ch0
										// [3] enable_ch3
										// [2] enable_ch2
										// [1] enable_ch1
										// [0] enable_ch0
	PP_U32	reserved_0x04;	// 0x5404	// [31:0] reserved
	PP_U32	reserved_0x08;	// 0x5408	// [31:0] reserved
	PP_U32	reserved_0x0c;	// 0x540c	// [31:0] reserved
	PP_U32	reserved_0x10;	// 0x5410	// [31:0] reserved
	PP_U32	reserved_0x14;	// 0x5414	// [31:0] reserved
	PP_U32	reserved_0x18;	// 0x5418	// [31:0] reserved
	PP_U32	reserved_0x1c;	// 0x541c	// [31:0] reserved
	PP_U32	ch4_total;		// 0x5420	// [31:0] ch4_total
	PP_U32	ch4_word;		// 0x5424	// [31:0] ch4_word
	PP_U32	ch5_total;		// 0x5428	// [31:0] ch5_total
	PP_U32	ch5_word;		// 0x542c	// [31:0] ch5_word
	PP_U32	ch6_total;		// 0x5430	// [31:0] ch6_total
	PP_U32	ch6_word;		// 0x5434	// [31:0] ch6_word
	PP_U32	ch7_total;		// 0x5438	// [31:0] ch7_total
	PP_U32	ch7_word;		// 0x543c	// [31:0] ch7_word
	PP_U32	reserved[64-16];
} PP_OSD_MONITOR1_REG_S;

typedef volatile struct ppOSD_AREA_REG_S {
	PP_U32	base_addr;		//0x00		// [31:0] base_addr
	union {								// [31:24] reserved
		PP_U32 byte;		//0x04		// [23:0] byte_count : total number of bytes in this area for RLE mode
		PP_U32 stride;		//0x04		// [23:0] stride : DMA stride for RAW/RGB mode
	};
	PP_U32	image_size;		//0x08		// [31:28] reserved
										// [27:16] area_height
										// [15:12] reserved
										// [11:0] area_width
	PP_U32	config;			//0x0c		// [31:1] reserved
										// [0] enable
	PP_U32	base_addr2;		//0x10		// [31:0] base_addr2
	PP_U32	reserved [32-5];
} PP_OSD_AREA_REG_S;

typedef volatile struct ppOSD_CANVAS_REG_S {
	PP_U32	area0_start;	//0x00		// [31:28] reserved
										// [27:16] start_y
										// [15:12] reserved
 										// [11:0] start_x
	PP_U32	area1_start;	//0x04		// [31:28] reserved
										// [27:16] start_y
										// [15:12] reserved
 										// [11:0] start_x
	PP_U32	area2_start;	//0x08		// [31:28] reserved
										// [27:16] start_y
										// [15:12] reserved
 										// [11:0] start_x
	PP_U32	area3_start;	//0x0c		// [31:28] reserved
										// [27:16] start_y
										// [15:12] reserved
 										// [11:0] start_x
	PP_U32	canvas_size;	//0x10		// [31:28] reserved
										// [27:16] canvas_height
										// [15:12] reserved
 										// [11:0] canvas_width
	PP_U32	canvas_color;	//0x14		// [31:0] canvas_color
	PP_U32	reserved_0x18;	//0x18		// [31:0] reserved
	PP_U32	layer_state;	//0x1c		// [31:24] reserved
										// [23] area3 buffer empty
										// [22] area2 buffer empty, 
										// [21] area1 buffer empty
										// [20] area0 buffer empty
										// [19:0] reserved
	PP_U32	reserved_0x20;	//0x20		// [31:0] reserved
	PP_U32	reserved_0x24;	//0x24		// [31:0] reserved
 	PP_U32	global_alpha;	//0x28		// [31:9] reserved
 										// [8] use_global_alpha
 										// [7:0] alpha value
 	PP_U32	layer_mode;		//0x2c		// [31:8] reserved
 										// [7] field_en
 										// [6:4] reserved
 										// [3:1] raw_type
 										// [0] raw_image
	PP_U32	reserved [32-12];
} PP_OSD_CANVAS_REG_S;

//	Y = (R * ycoef0 + G * ycoef1 + B * ycoef2 + ycoef3) / 512
//	U = (R * ucoef0 + G * ucoef1 + B * ucoef2 + ucoef3) / 512
//	V = (R * vcoef0 + G * vcoef1 + B * vcoef2 + vcoef3) / 512
typedef volatile struct ppOSD_RGB2YUV_REG_S {
	PP_U32	ycoef0;			//0x80		// [31:13] reserved
										// [12] bypass
										// [11:0] ycoef0
	PP_U32	ycoef1;			//0x84		// [31:12] reserved
										// [11:0] ycoef1
	PP_U32	ycoef2;			//0x88		// [31:12] reserved
										// [11:0] ycoef2
	PP_U32	ycoef3;			//0x8c		// [31:20] reserved
										// [19:0] ycoef3
	PP_U32	ucoef0;			//0x90		// [31:12] reserved
										// [11:0] ucoef0
	PP_U32	ucoef1;			//0x94		// [31:12] reserved
										// [11:0] ucoef1
	PP_U32	ucoef2;			//0x98		// [31:12] reserved
										// [11:0] ucoef2
	PP_U32	ucoef3;			//0x9c		// [31:20] reserved
										// [19:0] ucoef3
	PP_U32	vcoef0;			//0xa0		// [31:12] reserved
										// [11:0] vcoef0
	PP_U32	vcoef1;			//0xa4		// [31:12] reserved
										// [11:0] vcoef1
	PP_U32	vcoef2;			//0xa8		// [31:12] reserved
										// [11:0] vcoef2
	PP_U32	vcoef3;			//0xac		// [31:20] reserved
										// [19:0] vcoef3
	PP_U32	reserved [32-12];
} PP_OSD_RGB2YUV_REG_S;

typedef volatile struct ppOSD_LUT_REG_S {
	PP_U32	argb [256];		//0x00		// [31:0] ARGB
} PP_OSD_LUT_REG_S;

typedef volatile struct ppOSD_RLE_REG_S {
	PP_OSD_AREA_REG_S		area[4];			// 0x5800 & 0x6000 & 0x6800 & 0x7000 & 0x7800
												// 0x5880 & 0x6080 & 0x6880 & 0x7080 & 0x7880
												// 0x5900 & 0x6100 & 0x6900 & 0x7100 & 0x7900
												// 0x5980 & 0x6180 & 0x6980 & 0x7180 & 0x7980
	PP_U32					reserved_0x00[32];	// 0x5a00 & 0x6200 & 0x6a00 & 0x7200 & 0x7a00
	PP_U32					reserved_0x80[32];	// 0x5a80 & 0x6280 & 0x6a80 & 0x7280 & 0x7a80
	PP_OSD_CANVAS_REG_S		canvas;             // 0x5b00 & 0x6300 & 0x6b00 & 0x7300 & 0x7b00
	PP_OSD_RGB2YUV_REG_S	rgb2yuv;        	// 0x5b80 & 0x6380 & 0x6b80 & 0x7380 & 0x7b80
	PP_OSD_LUT_REG_S		lut;                // 0x5c00 & 0x6400 & 0x6c00 & 0x7400 & 0x7c00
} PP_OSD_RLE_REG_S;

typedef volatile struct ppDU_OSD_REG_S {
	PP_OSD_CTRL_REG_S		ctrl;					// 0x4000
	PP_U32					reserved_0x4100[64];	// 0x4100
	PP_U32					reserved_0x4200[64];	// 0x4200
	PP_U32					reserved_0x4300[64];	// 0x4300
	PP_U32					reserved_0x4400[64];	// 0x4400
	PP_OSD_TG_REG_S			tg;						// 0x4500
	PP_U32					reserved_0x4600[64];	// 0x4600
	PP_U32					reserved_0x4700[64];	// 0x4700
	PP_U32					reserved_0x4800[64];	// 0x4800
	PP_U32					reserved_0x4900[64];	// 0x4900
	PP_U32					reserved_0x4a00[64];	// 0x4a00
	PP_U32					reserved_0x4b00[64];	// 0x4b00
	PP_U32					reserved_0x4c00[64];	// 0x4c00
	PP_U32					reserved_0x4d00[64];	// 0x4d00
	PP_U32					reserved_0x4e00[64];	// 0x4e00
	PP_U32					reserved_0x4f00[64];	// 0x4f00
	PP_OSD_MONITOR0_REG_S	monitor0;				// 0x5000
	PP_U32					reserved_0x5100[64];	// 0x5100
	PP_U32					reserved_0x5200[64];	// 0x5200
	PP_U32					reserved_0x5300[64];	// 0x5300
	PP_OSD_MONITOR1_REG_S	monitor1;				// 0x5400
	PP_U32					reserved_0x5500[64];	// 0x5500
	PP_U32					reserved_0x5600[64];	// 0x5600
	PP_U32					reserved_0x5700[64];	// 0x5700
	PP_OSD_RLE_REG_S		rle[5];					// 0x5800 & 0x6000 & 0x6800	& 0x7000 & 0x7800
} PP_DU_OSD_REG_S;

//----------------------------------------------------------------------------
//  du register map
//----------------------------------------------------------------------------
typedef volatile struct ppDU_REG_S {
	PP_DU_CTRL_REG_S		ctrl;					// 0x0000
	PP_DU_BTO_REG_S			bto;					// 0x0800
	PP_U32					reserved_0x1000[512];	// 0x1000
	PP_U32					reserved_0x1800[512];	// 0x1800
	PP_U32					reserved_0x2000[512];	// 0x2000
	PP_DU_BTO_CRC_REG_S		bto_crc;				// 0x2800
	PP_DU_BUS_TEST_S		bus_test;				// 0x3000
	PP_U32					reserved_0x3800[512];	// 0x3800
	PP_DU_OSD_REG_S			osd;					// 0x4000 ~ 0x7ffc
} PP_DU_REG_S;


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/



/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/


#ifdef __cplusplus
}
#endif

#endif  // __DU_REGISTER_H__

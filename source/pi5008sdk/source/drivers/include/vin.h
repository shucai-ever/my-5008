/*
 * vin.h
 *
 *  Created on: 2017. 4. 14.
 *      Author: ihkong
 */

#ifndef _PI5008_VIN_H_
#define _PI5008_VIN_H_

#include "system.h"
#include "type.h"
#include "error.h"

#ifndef _VER_VIN
#define _VER_VIN "1_000"
#endif //_VER_VIN

typedef enum {
	vres_720x480i60 = 0,  	// 0 
	vres_720x576i50,  		// 1 
	vres_960x480i60,  		// 2 
	vres_960x576i50,  		// 3 
	vres_720x480p60,  		// 4 
	vres_720x576p50,  		// 5 
	vres_960x480p60,  		// 6 
	vres_960x576p50,  		// 7 
	vres_1280x720p60,  		// 8 
	vres_1280x720p50,  		// 9 
	vres_1280x720p30,  		//10 
	vres_1280x720p25,  		//11 
	vres_1280x960p30,  		//12 
	vres_1280x960p25,  		//13 
	vres_1920x1080p30, 		//14 
	vres_1920x1080p25, 		//15 
	vres_800x480p60, 		//16  ----- don't suppot pvi  
	vres_800x480p50, 		//17  ----- don't suppot pvi 
	vres_1024x600p60, 		//18  ----- don't suppot pvi 
	vres_1024x600p50, 		//19  ----- don't suppot pvi 
	max_vid_resol
}_VID_RESOL;

typedef enum {
	vin_RAW6 = 0,  		// 0 
	vin_RAW7,  		// 1 
	vin_RAW8,  		// 2 
	vin_RAW10,  		// 3 
	vin_RAW12,  		// 4 
	vin_RAW14,  		// 5 
	vin_YUV8_2xRAW8,  		// 6 
	vin_2xRAW10,  		// 7 
	max_vin_datatype
}_VIN_DATATYPE;

typedef enum {
	vinVidInPath_PARA0 = 0,  		// 0 
	vinVidInPath_PARA1,  		// 1 
	vinVidInPath_MIPI,  		// 2 
	vinVidInPath_PVI,  		// 3 
	max_vinVidInPath
}_VIN_VIDIN_PATH;

typedef enum {
	vinVidInPort0 = 0,
	vinVidInPort1,
	vinVidInPort2,
	vinVidInPort3,
}_VIN_VIDIN_PORT;

typedef enum {
	vinVidResyncPath_VIDEO = 0,  		// 0 
	vinVidResyncPath_ISP,  		// 1 
	max_vinVidResyncPath
}_VIN_VIDRESYNC_PATH;

typedef enum {
	vinSvmInPath_VidIn = 0,  		// 0 
	vinSvmInPath_Pb,  		// 1 
	vinSvmInPath_Quad,  		// 2 
	vinSvmInPath_TP,  		// 3 
	max_vinSvmInPath
}_VIN_SVMIN_PATH;

typedef enum {
	vinSvmPort0 = 0,  		// 0 
	vinSvmPort1,  		// 1 
	vinSvmPort2,  		// 2 
	vinSvmPort3,  		// 3 
	max_vinSvmPort0
}_VIN_SVM_PORT0;

typedef enum {
	vinSvmPort4 = 0,  		// 0 
	vinSvmPortQuad,  		// 1 
	vinSvmPortTP,  		// 2 
	max_vinSvmPort1
}_VIN_SVM_PORT1;

typedef enum {
	vinRecInPath_VidIn = 0,  		// 0 
	vinRecInPath_Pb,  		// 1 
	vinRecInPath_TP,  		// 2 
	max_vinRecInPath
}_VIN_RECIN_PATH;

typedef enum {
	vinRecOutPort0 = 0,  		// 0 
	vinRecOutPort1,  		// 1 
	vinRecOutPort2,  		// 2 
	vinRecOutPort3,  		// 3 
	max_vinRecOutPort0
}_VIN_REC_PORT0;

typedef enum {
	vinRecOutPort4 = 0,  		// 0 
	vinRecOutPortQuad,  		// 1 
	vinRecOutPortTP,  		// 2 
	max_vinRecOutPort1
}_VIN_REC_PORT1;

typedef enum {
	vinQuadInPath_VidIn = 0,  	// 0 
	vinQuadInPath_Pb,  		// 1 
	vinQuadInPath_TP,  		// 1 
	max_vinQuadInPath
}_VIN_QUADIN_PATH;

typedef enum {
	vinQuadPort0 = 0,  		// 0 
	vinQuadPort1,  		// 1 
	vinQuadPort2,  		// 2 
	vinQuadPort3,  		// 3 
	max_vinQuadPort0
}_VIN_QUAD_PORT0;

typedef enum {
	vinQuadPort4 = 0,  		// 0 
	vinQuadPortTP,  		// 1 
	max_vinQuadPort1
}_VIN_QUAD_PORT1;

typedef enum {
	vinVpuOutPath_VidIn = 0,  		// 0 
	vinVpuOutPath_Pb,  		// 1 
	vinVpuOutPath_Quad,  		// 2 
	vinVpuOutPath_TP,  		// 3 
	max_vinVpuOutPath
}_VIN_VPUOUT_PATH;

typedef enum {
	vinVpuOutPort0 = 0,  		// 0 
	vinVpuOutPort1,  		// 1 
	vinVpuOutPort2,  		// 2 
	vinVpuOutPort3,  		// 3 
	max_vinVpuOutPort0
}_VIN_VPU_PORT0;

typedef enum {
	vinVpuOutPort4 = 0,  		// 0 
	vinVpuOutPortQuad,  		// 1 
	vinVpuOutPortTP,  		// 2 
	max_vinVpuOutPort1
}_VIN_VPU_PORT1;

typedef enum {
	vinCannyOutPath_VidIn = 0,  		// 0 
	vinCannyOutPath_Pb,  		// 1 
	vinCannyOutPath_Quad,  		// 2 
	vinCannyOutPath_TP,  		// 3 
	max_vinCannyOutPath
}_VIN_CANNYOUT_PATH;

typedef enum {
	vinCannyOutPort0 = 0,  		// 0 
	vinCannyOutPort1,  		// 1 
	vinCannyOutPort2,  		// 2 
	vinCannyOutPort3,  		// 3 
	max_vinCannyOutPort0
}_VIN_CANNY_PORT0;

typedef enum {
	vinCannyOutPort4 = 0,  		// 0 
	vinCannyOutPortQuad,  		// 1 
	vinCannyOutPortTP,  		// 2 
	max_vinCannyOutPort1
}_VIN_CANNY_PORT1;

typedef enum {
	vinRecOutPath_VidIn = 0,  		// 0 
	vinRecOutPath_Pb,  		// 1 
	vinRecOutPath_Quad,  		// 2 
	vinRecOutPath_TP,  		// 3 
	max_vinRecOutPath
}_VIN_RECOUT_PATH;

typedef enum {
	vinParSync_Ext = 0,
	vinParSync_Emb,
	max_vinParSync
}_VIN_PAR_SYNC;

typedef enum {
	vinParBitMapping_MSB = 0,
	vinParBitMapping_LSB,
	max_vinParBitMapping
}_VIN_PAR_BIT_MAPPING;

typedef enum {
	vinPBByteSwap_RGB = 0,
	vinPBByteSwap_GBR,	//1
	vinPBByteSwap_BRG,	//2
	vinPBByteSwap_8x8,	//3
	vinPBByteSwap_BGR,	//4
	vinPBByteSwap_GRB,	//5
	vinPBByteSwap_RBG,	//6
	vinPBByteSwap_8x8_Rev,	//7

}_VIN_PB_BYTE_SWAP;

typedef enum{
	vinPBInType_RGB_YUV_24Bit = 0,
	vinPBInType_YC_16Bit,
}_VIN_PBIN_TYPE;

typedef struct {
	_VIN_VIDIN_PATH	vidIn[5];
	_VIN_VIDIN_PORT vidPort[5];
	_VIN_VIDRESYNC_PATH	vidResync[4];
	_VIN_SVMIN_PATH	svmIn[4];
	_VIN_SVM_PORT0 svmPort[4];
	_VIN_QUADIN_PATH quadIn[4];
	_VIN_QUAD_PORT0 quadPort[4];
	_VIN_VPUOUT_PATH vpuOut;
	_VIN_VPU_PORT0 vpuPort;
	_VIN_CANNYOUT_PATH cannyOut;
	_VIN_CANNY_PORT0 cannyPort;
	_VIN_RECOUT_PATH recOut[4];
	_VIN_REC_PORT0 recPort[4];
}_VIN_PATH;

///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagVIN_CTRL_REG_T
{
	vuint32 ctrl;						/* 0x040 		- Input control */
	vuint32 htotal;						/* 0x044 		- Input H total */
	vuint32 vtotal;						/* 0x048 		- Input V total */
	vuint32 fld_os;						/* 0x04c 		- Input field sel */
}VIN_CTRL_REG_T;

typedef struct tagPAR_SYNC_CTRL_REG_T
{
	vuint32 vsync;						/* 0x0c0 		- vsync control */
	vuint32 hsync;						/* 0x0c4 		- hsync control */
}PAR_SYNC_CTRL_REG_T;

typedef struct tagQUAD_IN_CTRL_REG_T
{
	vuint32 ctrl;						/* 0x100 		- control */
	vuint32 hvscl;						/* 0x104 		- h/v scale ratio */
	vuint32 hvdel;						/* 0x108 		- h/v delay */
	vuint32 hvact;						/* 0x10c 		- h/v active size */
}QUAD_IN_CTRL_REG_T;

typedef struct tagBIST_INFO_REG_T
{
	vuint32 info[4];					/* 0x280~0x2dc	- quad crc bist info */
}BIST_INFO_REG_T;

typedef struct tagCRC_CTRL_REG_T
{
	vuint32 ctrl;						/* 0x340		- roi region control */
	vuint32 strt;						/* 0x344		- roi region start location control */
	vuint32 end;						/* 0x348		- roi region end location control */
	vuint32 rsv;						/* 0x34c		- reserved */
}CRC_CTRL_REG_T;

typedef struct tagSYNC_SIZE_CTL_REG_T
{
	vuint32 hsize;						/* 0x440		- sync detection h size control */
	vuint32 fsize;						/* 0x444		- sync detection frame size */
	vuint32 hvact;						/* 0x448		- sync detection frame size */
}SYNC_SIZE_CTL_REG_T;


typedef struct tagVIN_REG_T
{
	vuint32 par_in_fmt;						/* 0x000 		- Parallel input port format */
	vuint32 par_in_mux;						/* 0x004 		- Parallel MUX Mode Define */
	vuint32 mipi_in_fmt;						/* 0x008 		- MIPI input port format */
	vuint32 vid_in_sel;						/* 0x00c 		- Video input path sel */
	vuint32 isp_in_sel;						/* 0x010 		- ISP input path sel */
	vuint32 svm_in_sel;						/* 0x014 		- SVM input path sel */
	vuint32 quad_in_sel;						/* 0x018		- QUAD input sel */
	vuint32 rec_ch_sel;						/* 0x01c		- REC output channel sel */
	vuint32 vid_in_state[5];					/* 0x020~0x30 		- Video input status for Fsync/Vsync/Vloss/Hloss/Hoverflow */
	vuint32 mipi_status;						/* 0x034		- MIPI status */
	vuint32 svm_tp_sel;						/* 0x038		- SVM TP Sel */
	vuint32 quad_ddr_overflow;					/* 0x03c  		*/
	VIN_CTRL_REG_T vid_in_ch_ctrl[4];				/* 0x040~0x7c	- Video input format ch control */
	VIN_CTRL_REG_T rec_in_ch_ctrl[4];				/* 0x080~0xbc	- Record input format ch control */
	PAR_SYNC_CTRL_REG_T par_sync_ch_ctrl[4];			/* 0x0c0~0xdc	- Parallel sync control */
	vuint32 rsv3[8];							/* 0x0e0~0x0fc  - Reserved */
	QUAD_IN_CTRL_REG_T quad_in_ch_ctrl[4];				/* 0x100~0x13c	- Quad input control */
	vuint32 quad_mem_loc[4];					/* 0x140~0x14c	- Quad DDR memory start location */
	vuint32 quad_win_size;						/* 0x150 		- Quad window size */
	vuint32 quad_win_wr_en;						/* 0x154 		- Quad window write enable */
	vuint32 quad_win_rd_en;						/* 0x158 		- Quad window read enable */
	vuint32 quad_win_hvos;						/* 0x15c 		- Quad window hv start offset */
	vuint32 quad_rd_hvsize;						/* 0x160 		- Quad output h/v active size */
	vuint32 quad_rd_htotal;						/* 0x164 		- Quad htotal size */
	vuint32 quad_rd_vtotal;						/* 0x168 		- Quad vtotal size */
	vuint32 quad_rd_fld_os;						/* 0x16c 		- Quad field sel */
	vuint32 quad_out_fmt;						/* 0x170 		- Quad out fmt */
	vuint32 quad_full_md;						/* 0x174 		- Quad full md */
	vuint32 rsv4[2];							/* 0x178~0x17c  - Reserved */
	vuint32 rec_fmt_ctrl;						/* 0x180 		- Record format sel */
	vuint32 rec0_ch_sel;						/* 0x184 		- Record 0 path output sel */
	vuint32 rec1_ch_sel;						/* 0x188 		- Record 1 path output sel */
	vuint32 rsv5[13];						/* 0x18c~0x1bc  - Reserved */
	vuint32 par_pb_fmt;						/* 0x1c0 		- 16/24 Bits Parallel Input Mode */
	vuint32 rsv6[3];						/* 0x1c4~0x1cc  - Reserved */
	vuint32 par_pb_ycoef[4];					/* 0x1d0~0x1dc	- Coefficient for Paralle Color Converter */
	vuint32 par_pb_ucoef[4];					/* 0x1e0~0x1ec	- Coefficient for Paralle Color Converter */
	vuint32 par_pb_vcoef[4];					/* 0x1f0~0x1fc	- Coefficient for Paralle Color Converter */
	vuint32 genlock_low_width;					/* 0x200 		- Genlock pulse low duration control */
	vuint32 genlock_high_width;					/* 0x204 		- Genlock pulse high duration control */
	vuint32 rsv7[2];							/* 0x208~0x20c  - Reserved */
	vuint32 genlock_info[4];					/* 0x210~0x21c 	- Genlock info */
	vuint32 genlock_strt_os;					/* 0x220 		- Genlock detection control */
	vuint32 genlock_end_os;						/* 0x224 		- Genlock detection control */
	vuint32 rsv8[6];							/* 0x228~0x23c  - Reserved */
	VIN_CTRL_REG_T pbvid_in_ch_ctrl;				/* 0x240~0x24c	- PB Video input format ch control */
	VIN_CTRL_REG_T pbrec_in_ch_ctrl;				/* 0x250~0x25c	- PB Record input format ch control */
	vuint32 rsv9[8];							/* 0x260~0x27c  - Reserved */
	BIST_INFO_REG_T crc_bist_info[6];				/* 0x280~0x2dc 	- Genlock info */
	vuint32 rsv10[8];							/* 0x2e0~0x2fc  - Reserved */

	CRC_CTRL_REG_T crc_ctrl[6];			/* 0x300~0x35c 	- quad roi region control */
	vuint32 rsv11[24];							/* 0x360~0x3bc  - Reserved */
	vuint32 quad_ddr_crc_ctrl;					/* 0x3c0		- Quad ROI Region 3 Control */
	vuint32 rsv12[3];							/* 0x3c4~0x3cc  - Reserved */
	vuint32 quad_ddrwo_crc_data[4];						/* 0x3d0~0x3dc  - Quad DDR Write CRC Checker Output Data Bit */
	vuint32 quad_ddrro_crc_data[4];						/* 0x3e0~0x3ec  - Quad DDR Read CRC Checker Output Data Bit */
	vuint32 rsv13[4];							/* 0x3f0~0x3fc  - Reserved */
	SYNC_SIZE_CTL_REG_T sync_in_hvsize_ch_info[5]; 			/* 0x400~0x438 	- sync h/v size info */
	vuint32 rsv14[1];							/* 0x43c  - Reserved */
	SYNC_SIZE_CTL_REG_T sync_vin_hvsize_ch[5]; 			/* 0x440~0x478 	- sync h/v size */

}VIN_REG_T;

#define VIN_CTRL_REG			((VIN_REG_T*)			(VIN_BASE_ADDR))

extern int gVinRecFmtCtrl_DataRate; //0:148Mhz(8b), 1:74Mhz(8b), 2:36Mhz(8b), 3:27Mhz(8b)
extern int gVinRecCh0Sel_Clock; //0:148Mhz, 1:74Mhz, 2:37Mhz
extern int gVinRecCh0Sel_ClockInv; //0:normal, 1:invert
extern int gVinRecCh1Sel_Clock; //0:148Mhz, 1:74Mhz, 2:37Mhz
extern int gVinRecCh1Sel_ClockInv; //0:normal, 1:invert
extern int gVinRecFmtCtrl_MuxChMode; //0:1Ch, 1:2Ch, 2:4Ch

void PPDRV_VIN_Initialize(const int defVinFmt, const int defVoutFmt, const int defQuadOutFmt);

PP_RESULT_E PPDRV_VIN_GetResol(const PP_S32 defVideoFmt, PP_S32 *pRetWidth, PP_S32 *pRetHeight, _VID_RESOL *peRetResol);
void PPDRV_VIN_set_active_size(uint32 ch, uint32 size_h, uint32 size_v);
void PPDRV_VIN_set_par_sync_type(_VIN_VIDIN_PORT port, _VIN_PAR_SYNC sync_type);
void PPDRV_VIN_set_par_bit_mapping(_VIN_VIDIN_PORT port, _VIN_PAR_BIT_MAPPING bit_mapping);
void PPDRV_VIN_set_pb_fmt(_VIN_PB_BYTE_SWAP swap_mode, _VIN_PAR_BIT_MAPPING bit_mapping_8x8, _VIN_PBIN_TYPE in_type, uint8 isRGB, _VIN_PAR_SYNC sync_type);
void PPDRV_VIN_set_par_chmux(uint8 path, uint8 muxch);

void PPDRV_VIN_svm_input_sel(uint32 svm_ch, uint32 in_port, uint32 path);
void PPDRV_VIN_svm_input_tp_resol(uint32 resol);
void PPDRV_VIN_svm_intput_tp_sel(uint32 svm_ch, uint32 test_pattern);

void PPDRV_VIN_vpu_input_sel(uint32 in_port, const _VIN_VPUOUT_PATH path);

void PPDRV_VIN_GetInputInfo(const uint8 ch, uint32 *pu32InputInfo);
#endif /* _PI5008_VIN_H_ */

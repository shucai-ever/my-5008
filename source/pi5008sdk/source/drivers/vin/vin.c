/*
 * vin.c
 *
 *  Created on: 2017. 4. 18.
 *      Author: ihkong
 */

#include <stdio.h>
#include <unistd.h>
#include <nds32_intrinsic.h>
#include "pi5008.h"

#include "osal.h"
#include "system.h"
#include "interrupt.h"
#include "dram_config.h"
#include "proc.h"

#include "vin.h"
#include "api_vin.h"
#include "vin_user_config.h"
#include "video_config.h"
#include "debug.h"

extern _gstDramReserved gstDramReserved;

static VIN_REG_T *gpVIN = VIN_CTRL_REG;

static uint8 gu8VinInitialized;
/////////////////////////////////////////////////////////////////////////////////////////////////////
void PPDRV_VIN_Initialize(const int defVinFmt, const int defVoutFmt, const int defQuadOutFmt)
{/*{{{*/
	_VIN_PATH vinPath;
	_VIN_USER_CONFIG_REG vinUserReg;

	uint32 reg;
	sint32 i;
	uint8 vinCh;

	int vinWidth, vinHeight;
	_VID_RESOL eVidResol;
	int voutWidth, voutHeight;
	_VID_RESOL eVoutResol;
	int quadOutWidth, quadOutHeight;
	_VID_RESOL eQuadOutResol;
	uint8 bQuadP2I, bQuadI2P;
	int iQuadRate = 0; //-1:down(60->30), 0:equal, 1:up(30->60)
	uint8 bQuadHeightOverSize = FALSE, bQuadWidthOverSize = FALSE;

	PI_VIN_GetUserConfigVar(&vinPath, &vinUserReg);

	if( PPDRV_VIN_GetResol(defVinFmt, &vinWidth, &vinHeight, &eVidResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return;
    }
	if( PPDRV_VIN_GetResol(defVoutFmt, &voutWidth, &voutHeight, &eVoutResol) != eSUCCESS )
    {
        LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return;
    }
	if( PPDRV_VIN_GetResol(defQuadOutFmt, &quadOutWidth, &quadOutHeight, &eQuadOutResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return;
    }

	bQuadP2I = FALSE;
	bQuadI2P = FALSE;
	iQuadRate = 0; //-1:down(60->30), 0:equal, 1:up(30->60)
	if( (eVidResol>=vres_720x480p60) && (eQuadOutResol<=vres_960x576i50) )
	{
		bQuadP2I = TRUE;
	}
	if( (eVidResol<=vres_960x576i50) && (eQuadOutResol>=vres_720x480p60) )
	{
		bQuadI2P = TRUE;
	}
	if( (eVidResol<=vres_1280x720p50) && (eQuadOutResol>=vres_1280x720p30) )
	{
		iQuadRate = -1; //-1:down
	}
	if( (eVidResol>=vres_1280x720p30) && (eQuadOutResol<=vres_1280x720p50) )
	{
		iQuadRate = 1; //1:up
	}

	/* check quad outsize is over */
	if( (vinHeight) < (quadOutHeight/2) )
	{
		bQuadHeightOverSize = TRUE;
	}
	if( (vinWidth) < (quadOutWidth/2) )
	{
		bQuadWidthOverSize = TRUE;
	}

	LOG_DEBUG("%s: vin:%s, vout:%s, quad:%s, quadp2i:%d, quadi2p:%d, quadrate:%d quadover(%d,%d)\n", __FUNCTION__,
		DEF_STR_VIN_RESOL[eVidResol], DEF_STR_VIN_RESOL[eVoutResol], DEF_STR_VIN_RESOL[eQuadOutResol], 
		bQuadP2I, bQuadI2P, iQuadRate, bQuadWidthOverSize, bQuadHeightOverSize);

	gpVIN->par_in_fmt = vinUserReg.PI_BD_CONFIG_PAR_IN_FMT_port.var;

	gpVIN->par_in_mux = vinUserReg.PI_BD_CONFIG_PAR_IN_MUX_val.var;

	gpVIN->mipi_in_fmt = vinUserReg.PI_BD_CONFIG_MIPI_IN_FMT_val.var;

	gpVIN->vid_in_sel = vinUserReg.PI_BD_CONFIG_VID_IN_SEL_val.var;

	gpVIN->isp_in_sel = vinUserReg.PI_BD_CONFIG_ISP_IN_SEL_val.var;

	gpVIN->svm_in_sel = vinUserReg.PI_BD_CONFIG_SVM_IN_SEL_val.var;

	gpVIN->quad_in_sel = vinUserReg.PI_BD_CONFIG_QUAD_CH_SEL_val.var;

	gpVIN->rec_ch_sel = vinUserReg.PI_BD_CONFIG_REC_CH_SEL_val.var;

	gpVIN->svm_tp_sel = vinUserReg.PI_BD_CONFIG_SVM_TP_SEL_val.var;

	for(i = 0; i < 4; i++)
	{
		gpVIN->vid_in_ch_ctrl[i].ctrl = vinUserReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->vid_in_ch_ctrl[i].htotal = vinUserReg.PI_BD_CONFIG_VID_IN_HTOTAL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->vid_in_ch_ctrl[i].vtotal = vinUserReg.PI_BD_CONFIG_VID_IN_VTOTAL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->vid_in_ch_ctrl[i].fld_os = vinUserReg.PI_BD_CONFIG_VID_IN_FLD_OS_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->rec_in_ch_ctrl[i].ctrl = vinUserReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->rec_in_ch_ctrl[i].htotal = vinUserReg.PI_BD_CONFIG_REC_IN_HTOTAL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->rec_in_ch_ctrl[i].vtotal = vinUserReg.PI_BD_CONFIG_REC_IN_VTOTAL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->rec_in_ch_ctrl[i].fld_os = vinUserReg.PI_BD_CONFIG_REC_IN_FLD_OS_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->par_sync_ch_ctrl[i].vsync = vinUserReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->par_sync_ch_ctrl[i].hsync = vinUserReg.PI_BD_CONFIG_PAR_HSYNC_CTRL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->quad_in_ch_ctrl[i].ctrl = vinUserReg.PI_BD_CONFIG_QUAD_CTRL_val[i].var;
	}

	/* quad_in_ch_ctrl.quadx_hvscl */
	/*
	 * [31:16]	16'b0	RW		"Quad Scaling Ratio Selection for Vertical Direction
	 *  0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale"
	 * [15:00]	16'b0			"Quad Scaling Ratio Selection for Horizontal Direction
	 *  0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale"
	 */
	for(i = 0; i < 4; i++)
	{
		//gpVIN->quad_in_ch_ctrl[i].hvscl = vinUserReg.PI_BD_CONFIG_QUAD_HVSCL_val[i].var;
		if(bQuadWidthOverSize)
		{
			reg = 0xFFFF;
		}
		else
		{
			reg = ( (((0xFFFF * quadOutWidth)/2)/vinWidth) << 0 );
		}

		if(bQuadHeightOverSize)
		{
			reg |= 0xFFFF<<16;
		}
		else
		{
			reg |= ( (((0xFFFF * quadOutHeight * ((eQuadOutResol>=vres_720x480p60)?1:2))/2)/(vinHeight*((eVidResol>=vres_720x480p60)?1:2))) << 16);
		}
		gpVIN->quad_in_ch_ctrl[i].hvscl = reg;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->quad_in_ch_ctrl[i].hvdel = vinUserReg.PI_BD_CONFIG_QUAD_HVDEL_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->quad_in_ch_ctrl[i].hvact = vinUserReg.PI_BD_CONFIG_QUAD_HVACT_val[i].var;
	}

	/* quad_in_ch_ctrl.quadx_mem_loc */
	/*
	 * [31:00]	32'b0	RW		"Quad DDR Memory Start Location for FHD
	 *  0x2200_0000 for Quad Ch 0
	 *  0x2240_0000 for Quad Ch 1
	 *  0x2280_0000 for Quad Ch 2
	 *  0x22C0_0000 for Quad Ch 3"
	 */
#if 0
#ifndef ALLOC_DRAM_RESERVED
	/* (16bit*Horizon*Vertical)/4(quad)*4(page) */
#define MEMSIZE_QUAD_1280x720  (((2*1280*720)>>2)*4) //0x001C2000
#define MEMSIZE_QUAD_1920x1080  (((2*1920*1080)>>2)*4) //0x003F4800
#define MAX_MEMSIZE_QUAD (MEMSIZE_QUAD_1920x1080*4) //4channel
	{
		uint32_t memSize;
		uint32_t addrMEM = 0;
		uint32_t *pMEM = NULL;

		memSize = (MAX_MEMSIZE_QUAD+255)&0xFFFFFF00; //align 256byte
		//memSize = ((MEMSIZE_QUAD_1920x1080>>16)+1)<<16; //align 0x10000(64KB)
		//memSize = ((MEMSIZE_QUAD_1280x720>>16)+1)<<16; //align 0x10000(64KB)

#ifdef PLATFORM //OS malloc function.
		pMEM = OSAL_malloc(memSize+255);
		if(pMEM == NULL)
		{
			printf("%s:Fail malloc VIN Quad\n", __FUNCTION__);
			return;
		}
		addrMEM = ((uint32_t)pMEM+255)&0xFFFFFF00;
		printf("malloc VIN Quad memory(size:0x%08lx): addr:0x%08lx\n", memSize, addrMEM);
#else
		addrMEM = (uint32_t)0x20080000; //Temporary.
		printf("static VIN Quad memory(size:0x%08lx): addr:0x%08lx\n", memSize, addrMEM);
#endif
		for(i = 0; i < 4; i++)
		{

			{
				reg = (
						(((uint32_t)(addrMEM)+(i*MEMSIZE_QUAD_1920x1080))<<0)
				      );
				gpVIN->quad_mem_loc[i] = reg;
			}
		}
	}
#endif // ALLOC_DRAM_RESERVED
#endif

    for(i = 0; i < 4; i++) //4ch
    {
		reg = gstDramReserved.u32QuadRsvCh[i];
		gpVIN->quad_mem_loc[i] = reg;
    }	

	/* quad_win_size */
	/*
	 * [29:28]	2'b0	RW		"Quad Frame Memory Size
	 *  0 : 1 Page, ~ 3 : 4 Page"
	 * [25:19]	7'b0			"Quad Window Hsize (8 Pixels Domain) = Active H Size / 2
	 *  960/8 = 120 for FHD, 640/8 = 80 for HD"
	 * [09:00]	10'b0			"Quad Window Vsize (1 Lines Domain) = Active V Size / 2
	 *  540 for FHD, 360 for HD"
	 */
	//gpVIN->quad_win_size = vinUserReg.PI_BD_CONFIG_QUAD_WIN_SIZE_val.var;
	if(bQuadHeightOverSize)
	{
		reg = (((vinHeight)&0x3FF)<<0);
	}
	else
	{
		reg = (((DEF_VIN_QUAD_HV_WIN_SIZE[eQuadOutResol][1]*((bQuadP2I)?2:1))&0x3FF)<<0);
	}
	if(bQuadWidthOverSize)
	{
		reg |= ((vinWidth&0x7F)<<19);
	}
	else
	{
		reg |= ((DEF_VIN_QUAD_HV_WIN_SIZE[eQuadOutResol][0]&0x7F)<<19);
	}
	reg |= (3<<28);
	gpVIN->quad_win_size = reg;

	/* quad_win_wr_en */
	reg = vinUserReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.var;
	/* 60i to 30p/60p, or 50i to 25p/50p => 1, else 0 */
	vinCh = 0; 
	reg &= ~(0x1<<(1*vinCh+16));
	if( (eVidResol<=vres_960x576i50) && (eQuadOutResol>=vres_720x480p60) )
	{
		reg |= (0x1)<<(1*vinCh+16);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+16);
	}
	vinCh = 1; 
	reg &= ~(0x1<<(1*vinCh+16));
	if( (eVidResol<=vres_960x576i50) && (eQuadOutResol>=vres_720x480p60) )
	{
		reg |= (0x1)<<(1*vinCh+16);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+16);
	}
	vinCh = 2; 
	reg &= ~(0x1<<(1*vinCh+16));
	if( (eVidResol<=vres_960x576i50) && (eQuadOutResol>=vres_720x480p60) )
	{
		reg |= (0x1)<<(1*vinCh+16);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+16);
	}
	vinCh = 3; 
	reg &= ~(0x1<<(1*vinCh+16));
	if( (eVidResol<=vres_960x576i50) && (eQuadOutResol>=vres_720x480p60) )
	{
		reg |= (0x1)<<(1*vinCh+16);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+16);
	}

	/* 50p/60p to 60i/50i or 50p/60p to 25p/30p => 1, else 0 */
	vinCh = 0; 
	reg &= ~(0x1<<(1*vinCh+20));
	if( (((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol<=vres_960x576i50)) ||
		(((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol>=vres_1280x720p30)) )
	{
		reg |= (0x1)<<(1*vinCh+20);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+20);
	}
	vinCh = 1; 
	reg &= ~(0x1<<(1*vinCh+20));
	if( (((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol<=vres_960x576i50)) ||
		(((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol>=vres_1280x720p30)) )
	{
		reg |= (0x1)<<(1*vinCh+20);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+20);
	}
	vinCh = 2; 
	reg &= ~(0x1<<(1*vinCh+20));
	if( (((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol<=vres_960x576i50)) ||
		(((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol>=vres_1280x720p30)) )
	{
		reg |= (0x1)<<(1*vinCh+20);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+20);
	}
	vinCh = 3; 
	reg &= ~(0x1<<(1*vinCh+20));
	if( (((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol<=vres_960x576i50)) ||
		(((eVidResol>=vres_720x480p60)&&(eVidResol<=vres_1280x720p50)) && (eQuadOutResol>=vres_1280x720p30)) )
	{
		reg |= (0x1)<<(1*vinCh+20);
	}
	else
	{
		reg |= (0x0)<<(1*vinCh+20);
	}

	vinCh = 0; 
	reg &= ~(0x1<<(1*vinCh+24));
	reg |= ((((eVidResol>=vres_720x480p60)?0:1))<<(1*vinCh+24));

	vinCh = 1;
	reg &= ~(0x1<<(1*vinCh+24));
	reg |= ((((eVidResol>=vres_720x480p60)?0:1))<<(1*vinCh+24));

	vinCh = 2;
	reg &= ~(0x1<<(1*vinCh+24));
	reg |= ((((eVidResol>=vres_720x480p60)?0:1))<<(1*vinCh+24));

	vinCh = 3;
	reg &= ~(0x1<<(1*vinCh+24));
	reg |= ((((eVidResol>=vres_720x480p60)?0:1))<<(1*vinCh+24));

	gpVIN->quad_win_wr_en = reg;

	/* quad_win_rd_en */
	reg = vinUserReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.var;
	/* progressive to interlace => 1, else 0 */
	if( (eVidResol>=vres_720x480p60) && (eQuadOutResol<=vres_960x576i50) )
	{
		reg &= ~((0x1)<<29);
		reg |= (0x1)<<29;
	}
	else
	{
		reg &= ~((0x1)<<29);
		reg |= (0x0)<<29;
	}
	gpVIN->quad_win_rd_en = reg;

	gpVIN->quad_win_hvos = vinUserReg.PI_BD_CONFIG_QUAD_WIN_HVOS_val.var;

	gpVIN->quad_rd_hvsize = vinUserReg.PI_BD_CONFIG_QUAD_RD_HVSIZE_val.var;

	gpVIN->quad_rd_htotal = vinUserReg.PI_BD_CONFIG_QUAD_RD_HTOTAL_val.var;

	gpVIN->quad_rd_vtotal = vinUserReg.PI_BD_CONFIG_QUAD_RD_VTOTAL_val.var;

	gpVIN->quad_rd_fld_os = vinUserReg.PI_BD_CONFIG_QUAD_RD_FLD_OS_val.var;

	gpVIN->quad_out_fmt = vinUserReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.var;

	gpVIN->quad_full_md = vinUserReg.PI_BD_CONFIG_QUAD_FULL_MD_val.var;

	gpVIN->rec_fmt_ctrl = vinUserReg.PI_BD_CONFIG_REC_FMT_CTRL_val.var;

	gpVIN->rec0_ch_sel = vinUserReg.PI_BD_CONFIG_REC0_CH_SEL_val.var;

	gpVIN->rec1_ch_sel = vinUserReg.PI_BD_CONFIG_REC1_CH_SEL_val.var;

	gpVIN->par_pb_fmt = vinUserReg.PI_BD_CONFIG_PAR_PB_FMT_val.var;

	for(i = 0; i < 4; i++)
	{
		gpVIN->par_pb_ycoef[i] = vinUserReg.PI_BD_CONFIG_PAR_PB_YCOEF_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->par_pb_ucoef[i] = vinUserReg.PI_BD_CONFIG_PAR_PB_UCOEF_val[i].var;
	}

	for(i = 0; i < 4; i++)
	{
		gpVIN->par_pb_vcoef[i] = vinUserReg.PI_BD_CONFIG_PAR_PB_VCOEF_val[i].var;
	}

	gpVIN->genlock_low_width = vinUserReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.var;

	gpVIN->genlock_high_width = vinUserReg.PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val.var;

	gpVIN->genlock_strt_os = vinUserReg.PI_BD_CONFIG_GENLOCK_START_OS_val.var;

	gpVIN->genlock_end_os = vinUserReg.PI_BD_CONFIG_GENLOCK_END_OS_val.var;

	gpVIN->pbvid_in_ch_ctrl.ctrl = vinUserReg.PI_BD_CONFIG_PB_IN_CTRL_val.var;

	gpVIN->pbvid_in_ch_ctrl.htotal = vinUserReg.PI_BD_CONFIG_PB_IN_HTOTAL_val.var;

	gpVIN->pbvid_in_ch_ctrl.vtotal = vinUserReg.PI_BD_CONFIG_PB_IN_VTOTAL_val.var;

	gpVIN->pbvid_in_ch_ctrl.fld_os = vinUserReg.PI_BD_CONFIG_PB_IN_FLD_OS_val.var;

	gpVIN->pbrec_in_ch_ctrl.ctrl = vinUserReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.var;

	gpVIN->pbrec_in_ch_ctrl.htotal = vinUserReg.PI_BD_CONFIG_PBREC_IN_HTOTAL_val.var;

	gpVIN->pbrec_in_ch_ctrl.vtotal = vinUserReg.PI_BD_CONFIG_PBREC_IN_VTOTAL_val.var;

	gpVIN->pbrec_in_ch_ctrl.fld_os = vinUserReg.PI_BD_CONFIG_PBREC_IN_FLD_OS_val.var;

	for(i = 0; i < 6; i++)
	{
		gpVIN->crc_ctrl[i].ctrl = vinUserReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].var;
	}

	for(i = 0; i < 6; i++)
	{
		gpVIN->crc_ctrl[i].strt = vinUserReg.PI_BD_CONFIG_VID_CRC_STRT_val[i].var;
	}

	for(i = 0; i < 6; i++)
	{
		gpVIN->crc_ctrl[i].end = vinUserReg.PI_BD_CONFIG_VID_CRC_END_val[i].var;
	}

	gpVIN->quad_ddr_crc_ctrl = vinUserReg.PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val.var;

	for(i = 0; i < 5; i++)
	{
		gpVIN->sync_vin_hvsize_ch[i].hsize = vinUserReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].var;
	}

	for(i = 0; i < 5; i++)
	{
		gpVIN->sync_vin_hvsize_ch[i].fsize = vinUserReg.PI_BD_CONFIG_SYNC_VIN_FSIZE_val[i].var;
	}

	for(i = 0; i < 5; i++)
	{

		//printf("%s: %s, %s\n", __FUNCTION__, _STR_VIN_DATATYPE[rawType], _STR_VIN_RESOL[eVidResol]);
		if( (PI_BD_CONFIG_VinRawType[i] == vin_RAW6) || (PI_BD_CONFIG_VinRawType[i] == vin_RAW7) || (PI_BD_CONFIG_VinRawType[i] == vin_RAW8) )
		{
			reg = (
					((vinUserReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].param.sync_vact)<<16) |
					(vinUserReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].param.sync_hact/2)
			      );

		}
		else
		{
			reg = (
					((vinUserReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].param.sync_vact)<<16) |
					(vinUserReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].param.sync_hact)
			      );
		}
		gpVIN->sync_vin_hvsize_ch[i].hvact = reg;
	}

	gu8VinInitialized = TRUE;
}/*}}}*/

PP_RESULT_E PPDRV_VIN_GetResol(const PP_S32 defVideoFmt, PP_S32 *pRetWidth, PP_S32 *pRetHeight, _VID_RESOL *peRetResol)
{
	/*  get in/out resolution. */
	if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
		{
			*pRetWidth = 720;
			*pRetHeight = 240;
			*peRetResol = vres_720x480i60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
		{
			*pRetWidth = 720;
			*pRetHeight = 288;
			*peRetResol = vres_720x576i50;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol720H\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
		{
			*pRetWidth = 960;
			*pRetHeight = 240;
			*peRetResol = vres_960x480i60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
		{
			*pRetWidth = 960;
			*pRetHeight = 288;
			*peRetResol = vres_960x576i50;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol960H\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_SDH720)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
		{
			*pRetWidth = 720;
			*pRetHeight = 480;
			*peRetResol = vres_720x480p60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
		{
			*pRetWidth = 720;
			*pRetHeight = 576;
			*peRetResol = vres_720x576p50;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResolSDH720\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_SDH960)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
		{
			*pRetWidth = 960;
			*pRetHeight = 480;
			*peRetResol = vres_960x480p60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
		{
			*pRetWidth = 960;
			*pRetHeight = 576;
			*peRetResol = vres_960x576p50;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResolSDH960\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
		{
			*pRetWidth = 1280;
			*pRetHeight = 720;
			*peRetResol = vres_1280x720p60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
		{
			*pRetWidth = 1280;
			*pRetHeight = 720;
			*peRetResol = vres_1280x720p50;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
		{
			*pRetWidth = 1280;
			*pRetHeight = 720;
			*peRetResol = vres_1280x720p30;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
		{
			*pRetWidth = 1280;
			*pRetHeight = 720;
			*peRetResol = vres_1280x720p25;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol720P\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
		{
			*pRetWidth = 1280;
			*pRetHeight = 960;
			*peRetResol = vres_1280x960p30;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
		{
			*pRetWidth = 1280;
			*pRetHeight = 960;
			*peRetResol = vres_1280x960p25;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol960P\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
		{
			*pRetWidth = 1920;
			*pRetHeight = 1080;
			*peRetResol = vres_1920x1080p30;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
		{
			*pRetWidth = 1920;
			*pRetHeight = 1080;
			*peRetResol = vres_1920x1080p25;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol1080P\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_HD800_480P)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
		{
			*pRetWidth = 800;
			*pRetHeight = 480;
			*peRetResol = vres_800x480p60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
		{
			*pRetWidth = 800;
			*pRetHeight = 480;
			*peRetResol = vres_800x480p50;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol800_480P\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else if((defVideoFmt & VID_RESOL_MASKBIT) == VID_RESOL_HD1024_600P)
	{
		if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
		{
			*pRetWidth = 1024;
			*pRetHeight = 600;
			*peRetResol = vres_1024x600p60;
		}
		else if((defVideoFmt & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
		{
			*pRetWidth = 1024;
			*pRetHeight = 600;
			*peRetResol = vres_1024x600p50;
		}
		else
		{
			LOG_DEBUG("Error. Invalid vResol1024_600P\n");
			return(eERROR_NOT_SUPPORT);
		}
	}
	else
	{
		LOG_DEBUG("Error. Invalid defVideoFmt\n");
		return(eERROR_INVALID_ARGUMENT);
	}
	return(eSUCCESS);
}

void PPDRV_VIN_set_active_size(_VIN_VIDIN_PORT port, uint32 size_h, uint32 size_v)
{
	uint32 reg;

	reg = gpVIN->vid_in_ch_ctrl[port].ctrl;
	reg &= (~((0x7ff<<16) | (0x7ff)));
	reg |= (((size_h&0x7ff)<<16) | (size_v&0x7ff));
	gpVIN->vid_in_ch_ctrl[port].ctrl = reg;

}


// set parallel sync type: external sync, embedded sync
void PPDRV_VIN_set_par_sync_type(_VIN_VIDIN_PORT port, _VIN_PAR_SYNC sync_type)
{
	uint32 reg;

	reg = gpVIN->par_in_fmt;

	reg &= (~(1 << (6 + port*8)));
	reg |= ( (sync_type&1) << (6 + port*8) );

	gpVIN->par_in_fmt = reg;
}

// set parallel bit mapping: MSB mapping, LSB mapping
void PPDRV_VIN_set_par_bit_mapping(_VIN_VIDIN_PORT port, _VIN_PAR_BIT_MAPPING bit_mapping)
{
	uint32 reg;

	reg = gpVIN->par_in_fmt;

	reg &= (~(1 << (3 + port*8)));
	reg |= ( (bit_mapping&1) << (3 + port*8) );

	gpVIN->par_in_fmt = reg;

}

void PPDRV_VIN_set_pb_fmt(_VIN_PB_BYTE_SWAP swap_mode, _VIN_PAR_BIT_MAPPING bit_mapping_8x8, _VIN_PBIN_TYPE in_type, uint8 isRGB, _VIN_PAR_SYNC sync_type)
{
	uint32 reg;

	reg = (
		((bit_mapping_8x8 & 1) << 0)	|
		((in_type & 1) << 1)			|
		((isRGB & 1) << 2)				|
		(0 << 3)						|	// bit swap
		((swap_mode & 7) << 4)			|
		((sync_type & 1) << 7)			|
		(0 << 8)						|	// embedded sync position
		(0 << 10)							// input sync mode - bt1120
	);

	gpVIN->par_pb_fmt &= 0x80000000;
	gpVIN->par_pb_fmt |= reg;

}

// [muxch]
// 0: 1ch, 1: 2ch, 2: 4ch
void PPDRV_VIN_set_par_chmux(uint8 path, uint8 muxch)
{
	uint32 reg;

	reg = gpVIN->par_in_mux;

	reg &= (~(0x1f<<(path*8)));
	reg |= ( ( ((muxch == 0 ? 1 : 0)<<4) | (muxch) )<<(path*8) ); // bit[12], bit[4]: 0ch -> 1(id fixed), 2/4ch -> 0

	gpVIN->par_in_mux = reg;
}

void PPDRV_VIN_svm_input_sel(uint32 svm_ch, uint32 in_port, uint32 path)
{
	uint32 val;
	
	val = ( ((path&3)<<2) | in_port );
	gpVIN->svm_in_sel = utilPutBits( gpVIN->svm_in_sel, svm_ch*4, 4, val );

}

void PPDRV_VIN_svm_input_tp_resol(uint32 resol)
{
	gpVIN->svm_in_sel = utilPutBits( gpVIN->svm_in_sel, 20, 4, resol );
}

void PPDRV_VIN_svm_input_tp_sel(uint32 svm_ch, uint32 test_pattern)
{
	uint32 val;

	val = ((3<<2) | 1);

	gpVIN->svm_in_sel = utilPutBits( gpVIN->svm_in_sel, 16, 4, test_pattern );
	gpVIN->svm_in_sel = utilPutBits( gpVIN->svm_in_sel, svm_ch*4, 4, val );
}

void PPDRV_VIN_vpu_input_sel(uint32 in_port, const _VIN_VPUOUT_PATH path)
{
	uint32 val;
	
	val = ( ((path&3)<<2) | in_port );
	gpVIN->rec_ch_sel = utilPutBits( gpVIN->rec_ch_sel, 24, 4, val );
}

void PPDRV_VIN_GetInputInfo(const uint8 ch, uint32 *pu32InputInfo)
{
    if(ch > 5) return;

	pu32InputInfo[0] = gpVIN->sync_in_hvsize_ch_info[ch].hsize;
	pu32InputInfo[1] = gpVIN->sync_in_hvsize_ch_info[ch].fsize;
	pu32InputInfo[2] = gpVIN->sync_in_hvsize_ch_info[ch].hvact;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern struct proc_device_struct stVIN_PROC_devices[];
PP_RESULT_E VIN_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	int i;
	uint32 reg;
	_VIN_USER_CONFIG_REG stCurReg;

	if( (argc) && (strcmp(argv[0], stVIN_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}
	printf("\n%s Device Info -------------\n", stVIN_PROC_devices[0].pName);

	/* get current register value */
	{/*{{{*/
		printf("### VIN info(Driver Version : v%s) ### \n", _VER_VIN);

		printf("---------------------- VIN info ---------------------\n");
		printf("                (Initialized:%s)\n", ((gu8VinInitialized == 0)?"Fail":"Success")); 

		printf("------------------ vin setting info -------------------------\n");
		printf("  PAR IN FMT: \n");
		stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.var = gpVIN->par_in_fmt;
		printf("    ch datatype bit_lsb bit_swap bt1120 sync byte_swap\n");
		for(i = 0; i < 4; i++)
		{
			printf("    %02d %8d %7d %8d %6d %4d %9d\n", i,
					stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.param.par0_data_type,
					stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.param.par0_bit_lsb,
					stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.param.par0_bit_swap,
					stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.param.par0_bt1120_md,
					stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.param.par0_sync_md,
					stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.param.par0_byte_swap);
			stCurReg.PI_BD_CONFIG_PAR_IN_FMT_port.var >>= (8);
		}

		printf("  PAR IN MUX: \n");
		stCurReg.PI_BD_CONFIG_PAR_IN_MUX_val.var = gpVIN->par_in_mux;
		printf("    par0_in_mux par0_id_md par1_in_mux par1_id_md\n");
		printf("    %11d %10d %11d %10d\n",
				stCurReg.PI_BD_CONFIG_PAR_IN_MUX_val.param.par0_in_mux,
				stCurReg.PI_BD_CONFIG_PAR_IN_MUX_val.param.par0_id_md,
				stCurReg.PI_BD_CONFIG_PAR_IN_MUX_val.param.par1_in_mux,
				stCurReg.PI_BD_CONFIG_PAR_IN_MUX_val.param.par1_id_md);

		printf("  MIPI IN FMT: \n");
		stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.var = gpVIN->mipi_in_fmt;
		printf("    ch_hsize vc_size vc_md data_type async_md clk_sel\n");
		printf("    %8d %7d %5d %9d %8d %7d\n",
				stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.param.mipi_ch_hsize,
				stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.param.mipi_vc_size,
				stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.param.mipi_vc_md,
				stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.param.mipi_data_type,
				stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.param.mipi_async_md,
				stCurReg.PI_BD_CONFIG_MIPI_IN_FMT_val.param.mipi_clk_sel);

		printf("  VID IN SEL: \n");
		stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.var = gpVIN->vid_in_sel;
		printf("    pvi_async mipi_async par_async\n");
		printf("    %9x %10x %9x\n",
				stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.param.pvi_as_en,
				stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.param.mipi_as_en,
				stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.param.par_as_en);
		printf("    ch port path\n");
		for(i = 0; i < 4; i++)
		{
			printf("    %02d %4d %4d\n", i,
					stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.param.vid0_in_sel_portsel,
					stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.param.vid0_in_sel_pathsel);
			stCurReg.PI_BD_CONFIG_VID_IN_SEL_val.var >>= (4);
		}

		printf("  ISP IN SEL: \n");
		stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.var = gpVIN->isp_in_sel;
		printf("    tst_pt_sel tst_mode tst_en in_async in_async_chg out_async out_async_chg rec_in_sel isp_tp_tst_sim1\n");
		printf("    %10d %8d %6d %8d %12d %9d %13d %10d %15d\n",
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_tst_pt_sel,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_tst_mode,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_tp_en,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_in_async_en,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_in_async_chg_en,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_out_async_en,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_out_async_chg_en,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.rec_in_sel,
				stCurReg.PI_BD_CONFIG_ISP_IN_SEL_val.param.isp_tp_tst_sim1);

		printf("  SVM IN SEL: \n");
		stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.var = gpVIN->svm_in_sel;
		printf("    tp_tst_sim1 vsync_md fld_inv\n");
		printf("    %11d %8d %7d\n",
				stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.param.svm_tp_tst_sim1,
				stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.param.sel_vsync_md,
				stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.param.svm_fld_inv);
		printf("    ch port path\n");
		for(i = 0; i < 4; i++)
		{
			printf("    %02d %4d %4d\n", i,
					stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.param.svm0_in_sel_portsel,
					stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.param.svm0_in_sel_pathsel);
			stCurReg.PI_BD_CONFIG_SVM_IN_SEL_val.var >>= (4);
		}

		printf("  QUAD CH SEL: \n");
		stCurReg.PI_BD_CONFIG_QUAD_CH_SEL_val.var = gpVIN->quad_in_sel;
		printf("    fld_inv\n");
		printf("    %7d\n",
				stCurReg.PI_BD_CONFIG_QUAD_CH_SEL_val.param.quad_fld_inv);
		printf("    ch port path\n");
		for(i = 0; i < 4; i++)
		{
			printf("    %02d %4d %4d\n", i,
					stCurReg.PI_BD_CONFIG_QUAD_CH_SEL_val.param.quad0_ch_sel_portsel,
					stCurReg.PI_BD_CONFIG_QUAD_CH_SEL_val.param.quad0_ch_sel_pathsel);
			stCurReg.PI_BD_CONFIG_QUAD_CH_SEL_val.var >>= (4);
		}

		printf("  REC IN SEL: \n");
		stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.var = gpVIN->rec_ch_sel;
		reg = stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.var;
		printf("    ch port path\n");
		for(i = 0; i < 4; i++)
		{
			printf("    %02d %4d %4d\n", i,
					stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.rec0_ch_sel_portsel,
					stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.rec0_ch_sel_pathsel);
			stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.var >>= (4);
		}
		stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.var = reg;
		printf("    vpu:fld_sel fld_pol port path\n");
		printf("        %7d %7d %4d %4d\n",
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.vpu_fld_sel,
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.vpu_fld_pol,
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.vpu_ch_sel_portsel,
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.vpu_ch_sel_pathsel);

		printf("    canny:fld_sel fld_pol port path\n");
		printf("          %7d %7d %4d %4d\n",
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.canny_fld_sel,
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.canny_fld_pol,
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.canny_ch_sel_portsel,
				stCurReg.PI_BD_CONFIG_REC_CH_SEL_val.param.canny_ch_sel_pathsel);

		printf("  SVM TP SEL: \n");
		stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.var = gpVIN->svm_tp_sel;
		printf("    tp_sel      tp_mode\n");
		printf("    %2d/%2d/%2d/%2d %2d/%2d/%2d/%2d\n",
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm0_tp_sel,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm0_tp_mode,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm1_tp_sel,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm1_tp_mode,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm2_tp_sel,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm2_tp_mode,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm3_tp_sel,
				stCurReg.PI_BD_CONFIG_SVM_TP_SEL_val.param.svm3_tp_mode);

		printf("  VID IN CTRL: \n");
		printf("    ch en int_md vblk_md hsize sync_os raw_md fld_pol vsize\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].var = gpVIN->vid_in_ch_ctrl[i].ctrl;
			printf("    %02d %2d %6d %7d %5d %7d %6d %7d %5d\n", i,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_en,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_int_md,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_vblk_md,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_hsize,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_sync_os,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_raw_md,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_fld_pol,
					stCurReg.PI_BD_CONFIG_VID_IN_CTRL_val[i].param.vid_rd_vsize);
		}

		printf("  VID IN HTOTAL: \n");
		printf("    ch swap htotal\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_IN_HTOTAL_val[i].var = gpVIN->vid_in_ch_ctrl[i].htotal;
			printf("    %02d %4d %6d\n", i,
					stCurReg.PI_BD_CONFIG_VID_IN_HTOTAL_val[i].param.vid_rd_swap,
					stCurReg.PI_BD_CONFIG_VID_IN_HTOTAL_val[i].param.vid_rd_htotal);
		}

		printf("  VID IN VTOTAL: \n");
		printf("    ch    odd   even\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_IN_VTOTAL_val[i].var = gpVIN->vid_in_ch_ctrl[i].vtotal;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_VID_IN_VTOTAL_val[i].param.vid_rd_vtotal_odd,
					stCurReg.PI_BD_CONFIG_VID_IN_VTOTAL_val[i].param.vid_rd_vtotal_even);
		}

		printf("  VID IN FLD OFFSET: \n");
		printf("    ch    odd   even\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_IN_FLD_OS_val[i].var = gpVIN->vid_in_ch_ctrl[i].fld_os;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_VID_IN_FLD_OS_val[i].param.vid_rd_fld_os_odd,
					stCurReg.PI_BD_CONFIG_VID_IN_FLD_OS_val[i].param.vid_rd_fld_os_even);
		}

		printf("  RESYNC CTRL: \n");
		printf("    ch en int_md vblk_md hsize fld_pol vsize\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].var = gpVIN->rec_in_ch_ctrl[i].ctrl;
			printf("    %02d %2d %6d %7d %5d %7d %5d\n", i,
					stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].param.resync_rd_en,
					stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].param.resync_rd_int_md,
					stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].param.resync_rd_vblk_md,
					stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].param.resync_rd_hsize,
					stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].param.resync_rd_fld_pol,
					stCurReg.PI_BD_CONFIG_REC_IN_CTRL_val[i].param.resync_rd_vsize);
		}

		printf("  RESYNC HTOTAL: \n");
		printf("    ch htotal brightness\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_REC_IN_HTOTAL_val[i].var = gpVIN->rec_in_ch_ctrl[i].htotal;
			printf("    %02d %6d %10d\n", i,
					stCurReg.PI_BD_CONFIG_REC_IN_HTOTAL_val[i].param.resync_rd_htotal,
					stCurReg.PI_BD_CONFIG_REC_IN_HTOTAL_val[i].param.resync_brt_ctrl);
		}

		printf("  RESYNC VTOTAL: \n");
		printf("    ch    odd   even\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_REC_IN_VTOTAL_val[i].var = gpVIN->rec_in_ch_ctrl[i].vtotal;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_REC_IN_VTOTAL_val[i].param.resync_rd_vtotal_odd,
					stCurReg.PI_BD_CONFIG_REC_IN_VTOTAL_val[i].param.resync_rd_vtotal_even);
		}

		printf("  RESYNC FLD OFFSET: \n");
		printf("    ch    odd   even\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_REC_IN_FLD_OS_val[i].var = gpVIN->rec_in_ch_ctrl[i].fld_os;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_REC_IN_FLD_OS_val[i].param.resync_rd_fld_os_odd,
					stCurReg.PI_BD_CONFIG_REC_IN_FLD_OS_val[i].param.resync_rd_fld_os_even);
		}


		printf("  PAR VSYNC CTRL: \n");
		printf("    ch mode vsv_pol vsv_os fld_pol vav_end_md vav_size\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].var = gpVIN->par_sync_ch_ctrl[i].vsync;
			printf("    %02d %4d %7d %6d %7d %10d %8d\n", i,
					stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].param.par_vsync_md,
					stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].param.par_vsv_pol,
					stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].param.par_vsv_os,
					stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].param.par_fld_pol,
					stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].param.par_vav_end_md,
					stCurReg.PI_BD_CONFIG_PAR_VSYNC_CTRL_val[i].param.par_vav_size);
		}

		printf("  PAR HSYNC CTRL: \n");
		printf("    ch mode hav_pol hav_os hav_size\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_PAR_HSYNC_CTRL_val[i].var = gpVIN->par_sync_ch_ctrl[i].hsync;
			printf("    %02d %4d %7d %6d %8d\n", i,
					stCurReg.PI_BD_CONFIG_PAR_HSYNC_CTRL_val[i].param.par_hsync_md,
					stCurReg.PI_BD_CONFIG_PAR_HSYNC_CTRL_val[i].param.par_hav_pol,
					stCurReg.PI_BD_CONFIG_PAR_HSYNC_CTRL_val[i].param.par_hav_os,
					stCurReg.PI_BD_CONFIG_PAR_HSYNC_CTRL_val[i].param.par_hav_size);
		}

		printf("  QUAD CTRL: \n");
		printf("    ch ven hen tst_mem tst_vscl tst_lat tst_byp v_ofst intl vscl hlpfc hlpfy svm_tp_chg\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].var = gpVIN->quad_in_ch_ctrl[i].ctrl;
			printf("    %02d %3d %3d %7d %8d %7d %7d %6d %4d %4d %5d %5d %04d/%04d/%04d/%04d\n", i,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_vav_con_en,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_hav_con_en,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_tst_mem_sim,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_tst_vscl,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_tst_lat,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_tst_byp,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_v_ofst,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_intl_md,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_vscl_md,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_hlpf_md_c,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.quad_hlpf_md_y,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.svm_tp0_chg,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.svm_tp1_chg,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.svm_tp2_chg,
					stCurReg.PI_BD_CONFIG_QUAD_CTRL_val[i].param.svm_tp3_chg);
		}

		printf("  QUAD SCL: \n");
		printf("    ch hscl vscl\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_QUAD_HVSCL_val[i].var = gpVIN->quad_in_ch_ctrl[i].hvscl;
			printf("    %02d %04x %04x\n", i,
					stCurReg.PI_BD_CONFIG_QUAD_HVSCL_val[i].param.quad_hscl,
					stCurReg.PI_BD_CONFIG_QUAD_HVSCL_val[i].param.quad_vscl);
		}

		printf("  QUAD DELAY: \n");
		printf("    ch hdelay vdelay\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_QUAD_HVDEL_val[i].var = gpVIN->quad_in_ch_ctrl[i].hvdel;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_QUAD_HVDEL_val[i].param.quad_hdelay,
					stCurReg.PI_BD_CONFIG_QUAD_HVDEL_val[i].param.quad_vdelay);
		}

		printf("  QUAD ACTIVE: \n");
		printf("    ch hactive vactive\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_QUAD_HVACT_val[i].var = gpVIN->quad_in_ch_ctrl[i].hvact;
			printf("    %02d %7d %7d\n", i,
					stCurReg.PI_BD_CONFIG_QUAD_HVACT_val[i].param.quad_hactive,
					stCurReg.PI_BD_CONFIG_QUAD_HVACT_val[i].param.quad_vactive);
		}

		printf("  QUAD MEM LOC: \n");
		printf("    ch      mem \n");
		for(i = 0; i < 4; i++)
		{
			reg = gpVIN->quad_mem_loc[i];
			printf("    %02d %08x\n", i,
					reg);
		}

		printf("  QUAD WIN SIZE: \n");
		stCurReg.PI_BD_CONFIG_QUAD_WIN_SIZE_val.var = gpVIN->quad_win_size;
		printf("    wr_page hsize(x8)   vsize(x1)\n");
		printf("    %7d %5d(%4d) %5d\n",
				stCurReg.PI_BD_CONFIG_QUAD_WIN_SIZE_val.param.quad_wr_page,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_SIZE_val.param.quad_win_hsize,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_SIZE_val.param.quad_win_hsize*8,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_SIZE_val.param.quad_win_vsize);

		printf("  QUAD WIN WR: \n");
		stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.var = gpVIN->quad_win_wr_en;
		printf("    en int_md period i2p_en rdelay wdelay\n");
		printf("    %2x %6d %6d %6d %6d %6d\n",
				stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.param.quad_wr_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.param.quad_wr_int_md,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.param.quad_wr_period,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.param.quad_wr_i2p_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.param.quad_ddr_rd_delay,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_WR_EN_val.param.quad_ddr_wr_delay);

		printf("  QUAD WIN RD: \n");
		stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.var = gpVIN->quad_win_rd_en;
		printf("    en int_md p2i_en p2i_fld win_en win_blk_en win_bnd_en bnd_size blk_col bgd_col bnd_col\n");
		printf("    %2d %6d %6d %7d %6d %10d %10d %8d %7d %7d %7d\n",
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_rd_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_rd_int_md,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_rd_p2i_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_rd_p2i_fld,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_win_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_win_blk_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_win_bnd_en,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_win_bnd_size,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_blk_col,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_bgd_col,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_RD_EN_val.param.quad_bnd_col);

		printf("  QUAD WIN HVOS: \n");
		stCurReg.PI_BD_CONFIG_QUAD_WIN_HVOS_val.var = gpVIN->quad_win_hvos;
		printf("    hoffset voffset\n");
		printf("    %7d %7d\n",
				stCurReg.PI_BD_CONFIG_QUAD_WIN_HVOS_val.param.quad_win_hos,
				stCurReg.PI_BD_CONFIG_QUAD_WIN_HVOS_val.param.quad_win_vos);

		printf("  QUAD RD HVSIZE: \n");
		stCurReg.PI_BD_CONFIG_QUAD_RD_HVSIZE_val.var = gpVIN->quad_rd_hvsize;
		printf("    hsize vsize\n");
		printf("    %5d %5d\n",
				stCurReg.PI_BD_CONFIG_QUAD_RD_HVSIZE_val.param.quad_rd_hsize,
				stCurReg.PI_BD_CONFIG_QUAD_RD_HVSIZE_val.param.quad_rd_vsize);

		printf("  QUAD RD HTOTAL: \n");
		stCurReg.PI_BD_CONFIG_QUAD_RD_HTOTAL_val.var = gpVIN->quad_rd_htotal;
		printf("    htotal\n");
		printf("    %6d\n",
				stCurReg.PI_BD_CONFIG_QUAD_RD_HTOTAL_val.param.quad_rd_htotal);

		printf("  QUAD RD VTOTAL: \n");
		stCurReg.PI_BD_CONFIG_QUAD_RD_VTOTAL_val.var = gpVIN->quad_rd_vtotal;
		printf("       odd   even\n");
		printf("    %6d %6d\n",
				stCurReg.PI_BD_CONFIG_QUAD_RD_VTOTAL_val.param.quad_rd_vtotal_odd,
				stCurReg.PI_BD_CONFIG_QUAD_RD_VTOTAL_val.param.quad_rd_vtotal_even);

		printf("  QUAD RD FLD_OS: \n");
		stCurReg.PI_BD_CONFIG_QUAD_RD_FLD_OS_val.var = gpVIN->quad_rd_fld_os;
		printf("       odd   even\n");
		printf("    %6d %6d\n",
				stCurReg.PI_BD_CONFIG_QUAD_RD_FLD_OS_val.param.quad_rd_fld_os_odd,
				stCurReg.PI_BD_CONFIG_QUAD_RD_FLD_OS_val.param.quad_rd_fld_os_even);

		printf("  QUAD OUT FMT: \n");
		stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.var = gpVIN->quad_out_fmt;
		printf("    pvi - async_en asyncchg_en outlim_en outyc_inv outfld_inv outbt656_md\n");
		printf("          %8d %11d %9d %9d %10d %11d\n",
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_pvi_async_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_pvi_async_chg_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_pvi_out_lim_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_pvi_out_yc_inv,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_pvi_out_fld_inv,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_pvi_out_bt656_md);
		printf("    rec_async_en rec_asyncchg_en vpu_fld_pol vpu_fld_sel\n");
		printf("    %12d %15d %11d %11d\n",
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_rec_async_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_rec_async_chg_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_vpu_fld_pol,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_vpu_fld_sel);
		printf("    out - async_en asyncchg_en validb 16bit_md lim_en yc_inv fld_inv bt656_md clk1 clk0\n");
		printf("          %8d %11d %6d %8d %6d %6d %7d %8d %4d %4d\n",
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_async_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_async_chg_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_validb,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_16bit_md,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_lim_en,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_yc_inv,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_fld_inv,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_bt656_md,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_clk1_md,
				stCurReg.PI_BD_CONFIG_QUAD_OUT_FMT_val.param.quad_out_clk0_md);

		printf("  QUAD FULL MD: \n");
		stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.var = gpVIN->quad_full_md;
		printf("    quadfull single busy strobe wrp3 wrp2 wrp1 wrp0\n");
		printf("    %8d %6x %4x %6x %4x %4x %4x %4x\n",
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad_full_md,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad_single_md,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad_wr_busy_det,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad_wr_strobe,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad3_wr_page,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad2_wr_page,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad1_wr_page,
				stCurReg.PI_BD_CONFIG_QUAD_FULL_MD_val.param.quad0_wr_page);

		printf("  REC FMT CTRL: \n");
		stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.var = gpVIN->rec_fmt_ctrl;
		printf("    sd_p2i_en sd_960h_md sd_pal ch_mux_md bt1120_lim_rec rec_async_en rec_asyncchg_en rec_chid_en\n");
		printf("    %9d %10d %6d %9d %14d %12d %15d %11d\n",
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.sd_p2i_en,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.sd_960h_md,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.sd_pal_ntb,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.bt1120_lim_rec,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.rec_async_en,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.rec_async_chg_en,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.rec_chid_en);
		printf("    out - fld_pol 16bit bt656 yc_inv rate\n");
		printf("          %7d %5d %5d %6d %4d\n",
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_fld_pol,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_16bit,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_bt656,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_yc_inv,
				stCurReg.PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate);

		printf("  REC CH0 SEL: \n");
		stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.var = gpVIN->rec0_ch_sel;
		printf("    clk_phase chid0 chid1 chid2 chid3 sel_a sel_b sel_c sel_d\n");
		printf("    %9d %5d %5d %5d %5d %5d %5d %5d %5d\n",
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.chid_num_0,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.chid_num_1,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.chid_num_2,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.chid_num_3,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.ch_sel_a_0,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.ch_sel_b_0,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.ch_sel_c_0,
				stCurReg.PI_BD_CONFIG_REC0_CH_SEL_val.param.ch_sel_d_0);

		printf("  REC CH1 SEL: \n");
		stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.var = gpVIN->rec1_ch_sel;
		printf("    clk_phase chid0 chid1 chid2 chid3 sel_a sel_b sel_c sel_d\n");
		printf("    %9d %5d %5d %5d %5d %5d %5d %5d %5d\n",
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.chid_num_0,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.chid_num_1,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.chid_num_2,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.chid_num_3,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.ch_sel_a_0,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.ch_sel_b_0,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.ch_sel_c_0,
				stCurReg.PI_BD_CONFIG_REC1_CH_SEL_val.param.ch_sel_d_0);

		printf("  PAR PB FMT: \n");
		stCurReg.PI_BD_CONFIG_PAR_PB_FMT_val.var = gpVIN->par_pb_fmt;
		printf("    16/24bit format\n");
		printf("          %2d %6d\n",
				stCurReg.PI_BD_CONFIG_PAR_PB_FMT_val.param.par_pb_input_mode,
				stCurReg.PI_BD_CONFIG_PAR_PB_FMT_val.param.par_pb_fmt);

		printf("  PB YUV COEF: \n");
		printf("    ch ycoef ucoef vcoef\n");
		for(i = 0; i < 4; i++)
		{
			stCurReg.PI_BD_CONFIG_PAR_PB_YCOEF_val[i].var = gpVIN->par_pb_ycoef[i];
			stCurReg.PI_BD_CONFIG_PAR_PB_UCOEF_val[i].var = gpVIN->par_pb_ucoef[i];
			stCurReg.PI_BD_CONFIG_PAR_PB_VCOEF_val[i].var = gpVIN->par_pb_vcoef[i];
			printf("    %02d %05x %05x %05x\n", i,
					stCurReg.PI_BD_CONFIG_PAR_PB_YCOEF_val[i].param.par_pb_ycoef,
					stCurReg.PI_BD_CONFIG_PAR_PB_UCOEF_val[i].param.par_pb_ucoef,
					stCurReg.PI_BD_CONFIG_PAR_PB_VCOEF_val[i].param.par_pb_vcoef);
		}

		printf("  GENLOCK CTRL: \n");
		stCurReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.var = gpVIN->genlock_low_width;
		stCurReg.PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val.var = gpVIN->genlock_high_width;
		printf("    en pluse_en pulse_pol low_save_sel low_width high_save_pol high_save_sel high_width\n");
		printf("    %2d %8d %9d %12d %9d %13d %13d %10d\n",
				stCurReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.param.genlock_en,
				stCurReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.param.genlock_pulse_en,
				stCurReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.param.genlock_pulse_pol,
				stCurReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.param.genlock_save_sel,
				stCurReg.PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val.param.genlock_low_width,
				stCurReg.PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val.param.genlock_save_pol,
				stCurReg.PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val.param.genlock_save_sel,
				stCurReg.PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val.param.genlock_high_width);

		printf("  GENLOCK START OS CONFIG: \n");
		stCurReg.PI_BD_CONFIG_GENLOCK_START_OS_val.var = gpVIN->genlock_strt_os;
		printf("    chk_en ref_sel start_os\n");
		printf("    %6d %7d %8d\n",
				stCurReg.PI_BD_CONFIG_GENLOCK_START_OS_val.param.genlock_chk_en,
				stCurReg.PI_BD_CONFIG_GENLOCK_START_OS_val.param.genlock_ref_sel,
				stCurReg.PI_BD_CONFIG_GENLOCK_START_OS_val.param.genlock_start_os);

		printf("  GENLOCK END OS CONFIG: \n");
		stCurReg.PI_BD_CONFIG_GENLOCK_END_OS_val.var = gpVIN->genlock_end_os;
		printf("    chk_pol ref_pol fld_sel end_os\n");
		printf("    %7d %7d %7d %6d\n",
				stCurReg.PI_BD_CONFIG_GENLOCK_END_OS_val.param.genlock_chk_pol,
				stCurReg.PI_BD_CONFIG_GENLOCK_END_OS_val.param.genlock_ref_pol,
				stCurReg.PI_BD_CONFIG_GENLOCK_END_OS_val.param.genlock_fld_sel,
				stCurReg.PI_BD_CONFIG_GENLOCK_END_OS_val.param.genlock_end_os);

		printf("  PB IN CTRL: \n");
		stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.var = gpVIN->pbvid_in_ch_ctrl.ctrl;
		printf("    en int_md vblk_md hsize sync_os raw_md fld_pol vsize\n");
		printf("    %2d %6d %7d %5d %7d %6d %7d %5d\n",
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_en,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_int_md,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_vblk_md,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_hsize,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_sync_os,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_raw_md,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_fld_pol,
				stCurReg.PI_BD_CONFIG_PB_IN_CTRL_val.param.vid_rd_vsize);

		printf("  PB IN HTOTAL: \n");
		stCurReg.PI_BD_CONFIG_PB_IN_HTOTAL_val.var = gpVIN->pbvid_in_ch_ctrl.htotal;
		printf("    swap htotal\n");
		printf("    %4d %6d\n",
				stCurReg.PI_BD_CONFIG_PB_IN_HTOTAL_val.param.vid_rd_swap,
				stCurReg.PI_BD_CONFIG_PB_IN_HTOTAL_val.param.vid_rd_htotal);

		printf("  PB IN VTOTAL: \n");
		stCurReg.PI_BD_CONFIG_PB_IN_VTOTAL_val.var = gpVIN->pbvid_in_ch_ctrl.vtotal;
		printf("       odd   even\n");
		printf("    %6d %6d\n",
				stCurReg.PI_BD_CONFIG_PB_IN_VTOTAL_val.param.vid_rd_vtotal_odd,
				stCurReg.PI_BD_CONFIG_PB_IN_VTOTAL_val.param.vid_rd_vtotal_even);

		printf("  PB IN FLD OFFSET: \n");
		stCurReg.PI_BD_CONFIG_PB_IN_FLD_OS_val.var = gpVIN->pbvid_in_ch_ctrl.fld_os;
		printf("       odd   even\n");
		printf("    %6d %6d\n",
				stCurReg.PI_BD_CONFIG_PB_IN_FLD_OS_val.param.vid_rd_fld_os_odd,
				stCurReg.PI_BD_CONFIG_PB_IN_FLD_OS_val.param.vid_rd_fld_os_even);

		printf("  PBREC CTRL: \n");
		stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.var = gpVIN->pbrec_in_ch_ctrl.ctrl;
		printf("    en int_md vblk_md hsize fld_pol vsize\n");
		printf("    %2d %6d %7d %5d %7d %5d\n",
				stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.param.resync_rd_en,
				stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.param.resync_rd_int_md,
				stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.param.resync_rd_vblk_md,
				stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.param.resync_rd_hsize,
				stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.param.resync_rd_fld_pol,
				stCurReg.PI_BD_CONFIG_PBREC_IN_CTRL_val.param.resync_rd_vsize);

		printf("  PBREC HTOTAL: \n");
		stCurReg.PI_BD_CONFIG_PBREC_IN_HTOTAL_val.var = gpVIN->pbrec_in_ch_ctrl.htotal;
		printf("    htotal brightness\n");
		printf("    %6d %10d\n",
				stCurReg.PI_BD_CONFIG_PBREC_IN_HTOTAL_val.param.resync_rd_htotal,
				stCurReg.PI_BD_CONFIG_PBREC_IN_HTOTAL_val.param.resync_brt_ctrl);

		printf("  PBREC VTOTAL: \n");
		stCurReg.PI_BD_CONFIG_PBREC_IN_VTOTAL_val.var = gpVIN->pbrec_in_ch_ctrl.vtotal;
		printf("       odd   even\n");
		printf("    %6d %6d\n",
				stCurReg.PI_BD_CONFIG_PBREC_IN_VTOTAL_val.param.resync_rd_vtotal_odd,
				stCurReg.PI_BD_CONFIG_PBREC_IN_VTOTAL_val.param.resync_rd_vtotal_even);

		printf("  PBREC FLD OFFSET: \n");
		stCurReg.PI_BD_CONFIG_PBREC_IN_FLD_OS_val.var = gpVIN->pbrec_in_ch_ctrl.fld_os;
		printf("       odd   even\n");
		printf("    %6d %6d\n",
				stCurReg.PI_BD_CONFIG_PBREC_IN_FLD_OS_val.param.resync_rd_fld_os_odd,
				stCurReg.PI_BD_CONFIG_PBREC_IN_FLD_OS_val.param.resync_rd_fld_os_even);

		printf("  VID CRC CTRL: \n");
		printf("    ch bist_en roi_en roi_md int_md fld_sel fr_period ref_data\n");
		for(i = 0; i < 6; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].var = gpVIN->crc_ctrl[i].ctrl;
			printf("    %02d %7d %6d %6d %6d %7d %9d %08x\n", i,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_bist_en,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_roi_en,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_roi_md,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_int_md,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_fld_sel,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_fr_period,
					stCurReg.PI_BD_CONFIG_VID_CRC_CTRL_val[i].param.vid_crc_ref_data);
		}

		printf("  VID CRC START: \n");
		printf("    ch hstart vstart\n");
		for(i = 0; i < 6; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_CRC_STRT_val[i].var = gpVIN->crc_ctrl[i].strt;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_VID_CRC_STRT_val[i].param.vid_crc_hstrt,
					stCurReg.PI_BD_CONFIG_VID_CRC_STRT_val[i].param.vid_crc_vstrt);
		}

		printf("  VID CRC END: \n");
		printf("    ch   hend   vend\n");
		for(i = 0; i < 6; i++)
		{
			stCurReg.PI_BD_CONFIG_VID_CRC_END_val[i].var = gpVIN->crc_ctrl[i].end;
			printf("    %02d %6d %6d\n", i,
					stCurReg.PI_BD_CONFIG_VID_CRC_END_val[i].param.vid_crc_hend,
					stCurReg.PI_BD_CONFIG_VID_CRC_END_val[i].param.vid_crc_vend);
		}

		printf("  QUAD CRC CTRL: \n");
		stCurReg.PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val.var = gpVIN->quad_ddr_crc_ctrl;
		printf("    wo_en ro_en ro_mode\n");
		printf("    %5d %5d %7d\n",
				stCurReg.PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val.param.quad_cc_ddrwo_en,
				stCurReg.PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val.param.quad_cc_ddrro_en,
				stCurReg.PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val.param.quad_crc_ddrro_mode);

		printf("  VIN INFO HSIZE: \n");
		printf("    ch format_det video_loss_det synclock[vact, hact, fsize, hsize] hsize\n");
		for(i = 0; i < 5; i++)
		{
			stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].var = gpVIN->sync_in_hvsize_ch_info[i].hsize;
			printf("    %02d %10d %14d %13d  %4d  %5d  %5d  %5d\n", i,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.format_det,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.video_loss_det,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.sync_lock_vactive,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.sync_lock_hactive,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.sync_lock_fsize,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.sync_lock_hsize,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[i].param.hsize);
		}

		printf("  VIN INFO FSIZE: \n");
		printf("    ch fsize\n");
		for(i = 0; i < 5; i++)
		{
			stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_FSIZE_val[i].var = gpVIN->sync_in_hvsize_ch_info[i].fsize;
			printf("    %02d %5d\n", i,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_FSIZE_val[i].param.fsize);
		}

		printf("  VIN INFO HVACT: \n");
		printf("    ch hactive vactive\n");
		for(i = 0; i < 5; i++)
		{
			stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HVACT_val[i].var = gpVIN->sync_in_hvsize_ch_info[i].hvact;
			printf("    %02d %7d %7d\n", i,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HVACT_val[i].param.hact,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_INFO_HVACT_val[i].param.vact);
		}

		printf("  SYNC VIN HSIZE: \n");
		printf("    ch vloss_det_en sync_int_md sync_det_th format_det_en sync_hsize\n");
		for(i = 0; i < 5; i++)
		{
			stCurReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].var = gpVIN->sync_vin_hvsize_ch[i].hsize;
			printf("    %02d %12d %11d %11d %13x %10d\n", i,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].param.vloss_det_en,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].param.sync_int_md,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].param.sync_det_th,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].param.fomat_det_en,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HSIZE_val[i].param.sync_hsize);
		}

		printf("  SYNC VIN FSIZE: \n");
		printf("    ch hvact_md    fsize\n");
		for(i = 0; i < 5; i++)
		{
			stCurReg.PI_BD_CONFIG_SYNC_VIN_FSIZE_val[i].var = gpVIN->sync_vin_hvsize_ch[i].fsize;
			printf("    %02d %8d %8d\n", i,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_FSIZE_val[i].param.sync_hvact_md,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_FSIZE_val[i].param.sync_fsize);
		}

		printf("  SYNC VIN HVACT: \n");
		printf("    ch hactive vactive\n");
		for(i = 0; i < 5; i++)
		{
			stCurReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].var = gpVIN->sync_vin_hvsize_ch[i].hvact;
			printf("    %02d %7d %7d\n", i,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].param.sync_hact,
					stCurReg.PI_BD_CONFIG_SYNC_VIN_HVACT_val[i].param.sync_vact);
		}

	}/*}}}*/

	return(eSUCCESS);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



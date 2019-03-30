/*
 * api_vin.c
 *
 *  Created on: 2017. 4. 18.
 *      Author: ihkong
 */

#include <stdio.h>
#include <unistd.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "prj_config_scenario.h"
#include "debug.h"
#include "system.h"
#include "interrupt.h"
#include "proc.h"

#include "vin.h"
#include "api_vin.h"
#include "vin_user_config.h"
#include "video_config.h"

#include "api_vpu.h"
#include "api_ipc.h"
#include "sys_api.h"

#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI) )
#include "api_pvirx_func.h"
#endif //( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI) )

static VIN_REG_T *gpVIN = VIN_CTRL_REG;

//PROC device
extern PP_RESULT_E VIN_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stVIN_PROC_devices[] = {
	    { .pName = "VIN",   .fn = VIN_PROC_DEVICE,    .next = (void*)0, },
};

PP_VOID PPAPI_VIN_Initialize(PP_VOID)
{
	PPDRV_VIN_Initialize(BD_VIN_FMT, BD_DU_OUT_FMT, BD_QUAD_OUT_FMT);

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) 
    {
        uint32 i;

        for(i=0;i<4;i++){
            PPDRV_VIN_set_par_sync_type(vinVidInPort0 + i, vinParSync_Ext);
            PPDRV_VIN_set_par_bit_mapping(vinVidInPort0 + i, vinParBitMapping_MSB);
        }
        PPDRV_VIN_set_par_chmux(0, 0);
        PPDRV_VIN_set_par_chmux(1, 0);
    }
#elif (VIDEO_IN_TYPE_PB_SUPPORT == 1) //Video In playback(PB)
    {
        uint32 i;

        for(i=0;i<4;i++){
            PPDRV_VIN_set_par_sync_type(vinVidInPort0 + i, vinParSync_Emb);
            PPDRV_VIN_set_par_bit_mapping(vinVidInPort0 + i, vinParBitMapping_MSB);
        }

        PPDRV_VIN_set_par_chmux(0, 0);
        PPDRV_VIN_set_par_chmux(1, 0);

#if ( ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_RGB24) )
        PPDRV_VIN_set_pb_fmt(vinPBByteSwap_RGB, vinParBitMapping_MSB, vinPBInType_RGB_YUV_24Bit, 1, vinParSync_Ext);
#elif ( ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YUV24) )
        PPDRV_VIN_set_pb_fmt(vinPBByteSwap_RGB, vinParBitMapping_MSB, vinPBInType_RGB_YUV_24Bit, 0, vinParSync_Ext);
#else
        PPDRV_VIN_set_pb_fmt(vinPBByteSwap_8x8, vinParBitMapping_LSB, vinPBInType_YC_16Bit, 0, vinParSync_Emb);
#endif
    }
#endif

	//register proc infomation.
	SYS_PROC_addDevice(stVIN_PROC_devices);

}

PP_VOID PPAPI_VIN_SetGenlockParam(PP_VOID)
{
	uint32 reg, highTH, lowTH;
	int count = 100;

	if( ((GetRegValue(0xF0F00220)>>24)&0x7) == 0) //genlock reference channel 0 ?
	{
		LOG_DEBUG("Set Genlock reference param channel 0.\n");
		/* check hsize lock det */
		do
		{
			reg = GetRegValue(0xF0F00400);
			//LOG_DEBUG("ch0 0xf0f00400:0x%08x\n", reg);
		}while(((reg&0x01000000) != 0x01000000) && (count--));
		if(count <= 0) return;

		/* get genlock info */
		reg = GetRegValue(0xF0F00210 + 0x0);
		//LOG_DEBUG("ch0 0xf0f00210:0x%08x\n", reg);

		/* set limit low/high TH */
		highTH = reg + (reg/10);
		lowTH = reg - (reg/10);

		/* set genlock start(low) offset */
		reg = GetRegValue(0xF0F00220);
		reg &= ~0x00FFFFFF;
		reg |= (lowTH & 0x00FFFFFF);
		SetRegValue(0xF0F00220, reg);
		//LOG_DEBUG("ch0 startoffset:0x%08x\n", reg);

		/* set genlock end(high) offset */
		reg = GetRegValue(0xF0F00224);
		reg &= ~0x00FFFFFF;
		reg |= (highTH & 0x00FFFFFF);
		SetRegValue(0xF0F00224, reg);
		//LOG_DEBUG("ch0 endoffset:0x%08x\n", reg);
	}
	else if( ((GetRegValue(0xF0F00220)>>24)&0x7) == 1) //genlock reference channel 1 ?
	{
		LOG_DEBUG("Set Genlock reference param channel 1.\n");
		/* check hsize lock det */
		do{
			reg = GetRegValue(0xF0F0040C);
			//LOG_DEBUG("ch1 0xf0f0040C:0x%08x\n", reg);
		}while(((reg&0x01000000) != 0x01000000) && (count--));
		if(count <= 0) return;

		/* get genlock info */
		reg = GetRegValue(0xF0F00210 + 0x4);
		//LOG_DEBUG("ch1 0xf0f00214:0x%08x\n", reg);

		/* set limit low/high TH */
		highTH = reg + (reg/10);
		lowTH = reg - (reg/10);

		/* set genlock start(low) offset */
		reg = GetRegValue(0xF0F00220);
		reg &= ~0x00FFFFFF;
		reg |= (lowTH & 0x00FFFFFF);
		SetRegValue(0xF0F00220, reg);
		//LOG_DEBUG("ch1 startoffset:0x%08x\n", reg);

		/* set genlock end(high) offset */
		reg = GetRegValue(0xF0F00224);
		reg &= ~0x00FFFFFF;
		reg |= (highTH & 0x00FFFFFF);
		SetRegValue(0xF0F00224, reg);
		//LOG_DEBUG("ch1 endoffset:0x%08x\n", reg);
	}
	else if( ((GetRegValue(0xF0F00220)>>24)&0x7) == 2) //genlock reference channel 2 ?
	{
		LOG_DEBUG("Set Genlock reference param channel 2.\n");
		/* check hsize lock det */
		do{
			reg = GetRegValue(0xF0F00418);
			//LOG_DEBUG("ch2 0xf0f00418:0x%08x\n", reg);
		}while(((reg&0x01000000) != 0x01000000) && (count--));
		if(count <= 0) return;

		/* get genlock info */
		reg = GetRegValue(0xF0F00210 + 0x8);
		//LOG_DEBUG("ch2 0xf0f00218:0x%08x\n", reg);

		/* set limit low/high TH */
		highTH = reg + (reg/10);
		lowTH = reg - (reg/10);

		/* set genlock start(low) offset */
		reg = GetRegValue(0xF0F00220);
		reg &= ~0x00FFFFFF;
		reg |= (lowTH & 0x00FFFFFF);
		SetRegValue(0xF0F00220, reg);
		//LOG_DEBUG("ch2 startoffset:0x%08x\n", reg);

		/* set genlock end(high) offset */
		reg = GetRegValue(0xF0F00224);
		reg &= ~0x00FFFFFF;
		reg |= (highTH & 0x00FFFFFF);
		SetRegValue(0xF0F00224, reg);
		//LOG_DEBUG("ch2 endoffset:0x%08x\n", reg);
	}
	else if( ((GetRegValue(0xF0F00220)>>24)&0x7) == 3) //genlock reference channel 3 ?
	{
		LOG_DEBUG("Set Genlock reference param channel 3.\n");
		/* check hsize lock det */
		do{
			reg = GetRegValue(0xF0F00424);
			//LOG_DEBUG("ch3 0xf0f00424:0x%08x\n", reg);
		}while(((reg&0x01000000) != 0x01000000) && (count--));
		if(count <= 0) return;

		/* get genlock info */
		reg = GetRegValue(0xF0F00210 + 0xC);
		//LOG_DEBUG("ch3 0xf0f0021C:0x%08x\n", reg);

		/* set limit low/high TH */
		highTH = reg + (reg/10);
		lowTH = reg - (reg/10);

		/* set genlock start(low) offset */
		reg = GetRegValue(0xF0F00220);
		reg &= ~0x00FFFFFF;
		reg |= (lowTH & 0x00FFFFFF);
		SetRegValue(0xF0F00220, reg);
		//LOG_DEBUG("ch3 startoffset:0x%08x\n", reg);

		/* set genlock end(high) offset */
		reg = GetRegValue(0xF0F00224);
		reg &= ~0x00FFFFFF;
		reg |= (highTH & 0x00FFFFFF);
		SetRegValue(0xF0F00224, reg);
		//LOG_DEBUG("ch3 endoffset:0x%08x\n", reg);
	}

}

PP_RESULT_E PPAPI_VIN_GetResol(const PP_S32 defVideoFmt, PP_S32 *pRetWidth, PP_S32 *pRetHeight, _VID_RESOL *peRetResol)
{
    PP_RESULT_E ret;

    ret = PPDRV_VIN_GetResol(defVideoFmt, pRetWidth, pRetHeight, peRetResol);

    return(ret);
}

/* (bQuadView == 1) ? quad : full */
PP_RESULT_E PPAPI_VIN_SetQuadViewMode(const PP_S32 defVideoFmt, const PP_U8 bQuadView, const PP_S32 chSel, const PP_S32 pathSel)
{
//	int i;
	VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
	VIN_QUAD_WIN_WR_EN_CONFIG_U regQuadWinWrEn;
	VIN_QUAD_WIN_RD_EN_CONFIG_U regQuadWinRdEn;
	VIN_QUAD_CH_SEL_CONFIG_U regQuadChSel;
	int inWidth = 0, inHeight = 0;
	_VID_RESOL eVidResol = 0;
//	uint32 u32WaitDelay;

    regQuadWinWrEn.var = gpVIN->quad_win_wr_en;
    regQuadWinWrEn.param.quad_wr_en = 0; //disable wren
    gpVIN->quad_win_wr_en = regQuadWinWrEn.var;

    regQuadWinRdEn.var = gpVIN->quad_win_rd_en;
    regQuadWinRdEn.param.quad_rd_en = 0; //disble rden
    gpVIN->quad_win_rd_en = regQuadWinRdEn.var;

    if(bQuadView)
    {
        int defVinFmt = BD_VIN_FMT;
        int defVoutFmt = BD_DU_OUT_FMT;
        int defQuadOutFmt = BD_QUAD_OUT_FMT;

        _VIN_PATH vinPath;
        _VIN_USER_CONFIG_REG vinUserReg;

        uint32 reg;
        sint32 i;
//        uint8 vinCh;

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

        if( PPAPI_VIN_GetResol(defVinFmt, &vinWidth, &vinHeight, &eVidResol) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        if( PPAPI_VIN_GetResol(defVoutFmt, &voutWidth, &voutHeight, &eVoutResol) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        if( PPAPI_VIN_GetResol(defQuadOutFmt, &quadOutWidth, &quadOutHeight, &eQuadOutResol) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
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


        gpVIN->quad_in_sel = vinUserReg.PI_BD_CONFIG_QUAD_CH_SEL_val.var;

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

        regQuadWinWrEn.param.quad_wr_en = 0xF; //enable wren
        gpVIN->quad_win_wr_en = regQuadWinWrEn.var;

        gpVIN->quad_full_md = vinUserReg.PI_BD_CONFIG_QUAD_FULL_MD_val.var;

        regQuadWinRdEn.param.quad_rd_en = 1; //enable rden
        regQuadWinRdEn.param.quad_win_en = 0xF; 
        gpVIN->quad_win_rd_en = regQuadWinRdEn.var;
    }
    else //Full mode
    {
        /* set ch0 to full image capture mode */
        regQuadChSel.var = gpVIN->quad_in_sel;
        regQuadChSel.param.quad0_ch_sel_portsel = chSel&0x3;
        regQuadChSel.param.quad0_ch_sel_pathsel = pathSel&0x3;
        gpVIN->quad_in_sel = regQuadChSel.var;

        gpVIN->quad_in_ch_ctrl[0].hvscl = 0xFFFFFFFF; //full scale

#if 1
        if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        gpVIN->quad_win_size = (3<<28) | (((DEF_VIN_QUAD_HV_WIN_SIZE[eVidResol][0])&0x7F)<<19) | ((DEF_VIN_QUAD_HV_WIN_SIZE[eVidResol][1])&0x3FF);
#endif

        regQuadWinWrEn.param.quad_wr_en = (1<<0); //enable wren
        gpVIN->quad_win_wr_en = regQuadWinWrEn.var;

        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 1;
        regQuadFullMd.param.quad_single_md = 0; //0:live
        regQuadFullMd.param.quad_wr_strobe = (1<<0);
        gpVIN->quad_full_md = regQuadFullMd.var;

        regQuadWinRdEn.param.quad_rd_en = 1; //enable rden
        regQuadWinRdEn.param.quad_win_en = (1<<0); //ch0 full only
        gpVIN->quad_win_rd_en = regQuadWinRdEn.var;
    }

    return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_SetCaptureMode(const PP_S32 defVideoFmt, const PP_S32 chSel, const PP_S32 pathSel)
{
//	int i;
	VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
	VIN_QUAD_WIN_WR_EN_CONFIG_U regQuadWinWrEn;
	VIN_QUAD_CH_SEL_CONFIG_U regQuadChSel;
	int inWidth = 0, inHeight = 0;
	_VID_RESOL eVidResol = 0;
	uint32 u32WaitDelay;
	int captureTryCnt = 2;

	regQuadWinWrEn.var = gpVIN->quad_win_wr_en;
	regQuadWinWrEn.param.quad_wr_en = 0; //disable wren
	regQuadWinWrEn.param.quad_wr_i2p_en = 0; //normal mode.
	gpVIN->quad_win_wr_en = regQuadWinWrEn.var;
	gpVIN->quad_win_rd_en = 0; //disable rden

	/* set ch0 to full image capture mode */
	regQuadChSel.var = gpVIN->quad_in_sel;
	regQuadChSel.param.quad0_ch_sel_portsel = chSel&0x3;
	regQuadChSel.param.quad0_ch_sel_pathsel = pathSel&0x3;
	gpVIN->quad_in_sel = regQuadChSel.var;

	gpVIN->quad_in_ch_ctrl[0].hvscl = 0xFFFFFFFF; //full scale
	gpVIN->quad_in_ch_ctrl[1].hvscl = 0xFFFFFFFF; //full scale
	gpVIN->quad_in_ch_ctrl[2].hvscl = 0xFFFFFFFF; //full scale
	gpVIN->quad_in_ch_ctrl[3].hvscl = 0xFFFFFFFF; //full scale

	#if 1
	if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }
	gpVIN->quad_win_size = (3<<28) | (((DEF_VIN_QUAD_HV_WIN_SIZE[eVidResol][0])&0x7F)<<19) | ((DEF_VIN_QUAD_HV_WIN_SIZE[eVidResol][1])&0x3FF);
	#endif

    while(captureTryCnt--)
    {
        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 1;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = 0xF;
        gpVIN->quad_full_md = regQuadFullMd.var;

        regQuadWinWrEn.param.quad_wr_en = (1<<0); //enable wren
        gpVIN->quad_win_wr_en = regQuadWinWrEn.var;

        /* wait 1frame delay */
        u32WaitDelay = 40000*50;
        do
        {
            asm volatile("nop");
        } while( (u32WaitDelay-- > 0) );

        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 1;
        regQuadFullMd.param.quad_single_md = (1<<0);
        regQuadFullMd.param.quad_wr_strobe = (1<<0);
        gpVIN->quad_full_md = regQuadFullMd.var;

        /* wait capture done */
        u32WaitDelay = 0x3FFFFFF;
        do
        {
            asm volatile("nop");
            regQuadFullMd.var = gpVIN->quad_full_md;
        } while( (u32WaitDelay-- > 0) && ((regQuadFullMd.param.quad_wr_busy_det & (1<<0)) || (regQuadFullMd.param.quad_wr_strobe & (1<<0))) );

        if(u32WaitDelay == 0) 
        {
            LOG_DEBUG("TimeOut\n");
            return(eERROR_TIMEOUT);
        }
    }

	return(eSUCCESS);
}


PP_RESULT_E PPAPI_VIN_GetCaptureImage(const PP_S32 defVideoFmt, const PP_S32 bYOnly, PP_U32 u32BufPAddr, PP_U32 *pRetBufSize, PP_QUAD_CH_E chan, PP_BOOL bFull)
{
	int inWidth = 0, inHeight = 0;
	_VID_RESOL eVidResol = 0;
	uint32 captureSize;
	uint32 baseBufAddr;
	uint32 baseBufAddrOdd = 0, baseBufAddrEven = 0;
	uint32 u32WaitDelay;
	uint8 u8WrPage;
	VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
    int bI2P = 0;

	if (chan > 3)
		chan = 0;

	if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

	if(bFull) //capture full size
	{
		baseBufAddr = gpVIN->quad_mem_loc[0];

		#ifndef BD_SLT
		LOG_DEBUG("Capture(%dx%d)-%s\n", inWidth, inHeight, (bYOnly)?"Yonly":"UYVY");
		#endif
		//make forcly width 32Byte align */
		inWidth <<= 2; 
		inHeight >>= 2;
		captureSize = inWidth * inHeight * 2;

		gpVIN->quad_in_ch_ctrl[chan].hvscl = 0xFFFFFFFF; //full scale

		regQuadFullMd.var = 0;
		regQuadFullMd.param.quad_full_md = 1;
		regQuadFullMd.param.quad_single_md = (1<<chan);
		regQuadFullMd.param.quad_wr_strobe = (1<<chan);
		gpVIN->quad_full_md = regQuadFullMd.var;
	}
#if 1
	else //capture quad image
	{
		baseBufAddr = gpVIN->quad_mem_loc[chan];

		/* quad size */
		inWidth >>= 1; // 1/2 
		inHeight >>= 1; // 1/2
		#ifndef BD_SLT
		LOG_DEBUG("Capture(%dx%d)-%s\n", inWidth, inHeight, (bYOnly)?"Yonly":"UYVY");
		#endif
		//make forcly width 32Byte align */
		inWidth <<= 2; 
		inHeight >>= 2;
		captureSize = (inWidth * inHeight * 2);

		regQuadFullMd.var = 0;
		regQuadFullMd.param.quad_full_md = 0;
		regQuadFullMd.param.quad_single_md = (1<<chan);
		regQuadFullMd.param.quad_wr_strobe = (1<<chan);
		gpVIN->quad_full_md = regQuadFullMd.var;
	}
#endif

	/* wait capture done */
	u32WaitDelay = 0x3FFFFFF;
	do
	{
		asm volatile("nop");
		regQuadFullMd.var = gpVIN->quad_full_md;
	} while( (u32WaitDelay-- > 0) && ((regQuadFullMd.param.quad_wr_busy_det & (1<<chan)) || (regQuadFullMd.param.quad_wr_strobe & (1<<chan))) );

	if(u32WaitDelay == 0) 
	{
		LOG_DEBUG("TimeOut\n");
		return(eERROR_TIMEOUT);
	}

	/* Switch to live mode */
	regQuadFullMd.var = gpVIN->quad_full_md;
	regQuadFullMd.param.quad_single_md &= ~(1<<chan);
	gpVIN->quad_full_md = regQuadFullMd.var;

	u8WrPage = (regQuadFullMd.var >> (chan*4))&0xF;

	if(eVidResol >= vres_720x480p60) //progressive
	{
        bI2P = 0;
		baseBufAddrOdd = baseBufAddr + ((u8WrPage&0x3) * captureSize); //odd field or progressive page.
		baseBufAddrEven = 0;
	}
	else //interlace
	{
        bI2P = 1;
		baseBufAddrOdd = baseBufAddr + ((u8WrPage&0x3) * captureSize); //odd field or progressive page.
		if( (u8WrPage&0x3) != ((u8WrPage>>2)&0x3) )
		{
			baseBufAddrEven = baseBufAddr + (((u8WrPage>>2)&0x3) * captureSize); //even field
		}
		else
		{
			baseBufAddrEven = 0;
		}
	}

	#ifndef BD_SLT
	LOG_DEBUG("capture ch%d, wrpage:0x%x, bufAddr:[0x%08x,0x%08x](%08x,%08x,%08x,%08x)\n", chan, u8WrPage, baseBufAddrOdd, baseBufAddrEven, 
		(baseBufAddr + (0 * captureSize)),
		(baseBufAddr + (1 * captureSize)),
		(baseBufAddr + (2 * captureSize)),
		(baseBufAddr + (3 * captureSize)));
	#endif

	/* DMA */
	{
		_eDMAMODE eDMAMode;
		_eDMA_MINORMODE eDMAMinorMode;
		_eRDMATYPE eRDMAType;
		_eWDMATYPE eWDMAType;
		_eOTF2DMATYPE eOTF2DMAType;
		uint32 u32RdAddr;
		uint32 u32RdStride, u32WrStride, u32Width, u32Height;
		uint32 u32WrAddr;
		uint32 u32TimeOut;

		if(bYOnly) // y only dma.
		{/*{{{*/
			eDMAMode = eDMA_2D;
			if(bYOnly%2)
			{
				eDMAMinorMode = eDMA_MINOR_ODDBYTE;
			}
			else
			{
				eDMAMinorMode = eDMA_MINOR_EVENBYTE;
			}
			eRDMAType = eRDMA_2D;
			eWDMAType = eWDMA_2D;
			eOTF2DMAType = eOTF2DMA_DISABLE;
			u32RdAddr = (uint32)baseBufAddrOdd;
			u32WrAddr = (uint32)u32BufPAddr;

			#ifndef BD_SLT
			LOG_DEBUG("VPU_DMA %s start\n", (eDMAMinorMode == eDMA_MINOR_ODDBYTE)?"Oddcopy":"Evencopy");
			#endif

			u32Width = inWidth*2; //16bit
			u32Height = inHeight;
			u32RdStride = u32Width;
			u32WrStride = u32Width;

			//LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

			//LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
			u32TimeOut = 1000;
			/* 
			PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
			LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
					(uint32)*((uint32*)u32RdAddr+0),
					(uint32)*((uint32*)u32RdAddr+1),
					(uint32)*((uint32*)u32RdAddr+2),
					(uint32)*((uint32*)u32RdAddr+3),
					(uint32)*((uint32*)u32RdAddr+4),
					(uint32)*((uint32*)u32RdAddr+5));
			*/
			PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
			PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
			/*
			PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
			LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
					(uint32)*((uint32*)u32WrAddr+0),
					(uint32)*((uint32*)u32WrAddr+1),
					(uint32)*((uint32*)u32WrAddr+2),
					(uint32)*((uint32*)u32WrAddr+3),
					(uint32)*((uint32*)u32WrAddr+4),
					(uint32)*((uint32*)u32WrAddr+5));
			*/

			*pRetBufSize = captureSize>>1; //Yonly

			if(baseBufAddrEven != 0)
			{
				u32RdAddr = (uint32)baseBufAddrEven;
				u32WrAddr = (uint32)u32BufPAddr + (captureSize>>1); //+Odd

				LOG_DEBUG("VPU_DMA even field %s start\n", (eDMAMinorMode == eDMA_MINOR_ODDBYTE)?"Oddcopy":"Evencopy");

				u32Width = inWidth*2; //16bit
				u32Height = inHeight;
				u32RdStride = u32Width;
				u32WrStride = u32Width;

				//LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

				//LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
				u32TimeOut = 1000;
				//PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
				PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
				//PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
				*pRetBufSize += captureSize>>1; //Yonly

			}
		}/*}}}*/
		else // yuv dma.
		{/*{{{*/
			eDMAMode = eDMA_2D;
			eDMAMinorMode = eDMA_MINOR_COPY;
			eRDMAType = eRDMA_2D;
			eWDMAType = eWDMA_2D;
			eOTF2DMAType = eOTF2DMA_DISABLE;
			u32RdAddr = (uint32)baseBufAddrOdd;
			u32WrAddr = (uint32)u32BufPAddr;

			#ifndef BD_SLT
			LOG_DEBUG("VPU_DMA start\n");
			#endif

			u32Width = inWidth*2; //16bit
			u32Height = inHeight;
			u32RdStride = u32Width;
			u32WrStride = u32Width;

			#ifndef BD_SLT
			LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);
			LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
			#endif

			u32TimeOut = 1000;
			/*
			PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
			LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
					(uint32)*((uint32*)u32RdAddr+0),
					(uint32)*((uint32*)u32RdAddr+1),
					(uint32)*((uint32*)u32RdAddr+2),
					(uint32)*((uint32*)u32RdAddr+3),
					(uint32)*((uint32*)u32RdAddr+4),
					(uint32)*((uint32*)u32RdAddr+5));
			*/

			PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, captureSize);
			PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
			/*
			PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, captureSize);
			LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
					(uint32)*((uint32*)u32WrAddr+0),
					(uint32)*((uint32*)u32WrAddr+1),
					(uint32)*((uint32*)u32WrAddr+2),
					(uint32)*((uint32*)u32WrAddr+3),
					(uint32)*((uint32*)u32WrAddr+4),
					(uint32)*((uint32*)u32WrAddr+5));
			*/


			*pRetBufSize = captureSize;

			if(baseBufAddrEven != 0)
			{
				u32RdAddr = (uint32)baseBufAddrEven;
				u32WrAddr = (uint32)u32BufPAddr + captureSize;

				LOG_DEBUG("VPU_DMA even field start\n");

				u32Width = inWidth*2;
				u32Height = inHeight;
				u32RdStride = u32Width;
				u32WrStride = u32Width;

				//LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

				//LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
				u32TimeOut = 1000;
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
				LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32RdAddr+0),
						(uint32)*((uint32*)u32RdAddr+1),
						(uint32)*((uint32*)u32RdAddr+2),
						(uint32)*((uint32*)u32RdAddr+3),
						(uint32)*((uint32*)u32RdAddr+4),
						(uint32)*((uint32*)u32RdAddr+5));
				*/

				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32WrAddr+0),
						(uint32)*((uint32*)u32WrAddr+1),
						(uint32)*((uint32*)u32WrAddr+2),
						(uint32)*((uint32*)u32WrAddr+3),
						(uint32)*((uint32*)u32WrAddr+4),
						(uint32)*((uint32*)u32WrAddr+5));
                */

				*pRetBufSize += captureSize;
			}
		}/*}}}*/

		/* make interlace to progressive image */
		if( (bI2P) && (baseBufAddrEven != 0) )
		{
			{/*{{{*/

				if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
                {
                    LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                    return eERROR_INVALID_ARGUMENT;
                }

				eDMAMode = eDMA_2D;
				eDMAMinorMode = eDMA_MINOR_COPY;
				eRDMAType = eRDMA_2D;
				eWDMAType = eWDMA_2D;
				eOTF2DMAType = eOTF2DMA_DISABLE;
				u32RdAddr = (uint32)u32BufPAddr;
				u32WrAddr = (uint32)baseBufAddrOdd;

				//memset((uint32 *)u32WrAddr, 0, (inWidth*2)*(inHeight*2));

				if(bYOnly) // y only dma.
				{
					captureSize /= 2;
				}
				LOG_DEBUG("i2p VPU_DMA oddcopy start\n");

				u32Width = inWidth*(bYOnly?1:2); //16bit
				u32Height = inHeight;
				u32RdStride = u32Width;
				u32WrStride = u32Width*2; //make I2P 

				//LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

				//LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
				u32TimeOut = 1000;
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
				LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32RdAddr+0),
						(uint32)*((uint32*)u32RdAddr+1),
						(uint32)*((uint32*)u32RdAddr+2),
						(uint32)*((uint32*)u32RdAddr+3),
						(uint32)*((uint32*)u32RdAddr+4),
						(uint32)*((uint32*)u32RdAddr+5));
				*/

				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
				/* 
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32WrAddr+0),
						(uint32)*((uint32*)u32WrAddr+1),
						(uint32)*((uint32*)u32WrAddr+2),
						(uint32)*((uint32*)u32WrAddr+3),
						(uint32)*((uint32*)u32WrAddr+4),
						(uint32)*((uint32*)u32WrAddr+5));
				*/

				u32RdAddr = (uint32)u32BufPAddr + (captureSize);
				u32WrAddr = (uint32)baseBufAddrOdd + inWidth*(bYOnly?1:2); //16bit

				LOG_DEBUG("i2p VPU_DMA evencopy start\n");

				u32Width = inWidth*(bYOnly?1:2); //16bit
				u32Height = inHeight;
				u32RdStride = u32Width;
				u32WrStride = u32Width*2; //make I2P 

				//LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

				//LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
				u32TimeOut = 1000;
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
				LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32RdAddr+0),
						(uint32)*((uint32*)u32RdAddr+1),
						(uint32)*((uint32*)u32RdAddr+2),
						(uint32)*((uint32*)u32RdAddr+3),
						(uint32)*((uint32*)u32RdAddr+4),
						(uint32)*((uint32*)u32RdAddr+5));
				*/
				    
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32WrAddr+0),
						(uint32)*((uint32*)u32WrAddr+1),
						(uint32)*((uint32*)u32WrAddr+2),
						(uint32)*((uint32*)u32WrAddr+3),
						(uint32)*((uint32*)u32WrAddr+4),
						(uint32)*((uint32*)u32WrAddr+5));
				*/

				eDMAMode = eDMA_2D;
				eDMAMinorMode = eDMA_MINOR_COPY;
				eRDMAType = eRDMA_2D;
				eWDMAType = eWDMA_2D;
				eOTF2DMAType = eOTF2DMA_DISABLE;
				u32RdAddr = (uint32)baseBufAddrOdd;
				u32WrAddr = (uint32)u32BufPAddr;

				LOG_DEBUG("i2p VPU_DMA user buffer start\n");

				u32Width = inWidth*(bYOnly?1:2); //16bit
				u32Height = inHeight*2; //progressive
				u32RdStride = u32Width;
				u32WrStride = u32Width;

				captureSize = u32Width * u32Height;

				//LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

				//LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
				u32TimeOut = 1000;
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
				LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32RdAddr+0),
						(uint32)*((uint32*)u32RdAddr+1),
						(uint32)*((uint32*)u32RdAddr+2),
						(uint32)*((uint32*)u32RdAddr+3),
						(uint32)*((uint32*)u32RdAddr+4),
						(uint32)*((uint32*)u32RdAddr+5));
				*/

				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
				/*
				PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
				LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
						(uint32)*((uint32*)u32WrAddr+0),
						(uint32)*((uint32*)u32WrAddr+1),
						(uint32)*((uint32*)u32WrAddr+2),
						(uint32)*((uint32*)u32WrAddr+3),
						(uint32)*((uint32*)u32WrAddr+4),
						(uint32)*((uint32*)u32WrAddr+5));
				*/

			}/*}}}*/
		}
	}

	return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_SetCaptureUserMode(const PP_S32 defVideoFmt, const PP_U32 sclWidth, const PP_U32 sclHeight, const PP_U8 chSelBit)
{
	int i;
	VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
	VIN_QUAD_WIN_WR_EN_CONFIG_U regQuadWinWrEn;
	VIN_QUAD_CH_SEL_CONFIG_U regQuadChSel;
	int inWidth = 0, inHeight = 0;
	_VID_RESOL eVidResol = 0;
	uint32 u32WaitDelay;
	int captureTryCnt = 2;

	regQuadWinWrEn.var = gpVIN->quad_win_wr_en;
	regQuadWinWrEn.param.quad_wr_en = 0; //disable wren
	regQuadWinWrEn.param.quad_wr_i2p_en = 0; //normal mode.
	gpVIN->quad_win_wr_en = regQuadWinWrEn.var;
	gpVIN->quad_win_rd_en = 0; //disable rden

    /* set chx capture mode */
    regQuadChSel.var = gpVIN->quad_in_sel;
    regQuadChSel.var &= 0xFFFF0000;
    for(i = 0; i < 4; i++)
    {
        regQuadChSel.var |= ((0<<2|i) << (i*4));
    }
    gpVIN->quad_in_sel = regQuadChSel.var;

#if 1
    if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }
#endif
    gpVIN->quad_win_size = (1<<28) | (((sclWidth/8)&0x7F)<<19) | ((sclHeight)&0x3FF);

    gpVIN->quad_in_ch_ctrl[0].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
    gpVIN->quad_in_ch_ctrl[1].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
    gpVIN->quad_in_ch_ctrl[2].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
	gpVIN->quad_in_ch_ctrl[3].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale

    while(captureTryCnt--)
    {
        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 0;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = chSelBit & 0xF;
        gpVIN->quad_full_md = regQuadFullMd.var;

        regQuadWinWrEn.param.quad_wr_en = (chSelBit & 0xF); //enable wren
        gpVIN->quad_win_wr_en = regQuadWinWrEn.var;

        /* wait 1frame delay */
        u32WaitDelay = 40000*50;
        do
        {
            asm volatile("nop");
        } while( (u32WaitDelay-- > 0) );

        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 0;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = chSelBit & 0xF;
        gpVIN->quad_full_md = regQuadFullMd.var;

        /* wait capture done */
        u32WaitDelay = 0x3FFFFFF;
        do
        {
            asm volatile("nop");
            regQuadFullMd.var = gpVIN->quad_full_md;
        } while( (u32WaitDelay-- > 0) && ((regQuadFullMd.param.quad_wr_busy_det) || (regQuadFullMd.param.quad_wr_strobe)) );

        if(u32WaitDelay == 0) 
        {
            LOG_DEBUG("TimeOut\n");
            return(eERROR_TIMEOUT);
        }
    }


	return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_GetCaptureUserImage(const PP_S32 defVideoFmt, const PP_U32 sclWidth, const PP_U32 sclHeight, const PP_U8 chSelBit, PP_U32 *pu32BufPAddr, PP_U32 *pRetBufSize)
{
    int chan = 0; 
    int inWidth = 0, inHeight = 0;
    _VID_RESOL eVidResol = 0;
    uint32 captureSize;
    uint32 baseBufAddr;
    uint32 baseBufAddrOdd = 0, baseBufAddrEven = 0;
    uint32 u32WaitDelay;
    uint8 u8WrPage;
    VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
    int bI2P = 0;
    const unsigned char bYOnly = 1;

    if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

    //capture image
    {
        /* quad size */
        inWidth = sclWidth;
        inHeight = sclHeight;
        LOG_DEBUG("Capture(%dx%d)-%s\n", inWidth, inHeight, (bYOnly)?"Yonly":"UYVY");
        //make forcly width 32Byte align */
        inWidth <<= 2; 
        inHeight >>= 2;
        captureSize = (inWidth * inHeight * 2);

        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 0;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = chSelBit & 0xF;
        gpVIN->quad_full_md = regQuadFullMd.var;
    }

    /* wait capture done */
    u32WaitDelay = 0x3FFFFFF;
    do
    {
        asm volatile("nop");
        regQuadFullMd.var = gpVIN->quad_full_md;
    } while( (u32WaitDelay-- > 0) && ((regQuadFullMd.param.quad_wr_busy_det) || (regQuadFullMd.param.quad_wr_strobe)) );
    if(u32WaitDelay == 0) 
    {
        LOG_DEBUG("TimeOut\n");
        return(eERROR_TIMEOUT);
    }

    for(chan = 0; chan < 4; chan++)
    {
        pRetBufSize[chan] = 0;
        if(chSelBit & (1<<chan))
        {
            u8WrPage = (regQuadFullMd.var >> (chan*4))&0xF;

            baseBufAddr = gpVIN->quad_mem_loc[chan];
            if(eVidResol >= vres_720x480p60) //progressive
            {
                bI2P = 0;
                baseBufAddrOdd = baseBufAddr + ((u8WrPage&0x3) * captureSize); //odd field or progressive page.
                baseBufAddrEven = 0;
            }
            else //interlace
            {
                bI2P = 1;
                baseBufAddrOdd = baseBufAddr + ((u8WrPage&0x3) * captureSize); //odd field or progressive page.
                if( (u8WrPage&0x3) != ((u8WrPage>>2)&0x3) )
                {
                    baseBufAddrEven = baseBufAddr + (((u8WrPage>>2)&0x3) * captureSize); //even field
                }
                else
                {
                    baseBufAddrEven = 0;
                }
            }

            /*
               LOG_DEBUG("capture ch%d, wrpage:0x%x, bufAddr:[0x%08x,0x%08x](%08x,%08x,%08x,%08x)\n", chan, u8WrPage, baseBufAddrOdd, baseBufAddrEven, 
               (baseBufAddr + (0 * captureSize)),
               (baseBufAddr + (1 * captureSize)),
               (baseBufAddr + (2 * captureSize)),
               (baseBufAddr + (3 * captureSize)));
               */

            /* DMA */
            {
                _eDMAMODE eDMAMode;
                _eDMA_MINORMODE eDMAMinorMode;
                _eRDMATYPE eRDMAType;
                _eWDMATYPE eWDMAType;
                _eOTF2DMATYPE eOTF2DMAType;
                uint32 u32RdAddr;
                uint32 u32RdStride, u32WrStride, u32Width, u32Height;
                uint32 u32WrAddr;
                uint32 u32TimeOut;

                if(bYOnly) // y only dma.
                {/*{{{*/
                    eDMAMode = eDMA_2D;
                    if(bYOnly%2)
                    {
                        eDMAMinorMode = eDMA_MINOR_ODDBYTE;
                    }
                    else
                    {
                        eDMAMinorMode = eDMA_MINOR_EVENBYTE;
                    }
                    eRDMAType = eRDMA_2D;
                    eWDMAType = eWDMA_2D;
                    eOTF2DMAType = eOTF2DMA_DISABLE;
                    u32RdAddr = (uint32)baseBufAddrOdd;
                    u32WrAddr = (uint32)pu32BufPAddr[chan];

                    LOG_DEBUG("VPU_DMA %s start\n", (eDMAMinorMode == eDMA_MINOR_ODDBYTE)?"Oddcopy":"Evencopy");

                    u32Width = inWidth*2; //16bit
                    u32Height = inHeight;
                    u32RdStride = u32Width;
                    u32WrStride = u32Width;

                    //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                    //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                    u32TimeOut = 1000;
                    /* 
                       PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                       LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (uint32)*((uint32*)u32RdAddr+0),
                       (uint32)*((uint32*)u32RdAddr+1),
                       (uint32)*((uint32*)u32RdAddr+2),
                       (uint32)*((uint32*)u32RdAddr+3),
                       (uint32)*((uint32*)u32RdAddr+4),
                       (uint32)*((uint32*)u32RdAddr+5));
                       */
                    PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
                    PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                    /*
                       PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
                       LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (uint32)*((uint32*)u32WrAddr+0),
                       (uint32)*((uint32*)u32WrAddr+1),
                       (uint32)*((uint32*)u32WrAddr+2),
                       (uint32)*((uint32*)u32WrAddr+3),
                       (uint32)*((uint32*)u32WrAddr+4),
                       (uint32)*((uint32*)u32WrAddr+5));
                       */

                    pRetBufSize[chan] = captureSize>>1; //Yonly

                    if(baseBufAddrEven != 0)
                    {
                        u32RdAddr = (uint32)baseBufAddrEven;
                        u32WrAddr = (uint32)pu32BufPAddr[chan] + (captureSize>>1); //+Odd

                        LOG_DEBUG("VPU_DMA even field %s start\n", (eDMAMinorMode == eDMA_MINOR_ODDBYTE)?"Oddcopy":"Evencopy");

                        u32Width = inWidth*2; //16bit
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width;

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        //PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                        PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
                        PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        //PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize>>1));
                        pRetBufSize[chan] += captureSize>>1; //Yonly

                    }
                }/*}}}*/
                else // yuv dma.
                {/*{{{*/
                    eDMAMode = eDMA_2D;
                    eDMAMinorMode = eDMA_MINOR_COPY;
                    eRDMAType = eRDMA_2D;
                    eWDMAType = eWDMA_2D;
                    eOTF2DMAType = eOTF2DMA_DISABLE;
                    u32RdAddr = (uint32)baseBufAddrOdd;
                    u32WrAddr = (uint32)pu32BufPAddr[chan];

                    LOG_DEBUG("VPU_DMA start\n");

                    u32Width = inWidth*2; //16bit
                    u32Height = inHeight;
                    u32RdStride = u32Width;
                    u32WrStride = u32Width;

                    //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                    //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                    u32TimeOut = 1000;
                    /*
                       PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                       LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (uint32)*((uint32*)u32RdAddr+0),
                       (uint32)*((uint32*)u32RdAddr+1),
                       (uint32)*((uint32*)u32RdAddr+2),
                       (uint32)*((uint32*)u32RdAddr+3),
                       (uint32)*((uint32*)u32RdAddr+4),
                       (uint32)*((uint32*)u32RdAddr+5));
                       */

                    PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, captureSize);
                    PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                    /*
                       PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, captureSize);
                       LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (uint32)*((uint32*)u32WrAddr+0),
                       (uint32)*((uint32*)u32WrAddr+1),
                       (uint32)*((uint32*)u32WrAddr+2),
                       (uint32)*((uint32*)u32WrAddr+3),
                       (uint32)*((uint32*)u32WrAddr+4),
                       (uint32)*((uint32*)u32WrAddr+5));
                       */


                    pRetBufSize[chan] = captureSize;

                    if(baseBufAddrEven != 0)
                    {
                        u32RdAddr = (uint32)baseBufAddrEven;
                        u32WrAddr = (uint32)pu32BufPAddr[chan] + captureSize;

                        LOG_DEBUG("VPU_DMA even field start\n");

                        u32Width = inWidth*2;
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width;

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32RdAddr+0),
                           (uint32)*((uint32*)u32RdAddr+1),
                           (uint32)*((uint32*)u32RdAddr+2),
                           (uint32)*((uint32*)u32RdAddr+3),
                           (uint32)*((uint32*)u32RdAddr+4),
                           (uint32)*((uint32*)u32RdAddr+5));
                           */

                        PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                        PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32WrAddr+0),
                           (uint32)*((uint32*)u32WrAddr+1),
                           (uint32)*((uint32*)u32WrAddr+2),
                           (uint32)*((uint32*)u32WrAddr+3),
                           (uint32)*((uint32*)u32WrAddr+4),
                           (uint32)*((uint32*)u32WrAddr+5));
                           */

                        pRetBufSize[chan] += captureSize;
                    }
                }/*}}}*/

                /* make interlace to progressive image */
                if( (bI2P) && (baseBufAddrEven != 0) )
                {
                    {/*{{{*/

                        if( PPAPI_VIN_GetResol(defVideoFmt, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
                        {
                            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                            return eERROR_INVALID_ARGUMENT;
                        }

                        eDMAMode = eDMA_2D;
                        eDMAMinorMode = eDMA_MINOR_COPY;
                        eRDMAType = eRDMA_2D;
                        eWDMAType = eWDMA_2D;
                        eOTF2DMAType = eOTF2DMA_DISABLE;
                        u32RdAddr = (uint32)pu32BufPAddr[chan];
                        u32WrAddr = (uint32)baseBufAddrOdd;

                        //memset((uint32 *)u32WrAddr, 0, (inWidth*2)*(inHeight*2));

                        if(bYOnly) // y only dma.
                        {
                            captureSize /= 2;
                        }
                        LOG_DEBUG("i2p VPU_DMA oddcopy start\n");

                        u32Width = inWidth*(bYOnly?1:2); //16bit
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width*2; //make I2P 

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32RdAddr+0),
                           (uint32)*((uint32*)u32RdAddr+1),
                           (uint32)*((uint32*)u32RdAddr+2),
                           (uint32)*((uint32*)u32RdAddr+3),
                           (uint32)*((uint32*)u32RdAddr+4),
                           (uint32)*((uint32*)u32RdAddr+5));
                           */

                        PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                        PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /* 
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32WrAddr+0),
                           (uint32)*((uint32*)u32WrAddr+1),
                           (uint32)*((uint32*)u32WrAddr+2),
                           (uint32)*((uint32*)u32WrAddr+3),
                           (uint32)*((uint32*)u32WrAddr+4),
                           (uint32)*((uint32*)u32WrAddr+5));
                           */

                        u32RdAddr = (uint32)pu32BufPAddr[chan] + (captureSize);
                        u32WrAddr = (uint32)baseBufAddrOdd + inWidth*(bYOnly?1:2); //16bit

                        LOG_DEBUG("i2p VPU_DMA evencopy start\n");

                        u32Width = inWidth*(bYOnly?1:2); //16bit
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width*2; //make I2P 

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32RdAddr+0),
                           (uint32)*((uint32*)u32RdAddr+1),
                           (uint32)*((uint32*)u32RdAddr+2),
                           (uint32)*((uint32*)u32RdAddr+3),
                           (uint32)*((uint32*)u32RdAddr+4),
                           (uint32)*((uint32*)u32RdAddr+5));
                           */

                        PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                        PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32WrAddr+0),
                           (uint32)*((uint32*)u32WrAddr+1),
                           (uint32)*((uint32*)u32WrAddr+2),
                           (uint32)*((uint32*)u32WrAddr+3),
                           (uint32)*((uint32*)u32WrAddr+4),
                           (uint32)*((uint32*)u32WrAddr+5));
                           */

                        eDMAMode = eDMA_2D;
                        eDMAMinorMode = eDMA_MINOR_COPY;
                        eRDMAType = eRDMA_2D;
                        eWDMAType = eWDMA_2D;
                        eOTF2DMAType = eOTF2DMA_DISABLE;
                        u32RdAddr = (uint32)baseBufAddrOdd;
                        u32WrAddr = (uint32)pu32BufPAddr[chan];

                        LOG_DEBUG("i2p VPU_DMA user buffer start\n");

                        u32Width = inWidth*(bYOnly?1:2); //16bit
                        u32Height = inHeight*2; //progressive
                        u32RdStride = u32Width;
                        u32WrStride = u32Width;

                        captureSize = u32Width * u32Height;

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32RdAddr+0),
                           (uint32)*((uint32*)u32RdAddr+1),
                           (uint32)*((uint32*)u32RdAddr+2),
                           (uint32)*((uint32*)u32RdAddr+3),
                           (uint32)*((uint32*)u32RdAddr+4),
                           (uint32)*((uint32*)u32RdAddr+5));
                           */

                        PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                        PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /*
                           PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (uint32)*((uint32*)u32WrAddr+0),
                           (uint32)*((uint32*)u32WrAddr+1),
                           (uint32)*((uint32*)u32WrAddr+2),
                           (uint32)*((uint32*)u32WrAddr+3),
                           (uint32)*((uint32*)u32WrAddr+4),
                           (uint32)*((uint32*)u32WrAddr+5));
                           */

                    }/*}}}*/
                }
            }
        }
    }

	return(eSUCCESS);
}

#include <cache.h>

/* Low-level port I/O */
#define PI5008_GetRegValue(reg)		(*((volatile unsigned int *)(reg)))
#define PI5008_SetRegValue(reg, data)	((*((volatile unsigned int *)(reg))) = (unsigned int)(data))
#define PI5008_BITCLR(reg, mask)		((*((volatile unsigned int *)(reg))) &= ~(unsigned int)(mask))
#define PI5008_BITSET(reg, mask)		((*((volatile unsigned int *)(reg))) |= (unsigned int)(mask))

typedef union tagPI5008_VIN_QUAD_FULL_MD_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad0_wr_page		:	4,
				quad1_wr_page		:	4,
				quad2_wr_page		:	4,
				quad3_wr_page		:	4,
				quad_wr_strobe		:	4,
				quad_wr_busy_det	:	4,
				quad_single_md		:	4,
				reserved0		:	3,
				quad_full_md		:	1;
	}param;
} PI5008_VIN_QUAD_FULL_MD_CONFIG_U;

typedef union tagPI5008_VIN_QUAD_WIN_WR_EN_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_ddr_wr_delay	:	8,
				quad_ddr_rd_delay	:	8,
				quad_wr_i2p_en		:	4,
				quad_wr_period		:	4,
				quad_wr_int_md		:	4,
				quad_wr_en		:	4;
	}param;
} PI5008_VIN_QUAD_WIN_WR_EN_CONFIG_U;

typedef union tagPI5008_VIN_QUAD_CH_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad0_ch_sel_portsel	:	2,
		                quad0_ch_sel_pathsel	:	2,
		                quad1_ch_sel_portsel	:	2,
		                quad1_ch_sel_pathsel	:	2,
		                quad2_ch_sel_portsel	:	2,
		                quad2_ch_sel_pathsel	:	2,
		                quad3_ch_sel_portsel	:	2,
		                quad3_ch_sel_pathsel	:	2,
				quad_fld_inv	:	4,
		                reserved0	:	12;
	}param;
} PI5008_VIN_QUAD_CH_SEL_CONFIG_U;

int PPAPI_BARE_VIN_SetCaptureUserMode(const unsigned int sclWidth, const unsigned int sclHeight, const unsigned char chSelBit)
{
    int i;
    PI5008_VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
    PI5008_VIN_QUAD_WIN_WR_EN_CONFIG_U regQuadWinWrEn;
    PI5008_VIN_QUAD_CH_SEL_CONFIG_U regQuadChSel;
    int inWidth = 0, inHeight = 0;
    unsigned int u32WaitDelay;
    int captureTryCnt = 2;

    // define input camera image size
    inWidth = 1280; inHeight = 720;

    regQuadWinWrEn.var = PI5008_GetRegValue(0xF0F00000 + 0x154);
    regQuadWinWrEn.param.quad_wr_en = 0; //disable wren
    regQuadWinWrEn.param.quad_wr_i2p_en = 0; //normal mode.
	//gpVIN->quad_win_wr_en = regQuadWinWrEn.var;
    PI5008_SetRegValue( (0xF0F00000 + 0x154), regQuadWinWrEn.var);
	//gpVIN->quad_win_rd_en = 0; //disable rden
    PI5008_SetRegValue( (0xF0F00000 + 0x158), 0); //disable rden

    /* set chx capture mode */
    //regQuadChSel.var = gpVIN->quad_in_sel;
    regQuadChSel.var = PI5008_GetRegValue(0xF0F00000 + 0x018);
    regQuadChSel.var &= 0xFFFF0000;
    for(i = 0; i < 4; i++)
    {
        regQuadChSel.var |= ((0<<2|i) << (i*4));
    }
    //gpVIN->quad_in_sel = regQuadChSel.var;
    PI5008_SetRegValue( (0xF0F00000 + 0x018), regQuadChSel.var);

    //gpVIN->quad_win_size = (1<<28) | (((sclWidth/8)&0x7F)<<19) | ((sclHeight)&0x3FF);
    PI5008_SetRegValue( (0xF0F00000 + 0x150), ((1<<28) | (((sclWidth/8)&0x7F)<<19) | ((sclHeight)&0x3FF)) );

    //gpVIN->quad_in_ch_ctrl[0].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
    PI5008_SetRegValue( (0xF0F00000 + 0x104), ((((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth)) ); //vertical | horizontal scale
    //gpVIN->quad_in_ch_ctrl[1].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
    PI5008_SetRegValue( (0xF0F00000 + 0x114), ((((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth)) ); //vertical | horizontal scale
    //gpVIN->quad_in_ch_ctrl[2].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
    PI5008_SetRegValue( (0xF0F00000 + 0x124), ((((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth)) ); //vertical | horizontal scale
	//gpVIN->quad_in_ch_ctrl[3].hvscl = (((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth); //vertical | horizontal scale
    PI5008_SetRegValue( (0xF0F00000 + 0x134), ((((sclHeight*0xFFFF)/inHeight)<<16) | ((sclWidth*0xFFFF)/inWidth)) ); //vertical | horizontal scale

    while(captureTryCnt--)
    {
        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 0;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = chSelBit & 0xF;
        //gpVIN->quad_full_md = regQuadFullMd.var;
        PI5008_SetRegValue( (0xF0F00000 + 0x174), regQuadFullMd.var );

        regQuadWinWrEn.param.quad_wr_en = (chSelBit & 0xF); //enable wren
        //gpVIN->quad_win_wr_en = regQuadWinWrEn.var;
        PI5008_SetRegValue( (0xF0F00000 + 0x154), regQuadWinWrEn.var);

        /* wait 1frame delay */
        u32WaitDelay = 40000*50;
        do
        {
            asm volatile("nop");
        } while( (u32WaitDelay-- > 0) );

        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 0;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = chSelBit & 0xF;
        //gpVIN->quad_full_md = regQuadFullMd.var;
        PI5008_SetRegValue( (0xF0F00000 + 0x174), regQuadFullMd.var );

        /* wait capture done */
        u32WaitDelay = 0x3FFFFFF;
        do
        {
            asm volatile("nop");
            //regQuadFullMd.var = gpVIN->quad_full_md;
            regQuadFullMd.var = PI5008_GetRegValue(0xF0F00000 + 0x174);
        } while( (u32WaitDelay-- > 0) && ((regQuadFullMd.param.quad_wr_busy_det) || (regQuadFullMd.param.quad_wr_strobe)) );

        if(u32WaitDelay == 0) 
        {
            LOG_DEBUG("TimeOut\n");
            return(-1);
        }
    }


	return(0);
}

typedef enum 
{
    ePI5008_DMA_2D = 0,
    ePI5008_DMA_RLE = 2,
    ePI5008_DMA_OTF = 7
}_ePI5008_DMAMODE;
typedef enum 
{
    ePI5008_DMA_MINOR_COPY = 0,
    ePI5008_DMA_MINOR_EVENBYTE = 2, //0,2,4,...
    ePI5008_DMA_MINOR_ODDBYTE = 3,  //1,3,5,...
}_ePI5008_DMA_MINORMODE;
typedef enum 
{
    ePI5008_RDMA_1D = 0,
    ePI5008_RDMA_2D
}_ePI5008_RDMATYPE;
typedef enum 
{
    ePI5008_WDMA_1D = 0,
    ePI5008_WDMA_2D
}_ePI5008_WDMATYPE;
typedef enum 
{
    ePI5008_OTF2DMA_DISABLE = 0,
    ePI5008_OTF2DMA_SCLOUT = 1,
    ePI5008_OTF2DMA_FILTER_FAST = 2,
    ePI5008_OTF2DMA_FILTER_BRIEF = 3
}_ePI5008_OTF2DMATYPE;

int PPAPI_BARE_VPU_DMA_Start(const _ePI5008_DMAMODE eDMAMode, const _ePI5008_DMA_MINORMODE eDMAMinorMode, const _ePI5008_RDMATYPE eRDMAType, const _ePI5008_WDMATYPE eWDMAType, const _ePI5008_OTF2DMATYPE eOTF2DMAType, const unsigned int u32RdPAddr, const unsigned int u32RdStride, const unsigned short u16RdWidth, const unsigned short u16RdHeight, const unsigned int u32WrPAddr, const unsigned int u32WrStride, unsigned int u32TimeOut)
{
    int ret = 0;

    unsigned int u32DmaSize = 0;

    //wait previous dma done.

    if( (u32RdPAddr == 0) || (u32RdStride == 0) || (u32WrStride == 0) || (u16RdWidth == 0) || (u16RdHeight == 0) )
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(-1);
    }
    if((u16RdWidth & 0xF) != 0)
    {
        LOG_CRITICAL("Invalid dma param. Not align 16byte\n");
        return(-1);
    }
    if( (eDMAMinorMode == ePI5008_DMA_MINOR_EVENBYTE) || (eDMAMinorMode == ePI5008_DMA_MINOR_ODDBYTE) )
    {
        if(eWDMAType == ePI5008_WDMA_1D)
        {
            LOG_CRITICAL("Error! Don't support 1D WDMA. Set 2D WDMA\n");
            return(-1);
        }
    }

    u32DmaSize = u16RdWidth * u16RdHeight;
    if(u32DmaSize == 0)
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(-1);
    }
    if(eRDMAType == ePI5008_RDMA_2D)
    {
        if(eDMAMinorMode != ePI5008_DMA_MINOR_COPY)
        {
            if((u16RdWidth & 0x1F) != 0) //32byte align
            {
                LOG_CRITICAL("Invalid dma param. Not align 32byte\n");
                return(-1);
            }
        }
    }

    //LOG_DEBUG("Dma mode:%d-%d, typeRW:%d/%d, OTF2DMA type:%d, rd:%08x, strideR%08x, wd:%04x, ht:%04x, wr:%08x, strideW:%08x, time:%d",
    //        eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, u16RdWidth, u16RdHeight, u32WrPAddr, u32WrStride, u32TimeOut);
    //LOG_DEBUG("Dma size:0x%08x\n", u32DmaSize);

    //Clear dma irq_done
    //VPU_DMACTRL_REG->ctrl &=  ~0x2; 
	PI5008_SetRegValue( (0xF1400000 + 0x000), (PI5008_GetRegValue( (0xF1400000 + 0x000)) & ~0x2) );

    //Set RDMA param.
    //VPU_DMACTRL_REG->rdma2d_base = u32RdPAddr;
	PI5008_SetRegValue( (0xF1400000 + 0x004), u32RdPAddr);
    //VPU_DMACTRL_REG->rdma2d_stride = u32RdStride;
	PI5008_SetRegValue( (0xF1400000 + 0x008), u32RdStride);
    if(eRDMAType == ePI5008_RDMA_2D)
    {
        //VPU_DMACTRL_REG->rdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
        PI5008_SetRegValue( (0xF1400000 + 0x00C), (((u16RdWidth&0xFFFF) << 16) | (u16RdHeight&0xFFFF)) );
    }
    else //1D RDMA
    {
        //VPU_DMACTRL_REG->rdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (1&0xFFFF);
        PI5008_SetRegValue( (0xF1400000 + 0x00C), (((u16RdWidth&0xFFFF) << 16) | (1&0xFFFF)) );
    }
    //VPU_DMACTRL_REG->ctrl = 0x0F0F0F00 | ((eDMAMode)&0x7)<<4 | ((eDMAMinorMode)&0x3)<<2; //mode[0:2D dma, 2:RLE Encode, 7:DMA2OTF/OTF2DMA]
	PI5008_SetRegValue( (0xF1400000 + 0x000), (0x0F0F0F00 | ((eDMAMode)&0x7)<<4 | ((eDMAMinorMode)&0x3)<<2) ); //mode[0:2D dma, 2:RLE Encode, 7:DMA2OTF/OTF2DMA]

    //VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;
	PI5008_SetRegValue( (0xF1420000 + 0x000), (PI5008_GetRegValue( (0xF1420000 + 0x000)) & ~(1<<23)) );
    //VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
	PI5008_SetRegValue( (0xF1420000 + 0x000), (PI5008_GetRegValue( (0xF1420000 + 0x000)) & ~(3<<25)) );
    if(eDMAMode == ePI5008_DMA_OTF)
    {
        //VPU_INTC_REG->intc_mask |= (0x80); //mask 1:block, 0:release
        PI5008_SetRegValue( (0xF1421000 + 0x000), (PI5008_GetRegValue( (0xF1421000 + 0x000)) | 0x80) );
        //VPU_FB_CONFIG_REG->fld.dma2otf_enable = 1;
	    PI5008_SetRegValue( (0xF1420000 + 0x000), (PI5008_GetRegValue( (0xF1420000 + 0x000)) | (1<<23)) );
    }
    else
    {
        //VPU_INTC_REG->intc_mask &= ~(0x80); //mask 1:block, 0:release
        PI5008_SetRegValue( (0xF1421000 + 0x000), (PI5008_GetRegValue( (0xF1421000 + 0x000)) & ~(0x80)) );
        //VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;
	    PI5008_SetRegValue( (0xF1420000 + 0x000), (PI5008_GetRegValue( (0xF1420000 + 0x000)) & ~(1<<23)) );
    }

    //Set WDMA param.
    //VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
	PI5008_SetRegValue( (0xF1420000 + 0x000), (PI5008_GetRegValue( (0xF1420000 + 0x000)) & ~(3<<25)) );
    if(u32WrPAddr != 0)
    {
        if(eDMAMode == ePI5008_DMA_OTF)
        {
            //VPU_FB_CONFIG_REG->fld.otf2dma_enable = eOTF2DMAType;
	        PI5008_SetRegValue( (0xF1420000 + 0x000), (eOTF2DMAType<<25)&0x3 );
        }
        else if(eDMAMode == ePI5008_DMA_RLE)
        {
            //VPU_DMACTRL_REG->wdma_rlewaitdone_count = 0xFFFFFFFF;
        }

        if(eWDMAType == ePI5008_WDMA_2D)
        {
            if( (eDMAMinorMode == ePI5008_DMA_MINOR_EVENBYTE) || (eDMAMinorMode == ePI5008_DMA_MINOR_ODDBYTE) )
            {
                //VPU_DMACTRL_REG->wdma2d_base = u32WrPAddr;
	            PI5008_SetRegValue( (0xF1400000 + 0x010), u32WrPAddr);
                //VPU_DMACTRL_REG->wdma2d_stride = u32WrStride>>1;
	            PI5008_SetRegValue( (0xF1400000 + 0x014), u32WrStride>>1);
                //VPU_DMACTRL_REG->wdma2d_size = (((u16RdWidth>>1)&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
	            PI5008_SetRegValue( (0xF1400000 + 0x018), ((((u16RdWidth>>1)&0xFFFF) << 16) | (u16RdHeight&0xFFFF)) );
                //VPU_DMACTRL_REG->ctrl &= ~(1<<7); //1d wdma disable
	            PI5008_SetRegValue( (0xF1400000 + 0x000), (PI5008_GetRegValue( (0xF1400000 + 0x000)) & ~(1<<7)) );
            }
            else
            {
                //VPU_DMACTRL_REG->wdma2d_base = u32WrPAddr;
	            PI5008_SetRegValue( (0xF1400000 + 0x010), u32WrPAddr);
                //VPU_DMACTRL_REG->wdma2d_stride = u32WrStride;
	            PI5008_SetRegValue( (0xF1400000 + 0x014), u32WrStride);
                //VPU_DMACTRL_REG->wdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
	            PI5008_SetRegValue( (0xF1400000 + 0x018), ((((u16RdWidth)&0xFFFF) << 16) | (u16RdHeight&0xFFFF)) );
                //VPU_DMACTRL_REG->ctrl &= ~(1<<7); //1d wdma disable
	            PI5008_SetRegValue( (0xF1400000 + 0x000), (PI5008_GetRegValue( (0xF1400000 + 0x000)) & ~(1<<7)) );

            }
        }
        else //1D WDMA
        {
            //VPU_DMACTRL_REG->wdma1d_base = u32WrPAddr;
	        PI5008_SetRegValue( (0xF1400000 + 0x01C), u32WrPAddr);
            //VPU_DMACTRL_REG->ctrl |= 1<<7; //1d wdma enable
	        PI5008_SetRegValue( (0xF1400000 + 0x000), (PI5008_GetRegValue( (0xF1400000 + 0x000)) | (1<<7)) );
            //VPU_DMACTRL_REG->wdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (0x1&0xFFFF);
	        PI5008_SetRegValue( (0xF1400000 + 0x018), ((((u16RdWidth)&0xFFFF) << 16) | (0x1&0xFFFF)) );
        }
    }

    //VPU_DMACTRL_REG->ctrl |= 0x1; //b'0 : start
    PI5008_SetRegValue( (0xF1400000 + 0x000), (PI5008_GetRegValue( (0xF1400000 + 0x000)) | (0x1)) );

    if( (eDMAMode != ePI5008_DMA_OTF) )
    {
        if(u32TimeOut)
        {
            while(u32TimeOut--)
            {
                //ret = PPDRV_VPU_DMA_DrvWaitDone(u32TimeOut);
                if( (PI5008_GetRegValue( (0xF1421000 + 0x010)) & (0x80)) )
                {
                    ret = 0;
                    break;
                }
            }
            if(u32TimeOut == 0)
            {
                LOG_CRITICAL("Timeout dma.\n");
                ret = -1;
            }
        }
    }

    return(ret);
}

int PI5008_PPAPI_VIN_GetCaptureUserImage(const unsigned int sclWidth, const unsigned int sclHeight, const unsigned char chSelBit, unsigned int *pu32BufPAddr, unsigned int *pRetBufSize)
{
    int chan = 0; 
    int inWidth = 0, inHeight = 0;
    unsigned int captureSize;
    unsigned int baseBufAddr;
    unsigned int baseBufAddrOdd = 0, baseBufAddrEven = 0;
    unsigned int u32WaitDelay;
    unsigned char u8WrPage;
    PI5008_VIN_QUAD_FULL_MD_CONFIG_U regQuadFullMd;
    int bI2P = 0;
    const unsigned char bYOnly = 1;

    // define input camera image size
    inWidth = 1280; inHeight = 720;

    //capture image
    {
        /* quad size */
        inWidth = sclWidth;
        inHeight = sclHeight;
        LOG_DEBUG("Capture(%dx%d)-%s\n", inWidth, inHeight, (bYOnly)?"Yonly":"UYVY");
        //make forcly width 32Byte align */
        inWidth <<= 2; 
        inHeight >>= 2;
        captureSize = (inWidth * inHeight * 2);

        regQuadFullMd.var = 0;
        regQuadFullMd.param.quad_full_md = 0;
        regQuadFullMd.param.quad_single_md = 0xF;
        regQuadFullMd.param.quad_wr_strobe = chSelBit & 0xF;
        PI5008_SetRegValue( (0xF0F00000 + 0x174), regQuadFullMd.var );
    }

    /* wait capture done */
    u32WaitDelay = 0x3FFFFFF;
    do
    {
        asm volatile("nop");
        regQuadFullMd.var = PI5008_GetRegValue(0xF0F00000 + 0x174);
    } while( (u32WaitDelay-- > 0) && ((regQuadFullMd.param.quad_wr_busy_det) || (regQuadFullMd.param.quad_wr_strobe)) );
    if(u32WaitDelay == 0) 
    {
        LOG_DEBUG("TimeOut\n");
        return(-1);
    }

    for(chan = 0; chan < 4; chan++)
    {
        pRetBufSize[chan] = 0;
        if(chSelBit & (1<<chan))
        {
            u8WrPage = (regQuadFullMd.var >> (chan*4))&0xF;

            baseBufAddr = PI5008_GetRegValue(0xF0F00000 + 0x140 + (chan*4));
            //progressive
            {
                bI2P = 0;
                baseBufAddrOdd = baseBufAddr + ((u8WrPage&0x3) * captureSize); //odd field or progressive page.
                baseBufAddrEven = 0;
            }
#if 0
            //interlace
            {
                bI2P = 1;
                baseBufAddrOdd = baseBufAddr + ((u8WrPage&0x3) * captureSize); //odd field or progressive page.
                if( (u8WrPage&0x3) != ((u8WrPage>>2)&0x3) )
                {
                    baseBufAddrEven = baseBufAddr + (((u8WrPage>>2)&0x3) * captureSize); //even field
                }
                else
                {
                    baseBufAddrEven = 0;
                }
            }
#endif
               LOG_DEBUG("capture ch%d, wrpage:0x%x, bufAddr:[0x%08x,0x%08x](%08x,%08x,%08x,%08x)\n", chan, u8WrPage, baseBufAddrOdd, baseBufAddrEven, 
               (baseBufAddr + (0 * captureSize)),
               (baseBufAddr + (1 * captureSize)),
               (baseBufAddr + (2 * captureSize)),
               (baseBufAddr + (3 * captureSize)));
            /*
               */


            /* DMA */
            {
                _ePI5008_DMAMODE eDMAMode;
                _ePI5008_DMA_MINORMODE eDMAMinorMode;
                _ePI5008_RDMATYPE eRDMAType;
                _ePI5008_WDMATYPE eWDMAType;
                _ePI5008_OTF2DMATYPE eOTF2DMAType;
                unsigned int u32RdAddr;
                unsigned int u32RdStride, u32WrStride, u32Width, u32Height;
                unsigned int u32WrAddr;
                unsigned int u32TimeOut;

                if(bYOnly) // y only dma.
                {/*{{{*/
                    eDMAMode = ePI5008_DMA_2D;
                    if(bYOnly%2)
                    {
                        eDMAMinorMode = ePI5008_DMA_MINOR_ODDBYTE;
                    }
                    else
                    {
                        eDMAMinorMode = ePI5008_DMA_MINOR_EVENBYTE;
                    }
                    eRDMAType = ePI5008_RDMA_2D;
                    eWDMAType = ePI5008_WDMA_2D;
                    eOTF2DMAType = ePI5008_OTF2DMA_DISABLE;
                    u32RdAddr = (unsigned int)baseBufAddrOdd;
                    u32WrAddr = (unsigned int)pu32BufPAddr[chan];

                    LOG_DEBUG("VPU_DMA %s start\n", (eDMAMinorMode == ePI5008_DMA_MINOR_ODDBYTE)?"Oddcopy":"Evencopy");

                    u32Width = inWidth*2; //16bit
                    u32Height = inHeight;
                    u32RdStride = u32Width;
                    u32WrStride = u32Width;

                    //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                    //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                    u32TimeOut = 1000;
                    /* 
                       cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                       LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (unsigned int)*((unsigned int*)u32RdAddr+0),
                       (unsigned int)*((unsigned int*)u32RdAddr+1),
                       (unsigned int)*((unsigned int*)u32RdAddr+2),
                       (unsigned int)*((unsigned int*)u32RdAddr+3),
                       (unsigned int)*((unsigned int*)u32RdAddr+4),
                       (unsigned int)*((unsigned int*)u32RdAddr+5));
                       */
                    cache_inv_range ((unsigned int *)u32WrAddr, (captureSize>>1));
                    PPAPI_BARE_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                    /*
                       cache_inv_range ((unsigned int *)u32WrAddr, (captureSize>>1));
                       LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (unsigned int)*((unsigned int*)u32WrAddr+0),
                       (unsigned int)*((unsigned int*)u32WrAddr+1),
                       (unsigned int)*((unsigned int*)u32WrAddr+2),
                       (unsigned int)*((unsigned int*)u32WrAddr+3),
                       (unsigned int)*((unsigned int*)u32WrAddr+4),
                       (unsigned int)*((unsigned int*)u32WrAddr+5));
                       */

                    pRetBufSize[chan] = captureSize>>1; //Yonly

                    if(baseBufAddrEven != 0)
                    {
                        u32RdAddr = (unsigned int)baseBufAddrEven;
                        u32WrAddr = (unsigned int)pu32BufPAddr[chan] + (captureSize>>1); //+Odd

                        LOG_DEBUG("VPU_DMA even field %s start\n", (eDMAMinorMode == ePI5008_DMA_MINOR_ODDBYTE)?"Oddcopy":"Evencopy");

                        u32Width = inWidth*2; //16bit
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width;

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        //cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                        cache_inv_range ((unsigned int *)u32WrAddr, (captureSize>>1));
                        PPAPI_BARE_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        //cache_inv_range ((unsigned int *)u32WrAddr, (captureSize>>1));
                        pRetBufSize[chan] += captureSize>>1; //Yonly

                    }
                }/*}}}*/
                else // yuv dma.
                {/*{{{*/
                    eDMAMode = ePI5008_DMA_2D;
                    eDMAMinorMode = ePI5008_DMA_MINOR_COPY;
                    eRDMAType = ePI5008_RDMA_2D;
                    eWDMAType = ePI5008_WDMA_2D;
                    eOTF2DMAType = ePI5008_OTF2DMA_DISABLE;
                    u32RdAddr = (unsigned int)baseBufAddrOdd;
                    u32WrAddr = (unsigned int)pu32BufPAddr[chan];

                    LOG_DEBUG("VPU_DMA start\n");

                    u32Width = inWidth*2; //16bit
                    u32Height = inHeight;
                    u32RdStride = u32Width;
                    u32WrStride = u32Width;

                    //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                    //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                    u32TimeOut = 1000;
                    /*
                       cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                       LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (unsigned int)*((unsigned int*)u32RdAddr+0),
                       (unsigned int)*((unsigned int*)u32RdAddr+1),
                       (unsigned int)*((unsigned int*)u32RdAddr+2),
                       (unsigned int)*((unsigned int*)u32RdAddr+3),
                       (unsigned int)*((unsigned int*)u32RdAddr+4),
                       (unsigned int)*((unsigned int*)u32RdAddr+5));
                       */

                    cache_inv_range ((unsigned int *)u32WrAddr, captureSize);
                    PPAPI_BARE_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                    /*
                       cache_inv_range ((unsigned int *)u32WrAddr, captureSize);
                       LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                       (unsigned int)*((unsigned int*)u32WrAddr+0),
                       (unsigned int)*((unsigned int*)u32WrAddr+1),
                       (unsigned int)*((unsigned int*)u32WrAddr+2),
                       (unsigned int)*((unsigned int*)u32WrAddr+3),
                       (unsigned int)*((unsigned int*)u32WrAddr+4),
                       (unsigned int)*((unsigned int*)u32WrAddr+5));
                       */


                    pRetBufSize[chan] = captureSize;

                    if(baseBufAddrEven != 0)
                    {
                        u32RdAddr = (unsigned int)baseBufAddrEven;
                        u32WrAddr = (unsigned int)pu32BufPAddr[chan] + captureSize;

                        LOG_DEBUG("VPU_DMA even field start\n");

                        u32Width = inWidth*2;
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width;

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32RdAddr+0),
                           (unsigned int)*((unsigned int*)u32RdAddr+1),
                           (unsigned int)*((unsigned int*)u32RdAddr+2),
                           (unsigned int)*((unsigned int*)u32RdAddr+3),
                           (unsigned int)*((unsigned int*)u32RdAddr+4),
                           (unsigned int)*((unsigned int*)u32RdAddr+5));
                           */

                        cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                        PPAPI_BARE_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /*
                           cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32WrAddr+0),
                           (unsigned int)*((unsigned int*)u32WrAddr+1),
                           (unsigned int)*((unsigned int*)u32WrAddr+2),
                           (unsigned int)*((unsigned int*)u32WrAddr+3),
                           (unsigned int)*((unsigned int*)u32WrAddr+4),
                           (unsigned int)*((unsigned int*)u32WrAddr+5));
                           */

                        pRetBufSize[chan] += captureSize;
                    }
                }/*}}}*/

                /* make interlace to progressive image */
                if( (bI2P) && (baseBufAddrEven != 0) )
                {
                    {/*{{{*/
                        inWidth = 720; inHeight = 240;

                        eDMAMode = ePI5008_DMA_2D;
                        eDMAMinorMode = ePI5008_DMA_MINOR_COPY;
                        eRDMAType = ePI5008_RDMA_2D;
                        eWDMAType = ePI5008_WDMA_2D;
                        eOTF2DMAType = ePI5008_OTF2DMA_DISABLE;
                        u32RdAddr = (unsigned int)pu32BufPAddr[chan];
                        u32WrAddr = (unsigned int)baseBufAddrOdd;

                        //memset((unsigned int *)u32WrAddr, 0, (inWidth*2)*(inHeight*2));

                        if(bYOnly) // y only dma.
                        {
                            captureSize /= 2;
                        }
                        LOG_DEBUG("i2p VPU_DMA oddcopy start\n");

                        u32Width = inWidth*(bYOnly?1:2); //16bit
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width*2; //make I2P 

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32RdAddr+0),
                           (unsigned int)*((unsigned int*)u32RdAddr+1),
                           (unsigned int)*((unsigned int*)u32RdAddr+2),
                           (unsigned int)*((unsigned int*)u32RdAddr+3),
                           (unsigned int)*((unsigned int*)u32RdAddr+4),
                           (unsigned int)*((unsigned int*)u32RdAddr+5));
                           */

                        cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                        PPAPI_BARE_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /* 
                           cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32WrAddr+0),
                           (unsigned int)*((unsigned int*)u32WrAddr+1),
                           (unsigned int)*((unsigned int*)u32WrAddr+2),
                           (unsigned int)*((unsigned int*)u32WrAddr+3),
                           (unsigned int)*((unsigned int*)u32WrAddr+4),
                           (unsigned int)*((unsigned int*)u32WrAddr+5));
                           */

                        u32RdAddr = (unsigned int)pu32BufPAddr[chan] + (captureSize);
                        u32WrAddr = (unsigned int)baseBufAddrOdd + inWidth*(bYOnly?1:2); //16bit

                        LOG_DEBUG("i2p VPU_DMA evencopy start\n");

                        u32Width = inWidth*(bYOnly?1:2); //16bit
                        u32Height = inHeight;
                        u32RdStride = u32Width;
                        u32WrStride = u32Width*2; //make I2P 

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32RdAddr+0),
                           (unsigned int)*((unsigned int*)u32RdAddr+1),
                           (unsigned int)*((unsigned int*)u32RdAddr+2),
                           (unsigned int)*((unsigned int*)u32RdAddr+3),
                           (unsigned int)*((unsigned int*)u32RdAddr+4),
                           (unsigned int)*((unsigned int*)u32RdAddr+5));
                           */

                        cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                        PPAPI_BARE_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /*
                           cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32WrAddr+0),
                           (unsigned int)*((unsigned int*)u32WrAddr+1),
                           (unsigned int)*((unsigned int*)u32WrAddr+2),
                           (unsigned int)*((unsigned int*)u32WrAddr+3),
                           (unsigned int)*((unsigned int*)u32WrAddr+4),
                           (unsigned int)*((unsigned int*)u32WrAddr+5));
                           */

                        eDMAMode = ePI5008_DMA_2D;
                        eDMAMinorMode = ePI5008_DMA_MINOR_COPY;
                        eRDMAType = ePI5008_RDMA_2D;
                        eWDMAType = ePI5008_WDMA_2D;
                        eOTF2DMAType = ePI5008_OTF2DMA_DISABLE;
                        u32RdAddr = (unsigned int)baseBufAddrOdd;
                        u32WrAddr = (unsigned int)pu32BufPAddr[chan];

                        LOG_DEBUG("i2p VPU_DMA user buffer start\n");

                        u32Width = inWidth*(bYOnly?1:2); //16bit
                        u32Height = inHeight*2; //progressive
                        u32RdStride = u32Width;
                        u32WrStride = u32Width;

                        captureSize = u32Width * u32Height;

                        //LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x\n", eDMAMode, eDMAMinorMode, captureSize);

                        //LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                        u32TimeOut = 1000;
                        /*
                           cache_inv_range ((unsigned int *)u32RdAddr, captureSize);
                           LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32RdAddr+0),
                           (unsigned int)*((unsigned int*)u32RdAddr+1),
                           (unsigned int)*((unsigned int*)u32RdAddr+2),
                           (unsigned int)*((unsigned int*)u32RdAddr+3),
                           (unsigned int)*((unsigned int*)u32RdAddr+4),
                           (unsigned int)*((unsigned int*)u32RdAddr+5));
                           */

                        cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                        PPAPI_BARE_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                        /*
                           cache_inv_range ((unsigned int *)u32WrAddr, (captureSize));
                           LOG_DEBUG ("Wr : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
                           (unsigned int)*((unsigned int*)u32WrAddr+0),
                           (unsigned int)*((unsigned int*)u32WrAddr+1),
                           (unsigned int)*((unsigned int*)u32WrAddr+2),
                           (unsigned int)*((unsigned int*)u32WrAddr+3),
                           (unsigned int)*((unsigned int*)u32WrAddr+4),
                           (unsigned int)*((unsigned int*)u32WrAddr+5));
                           */

                    }/*}}}*/
                }
            }
        }
    }

	return(0);
}


PP_VOID PPAPI_VIN_EnableQuad(const PP_BOOL bEnable)
{
    LOG_DEBUG("VIN Quad:En(%d)\n", bEnable);
    if(bEnable)
    {
        gpVIN->quad_win_wr_en |= 0xF0000000;
        gpVIN->quad_win_rd_en |= 0x80000000;
    }
    else
    {
        gpVIN->quad_win_wr_en &= (uint32)~0xF0000000;
        gpVIN->quad_win_rd_en &= (uint32)~0x80000000;
    }
}

PP_RESULT_E PPAPI_VIN_GetVIDPortFromUserDefined(const PP_S8 s8VinPort[], PP_S8 s8RetVinPort[])
{
    PP_U8 u8PortNum = 0;
    PP_S8 s8VinPortCh = 0;
    int i;

    _VIN_PATH vinPath;

    PI_VIN_GetUserConfigVar(&vinPath, NULL);

    //LOG_DEBUG("VIN VID Port: Port0/1/2/3(%d/%d/%d/%d)\n", s8VinPort[0], s8VinPort[1], s8VinPort[2], s8VinPort[3]);

    if( (s8VinPort[0] > 3) || (s8VinPort[1] > 3) || (s8VinPort[2] > 3) || (s8VinPort[3] > 3) )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT. (%d/%d/%d/%d)\n", __FUNCTION__, __LINE__,
            s8VinPort[0],
            s8VinPort[1],
            s8VinPort[2],
            s8VinPort[3]);
        return(eERROR_INVALID_ARGUMENT);
    }
    if( (s8RetVinPort == NULL) )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_POINTER\n", __FUNCTION__, __LINE__);
	    return(eERROR_INVALID_POINTER);
    }

    for(i = 0; i < 4; i++)
    {
        s8RetVinPort[i] = -1;
    }

    for(u8PortNum = 0; u8PortNum < 4; u8PortNum++)
    {
        s8VinPortCh = s8VinPort[u8PortNum];
        for(i = 0; i < 4; i++)
        {
            if( vinPath.vidPort[i] == u8PortNum)
            {
                s8RetVinPort[i] = s8VinPortCh;
                break;
            }
        }
    }

	return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_SetVIDPort(const PP_S8 s8VinPort0, const PP_S8 s8VinPort1,  const PP_S8 s8VinPort2, const PP_S8 s8VinPort3, const PP_S8 s8VinPort4)
{
    PP_U32 u32Reg = 0;
    PP_U8 u8Mode = 0;

#if ((VIDEO_IN_TYPE==VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE==VIDEO_IN_TYPE_MIPI_YUV) )
    u8Mode = 2;
#elif (VIDEO_IN_TYPE==VIDEO_IN_TYPE_PVI)
    u8Mode = 3;
#elif (VIDEO_IN_TYPE==VIDEO_IN_TYPE_PARALLEL)
    u8Mode = 0;
#else
#error "unknown video in type"
#endif

    LOG_DEBUG("VIN VID Port: Port0/1/2/3/4(%d/%d/%d/%d/%d)\n", s8VinPort0, s8VinPort1, s8VinPort2, s8VinPort3, s8VinPort4);

    u32Reg = gpVIN->vid_in_sel;						/* 0x00c */

    if( (s8VinPort0 > 3) || (s8VinPort1 > 3) || (s8VinPort2 > 3) || (s8VinPort3 > 3)|| (s8VinPort4 > 3) )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return(eERROR_INVALID_ARGUMENT);
    }

    {/*{{{*/
        if( s8VinPort0 >= 0 )
        {
            u32Reg &= 0xFFFFFFF0;
            u32Reg |= ((s8VinPort0 & 0x3) | (u8Mode<<2)) << 0;
        }
        if( s8VinPort1 >= 0 )
        {
            u32Reg &= 0xFFFFFF0F;
            u32Reg |= ((s8VinPort1 & 0x3) | (u8Mode<<2)) << 4;
        }
        if( s8VinPort2 >= 0 )
        {
            u32Reg &= 0xFFFFF0FF;
            u32Reg |= ((s8VinPort2 & 0x3) | (u8Mode<<2)) << 8;
        }
        if( s8VinPort3 >= 0 )
        {
            u32Reg &= 0xFFFF0FFF;
            u32Reg |= ((s8VinPort3 & 0x3) | (u8Mode<<2)) << 12;
        }
        if( s8VinPort4 >= 0 )
        {
            u32Reg &= 0xFFF0FFFF;
            u32Reg |= ((s8VinPort4 & 0x3) | (u8Mode<<2)) << 16;
        }

        gpVIN->vid_in_sel = u32Reg;						/* 0x00c 		*/

    }/*}}}*/

	return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_SetSVMChannel(const PP_S8 s8SvmChannel, PP_S8 s8SvmPath, PP_S8 s8SvmPort)
{
    PP_U32 u32Reg = 0;

    //LOG_DEBUG("VIN SVM channel: Ch:%d, Path:%d, Port:%d. (-1:init value)\n", s8SvmChannel, s8SvmPath, s8SvmPort);

    u32Reg = gpVIN->svm_in_sel;						/* 0x014 */

    if( (s8SvmChannel > 3) || (s8SvmPath > 1) || (s8SvmPort > 3) )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return(eERROR_INVALID_ARGUMENT);
    }

    if( (s8SvmPort == -1) || (s8SvmPath == -1) )
    {
        _VIN_PATH vinPath;
        PI_VIN_GetUserConfigVar(&vinPath, NULL);

        if(s8SvmPort == -1) 
        {
            s8SvmPort = vinPath.svmPort[s8SvmChannel];
        }
        if(s8SvmPath == -1) 
        {
            s8SvmPath = vinPath.svmIn[s8SvmChannel];
        }
    }

    u32Reg &= (0xFFFFFFFF & ~(0xF<<(s8SvmChannel*4)));
    u32Reg |= (((s8SvmPath & 0x3)<<2) | ((s8SvmPort & 0x3)<<0)) << (s8SvmChannel*4);

    gpVIN->svm_in_sel = u32Reg;						/* 0x014 		*/

    return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_SetROChannel(const PP_U8 u8ROPath, const PP_U8 b8Bit, const PP_S8 s8OutCh0, const PP_S8 s8OutCh1,  const PP_S8 s8OutCh2, const PP_S8 s8OutCh3)
{
    PP_U32 u32RO0Reg = 0, u32RO1Reg = 0;

    LOG_DEBUG("VIN RO Channel: ROPath(%d), b8Bit(%d), ch(%d,%d,%d,%d)\n", u8ROPath, b8Bit, s8OutCh0, s8OutCh1, s8OutCh2, s8OutCh3);

    u32RO0Reg = gpVIN->rec0_ch_sel;						/* 0x184 		- Record 0 path output sel */
    u32RO1Reg = gpVIN->rec1_ch_sel;						/* 0x188 		- Record 1 path output sel */

    if( (s8OutCh0 > 3) || (s8OutCh1 > 3) || (s8OutCh2 > 3) || (s8OutCh3 > 3) )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return(eERROR_INVALID_ARGUMENT);
    }

    if(b8Bit)
    {
        if(u8ROPath == 0)
        {/*{{{*/
            if( s8OutCh0 >= 0 )
            {
                u32RO0Reg &= 0xFFFFFFF0;
                u32RO0Reg |= ((s8OutCh0*2) & 0xF) << 0; //0,2,4,6
            }
            if( s8OutCh1 >= 0 )
            {
                u32RO0Reg &= 0xFFFFFF0F;
                u32RO0Reg |= ((s8OutCh1*2) & 0xF) << 4; //0,2,4,6
            }
            if( s8OutCh2 >= 0 )
            {
                u32RO0Reg &= 0xFFFFF0FF;
                u32RO0Reg |= ((s8OutCh2*2) & 0xF) << 8; //0,2,4,6
            }
            if( s8OutCh3 >= 0 )
            {
                u32RO0Reg &= 0xFFFF0FFF;
                u32RO0Reg |= ((s8OutCh3*2) & 0xF) << 12; //0,2,4,6
            }

            gpVIN->rec0_ch_sel = u32RO0Reg;						/* 0x184 		- Record 0 path output sel */

        }/*}}}*/
        else
        {/*{{{*/
            if( s8OutCh0 >= 0 )
            {
                u32RO1Reg &= 0xFFFFFFF0;
                u32RO1Reg |= ((s8OutCh0*2) & 0xF) << 0; //0,2,4,6
            }
            if( s8OutCh1 >= 0 )
            {
                u32RO1Reg &= 0xFFFFFF0F;
                u32RO1Reg |= ((s8OutCh1*2) & 0xF) << 4; //0,2,4,6
            }
            if( s8OutCh2 >= 0 )
            {
                u32RO1Reg &= 0xFFFFF0FF;
                u32RO1Reg |= ((s8OutCh2*2) & 0xF) << 8; //0,2,4,6
            }
            if( s8OutCh3 >= 0 )
            {
                u32RO1Reg &= 0xFFFF0FFF;
                u32RO1Reg |= ((s8OutCh3*2) & 0xF) << 12; //0,2,4,6
            }

            gpVIN->rec1_ch_sel = u32RO1Reg;						/* 0x188 		- Record 1 path output sel */

        }/*}}}*/

    }
    else
    {/*{{{*/
        if( s8OutCh0 >= 0 )
        {
            u32RO0Reg &= 0xFFFFFFF0;
            u32RO0Reg |= ((s8OutCh0*2) & 0xF) << 0; //Y:0,2,4,6
            u32RO1Reg &= 0xFFFFFFF0;
            u32RO1Reg |= ((s8OutCh0*2+1) & 0xF) << 0; //C:1,3,5,7
        }
        if( s8OutCh1 >= 0 )
        {
            u32RO0Reg &= 0xFFFFFF0F;
            u32RO0Reg |= ((s8OutCh1*2) & 0xF) << 4; //Y:0,2,4,6
            u32RO1Reg &= 0xFFFFFF0F;
            u32RO1Reg |= ((s8OutCh1*2+1) & 0xF) << 4; //C:1,3,5,7
        }
        if( s8OutCh2 >= 0 )
        {
            u32RO0Reg &= 0xFFFFF0FF;
            u32RO0Reg |= ((s8OutCh2*2) & 0xF) << 8; //Y:0,2,4,6
            u32RO1Reg &= 0xFFFFF0FF;
            u32RO1Reg |= ((s8OutCh2*2+1) & 0xF) << 8; //C:1,3,5,7
        }
        if( s8OutCh3 >= 0 )
        {
            u32RO0Reg &= 0xFFFF0FFF;
            u32RO0Reg |= ((s8OutCh3*2) & 0xF) << 12; //Y:0,2,4,6
            u32RO1Reg &= 0xFFFF0FFF;
            u32RO1Reg |= ((s8OutCh3*2+1) & 0xF) << 12; //C:1,3,5,7
        }

        gpVIN->rec0_ch_sel = u32RO0Reg;						/* 0x184 		- Record 0 path output sel */
        gpVIN->rec1_ch_sel = u32RO1Reg;						/* 0x188 		- Record 1 path output sel */

    }/*}}}*/

	return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_GetInputInfo(const PP_U8 u8Channel, PP_U32 *pRetInputInfo)
{

	SYNC_SIZE_CTL_REG_T inputInfo;

	if( u8Channel > 5)
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

    PPDRV_VIN_GetInputInfo(u8Channel, (uint32 *)&inputInfo);

    pRetInputInfo[0] = inputInfo.hsize;
    pRetInputInfo[1] = inputInfo.fsize;
    pRetInputInfo[2] = inputInfo.hvact;

#if 0 //debug print
    {
        VIN_SYNC_VIN_HSIZE_INFO_CONFIG_U *pHsizeInfo;
        VIN_SYNC_VIN_FSIZE_INFO_CONFIG_U *pFsizeInfo;
        VIN_SYNC_VIN_HVACT_INFO_CONFIG_U *pHVactInfo;

        pHsizeInfo = (VIN_SYNC_VIN_HSIZE_INFO_CONFIG_U *)&inputInfo.hsize;
        pFsizeInfo = (VIN_SYNC_VIN_FSIZE_INFO_CONFIG_U *)&inputInfo.fsize;
        pHVactInfo = (VIN_SYNC_VIN_HVACT_INFO_CONFIG_U *)&inputInfo.hvact;

        LOG_DEBUG("Hsize:%d, Lock[H:%d, F:%d, Hact:%d, Vact:%d], Loss:%d, Format:%d\n", 
                pHsizeInfo->hsize,
                pHsizeInfo->sync_lock_hsize,
                pHsizeInfo->sync_lock_fsize,
                pHsizeInfo->sync_lock_hactive,
                pHsizeInfo->sync_lock_vactive,
                pHsizeInfo->video_loss_det,
                pHsizeInfo->format_det);
        LOG_DEBUG("Fsize:%d\n", pFsizeInfo->fsize);
        LOG_DEBUG("Hact:%d, Vact:%d\n", pHVactInfo->hact, pHVactInfo->vact,);
    }
#endif

	return(eSUCCESS);
}


PP_RESULT_E PPAPI_VIN_DiagCameraInput(const PP_U8 u8Channel)
{
	SYNC_SIZE_CTL_REG_T sync_ch_info;
	VIN_CTRL_REG_T vid_in_ctrl;

	PP_U32 u32DetHsize, u32DetVsize, u32DetHactive, u32DetVactive;
	PP_U32 u32SetHsize, u32SetVsize, u32SetHactive, u32SetVactive;
	PP_U32 u32DiffHsize, u32DiffVsize;

	if( u8Channel > 4)
    {
        LOG_DEBUG("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

    memcpy(&sync_ch_info, &gpVIN->sync_in_hvsize_ch_info[u8Channel], sizeof(SYNC_SIZE_CTL_REG_T));
	//VIN_CTRL_REG_T vid_in_ch_ctrl[4];				/* 0x040~0x7c	- Video input format ch control */
	//VIN_CTRL_REG_T rec_in_ch_ctrl[4];				/* 0x080~0xbc	- Record input format ch control */

	u32DetHsize = sync_ch_info.hsize & 0x3FFF;
	u32DetHsize++;
	u32DetHsize >>= 1; u32DetHsize <<= 1;
	u32DetVsize = (sync_ch_info.fsize & 0xFFFFFF)/u32DetHsize;
	u32DetVsize++;
	u32DetVsize >>= 1; u32DetVsize <<= 1;
	u32DetHactive = (sync_ch_info.hvact) & 0xFFF;
	u32DetVactive = (sync_ch_info.hvact >> 16) & 0x7FF;

    LOG_DEBUG("Camera diagnosis => Det Ch:%d, Hsize:%d, Vsize:%d, Hact:%d, Vact:%d\n", u8Channel,
            u32DetHsize, u32DetVsize, u32DetHactive, u32DetVactive); 
    
    memcpy(&vid_in_ctrl, &gpVIN->vid_in_ch_ctrl[u8Channel], sizeof(VIN_CTRL_REG_T));
	u32SetHsize = vid_in_ctrl.htotal & 0x3FFF; 
	u32SetVsize = vid_in_ctrl.vtotal & 0x3FFF; 
	u32SetHactive = (vid_in_ctrl.ctrl >> 16) & 0x7FF;
	u32SetVactive = vid_in_ctrl.ctrl & 0x7FF;

    if( (u32DetHsize != 0) && (u32DetVsize != 0) && (u32DetHactive != 0) && (u32DetVactive != 0) )
    {
        u32DiffHsize = (u32DetHsize >= u32SetHsize) ? (u32DetHsize - u32SetHsize):(u32SetHsize - u32DetHsize);
        u32DiffVsize = (u32DetVsize >= u32SetVsize) ? (u32DetVsize - u32SetVsize):(u32SetVsize - u32DetVsize);


        if( (u32DiffHsize >= 16) || (u32DiffVsize >= 16) )
        {
            LOG_CRITICAL("Camera diagnosis => Warning!!! Ch:%d, H/V total size value mismatch.\n", u8Channel);
            LOG_DEBUG("Camera diagnosis => current value Hsize:%d, Vsize:%d, Hact:%d, Vact:%d\n", u32SetHsize, u32SetVsize, u32SetHactive, u32SetVactive); 
            LOG_CRITICAL("Camera diagnosis => Change H/V total value to Htotal:%d, Vtotal:%d\n", u32DetHsize, u32DetVsize);
            return eERROR_INVALID_ARGUMENT;
        }
        else
        {
            LOG_DEBUG("Camera diagnosis => Ch:%d Camrea H/V total size is reasonable value.\n", u8Channel);
        }
    }

	return(eSUCCESS);
}

PP_RESULT_E PPAPI_VIN_GetCameraPlugStatus(PP_S8 pRetCamStatus[])
{
    PP_RESULT_E ret = eSUCCESS;

    PP_U32 u32CamCh = 0, u32PlugIn = 0;
    int portNum;

    if( pRetCamStatus == NULL)
    {
        LOG_DEBUG("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI) )
    {
        PP_U8 statusNoVideo = 0;

        for(portNum = 0; portNum < 4; portNum++)
        {
            statusNoVideo = 0;
            PPAPI_PVIRX_GetNovidStatus(portNum, &statusNoVideo);
            if(statusNoVideo) //Novideo. frame rate=>0
            {
                u32CamCh |= 0x0F<<(portNum*8);
                u32PlugIn |= 0x01<<(portNum*8);
            }
            else
            {
                u32CamCh |= portNum<<(portNum*8);
                u32PlugIn |= 0x02<<(portNum*8);
            }
        }
    }
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) )
    {
        if(PPAPI_IPC_ReqCamStatus(&u32CamCh, &u32PlugIn, 3000))
        {
            LOG_DEBUG("Error!!! ReqCamStatus Fail\n");
            return eERROR_TIMEOUT;
        }
    }
#endif

    if( (u32CamCh != 0) & (u32PlugIn != 0) )
    {
        //LOG_DEBUG("ReqCamStatus. CamCh: 0x%x, PlugIn: 0x%x\n", u32CamCh, u32PlugIn);
        /* Update VIN Ch & Port */
        {/*{{{*/
            PP_S8 s8CamCh[4] = {-1, };
            PP_U8 u8CamChStatus, u8PlugStatus;
            for(portNum = 0; portNum < 4; portNum++)
            {
                u8CamChStatus = u32CamCh>>(portNum*8)&0xFF;
                u8PlugStatus = u32PlugIn>>(portNum*8)&0xFF;
                if( (u8CamChStatus > 3) || (u8PlugStatus == 0x01) ) //Plug-Out
                {
                    //LOG_DEBUG("Cam: %d, PlugOut\n", portNum);
                    s8CamCh[portNum] = -1; //PlugOut
                }
                else if( (u8CamChStatus <= 3) && (u8PlugStatus == 0x02) ) //Plug-In
                {
                    //LOG_DEBUG("Cam: %d-%d, PlugIn\n", portNum, u8CamChStatus);
                    s8CamCh[portNum] = u8CamChStatus; //PlugIn camera
                }
                else
                {
                    //LOG_DEBUG("Error!!! Unknown cam ch status.\n");
                    s8CamCh[portNum] = -1;
                }

                pRetCamStatus[portNum] = s8CamCh[portNum];
            }
        }/*}}}*/

        ret = eSUCCESS;
    }


	return(ret);
}


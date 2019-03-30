#include "pvirx_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pvirx.h"
#include "pvirx_table.h"
#include "pvirx_func.h"
#include "pvirx_user_config.h"
#include "pvirx_ceqfunc.h"

#define PVIRX_CEQ_EST_COMP(x,y)        ((0x9600*x)/y)

PP_RESULT_E PPDRV_PVIRX_GetDetGainStatus(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_DetGainStatus OUT *pstDetGainStatus)
{
	PP_RESULT_E result = eERROR_FAILURE;

	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstDetGainStatus == NULL)
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	stReg.reg = 0x0003;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->syncLevel = u8RegData;

	stReg.reg = 0x0004;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->dcGain = (PP_U16)u8RegData<<8;

	stReg.reg = 0x0005;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->dcGain |= (PP_U16)u8RegData;

	stReg.reg = 0x0006;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->acGain = (PP_U16)u8RegData<<8;

	stReg.reg = 0x0007;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->acGain |= (PP_U16)u8RegData;

	stReg.reg = 0x0008;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->comp1 = (PP_U16)u8RegData<<8;

	stReg.reg = 0x0009;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->comp1 |= (PP_U16)u8RegData;

	stReg.reg = 0x000A;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->comp2 = (PP_U16)u8RegData<<8;

	stReg.reg = 0x000B;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->comp2 |= (PP_U16)u8RegData;

	stReg.reg = 0x000C;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->atten1 = (PP_U16)u8RegData<<8;

	stReg.reg = 0x000D;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->atten1 |= (PP_U16)u8RegData;

	stReg.reg = 0x000E;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->atten2 = (PP_U16)u8RegData<<8;

	stReg.reg = 0x000F;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	pstDetGainStatus->atten2 |= (PP_U16)u8RegData;

	PrDbg("%x-Read[syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstDetGainStatus->syncLevel COMMA pstDetGainStatus->dcGain COMMA pstDetGainStatus->acGain COMMA pstDetGainStatus->comp1 COMMA pstDetGainStatus->comp2 COMMA pstDetGainStatus->atten1 COMMA pstDetGainStatus->atten2);

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_GetEQGain(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, const _stPVIRX_DetGainStatus IN *pstDetGainStatus, _stPVIRX_MeasureEqInfo OUT *pstMeasureEqInfo)
{
	PP_RESULT_E result = eERROR_FAILURE;

	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;

	PP_U8 step = 0;
	PP_S32 tblReolution = 0;

	_stPVIRX_Table_CEQ_SDResolution *pTblCeqSDComp = NULL, *pTblCeqSDAtten = NULL;
	_stPVIRX_Table_CEQ_STD_HDResolution *pTblCeqHDComp = NULL, *pTblCeqHDAtten = NULL;
	PP_U8 vadcEqBandComp = 0, vadcEqBandAtten = 0;
	PP_U16 distComp = 0, distAtten = 0;
	PP_U8 eqStepCompNum = 0, eqStepAttenNum = 0;
        PP_U16 u16Comp = 0;

	if( (pstVidStatusReg == NULL) || (pstDetGainStatus == NULL) || (pstMeasureEqInfo == NULL) )
	{
		PrErrorString("invalid argu\n");
        return(eERROR_FAILURE);
    }

#ifdef SUPPORT_AUTODETECT_PVI
    //get information by rejudge
    PPDRV_PVIRX_GetStdResol(cid, chanAddr, pstVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, NULL);
    PrDbg("%x-EQ [Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
#else
    //read information by no rejudge.
    PPDRV_PVIRX_ReadStdResol(cid, chanAddr, pstVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
    PrDbg("%x-Fix EQ [Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
#endif // SUPPORT_AUTODETECT_PVI


	/* Get EQ table */
	switch(cameraStandard)
	{/*{{{*/
		case CVBS:
			{
				pTblCeqSDComp = (_stPVIRX_Table_CEQ_SDResolution *)pvirx_ceq_table_estComplex_SD;
				pTblCeqSDAtten = (_stPVIRX_Table_CEQ_SDResolution *)pvirx_ceq_table_estAtten2_SD;
				if(cameraResolution == camera_ntsc) 
				{
					tblReolution = 0; //NTSC
				}
				else
				{
					tblReolution = 1; //PAL
				}
			}
			break;
#if defined(SUPPORT_PVIRX_STD_PVI)
		case PVI:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_PVI;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_PVI;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
		case HDA:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_HDA;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_HDA;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
		case CVI:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_CVI;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_CVI;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
		case HDT_OLD:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_HDT_OLD;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_HDT_OLD;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
		case HDT_NEW:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_HDT_NEW;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_HDT_NEW;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
		default:
			{
				result = eERROR_FAILURE;
				PrError("Invalid camera Standard(%d)\n", cameraStandard);
				return(result);
			}
	}/*}}}*/

	if( pstDetGainStatus->comp2 != 0)
	{
		u16Comp = PVIRX_CEQ_EST_COMP(pstDetGainStatus->comp1, pstDetGainStatus->comp2);
	}
	PrDbg("%x-Get measure Comp:0x%x(%x/%x)\n", chanAddr COMMA u16Comp COMMA pstDetGainStatus->comp1 COMMA pstDetGainStatus->comp2);
	PrDbg("%x-Get measure Atten:(%x/%x)\n", chanAddr COMMA pstDetGainStatus->atten1 COMMA pstDetGainStatus->atten2);

	/* Get EQ value */
	if(cameraStandard == CVBS)
	{/*{{{*/
		//Get comp value.
		if(pTblCeqSDComp[MAX_PVIRX_CEQ_COMP_TBL_NUM-1].pData[tblReolution][1] < u16Comp) //check max
		{
			vadcEqBandComp = pTblCeqSDComp[MAX_PVIRX_CEQ_COMP_TBL_NUM-1].pData[tblReolution][2]; //factor
			distComp = pTblCeqSDComp[MAX_PVIRX_CEQ_COMP_TBL_NUM-1].distance;
			eqStepCompNum = 0xFF; //max
		}
		else if(pTblCeqSDComp[0].pData[tblReolution][0]> u16Comp) //check min
		{
			vadcEqBandComp = pTblCeqSDComp[0].pData[tblReolution][2]; //factor
			distComp = pTblCeqSDComp[0].distance;
			eqStepCompNum = 0; //min
		}
		else
		{
			for(step = 0; step < MAX_PVIRX_CEQ_COMP_TBL_NUM; step++)
			{
				if( (pTblCeqSDComp[step].pData[tblReolution][0]<= u16Comp) && (u16Comp <= pTblCeqSDComp[step].pData[tblReolution][1]) )
				{
					vadcEqBandComp = pTblCeqSDComp[step].pData[tblReolution][2]; //factor
					distComp = pTblCeqSDComp[step].distance;
					eqStepCompNum = step;
					break;
				}
			}
		}

		//Get atten value.
		if(pTblCeqSDAtten[MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1].pData[tblReolution][1] < pstDetGainStatus->atten2) //check max
		{
			vadcEqBandAtten = pTblCeqSDAtten[MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1].pData[tblReolution][2]; //factor
			distAtten = pTblCeqSDAtten[MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1].distance;
			eqStepAttenNum = 0xFF; //max
		}
		else if(pTblCeqSDAtten[0].pData[tblReolution][0]> pstDetGainStatus->atten2) //check min
		{
			vadcEqBandAtten = pTblCeqSDAtten[0].pData[tblReolution][2]; //factor
			distAtten = pTblCeqSDAtten[0].distance;
			eqStepAttenNum = 0; //min
		}
		else
		{
			for(step = 0; step < MAX_PVIRX_CEQ_ATTEN_TBL_NUM; step++)
			{
				if( (pTblCeqSDAtten[step].pData[tblReolution][0]<= pstDetGainStatus->atten2) && (pstDetGainStatus->atten2 <= pTblCeqSDAtten[step].pData[tblReolution][1]) )
				{
					vadcEqBandAtten = pTblCeqSDAtten[step].pData[tblReolution][2]; //factor
					distAtten = pTblCeqSDAtten[step].distance;
					eqStepAttenNum = step;
					break;
				}
			}
		}
	}/*}}}*/
	else
	{/*{{{*/
		//Get comp value.
		if(pTblCeqHDComp[MAX_PVIRX_CEQ_COMP_TBL_NUM-1].pData[tblReolution][1] < u16Comp) //check max
		{
			vadcEqBandComp = pTblCeqHDComp[MAX_PVIRX_CEQ_COMP_TBL_NUM-1].pData[tblReolution][2]; //factor
			distComp = pTblCeqHDComp[MAX_PVIRX_CEQ_COMP_TBL_NUM-1].distance;
			eqStepCompNum = 0xFF; //max
		}
		else if(pTblCeqHDComp[0].pData[tblReolution][0]> u16Comp) //check min
		{
			vadcEqBandComp = pTblCeqHDComp[0].pData[tblReolution][2]; //factor
			distComp = pTblCeqHDComp[0].distance;
			eqStepCompNum = 0; //min
		}
		else
		{
			for(step = 0; step < MAX_PVIRX_CEQ_COMP_TBL_NUM; step++)
			{
				if( (pTblCeqHDComp[step].pData[tblReolution][0]<= u16Comp) && (u16Comp <= pTblCeqHDComp[step].pData[tblReolution][1]) )
				{
					vadcEqBandComp = pTblCeqHDComp[step].pData[tblReolution][2]; //factor
					distComp = pTblCeqHDComp[step].distance;
					eqStepCompNum = step;
					break;
				}
			}
		}

		//Get atten value.
		if(pTblCeqHDAtten[MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1].pData[tblReolution][1] < pstDetGainStatus->atten2) //check max
		{
			vadcEqBandAtten = pTblCeqHDAtten[MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1].pData[tblReolution][2]; //factor
			distAtten = pTblCeqHDAtten[MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1].distance;
			eqStepAttenNum = 0xFF; //max
		}
		else if(pTblCeqHDAtten[0].pData[tblReolution][0]> pstDetGainStatus->atten2) //check min
		{
			vadcEqBandAtten = pTblCeqHDAtten[0].pData[tblReolution][2]; //factor
			distAtten = pTblCeqHDAtten[0].distance;
			eqStepAttenNum = 0; //min
		}
		else
		{
			for(step = 0; step < MAX_PVIRX_CEQ_ATTEN_TBL_NUM; step++)
			{
				if( (pTblCeqHDAtten[step].pData[tblReolution][0]<= pstDetGainStatus->atten2) && (pstDetGainStatus->atten2 <= pTblCeqHDAtten[step].pData[tblReolution][1]) )
				{
					vadcEqBandAtten = pTblCeqHDAtten[step].pData[tblReolution][2]; //factor
					distAtten = pTblCeqHDAtten[step].distance;
					eqStepAttenNum = step;
					break;
				}
			}
		}
    }/*}}}*/

    /* PI5008 case, don't care > 1000 */
	PrDbg("%x-Get measure Dist Comp:%d, Atten:%d\n", chanAddr COMMA distComp COMMA distAtten);
    if(distComp > 1200)
    {
        distComp = 1000;
    }

	pstMeasureEqInfo->vadcEqBandComp = vadcEqBandComp;
	pstMeasureEqInfo->distComp = distComp;
	pstMeasureEqInfo->eqStepCompNum = eqStepCompNum;

	pstMeasureEqInfo->vadcEqBandAtten = vadcEqBandAtten;
	pstMeasureEqInfo->distAtten = distAtten;
	pstMeasureEqInfo->eqStepAttenNum = eqStepAttenNum;

	if(distComp > 1200)
	{
		pstMeasureEqInfo->bTypeComp = 1;
		pstMeasureEqInfo->vadcEqBand = pstMeasureEqInfo->vadcEqBandComp;
		pstMeasureEqInfo->dist = pstMeasureEqInfo->distComp;
		pstMeasureEqInfo->eqStepNum = pstMeasureEqInfo->eqStepCompNum;
	}
	else
	{
		pstMeasureEqInfo->bTypeComp = 0;
		pstMeasureEqInfo->vadcEqBand = pstMeasureEqInfo->vadcEqBandAtten;
		pstMeasureEqInfo->dist = pstMeasureEqInfo->distAtten;
		pstMeasureEqInfo->eqStepNum = pstMeasureEqInfo->eqStepAttenNum;
	}

    pstMeasureEqInfo->manEqAcGainMd = 0;
    if( (cameraStandard == HDT_NEW) && (cameraResolution >= camera_1920x1080p30) )
    {
        if(distAtten < 800)
        {
            pstMeasureEqInfo->manEqAcGainMd = 0;
        }
        else if(distAtten < 900)
        {
            pstMeasureEqInfo->manEqAcGainMd = 2;
        }
        else if(distAtten < 1000)
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
        else
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
    }
    else if( (cameraStandard == CVI) && (cameraResolution >= camera_1920x1080p30) )
    {
        if(distAtten < 900)
        {
            pstMeasureEqInfo->manEqAcGainMd = 0;
        }
        else if(distAtten < 1000)
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
        else
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
    }
	result = eSUCCESS;
	PrPrint("%x-Get Comp(0x%x/%dm), Atten(0x%x/%dm), manEqAcGainMd(%d), result(vadcEqBand:0x%x/%dm)\n", chanAddr COMMA
			pstMeasureEqInfo->vadcEqBandComp COMMA pstMeasureEqInfo->distComp COMMA
			pstMeasureEqInfo->vadcEqBandAtten COMMA pstMeasureEqInfo->distAtten COMMA
			pstMeasureEqInfo->manEqAcGainMd COMMA
			pstMeasureEqInfo->vadcEqBand COMMA pstMeasureEqInfo->dist);

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_SetVADCGain(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, const _stPVIRX_DetGainStatus IN *pstDetGainStatus, _stPVIRX_MeasureEqInfo IN *pstMeasureEqInfo)
{
	PP_RESULT_E result = eERROR_FAILURE;

	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;

	PP_U16 dist = 0;
	PP_U8 vadcGain = 0;
	PP_U16 dcGain = 0; 
	PP_U8 gainfit = 0;
	PP_U8 y_out_gain = 0;

	PP_U8 step = 0;
	PP_S32 tblReolution = 0;
	_stPVIRX_Table_CEQ_VADC *pTblVadcHD = NULL;

	if( (pstVidStatusReg == NULL) || (pstDetGainStatus == NULL) || (pstMeasureEqInfo == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

#ifdef SUPPORT_AUTODETECT_PVI
    //get information by rejudge
    PPDRV_PVIRX_GetStdResol(cid, chanAddr, pstVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, NULL);
    PrDbg("%x-vadc cur[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
#else
    //read information by no rejudge.
    PPDRV_PVIRX_ReadStdResol(cid, chanAddr, pstVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
    PrDbg("%x-Fix vadc cur[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
#endif // SUPPORT_AUTODETECT_PVI

	dcGain = pstDetGainStatus->dcGain;

	pTblVadcHD = (_stPVIRX_Table_CEQ_VADC *)pvirx_ceq_table_vadc;
	if(cameraResolution > camera_1920x1080p25)
	{
		tblReolution = camera_1920x1080p25;
	}
	else
	{
		tblReolution = cameraResolution;
	}
	PrDbg("%x-tblReolution(%d)\n", chanAddr COMMA tblReolution);

	PrDbg("%x-distComp:%d\n", chanAddr COMMA pstMeasureEqInfo->distComp);
	if( (pstMeasureEqInfo->distComp > 1200) )
	{/*{{{*/
		dist = pstMeasureEqInfo->distComp;
		PrDbg("%x-dist(comp)%d\n", chanAddr COMMA dist);
		for(step = 0; step < MAX_PVIRX_CEQ_VADC_TBL_NUM-1; step++)
		{
			PrDbg("%x-dist(%d) step%d[%d-%d]\n", chanAddr COMMA dist COMMA step COMMA pTblVadcHD[step].distance COMMA pTblVadcHD[step].distance);
			if( (pTblVadcHD[step].distance <= dist) && (dist < pTblVadcHD[step+1].distance) )
			{
				vadcGain = pTblVadcHD[step].pData[tblReolution][0] & 0xFF;

				gainfit = (pTblVadcHD[step].pData[tblReolution][1]>>8) & 0xFF;
				y_out_gain = pTblVadcHD[step].pData[tblReolution][1] & 0xFF;
                		PrDbg("%x-find vadc param vadcGain:%02x\n", chanAddr COMMA vadcGain);
				break;
			}
		}
	}/*}}}*/
	else
	{/*{{{*/

		dist = pstMeasureEqInfo->distAtten;

		PrDbg("%x-dist(atten)%d\n", chanAddr COMMA dist);
		for(step = 0; step < MAX_PVIRX_CEQ_VADC_TBL_NUM-1; step++)
		{
			PrDbg("%x-dist(%d) step%d[%d-%d]\n", chanAddr COMMA dist COMMA step COMMA pTblVadcHD[step].distance COMMA pTblVadcHD[step].distance);
			if( (pTblVadcHD[step].distance <= dist) && (dist < pTblVadcHD[step+1].distance) )
			{
				if( dcGain <= 0x2E00) 
				{
					vadcGain = pTblVadcHD[step].pData[tblReolution][0] & 0xFF;
				}
				else 
				{
					vadcGain = (pTblVadcHD[step].pData[tblReolution][0]>>8) & 0xFF;
				}

				gainfit = (pTblVadcHD[step].pData[tblReolution][1]>>8) & 0xFF;
				y_out_gain = pTblVadcHD[step].pData[tblReolution][1] & 0xFF;
                PrDbg("%x-find vadc param vadcGain:%02x\n", chanAddr COMMA vadcGain);
				break;
			}
		}
	}/*}}}*/

	pstMeasureEqInfo->vadcGain = vadcGain;

    PrPrint("%x-Set Vadc:0x%x(detDcgain:0x%04x)]\n",chanAddr COMMA vadcGain COMMA dcGain);

    /* vadc_gain_sel */
	stReg.reg = 0x0011;
	u8RegMask = 0x70;
	u8RegData = (pstMeasureEqInfo->vadcGain&0x7)<<4;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid write\n");
		return(result);
	}
    /* man_eq_ac_gn */
	stReg.reg = 0x0012;
	u8RegMask = 0x1F;
	u8RegData = (pstMeasureEqInfo->manEqAcGainMd&0x1F);
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid write\n");
		return(result);
	}

	if(cameraStandard == CVI)
	{    
		PrDbg("%x-Special CVI mode. gainfit:0x%02x, youtgain:0x%02x\n", chanAddr COMMA gainfit COMMA y_out_gain);

		/* gainfit */
		stReg.reg = 0x0101;
		u8RegData = gainfit;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid write\n");
			return(result);
		}

		/* y_out_gain */
		stReg.reg = 0x0128;
		u8RegData = y_out_gain;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid write\n");
			return(result);
		}
	}

	return(result);
}


PP_RESULT_E PPDRV_PVIRX_SetEQGain(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, const _stPVIRX_DetGainStatus IN *pstDetGainStatus, _stPVIRX_MeasureEqInfo INOUT *pstMeasureEqInfo)
{
	PP_RESULT_E result = eERROR_FAILURE;

	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	PrDbg("%x-Set vadcEqBand:0x%x\n", chanAddr COMMA pstMeasureEqInfo->vadcEqBand);

	if( (pstVidStatusReg == NULL) || (pstDetGainStatus == NULL) || (pstMeasureEqInfo == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	/* write vadcEqBand */
	stReg.reg = 0x0014;
	u8RegMask = 0xF7;
	u8RegData = (pstMeasureEqInfo->vadcEqBand & 0x7) | (((pstMeasureEqInfo->vadcEqBand & 0x78)<<1)&0xF0);
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid write\n");
		return(result);
	}

	result = PPDRV_PVIRX_SetVADCGain(cid, chanAddr, pstVidStatusReg, pstDetGainStatus, pstMeasureEqInfo);
	return(result);
}

#if defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 

PP_RESULT_E PPDRV_PVIRX_GetTunnEQGain(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, const _stPVIRX_DetGainStatus IN *pstDetGainStatus, const _stPVIRX_TunnFactor IN *pstTunnFactor, _stPVIRX_MeasureEqInfo INOUT *pstMeasureEqInfo, PP_S32 OUT *ps32Ret)
{
	PP_RESULT_E result = eERROR_FAILURE;
	PP_S32 retValue = 0;

	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;
	PP_S32 reJudge = 0;

	PP_S32 tblReolution = 0;

	_stPVIRX_Table_CEQ_SDResolution *pTblCeqSDComp = NULL, *pTblCeqSDAtten = NULL;
	_stPVIRX_Table_CEQ_STD_HDResolution *pTblCeqHDComp = NULL, *pTblCeqHDAtten = NULL;
	PP_U8 vadcEqBandComp = 0, vadcEqBandAtten;
	PP_U16 distComp = 0, distAtten = 0;
	PP_U8 eqStepCompNum = 0, eqStepAttenNum = 0;
	PP_U8 bTypeComp = 0;

	if( (pstVidStatusReg == NULL) || (pstDetGainStatus == NULL) || (pstTunnFactor == NULL) || (pstMeasureEqInfo == NULL) || (ps32Ret == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	PPDRV_PVIRX_GetStdResol(cid, chanAddr, pstVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
	PrDbg("%x-TunnEQ [Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);

	/* Get EQ table */
	switch(cameraStandard)
	{/*{{{*/
		case CVBS:
			{
				pTblCeqSDComp = (_stPVIRX_Table_CEQ_SDResolution *)pvirx_ceq_table_estComplex_SD;
				pTblCeqSDAtten = (_stPVIRX_Table_CEQ_SDResolution *)pvirx_ceq_table_estAtten2_SD;
				if(cameraResolution == camera_ntsc) 
				{
					tblReolution = 0; //NTSC
				}
				else
				{
					tblReolution = 1; //PAL
				}
			}
			break;
#if defined(SUPPORT_PVIRX_STD_PVI)
		case PVI:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_PVI;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_PVI;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
		case HDA:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_HDA;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_HDA;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
		case CVI:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_CVI;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_CVI;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
		case HDT_OLD:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_HDT_OLD;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_HDT_OLD;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
		case HDT_NEW:
			{
				pTblCeqHDComp = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estComplex_HDT_NEW;
				pTblCeqHDAtten = (_stPVIRX_Table_CEQ_STD_HDResolution *)pvirx_ceq_std_table_estAtten2_HDT_NEW;
				tblReolution = cameraResolution - camera_1280x720p60;
				if(cameraResolution >= camera_1280x960p30)
				{
					/* use same table of 1280x720 ~ */
					tblReolution = cameraResolution - camera_1280x960p30;
				}
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
		default:
			{
				result = eERROR_FAILURE;
				PrError("Invalid camera Standard(%d)\n", cameraStandard);
				return(eERROR_FAILURE);
			}
	}/*}}}*/

	eqStepCompNum = pstMeasureEqInfo->eqStepCompNum;
	eqStepAttenNum = pstMeasureEqInfo->eqStepAttenNum;
	distComp = pstMeasureEqInfo->distComp;
	distAtten = pstMeasureEqInfo->distAtten;
	bTypeComp = pstMeasureEqInfo->bTypeComp;

	if(pstTunnFactor->stepDir > 0) //> 0:++, <0:--
	{
		if(bTypeComp)
		{
			if(eqStepCompNum < (MAX_PVIRX_CEQ_COMP_TBL_NUM-1)) 
			{ 
				eqStepCompNum++; 
			}
			else
			{
				result = eSUCCESS;
				retValue = 0; //max
				*ps32Ret = retValue;
				return(result);
			}
		}
		else
		{
			if(eqStepAttenNum < (MAX_PVIRX_CEQ_ATTEN_TBL_NUM-1)) 
			{ 
				eqStepAttenNum++; 
			}
			else
			{
				result = eSUCCESS;
				retValue = 0; //max
				*ps32Ret = retValue;
				return(result);
			}
		}
	}
	else
	{
		if(bTypeComp)
		{
			if(eqStepCompNum > 0) 
			{ 
				eqStepCompNum--; 
			}
			else
			{
				result = eSUCCESS;
				retValue = 0; //min
				*ps32Ret = retValue;
				return(result);
			}
		}
		else
		{
			if(eqStepAttenNum > 0) 
			{ 
				eqStepAttenNum--; 
			}
			else
			{
				result = eSUCCESS;
				retValue = 0; //min
				*ps32Ret = retValue;
				return(result);
			}
		}
	}

	/* Get Tunn EQ value */
	if(cameraStandard == CVBS)
	{/*{{{*/
		//Get comp value.
		vadcEqBandComp = pTblCeqSDComp[eqStepCompNum].pData[tblReolution][2]; //factor
		distComp = pTblCeqSDComp[eqStepCompNum].distance;

		//Get atten value.
		vadcEqBandAtten = pTblCeqSDAtten[eqStepAttenNum].pData[tblReolution][2]; //factor
		distAtten = pTblCeqSDAtten[eqStepAttenNum].distance;
	}/*}}}*/
	else
	{/*{{{*/
		//Get comp value.
		vadcEqBandComp = pTblCeqHDComp[eqStepCompNum].pData[tblReolution][2]; //factor
		distComp = pTblCeqHDComp[eqStepCompNum].distance;

		//Get atten value.
		vadcEqBandAtten = pTblCeqHDAtten[eqStepAttenNum].pData[tblReolution][2]; //factor
		distAtten = pTblCeqHDAtten[eqStepAttenNum].distance;
	}/*}}}*/

	pstMeasureEqInfo->vadcEqBandComp = vadcEqBandComp;
	pstMeasureEqInfo->distComp = distComp;
	pstMeasureEqInfo->eqStepCompNum = eqStepCompNum;

	pstMeasureEqInfo->vadcEqBandAtten = vadcEqBandAtten;
	pstMeasureEqInfo->distAtten = distAtten;
	pstMeasureEqInfo->eqStepAttenNum = eqStepAttenNum;

	if(bTypeComp)
	{
		pstMeasureEqInfo->vadcEqBand = pstMeasureEqInfo->vadcEqBandComp;
		pstMeasureEqInfo->dist = pstMeasureEqInfo->distComp;
		pstMeasureEqInfo->eqStepNum = pstMeasureEqInfo->eqStepCompNum;
	}
	else
	{
		pstMeasureEqInfo->vadcEqBand = pstMeasureEqInfo->vadcEqBandAtten;
		pstMeasureEqInfo->dist = pstMeasureEqInfo->distAtten;
		pstMeasureEqInfo->eqStepNum = pstMeasureEqInfo->eqStepAttenNum;
	}
    pstMeasureEqInfo->manEqAcGainMd = 0;
    if( (cameraStandard == HDT_NEW) && (cameraResolution >= camera_1920x1080p30) )
    {
        if(distAtten < 800)
        {
            pstMeasureEqInfo->manEqAcGainMd = 0;
        }
        else if(distAtten < 900)
        {
            pstMeasureEqInfo->manEqAcGainMd = 2;
        }
        else if(distAtten < 1000)
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
        else
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
    }
    else if( (cameraStandard == CVI) && (cameraResolution >= camera_1920x1080p30) )
    {
        if(distAtten < 900)
        {
            pstMeasureEqInfo->manEqAcGainMd = 0;
        }
        else if(distAtten < 1000)
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
        else
        {
            pstMeasureEqInfo->manEqAcGainMd = 4;
        }
    }
    result = eSUCCESS;
    retValue = 1;
    *ps32Ret = retValue;

	PrDbg("%x-Tunn Get(dir:%d, TypeComp:%d) Comp(0x%x/%d), Atten(0x%x/%d), manEqAcGainMd(%d), result(0x%x/%d)\n", chanAddr COMMA
			pstTunnFactor->stepDir COMMA pstMeasureEqInfo->bTypeComp COMMA
			pstMeasureEqInfo->vadcEqBandComp COMMA pstMeasureEqInfo->distComp COMMA
			pstMeasureEqInfo->vadcEqBandAtten COMMA pstMeasureEqInfo->distAtten COMMA
			pstMeasureEqInfo->manEqAcGainMd COMMA
			pstMeasureEqInfo->vadcEqBand COMMA pstMeasureEqInfo->dist);

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_SetTunnVADCGain(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, const _stPVIRX_DetGainStatus IN *pstDetGainStatus, _stPVIRX_TunnFactor IN *pstTunnFactor, _stPVIRX_MeasureEqInfo OUT *pstMeasureEqInfo, PP_S32 OUT *ps32Ret)
{
	PP_RESULT_E result = eERROR_FAILURE;
	PP_S32 retValue = 0;

	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	PP_U8 vadcGain = 0;

	if( (pstVidStatusReg == NULL) || (pstDetGainStatus == NULL) || (pstTunnFactor == NULL) || (pstMeasureEqInfo == NULL) || (ps32Ret == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	/* vadc_gain_sel */
	stReg.reg = 0x0011;
	u8RegMask = 0x70;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	vadcGain = (u8RegData>>4)&0x7;

	if(pstTunnFactor->stepDir > 0) //> 0:++, <0:--
	{
		if(vadcGain < 0x7) 
		{
			vadcGain++;
		}
		else
		{
            /* max. ignore tunning */
            result = eSUCCESS;
            retValue = 0;
            *ps32Ret = retValue;
            return(result);
        }
	}
	else
	{
		if(vadcGain > 0) 
		{
			vadcGain--;
		}
		else
		{
			/* min. ignore tunning */
            result = eSUCCESS;
            retValue = 0;
            *ps32Ret = retValue;
            return(result);
		}
	}

	pstMeasureEqInfo->vadcGain = vadcGain;

        PrDbg("%x-Set Tunn Vadc:0x%x\n",chanAddr COMMA pstMeasureEqInfo->vadcGain);

	/* vadc_gain_sel */
	stReg.reg = 0x0011;
	u8RegMask = 0x70;
	u8RegData = (pstMeasureEqInfo->vadcGain&0x7)<<4;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid write\n");
		return(result);
	}

    result = eSUCCESS;
    retValue = 1;
    *ps32Ret = retValue;
    return(result);
}

PP_RESULT_E PPDRV_PVIRX_SetTunnEQBand(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, const _stPVIRX_DetGainStatus IN *pstDetGainStatus, _stPVIRX_TunnFactor IN *pstTunnFactor,_stPVIRX_MeasureEqInfo OUT *pstMeasureEqInfo, PP_S32 OUT *ps32Ret)
{
	PP_RESULT_E result = eERROR_FAILURE;
	PP_S32 retValue = 0;

	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	PP_U8 vadcEqBand = 0;

	if( (pstVidStatusReg == NULL) || (pstDetGainStatus == NULL) || (pstTunnFactor == NULL) || (pstMeasureEqInfo == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	/* read vadcEqBand */
	stReg.reg = 0x0014;
	u8RegMask = 0xF7;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid write\n");
		return(result);
	}
	vadcEqBand = (u8RegData & 0x7) | ((u8RegData & 0xF0)>>1);

	if(pstTunnFactor->stepDir > 0) //> 0:++, <0:--
	{
		if(vadcEqBand < 0x7F) 
		{
			vadcEqBand++;
		}
		else
		{
			/* max. ignore tunning */
            result = eSUCCESS;
            retValue = 0;
            *ps32Ret = retValue;
            return(result);
        }
	}
	else
	{
		if(vadcEqBand > 0) 
		{
			vadcEqBand--;
		}
		else
		{
			/* min. ignore tunning */
            result = eSUCCESS;
            retValue = 0;
            *ps32Ret = retValue;
            return(result);
		}

	}


	pstMeasureEqInfo->vadcEqBand = vadcEqBand;

	PrDbg("%x-Set Tunn vadcEqBand:0x%x\n", chanAddr COMMA pstMeasureEqInfo->vadcEqBand);

	/* write vadcEqBand */
	stReg.reg = 0x0014;
	u8RegMask = 0xF7;
	u8RegData = (vadcEqBand & 0x7) | (((vadcEqBand & 0x78)<<1)&0xF0);
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid write\n");
		return(result);
	}

    result = eSUCCESS;
    retValue = 1;
    *ps32Ret = retValue;
    return(result);
}

#endif //defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 

static PP_RESULT_E PVIRX_SetTblBrief(const PP_S32 cid, const PP_U8 chanAddr, const enum _eCameraStandard cameraStandard, const enum _eVideoResolution videoResolution, const PP_U8 regMode, _stJobProc *pstJobProc)
{
	PP_RESULT_E result = eERROR_FAILURE;

	PP_S32 regInx = 0;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData = 0;
	PP_U8 u8RegMask;

	const _stPVIRX_Table_STD_HDResolution *pTblSTDHDResolution = NULL;
	const _stPVIRX_Table_EXT_HDResolution *pTblEXTHDResolution = NULL;
	const _stPVIRX_Table_SDResolution *pTblSDResolution = NULL;

	PrDbg("%x-SetTblBrief\n", chanAddr);

	pstJobProc->defChromaPhase = 0;
	regInx = 0;

	if(videoResolution > video_1920x1080p25)
	{/*{{{*/
		switch(cameraStandard)
		{
			case CVBS:
				{
					pTblSDResolution = (const _stPVIRX_Table_SDResolution *)stPVIRX_Table_SDResol;
				}
				break;
#if defined(SUPPORT_PVIRX_STD_PVI)
			case PVI:
				{
					pTblEXTHDResolution = (const _stPVIRX_Table_EXT_HDResolution *)stPVIRX_Table_EXT_PVIResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
			case HDA:
				{
					pTblEXTHDResolution = (const _stPVIRX_Table_EXT_HDResolution *)stPVIRX_Table_EXT_HDAResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
			case CVI:
				{
					pTblEXTHDResolution = (const _stPVIRX_Table_EXT_HDResolution *)stPVIRX_Table_EXT_CVIResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
			case HDT_OLD:
				{
					pTblEXTHDResolution = (const _stPVIRX_Table_EXT_HDResolution *)stPVIRX_Table_EXT_HDT_OLDResol;
				}
				break;
			case HDT_NEW:
				{
					pTblEXTHDResolution = (const _stPVIRX_Table_EXT_HDResolution *)stPVIRX_Table_EXT_HDT_NEWResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
			default:
				{
					PrError("Invalid camera Standard(%d)\n", cameraStandard);
					return(eERROR_FAILURE);
				}
		}
	}/*}}}*/
	else
	{/*{{{*/
		switch(cameraStandard)
		{
			case CVBS:
				{
					pTblSDResolution = (const _stPVIRX_Table_SDResolution *)stPVIRX_Table_SDResol;
				}
				break;
#if defined(SUPPORT_PVIRX_STD_PVI)
			case PVI:
				{
					pTblSTDHDResolution = (const _stPVIRX_Table_STD_HDResolution *)stPVIRX_Table_STD_PVIResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
			case HDA:
				{
					pTblSTDHDResolution = (const _stPVIRX_Table_STD_HDResolution *)stPVIRX_Table_STD_HDAResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
			case CVI:
				{
					pTblSTDHDResolution = (const _stPVIRX_Table_STD_HDResolution *)stPVIRX_Table_STD_CVIResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
			case HDT_OLD:
				{
					pTblSTDHDResolution = (const _stPVIRX_Table_STD_HDResolution *)stPVIRX_Table_STD_HDT_OLDResol;
				}
				break;
			case HDT_NEW:
				{
					pTblSTDHDResolution = (const _stPVIRX_Table_STD_HDResolution *)stPVIRX_Table_STD_HDT_NEWResol;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
			default:
				{
					PrError("Invalid camera Standard(%d)\n", cameraStandard);
					return(eERROR_FAILURE);
				}
		}
	}/*}}}*/

	if( (videoResolution <= video_960x576i50) )
	{
		stReg.reg = pTblSDResolution[regInx].stReg.reg;
	}
	else if( (video_1280x720p60 <= videoResolution) && (videoResolution <= video_1920x1080p25) )
	{
		stReg.reg = pTblSTDHDResolution[regInx].stReg.reg;
	}
	else if( (video_1280x960p30 <= videoResolution) && (videoResolution <= video_1280x960p25) )
	{
		stReg.reg = pTblEXTHDResolution[regInx].stReg.reg;
	}

	while(stReg.reg != 0xFFFF)
	{
		if( (videoResolution <= video_960x576i50) )
		{
			u8RegData = pTblSDResolution[regInx].u8Data[videoResolution];
		}
		else if( (video_1280x720p60 <= videoResolution) && (videoResolution <= video_1920x1080p25) )
		{
			u8RegData = pTblSTDHDResolution[regInx].u8Data[videoResolution-video_1280x720p60]; 
		}
		else if( (video_1280x960p30 <= videoResolution) && (videoResolution <= video_1280x960p25) )
		{
			u8RegData = pTblEXTHDResolution[regInx].u8Data[videoResolution-video_1280x960p30]; 
		}

		/* keep C phase ref */
		if(stReg.reg == 0x0146) 
		{
			pstJobProc->defChromaPhase |= (PP_U32)(u8RegData & 0x1F)<<16;
		}
		else if(stReg.reg == 0x0147) 
		{
			pstJobProc->defChromaPhase |= (PP_U32)(u8RegData & 0xFF)<<8;
		}
		else if(stReg.reg == 0x0148)
		{
			pstJobProc->defChromaPhase |= (PP_U32)(u8RegData & 0xFF);
		}

		if(regMode == 0) //Set only StResol, EqGain, CPhaseRef(keep)...
		{/*{{{*/
			if( ((0x0010 <= stReg.reg) && (stReg.reg <= 0x0014)) || /* StResol, EQ gain */
					((0x00E0 == stReg.reg)) ||
					((0x0111 <= stReg.reg) && (stReg.reg <= 0x011F)) || 
					((0x0131 <= stReg.reg) && (stReg.reg <= 0x0138)) || 
					((0x013A == stReg.reg)) ||
					((0x013D <= stReg.reg) && (stReg.reg <= 0x014C))
			  )
			{

				if(stReg.reg == 0x00E0)
				{
					u8RegMask = 0x0F;
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}
				else
				{
					PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}
			}
		}/*}}}*/
		else if(regMode == 1) // Set temporary register set. for checking det_chroma.
		{/*{{{*/
			if( ((0x0117 <= stReg.reg) && (stReg.reg <= 0x011E)) ||
					((0x0146 <= stReg.reg) && (stReg.reg <= 0x0148))
			  )
			{
				// cameraStandard == HDA) && (videoResolution == video_1280x720p60/p50)
#if 0
				if(cameraStandard == HDA)
				{/*{{{*/
					if( videoResolution == video_1280x720p60 )
					{
						if(0x0146 == stReg.reg) 
						{
							u8RegData = 0x64;
						}
						else if(0x0147 == stReg.reg) 
						{
							u8RegData = 0xEF;
						}
						else if(0x0148 == stReg.reg) 
						{
							u8RegData = 0x8E;
						}
					}
					else if( videoResolution == video_1280x720p50 )
					{
						if(0x0146 == stReg.reg) 
						{
							u8RegData = 0x64;
						}
						else if(0x0147 == stReg.reg) 
						{
							u8RegData = 0xEF;
						}
						else if(0x0148 == stReg.reg) 
						{
							u8RegData = 0xF8;
						}

					}
				}/*}}}*/
#else //new camera                    
				if(cameraStandard == HDA)
				{/*{{{*/
					if( videoResolution == video_1280x720p60 )  //0x14B17E
					{
						if(0x0146 == stReg.reg) 
						{
			                u8RegData = (((0x14B17E>>1)>>16)&0x1F);
						}
						else if(0x0147 == stReg.reg) 
						{
			                u8RegData = (((0x14B17E>>1)>>8)&0xFF);
						}
						else if(0x0148 == stReg.reg) 
						{
			                u8RegData = ((0x14B17E>>1)&0xFF);
						}
					}
                    else if( videoResolution == video_1280x720p50 ) //0x14B17E
					{
						if(0x0146 == stReg.reg) 
						{
			                u8RegData = (((0x14B17E>>1)>>16)&0x1F);
						}
						else if(0x0147 == stReg.reg) 
						{
			                u8RegData = (((0x14B17E>>1)>>8)&0xFF);
						}
						else if(0x0148 == stReg.reg) 
						{
			                u8RegData = ((0x14B17E>>1)&0xFF);
						}
					}
				}/*}}}*/
#endif
	
				if(0x0146 == stReg.reg) 
				{
					u8RegData |= 0x20;
				}
				PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}
		}/*}}}*/
		else if(regMode == 2) // When temporary register set for checking det_chroma, change forcely hdelay for normal view. 
		{/*{{{*/
			if( ((0x0111 == stReg.reg) || (stReg.reg == 0x0113)) ||
					((0x0129 <= stReg.reg) && (stReg.reg <= 0x012A))
			  )
			{
				PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}
		}/*}}}*/

		regInx++;

		if( (videoResolution <= video_960x576i50) )
		{
			stReg.reg = pTblSDResolution[regInx].stReg.reg;
		}
		else if( (video_1280x720p60 <= videoResolution) && (videoResolution <= video_1920x1080p25) )
		{
			stReg.reg = pTblSTDHDResolution[regInx].stReg.reg;
		}
		else if( (video_1280x960p30 <= videoResolution) && (videoResolution <= video_1280x960p25) )
		{
			stReg.reg = pTblEXTHDResolution[regInx].stReg.reg;
		}

	}
	/* Apply Set register */
	{/*{{{*/

		stReg.reg = 0x0154;
		u8RegData = 0x0E;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid write\n");
			return(result);
		}

		stReg.reg = 0x0154;
		u8RegData = 0x0F;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid write\n");
			return(result);
		}
	}/*}}}*/


	return(result);
}

PP_RESULT_E PPDRV_PVIRX_ProcessCameraPlugIn(void INOUT *pParam1,  uint32_t IN u32Param2)
{
    PP_S32 cid;
    PP_U8 chanAddr;
	PP_RESULT_E result = eERROR_FAILURE;

    _stPlugInParam *pstPlugInParam = (_stPlugInParam *)pParam1;
	_PviRxDrvHost *pHost = (_PviRxDrvHost *)pstPlugInParam->pHost;
	_stPVIRX_Isr *pstIsr = (_stPVIRX_Isr *)&pHost->stPVIRX_Isr;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegMask;
	PP_U8 u8RegData;
#ifdef SUPPORT_PVIRX_CEQ_PLUGIN
	PP_S32 s32Ret = 0;
#endif // SUPPORT_PVIRX_CEQ_PLUGIN
	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;
	PP_S32 reJudge = 0;
	_stPVIRX_VidStatusReg curStVidStatusReg;
	_stPVIRX_MeasureEqInfo stMeasureEqInfo;
	PP_U32 tunnChromaPhase = 0;
	PP_S32 bWaitStableStatus = 1;

	if( (pParam1 == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

    cid = pstPlugInParam->cid;
    chanAddr = pstPlugInParam->chanAddr;
	PrDbg("%x-numJobStep:%d\n", chanAddr COMMA pstIsr->stJobProc.numJobStep);

	if( pstIsr->stJobProc.numJobStep == 0 )
	{/*{{{*/
		PrErrorString("Invalid numJobStep\n");
		return(eERROR_FAILURE);
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 1 ) //Read std/resol, write man format
	{/*{{{*/
		PrDbg("%x-Read std/resol, write man format\n", chanAddr);

#ifdef SUPPORT_AUTODETECT_PVI
        PPDRV_PVIRX_ReadStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
        PrDbg("%x-Read [Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
#else
        switch( DEFAULT_CAMERA_STANDARD )
        {/*{{{*/
            case CVBS:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_PVI; //assume PVI.
                }
                break;
            case PVI:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_PVI;
                }
                break;
            case CVI:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;
                }
                break;
            case HDA:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_HDA;
                }
                break;
            case HDT_OLD:
            case HDT_NEW:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_HDT;
                }
                break;
            default:
                {
                    PrErrorString("Unknown camera standard\n");
                }
                break;
        }/*}}}*/
        switch( DEFAULT_CAMERA_RESOLUTION )
        {/*{{{*/
            case camera_ntsc:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_SD480i;
                }
                break;
            case camera_pal:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_SD576i;
                }
                break;
            case camera_1280x720p60:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_60Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD720p;
                }
                break;
            case camera_1280x720p50:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_50Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD720p;
                }
                break;
            case camera_1280x720p30:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_30Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD720p;
                }
                break;
            case camera_1280x720p25:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD720p;
                }
                break;
            case camera_1920x1080p30:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_30Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD1080p;
                }
                break;
            case camera_1920x1080p25:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD1080p;
                }
                break;
            case camera_1280x960p30:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_30Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD960p;
                }
                break;
            case camera_1280x960p25:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD960p;
                }
                break;
            default:
                {
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
                    pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD720p;
                }
                break;
        }/*}}}*/
        PPDRV_PVIRX_ReadStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
        PrDbg("%x-Fix [Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
#endif // SUPPORT_AUTODETECT_PVI

		PrDbg("%x-Set only StResol, EqGain, CPhaseRef(keep)...\n", chanAddr);

		//Set only StResol, EqGain, CPhaseRef(keep)...
		if( (result = PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 0, &pstIsr->stJobProc)) != eSUCCESS)
		{
			PrErrorString("Can't set table brief\n");
			return(result);
		}

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		memset(&pstIsr->stJobProc.stMeasureEqInfo, 0, sizeof(_stPVIRX_MeasureEqInfo));
		pstIsr->stJobProc.numJobStep = 2;

		result = eSUCCESS;
		return(result);
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 2 ) //Read gain & Set.
	{/*{{{*/
		PrDbg("%x-Read gain & Set.\n", chanAddr);
		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		memcpy(&pstIsr->stJobProc.stFirstDetGainStatus, &pstIsr->stJobProc.stDetGainStatus, sizeof(_stPVIRX_DetGainStatus));
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &stMeasureEqInfo);

		PrDbg("%x-Get Comp(0x%x/%dm), Atten(0x%x/%dm), select->(0x%x/%dm)\n", chanAddr COMMA
				stMeasureEqInfo.vadcEqBandComp COMMA stMeasureEqInfo.distComp COMMA
				stMeasureEqInfo.vadcEqBandAtten COMMA stMeasureEqInfo.distAtten COMMA
				stMeasureEqInfo.vadcEqBand COMMA stMeasureEqInfo.dist);
		PrDbg("%x- First [syncLevel:%02x, dc:%04x, ac:%04x, comp[0x%x(%04x, %04x)], atten[%04x, %04x]]\n", chanAddr COMMA 
			pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA 
			pstIsr->stJobProc.stDetGainStatus.dcGain COMMA 
			pstIsr->stJobProc.stDetGainStatus.acGain COMMA 
			((pstIsr->stJobProc.stDetGainStatus.comp2 != 0) ? PVIRX_CEQ_EST_COMP(pstIsr->stJobProc.stDetGainStatus.comp1, pstIsr->stJobProc.stDetGainStatus.comp2) : 0) COMMA
			pstIsr->stJobProc.stDetGainStatus.comp1 COMMA 
			pstIsr->stJobProc.stDetGainStatus.comp2 COMMA 
			pstIsr->stJobProc.stDetGainStatus.atten1 COMMA 
			pstIsr->stJobProc.stDetGainStatus.atten2);

		PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &stMeasureEqInfo);
		memcpy(&pstIsr->stJobProc.stMeasureEqInfo, &stMeasureEqInfo, sizeof(_stPVIRX_MeasureEqInfo));

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
        pstIsr->stJobProc.cntWait300MsecTime = 0;
#ifdef SUPPORT_AUTODETECT_PVI
        pstIsr->stJobProc.numJobStep = 3;
#else
        pstIsr->stJobProc.numJobStep = 40;
#endif // SUPPORT_AUTODETECT_PVI
        pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 3 ) //Read std/resol 
	{/*{{{*/
		PrDbg("%x-Read std/resol, & lock\n", chanAddr);
		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);

		if( ((pstIsr->stJobProc.stVidStatusReg.reg[0]&0x07) == (curStVidStatusReg.reg[0]&0x07)) &&
				(((pstIsr->stJobProc.stVidStatusReg.reg[0]&0x07) == 0) || ((pstIsr->stJobProc.stVidStatusReg.reg[0]&0x07) == 1))) //check CVBS
		{
			if( ((curStVidStatusReg.b.lock_gain) && (curStVidStatusReg.b.lock_clamp) && (curStVidStatusReg.b.lock_hperiod)) &&
					((pstIsr->stJobProc.stVidStatusReg.reg[0]&0x07) == (curStVidStatusReg.reg[0]&0x07)) ) //same std/resol ?
			{/*{{{*/
				/* save cur video status registers */
				memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 4;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
				//go directly next step. no return.
                //return(result);
			}/*}}}*/
			else
			{/*{{{*/
				if(pstIsr->stJobProc.cntJobTry++ >= 4)
				{
					PrErrorString("Can't get stable std/resol\n");
					PrDbg("%x-Forcely Set abnormal [CVI 1080p25]\n", chanAddr);
					pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;
					pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
					pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD1080p;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 4;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
				else
				{
					/* check standard changed ? */
					PrError("%x-Changed std/resol, recheck.\n", chanAddr);
					PrDbg("%x-Old VidStatus:0x%02x%02x%02x\n", chanAddr COMMA pstIsr->stJobProc.stVidStatusReg.reg[0] COMMA pstIsr->stJobProc.stVidStatusReg.reg[1] COMMA pstIsr->stJobProc.stVidStatusReg.reg[2]); 
					PrDbg("%x-Cur VidStatus:0x%02x%02x%02x\n", chanAddr COMMA curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1] COMMA curStVidStatusReg.reg[2]); 

					/* save cur video status registers */
					memcpy(&pstIsr->stJobProc.stStartVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
					memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 1;

                    result = eSUCCESS;
                    return(result);
                }
            }/*}}}*/
		}
		else
		{
			if( ((curStVidStatusReg.b.lock_gain) && (curStVidStatusReg.b.lock_clamp) && (curStVidStatusReg.b.lock_hperiod)) &&
					((pstIsr->stJobProc.stVidStatusReg.reg[0]&0xFF) == (curStVidStatusReg.reg[0]&0xFF)) ) //same std/resol ?
			{/*{{{*/
				/* save cur video status registers */
				memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 4;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}/*}}}*/
			else
			{/*{{{*/
				if(pstIsr->stJobProc.cntJobTry++ >= 4)
				{
					PrErrorString("Can't get stable std/resol\n");
					PrDbg("%x-Forcely Set abnormal [CVI 1080p25]\n", chanAddr);
					pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;
					pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
					pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD1080p;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 4;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
                else
				{
					/* check standard changed ? */
					PrError("%x-Changed std/resol, recheck.\n", chanAddr);
					PrDbg("%x-Old VidStatus:0x%02x%02x%02x\n", chanAddr COMMA pstIsr->stJobProc.stVidStatusReg.reg[0] COMMA pstIsr->stJobProc.stVidStatusReg.reg[1] COMMA pstIsr->stJobProc.stVidStatusReg.reg[2]); 
					PrDbg("%x-Cur VidStatus:0x%02x%02x%02x\n", chanAddr COMMA curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1] COMMA curStVidStatusReg.reg[2]); 

					/* save cur video status registers */
					memcpy(&pstIsr->stJobProc.stStartVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
					memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 1;

                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
		}

	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 4 ) //If HDA 720p, check is HDA or CVI 720p by chroma lock ?
	{/*{{{*/
		PrDbg("%x-Check HDA or CVI 720p30/25 ?\n", chanAddr);

		PPDRV_PVIRX_ReadStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
		PrDbg("%x-Cur [Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);

		if( ((cameraStandard == HDA)) && 
			((cameraResolution == camera_1280x720p60) || (cameraResolution == camera_1280x720p50) ||
			(cameraResolution == camera_1280x720p30) || (cameraResolution == camera_1280x720p25)) )
		{/*{{{*/
			if( (pstIsr->stJobProc.stVidStatusReg.b.det_chroma) )
			{/*{{{*/
				if( (pstIsr->stJobProc.stVidStatusReg.b.lock_chroma) )
				{
					PrDbg("%x-find valid format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA pstIsr->stJobProc.reJudgeStdResol);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 10;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    return(result);
                }
				else
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 5;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
			}/*}}}*/
			else
			{/*{{{*/
				if(pstIsr->stJobProc.cntJobTry++ >= 4)
				{
					PrDbg("%x-!det_chroma -> Assume CVI.\n", chanAddr);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 30;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
				else
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 4; //loop

                    result = eSUCCESS;
                    return(result);
                }

			}/*}}}*/
		}/*}}}*/
		else
		{/*{{{*/
			pstIsr->stJobProc.reJudgeStdResol = 0;
			PrDbg("%x-current format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA pstIsr->stJobProc.reJudgeStdResol);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 40;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
			//return(result);
		}/*}}}*/

	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 5 ) //HDA720p C_det & !C_lock check
	{/*{{{*/
		PrDbg("%x-HDA720p C_det & !C_lock check. tunn c_lock.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);
			tunnChromaPhase = pstIsr->stJobProc.defChromaPhase;

			stReg.reg = 0x0146;
			u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x00; //0x00: set auto 
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0147;
			u8RegData = (tunnChromaPhase>>8)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0148;
			u8RegData = (tunnChromaPhase)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

			/* Apply Set register */
			{/*{{{*/

				stReg.reg = 0x0154;
				u8RegData = 0x0E;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				stReg.reg = 0x0154;
				u8RegData = 0x0F;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			PrDbg("%x-Next Check CVI720p C_det & !C_lock check\n", chanAddr);

			/* forcely set to cvi */
			{/*{{{*/
				PrDbg("%x-Forcely Set (CVI)\n", chanAddr);
				pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;

				PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
				reJudge = 1;
				pstIsr->stJobProc.reJudgeStdResol = reJudge;

				PrDbg("%x-Set only StResol, EqGain, CPhaseRef(keep)...\n", chanAddr);

				//Set only StResol, EqGain, CPhaseRef(keep)...
				if( (result = PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 0, &pstIsr->stJobProc)) != eSUCCESS)
				{
					PrErrorString("Can't set table brief\n");
					return(result);
				}

			}/*}}}*/

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 6;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		} /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 10;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 5;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
		}
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 6 ) //CVI720p C_det & !C_lock check
	{/*{{{*/
		PrDbg("%x-CVI720p C_det & !C_lock check. tunn c_lock.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);
			tunnChromaPhase = pstIsr->stJobProc.defChromaPhase;

			stReg.reg = 0x0146;
			u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x00; //0x00: set auto 
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0147;
			u8RegData = (tunnChromaPhase>>8)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0148;
			u8RegData = (tunnChromaPhase)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

			/* Apply Set register */
			{/*{{{*/

				stReg.reg = 0x0154;
				u8RegData = 0x0E;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				stReg.reg = 0x0154;
				u8RegData = 0x0F;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 30;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
        } /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 20;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 6;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 10 ) //EQ Fine Tunning-1 (from numJob(5) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_PLUGIN
		PrDbg("%x-EQ Fine Tunning-1 from numJob(5)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 10;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 11;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
				else
				{

                    result = eSUCCESS;
                    return(result);
                }
			}
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 11;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}
		}/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 10;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 11;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 11;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(5)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 11;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
            //return(result);
		}/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;
		pstIsr->stJobProc.C_LOCK_CNT = 1;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 40;
		pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);
#endif
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 11 ) //EQ Fine Tunning-2 (from numJob(5) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_PLUGIN
		PrDbg("%x-EQ Fine Tunning-2 from numJob(5)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 11;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 40;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 40;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else 
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 11;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 40;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        //go directly next step. no return.
						//return(result);
					}
					else
					{

                        result = eSUCCESS;
                        return(result);
                    }
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 40;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
			}/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(5)\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 40;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;
		pstIsr->stJobProc.C_LOCK_CNT = 1;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 40;
		pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);

#endif
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 20 ) //EQ Fine Tunning-1 (from numJob(6) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_PLUGIN
		PrDbg("%x-EQ Fine Tunning-1 from numJob(6)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 20;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 21;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{

                    result = eSUCCESS;
                    return(result);
				}
			}
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 21;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
        }/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 20;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 21;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 21;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(6)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 21;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
            //return(result);
        }/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;
		pstIsr->stJobProc.C_LOCK_CNT = 1;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 40;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 21 ) //EQ Fine Tunning-2 (from numJob(6) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_PLUGIN
		PrDbg("%x-EQ Fine Tunning-2 from numJob(6)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 21;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 40;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 40;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
		}/*}}}*/
		else
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 21;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 40;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        //go directly next step. no return.
                        //return(result);
					}
					else
					{
                        result = eSUCCESS;
                        return(result);
                    }
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 40;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
			}/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(6)\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 40;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;
		pstIsr->stJobProc.C_LOCK_CNT = 1;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 40;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 30 ) //Abnormal case
	{/*{{{*/
		PrDbg("%x-Abnormal case\n", chanAddr);

		PrDbg("%x-Forcely Set abnormal STD(CVI)\n", chanAddr);
		pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;

		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		reJudge = 1;
		pstIsr->stJobProc.reJudgeStdResol = reJudge;

		PrDbg("%x-format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA reJudge);

		PrDbg("%x-Set only StResol, EqGain, CPhaseRef...\n", chanAddr);

		//Set only StResol, EqGain, CPhaseRef(keep)...
		if( (result = PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 0, &pstIsr->stJobProc)) != eSUCCESS)
		{
			PrErrorString("Can't set table brief\n");
			return(result);
		}

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 40;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 40 ) //inital dump register.
	{/*{{{*/
#ifdef SUPPORT_AUTODETECT_PVI
        if( PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge) != eSUCCESS)
        {
            PrError("%x-Invalid StdResol. Recheck.\n", chanAddr);

            /* initialize argument */
            memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));
            /* save cur video status registers */
            PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
            memcpy(&pstIsr->stJobProc.stStartVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
            memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));

            /* set start camera in flag */
            _SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

            /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
            pstIsr->stJobProc.cntWait300MsecTime = 0;
            pstIsr->stJobProc.numJobStep = 1;

            result = eSUCCESS;
            return(result);
        }

        if(pstIsr->stJobProc.reJudgeStdResol == 0)
        {
            pstIsr->stJobProc.reJudgeStdResol = reJudge;
        }
        PrDbg("%x-Set final vdec format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA pstIsr->stJobProc.reJudgeStdResol);
#else
        PPDRV_PVIRX_ReadStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
        pstIsr->stJobProc.reJudgeStdResol = TRUE;
        /* check hdt - 720p old/new standard */
        if( (cameraStandard == HDT_NEW) && ((cameraResolution == camera_1280x720p25) || (cameraResolution == camera_1280x720p30)) )
        {
            if( (pstIsr->stJobProc.stVidStatusReg.b.det_std_hdt_v == 1) && (pstIsr->stJobProc.stVidStatusReg.b.det_std_hdt_h1 == 1) && (pstIsr->stJobProc.stVidStatusReg.b.det_std_hdt_h0 == 1) )
            {
                cameraStandard = HDT_NEW;
            }
            else if( (pstIsr->stJobProc.stVidStatusReg.b.det_std_hdt_v == 1) && (pstIsr->stJobProc.stVidStatusReg.b.det_std_hdt_h1 == 0) && (pstIsr->stJobProc.stVidStatusReg.b.det_std_hdt_h0 == 1) )
            {
                cameraStandard = HDT_OLD;
            }
        }

        PrDbg("%x-Fix Set final vdec format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA pstIsr->stJobProc.reJudgeStdResol);
#endif // SUPPORT_AUTODETECT_PVI

#ifdef SUPPORT_PVIRX_AUTOSET_VIDEO_INDRIVER
#ifdef SUPPORT_AUTODETECT_PVI
	    PrPrint("%x-Init Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
        PPDRV_PVIRX_SetTableStdResol(cid, chanAddr, cameraStandard, cameraResolution, videoResolution, bWaitStableStatus);
#else
        if( (cameraStandard != gpPviRxDrvHost[chanAddr]->stPrRxMode.standard) ||
                (cameraResolution != gpPviRxDrvHost[chanAddr]->stPrRxMode.cameraResolution) ||
                (videoResolution != gpPviRxDrvHost[chanAddr]->stPrRxMode.vidOutResolution) )
        {
	        PrPrint("%x-Init Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
            PPDRV_PVIRX_SetTableStdResol(cid, chanAddr, cameraStandard, cameraResolution, videoResolution, bWaitStableStatus);
        }
#endif // SUPPORT_AUTODETECT_PVI
#endif // SUPPORT_PVIRX_AUTOSET_VIDEO_INDRIVER

		if(pstIsr->stJobProc.reJudgeStdResol)
		{
			/* stop VFD irq */
			PrDbg("%x-Stop VFD irq. becasue of rejudge flag.\n", chanAddr);
			{/*{{{*/
				stReg.reg = 0x0089;
				u8RegMask = 0x20;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			/* set manual std */
			PrDbg("%x-Set Manual Std\n", chanAddr);
			{/*{{{*/
				stReg.reg = 0x0011;
				u8RegMask = 0x04;
				u8RegData = 0x04;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
		}
        else
        {
#ifdef SUPPORT_AUTODETECT_PVI
            /* start VFD irq */
            PrDbg("%x-Start VFD irq\n", chanAddr);
            {/*{{{*/
                stReg.reg = 0x0089;
                u8RegMask = 0x20;
                u8RegData = 0x20;
                if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
                {
                    PrErrorString("invalid write\n");
                    return(result);
                }
            }/*}}}*/
#endif // SUPPORT_AUTODETECT_PVI
        }

#ifdef SUPPORT_PVIRX_UTC
#ifdef SUPPORT_AUTODETECT_PVI
		PPDRV_PVIRX_UTC_SetTable(cid, chanAddr, cameraStandard, cameraResolution);
#else
        if( (cameraStandard != gpPviRxDrvHost[chanAddr]->stPrRxMode.standard) ||
                (cameraResolution != gpPviRxDrvHost[chanAddr]->stPrRxMode.cameraResolution) )
        {
		    PPDRV_PVIRX_UTC_SetTable(cid, chanAddr, cameraStandard, cameraResolution);
        }
#endif // SUPPORT_AUTODETECT_PVI
#endif // SUPPORT_PVIRX_UTC

#ifdef SUPPORT_PVIRX_FPGA
		PPDRV_PVIRX_FPGA_SetTable(cid, chanAddr);
#endif // SUPPORT_FPGA

        /* Get video status after applied table and check chroma frequency all standard.  */
        PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
        PrDbg("%x-Cur VidStatus:0x%02x%02x%02x\n", chanAddr COMMA curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1] COMMA curStVidStatusReg.reg[2]); 
        if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
        {
            pstIsr->stJobProc.C_LOCK_CNT = 1;

            pstIsr->stJobProc.AC_GAIN_ADJ = 0;
            pstIsr->stJobProc.AC_GAIN_HOLD = 0;
            if(pstIsr->stJobProc.EQ_CNT == 0) { pstIsr->stJobProc.EQ_CNT = 1; }

            /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
            pstIsr->stJobProc.cntWait300MsecTime = 0;
            pstIsr->stJobProc.numJobStep = 50;
            pstIsr->stJobProc.cntJobTry = 0;

            /* complete camera in process. start periodic monitoring process. */
            _SET_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
            _SET_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

            /* set user flag */
            PrDbg("Set user flag(%d,%d,%d)\n", cameraStandard COMMA cameraResolution COMMA videoResolution);
            pHost->stPrRxMode.standard = cameraStandard;
            pHost->stPrRxMode.cameraResolution = cameraResolution;
            pHost->stPrRxMode.vidOutResolution = videoResolution;
            _SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
            _SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);

#ifdef SUPPORT_AUTODETECT_PVI
            PrPrint("%x-go normal monitor processing(C_det & C_lock)\n", chanAddr);

            result = eSUCCESS;
            return(result);
#else
            PrPrint("%x-Fix normal (C_det & C_lock)\n", chanAddr);

            /* disable processing in timer1/0 isr. *///periodic job 300msec, 4sec
            {
                stReg.reg = 0x0089;
                if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
                {
                    PrErrorString("invalid read\n");
                    return(result);
                }

                u8RegData &= 0x1F; //disable WAKEUP(TIMER1/0)

                if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS) 
                {    
                    PrErrorString("invalid write\n");
                    return(result);
                }
            }

            result = eSUCCESS;
            return(result);
#endif // SUPPORT_AUTODETECT_PVI
        }
        else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
        {
            pstIsr->stJobProc.cntChromaLockTunn = 0; 

            /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
            pstIsr->stJobProc.cntWait300MsecTime = 0;
            pstIsr->stJobProc.numJobStep = 41;
            pstIsr->stJobProc.cntJobTry = 0;

            PrPrint("%x-go abnormal processing(C_Det & !C_lock)\n", chanAddr);

            result = eSUCCESS;
            return(result);
        }
        else if( (!curStVidStatusReg.b.det_chroma) )
        {
            pstIsr->stJobProc.C_LOCK_CNT = 0;

            pstIsr->stJobProc.AC_GAIN_ADJ = 0;
            pstIsr->stJobProc.AC_GAIN_HOLD = 0;
            if(pstIsr->stJobProc.EQ_CNT == 0) { pstIsr->stJobProc.EQ_CNT = 1; }

            /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
            pstIsr->stJobProc.cntWait300MsecTime = 0;
            pstIsr->stJobProc.numJobStep = 50;
            pstIsr->stJobProc.cntJobTry = 0;

            /* complete camera in process. start periodic monitoring process. */
            _SET_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
            _SET_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

            /* set user flag */
            PrDbg("Set user flag(%d,%d,%d)\n", cameraStandard COMMA cameraResolution COMMA videoResolution);
            pHost->stPrRxMode.standard = cameraStandard;
            pHost->stPrRxMode.cameraResolution = cameraResolution;
            pHost->stPrRxMode.vidOutResolution = videoResolution;
            _SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
            _SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);

            PrPrint("%x-go abnormal monitor processing(!C_det)\n", chanAddr);

            result = eSUCCESS;
            return(result);
        }
    }/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 41 ) //1'st C_det & !C_lock check
	{/*{{{*/
		PrDbg("%x-Check 1'st C_det & !C_lock\n", chanAddr);

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
		{/*{{{*/
			pstIsr->stJobProc.C_LOCK_CNT = 1;
			PrDbg("%x-find valid chroma lock\n", chanAddr);

			pstIsr->stJobProc.AC_GAIN_ADJ = 0;
			pstIsr->stJobProc.AC_GAIN_HOLD = 0;
			if(pstIsr->stJobProc.EQ_CNT == 0) { pstIsr->stJobProc.EQ_CNT = 1; }

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

			/* complete camera in process. start periodic monitoring process. */
			_SET_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
			_SET_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

			/* set user flag */
			PrDbg("Set user flag(%d,%d,%d)\n", cameraStandard COMMA cameraResolution COMMA videoResolution);
			pHost->stPrRxMode.standard = cameraStandard;
			pHost->stPrRxMode.cameraResolution = cameraResolution;
			pHost->stPrRxMode.vidOutResolution = videoResolution;
			_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
			_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);
			PrPrint("%x-chroma lock\n", chanAddr);

            result = eSUCCESS;
            return(result);
		}/*}}}*/
		else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
		{/*{{{*/
			pstIsr->stJobProc.C_LOCK_CNT = 0;

			/* get current chromaphase register value */
			{/*{{{*/
				stReg.reg = 0x0146;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				pstIsr->stJobProc.defChromaPhase = (PP_U32)(u8RegData & 0x1F)<<16;

				stReg.reg = 0x0147;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				pstIsr->stJobProc.defChromaPhase |= (PP_U32)(u8RegData & 0xFF)<<8;

				stReg.reg = 0x0148;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				pstIsr->stJobProc.defChromaPhase |= (PP_U32)(u8RegData & 0xFF);

				PrDbg("%x-base CPhaseRefBase:0x%08lx\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase);
			}/*}}}*/

			pstIsr->stJobProc.cntChromaLockTunn = 0; 

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 42;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		}/*}}}*/
	}/*}}}*/

	if( pstIsr->stJobProc.numJobStep == 42 ) //2'nd C_det & !C_lock check & tunn c_lock
	{/*{{{*/
		PrDbg("%x-Check 2'nd C_det & !C_lock check and tunn c_lock.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);
			PrPrint("%x-Can't Det chroma or chroma lock.\n", chanAddr);
			tunnChromaPhase = pstIsr->stJobProc.defChromaPhase;

			stReg.reg = 0x0146;
			u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x00; //0x00: set auto 
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0147;
			u8RegData = (tunnChromaPhase>>8)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0148;
			u8RegData = (tunnChromaPhase)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

			/* Apply Set register */
			{/*{{{*/

				stReg.reg = 0x0154;
				u8RegData = 0x0E;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				stReg.reg = 0x0154;
				u8RegData = 0x0F;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			if(!curStVidStatusReg.b.det_chroma)
			{
				pstIsr->stJobProc.AC_GAIN_ADJ = 0;
				pstIsr->stJobProc.AC_GAIN_HOLD = 0;
				if(pstIsr->stJobProc.EQ_CNT == 0) { pstIsr->stJobProc.EQ_CNT = 1; }

				pstIsr->stJobProc.reJudgeStdResol = 1;

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

				/* complete camera in process. start periodic monitoring process. */
				_SET_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
				_SET_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

                result = eSUCCESS;
                return(result);
			}
			else
			{
				pstIsr->stJobProc.AC_GAIN_ADJ = 0;
				pstIsr->stJobProc.AC_GAIN_HOLD = 0;
				if(pstIsr->stJobProc.EQ_CNT == 0) { pstIsr->stJobProc.EQ_CNT = 1; }

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

				/* complete camera in process. start periodic monitoring process. */
				_SET_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
				_SET_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

                result = eSUCCESS;
                return(result);
            }
		} /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				pstIsr->stJobProc.C_LOCK_CNT = 1;
				PrDbg("%x-find valid chroma lock\n", chanAddr);

				pstIsr->stJobProc.AC_GAIN_ADJ = 0;
				pstIsr->stJobProc.AC_GAIN_HOLD = 0;
				if(pstIsr->stJobProc.EQ_CNT == 0) { pstIsr->stJobProc.EQ_CNT = 1; }

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

				/* complete camera in process. start periodic monitoring process. */
				_SET_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
				_SET_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

				/* set user flag */
				PrDbg("Set user flag(%d,%d,%d)\n", cameraStandard COMMA cameraResolution COMMA videoResolution);
				pHost->stPrRxMode.standard = cameraStandard;
				pHost->stPrRxMode.cameraResolution = cameraResolution;
				pHost->stPrRxMode.vidOutResolution = videoResolution;
				_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
				_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);
				PrPrint("%x-chroma lock\n", chanAddr);

                result = eSUCCESS;
                return(result);
            }/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 42;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
        }
	}/*}}}*/

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_ProcessCameraMonitor(void INOUT *pParam1, uint32_t IN u32Param2)
{
    PP_S32 cid;
    PP_U8 chanAddr;
    enum _eBitIrq bitIrq;
	PP_RESULT_E result = eERROR_FAILURE;

    _stMonitorParam *pstMonitorParam = (_stMonitorParam *)pParam1;
	_PviRxDrvHost *pHost = (_PviRxDrvHost *)pstMonitorParam->pHost;
	_stPVIRX_Isr *pstIsr = (_stPVIRX_Isr *)&pHost->stPVIRX_Isr;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
	PP_S32 s32Ret = 0;
#endif // SUPPORT_PVIRX_CEQ_MONITOR
	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;
	PP_S32 reJudge = 0;
	_stPVIRX_VidStatusReg curStVidStatusReg;
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
	_stPVIRX_MeasureEqInfo stMeasureEqInfo;
#endif // SUPPORT_PVIRX_CEQ_MONITOR
	PP_U32 tunnChromaPhase = 0;


	if( (pParam1 == NULL) )
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

    cid = pstMonitorParam->cid;
    chanAddr = pstMonitorParam->chanAddr;
    bitIrq = pstMonitorParam->eBitIrq;
	PrDbg("%x-CamMonNumJobStep:%d, irq:%d\n", chanAddr COMMA pstIsr->stJobProc.numJobStep COMMA bitIrq);
        // bitIrq => IRQ_TIMER0UP: 4sec
        // bitIrq => IRQ_TIMER1UP: 300Msec

	if( pstIsr->stJobProc.numJobStep < 50 )
	{/*{{{*/
		PrErrorString("Invalid numJobStep\n");
		return(eERROR_FAILURE);
	}/*}}}*/

	/* If pause, return */
	if( _TEST_BIT(JOB_PAUSE_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess) )
	{
		PrDbg("%x-Pause return(CamMonNumJobStep:%d, irq:%d)\n", chanAddr COMMA pstIsr->stJobProc.numJobStep COMMA bitIrq);

		result = eSUCCESS;
		return(result);
	}

	//assume timer0up. When set JOB_START_CAMERA_MONITOR, run right away 4sec process until JOB_DOING_CAMERA_MONITOR.
	if( _TEST_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess) && !_TEST_BIT(JOB_DOING_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess) )
	{
		PrDbg("%x-assume timer0up\n", chanAddr);
        	bitIrq = IRQ_TIMER0UP; 
	}

	if( (bitIrq == IRQ_TIMER0UP) && (pstIsr->stJobProc.numJobStep == 50) ) //Read std/resol, change ?
	{/*{{{*/
#ifdef SUPPORT_AUTODETECT_PVI
        /* if reJudgeStdResol case, disabled VFD. And change to manually checking. */
        if(pstIsr->stJobProc.reJudgeStdResol)
        {
            PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);

            if( ((curStVidStatusReg.b.det_ifmt_res != pstIsr->stJobProc.stStartVidStatusReg.b.det_ifmt_res) ||
                        (curStVidStatusReg.b.det_ifmt_ref != pstIsr->stJobProc.stStartVidStatusReg.b.det_ifmt_ref) ||
                        (curStVidStatusReg.b.det_ifmt_std != pstIsr->stJobProc.stStartVidStatusReg.b.det_ifmt_std)) )
            {/*{{{*/
                PrDbg("%x-Change: 0x00:0x%02x->0x%02x, 0x01:0x%02x\n", chanAddr COMMA pstIsr->stJobProc.stStartVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1]);

                /* initialize argument */
                memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));

                _SET_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrq);
                _CLEAR_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrqStatus);

                /* Write pre setting for 800p/900p. For valid operating 800p/900p */
                if( curStVidStatusReg.b.det_ifmt_res == DET_HD960p)
                {/*{{{*/
	                PP_S32 regInx = 0;
	                const _stPVIRX_Table_ETC_EXTResolution *pTblETCEXTResolution = NULL;

                    pTblETCEXTResolution = (const _stPVIRX_Table_ETC_EXTResolution *)stPVIRX_Table_ETC_EXTResol;
                    PrDbg("%x-Write pre setting for 960p(ref:%d)\n", chanAddr COMMA curStVidStatusReg.b.det_ifmt_ref);
                    switch( curStVidStatusReg.b.det_ifmt_ref )
                    {/*{{{*/
                        case DET_25Hz: videoResolution = video_1280x960p25; break;
                        case DET_30Hz: videoResolution = video_1280x960p30; break;
                        default: videoResolution = video_1280x960p25; break;
                    }/*}}}*/

                    regInx = 0;
                    stReg.reg = pTblETCEXTResolution[regInx].stReg.reg;
                    while(stReg.reg != 0xFFFF)
                    {
                        u8RegData = pTblETCEXTResolution[regInx].u8Data[videoResolution-video_1280x960p30]; 
                        PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);

                        if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
                        {
                            PrErrorString("invalid write\n");
                            return(result);
                        }

                        regInx++;
                        stReg.reg = pTblETCEXTResolution[regInx].stReg.reg;
                    }
                }/*}}}*/

                /* save cur video status registers */
                memcpy(&pstIsr->stJobProc.stStartVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
                memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
                pstIsr->stJobProc.cntJobTry = 0;

                /* set only start camera in flag */
                _SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

                pstIsr->stJobProc.numJobStep = 1;
                pstIsr->stJobProc.cntWait300MsecTime = 0;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
        }
#endif // SUPPORT_AUTODETECT_PVI

		/* start monitoring process */
		PrDbgString("Doing monitoring\n");
		_SET_BIT(JOB_DOING_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

		if( pstIsr->stJobProc.C_LOCK_CNT != 1)
		{
			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 51;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
            //return(result);
        }
		else
		{
			if( pstIsr->stJobProc.EQ_CNT == 1)
			{
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 80;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
			else
			{
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 100;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER0UP) && (pstIsr->stJobProc.numJobStep == 51) ) //check c_lock_cnt
	{/*{{{*/
		PrDbg("%x-check c_lock_cnt (C_LOCK_CNT != 1)\n", chanAddr);
		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &curStVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("==%x/%x/%x\n", curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1] COMMA curStVidStatusReg.reg[2]);
		if( ((cameraStandard == HDA)) && 
			((cameraResolution == camera_1280x720p60) || (cameraResolution == camera_1280x720p50) ||
			(cameraResolution == camera_1280x720p30) || (cameraResolution == camera_1280x720p25)) )
		{/*{{{*/
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 65;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }
            else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{
				pstIsr->stJobProc.cntChromaLockTunn = 0;
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 60;
				pstIsr->stJobProc.cntJobTry = 0;

				bitIrq = IRQ_TIMER1UP;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}
			else
			{
				// When temporary register set for checking det_chroma, change forcely hdelay for normal view. 
				if( (videoResolution == video_1280x720p30) || (videoResolution == video_1280x720p25) )
				{
					PVIRX_SetTblBrief(cid, chanAddr, HDA, videoResolution, 2, &pstIsr->stJobProc);
				}

				pstIsr->stJobProc.cntChromaLockTunn = 0;
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 70;
				pstIsr->stJobProc.cntJobTry = 0;

				bitIrq = IRQ_TIMER1UP;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else 
		{/*{{{*/
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 55;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{
				pstIsr->stJobProc.cntChromaLockTunn = 0;
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 52;
				pstIsr->stJobProc.cntJobTry = 0;

				bitIrq = IRQ_TIMER1UP;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
			else
			{
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }
		}/*}}}*/

	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 52) ) //PVI/TVI C_det & !C_lock check
	{/*{{{*/
		PrDbg("%x-PVI/TVI C_det & !C_lock check. tunn chroma phase.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &curStVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);
			tunnChromaPhase = pstIsr->stJobProc.defChromaPhase;

			stReg.reg = 0x0146;
			u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x00; //0x00: set auto 
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0147;
			u8RegData = (tunnChromaPhase>>8)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0148;
			u8RegData = (tunnChromaPhase)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

			/* Apply Set register */
			{/*{{{*/

				stReg.reg = 0x0154;
				u8RegData = 0x0E;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				stReg.reg = 0x0154;
				u8RegData = 0x0F;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
        } /*}}}*/
        else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				pstIsr->stJobProc.C_LOCK_CNT = 1;
				PrDbg("%x-find lock chroma\n", chanAddr);

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 55;
				pstIsr->stJobProc.cntJobTry = 0;

				/* set user flag */
				PrDbg("Set user flag(%d,%d,%d)\n", cameraStandard COMMA cameraResolution COMMA videoResolution);
				if( (pHost->stPrRxMode.standard != cameraStandard) ||
						(pHost->stPrRxMode.cameraResolution != cameraResolution) ||
						(pHost->stPrRxMode.vidOutResolution != videoResolution) )
				{
					pHost->stPrRxMode.standard = cameraStandard;
					pHost->stPrRxMode.cameraResolution = cameraResolution;
					pHost->stPrRxMode.vidOutResolution = videoResolution;
					_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
					_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);
				}

                result = eSUCCESS;
                return(result);
            }/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 52;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 55 ) ) //EQ Fine Tunning-1 (from numJob(52) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-1 from numJob(52)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 55;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 56;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
				else
				{

                    result = eSUCCESS;
                    return(result);
                }
			}
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 56;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 55;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 56;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 56;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(52)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 56;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
			//return(result);
		}/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 56 ) ) //EQ Fine Tunning-2 (from numJob(52) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-2 from numJob(52)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("%x-atten2:0x%04x, acGain:0x%04x\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.atten2 COMMA pstIsr->stJobProc.stDetGainStatus.acGain);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 56;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
                else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}
		}/*}}}*/
		else 
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 56;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 50;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        //go directly next step. no return.
                        //return(result);
					}
					else
					{
                        result = eSUCCESS;
                        return(result);
                    }
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
			}/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(52)\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 60) ) //CVI C_det & !C_lock check
	{/*{{{*/
		PrDbg("%x-CVI C_det & !C_lock check. tunn chroma phase.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);

			PrDbg("%x-Forcely Return set abnormal STD(CVI)\n", chanAddr);
			pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;

			PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);

			PrDbg("%x-format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA reJudge);

			PrDbg("%x-Init vdec, CPhaseRef...\n", chanAddr);
			pstIsr->stJobProc.defChromaPhase = 0;

			// Set temporary register set. for checking det_chroma.
			if( (result = PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 1, &pstIsr->stJobProc)) != eSUCCESS)
			{
				PrErrorString("Can't set table brief\n");
				return(result);
			}

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0; //if return cvi, set 0. else should >= 1.
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		} /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				pstIsr->stJobProc.C_LOCK_CNT = 1;
				PrDbg("%x-find lock chroma\n", chanAddr);

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 65;
				pstIsr->stJobProc.cntJobTry = 0;

				pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;
				PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);

				/* set user flag */
				PrDbg("Set user flag(%d,%d,%d)\n", cameraStandard COMMA cameraResolution COMMA videoResolution);
				if( (pHost->stPrRxMode.standard != cameraStandard) ||
						(pHost->stPrRxMode.cameraResolution != cameraResolution) ||
						(pHost->stPrRxMode.vidOutResolution != videoResolution) )
				{
					pHost->stPrRxMode.standard = cameraStandard;
					pHost->stPrRxMode.cameraResolution = cameraResolution;
					pHost->stPrRxMode.vidOutResolution = videoResolution;
					_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
					_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);
				}

                result = eSUCCESS;
                return(result);
            }/*}}}*/
            else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 60;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 65 ) ) //EQ Fine Tunning-1 (from numJob(60) )
	{/*{{{*/
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("%x-format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA reJudge);
		// When temporary register set for checking det_chroma, change forcely hdelay for normal view. 
		if( (videoResolution == video_1280x720p30) || (videoResolution == video_1280x720p25) )
		{
			PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 2, &pstIsr->stJobProc);
		}

#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-1 from numJob(60)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 65;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 66;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
                }
            }
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 66;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 65;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 66;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
				else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 66;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(60)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 66;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
			//return(result);
		}/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 66 ) ) //EQ Fine Tunning-2 (from numJob(60) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-2 from numJob(60)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("%x-atten2:0x%04x, acGain:0x%04x\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.atten2 COMMA pstIsr->stJobProc.stDetGainStatus.acGain);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 66;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
        }/*}}}*/
		else
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 66;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 50;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        //go directly next step. no return.
                        //return(result);
					}
					else
					{
                        result = eSUCCESS;
                        return(result);
					}
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
			}/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(60)\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 70) ) //Set HDA format
	{/*{{{*/
		PrDbg("%x-Set HDA format\n", chanAddr);

		PrDbg("%x-Forcely Set temporay abnormal STD(HDA)\n", chanAddr);
		pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_HDA;

		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);

		PrDbg("%x-format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA reJudge);

		PrDbg("%x-Set vdec, CPhaseRef...\n", chanAddr);
		pstIsr->stJobProc.defChromaPhase = 0;

		// Set temporary register set. for checking det_chroma.
		if( (result = PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 1, &pstIsr->stJobProc)) != eSUCCESS)
		{
			PrErrorString("Can't set table brief\n");
			return(result);
		}

		pstIsr->stJobProc.cntChromaLockTunn = 0;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 1; //if return cvi, set 0. else should >= 1.
		pstIsr->stJobProc.numJobStep = 71;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 71) ) //HDA C_det & !C_lock check
	{/*{{{*/
		PrDbg("%x-HDA C_det & !C_lock check. tunn chroma phase.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);

			PrDbg("%x-Forcely Return set abnormal STD(CVI)\n", chanAddr);
			pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;

			PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);

			PrDbg("%x-format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA reJudge);

			PrDbg("%x-Init vdec, CPhaseRef...\n", chanAddr);
			pstIsr->stJobProc.defChromaPhase = 0;

			// Set temporary register set. for checking det_chroma.
			if( (result = PVIRX_SetTblBrief(cid, chanAddr, cameraStandard, videoResolution, 1, &pstIsr->stJobProc)) != eSUCCESS)
			{
				PrErrorString("Can't set table brief\n");
				return(result);
			}

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0; //if return cvi, set 0. else should >= 1.
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		} /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
				PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
				PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &stMeasureEqInfo);
				PrDbg("%x- [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &stMeasureEqInfo);
				memcpy(&pstIsr->stJobProc.stMeasureEqInfo, &stMeasureEqInfo, sizeof(_stPVIRX_MeasureEqInfo));
#endif // SUPPORT_PVIRX_CEQ_MONITOR

				pstIsr->stJobProc.C_LOCK_CNT = 1;
				PrDbg("%x-find lock chroma\n", chanAddr);

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 75;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 71;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 75 ) ) //EQ Fine Tunning-1 (from numJob(71) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-1 from numJob(71)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 75;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 76;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
                else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 76;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
		}/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 75;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 76;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 76;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(71)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 76;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
			//return(result);
		}/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 76 ) ) //EQ Fine Tunning-2 (from numJob(71) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-2 from numJob(71)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("%x-atten2:0x%04x, acGain:0x%04x\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.atten2 COMMA pstIsr->stJobProc.stDetGainStatus.acGain);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 76;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* set start camera in flag */
					pstIsr->stJobProc.bitJobProcess = 0;
					_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 40;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* set start camera in flag */
				pstIsr->stJobProc.bitJobProcess = 0;
				_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 40;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
            }
		}/*}}}*/
		else 
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 76;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* set start camera in flag */
						pstIsr->stJobProc.bitJobProcess = 0;
						_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 40;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        //go directly next step. no return.
                        //return(result);
                    }
					else
					{
                        result = eSUCCESS;
                        return(result);
					}
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* set start camera in flag */
					pstIsr->stJobProc.bitJobProcess = 0;
					_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 40;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
			}/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(71). Camera In.\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* set start camera in flag */
				pstIsr->stJobProc.bitJobProcess = 0;
				_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 40;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* set start camera in flag */
		pstIsr->stJobProc.bitJobProcess = 0;
		_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 40;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER0UP) && (pstIsr->stJobProc.numJobStep == 80) ) //EQ_CNT==1 & C_det
	{/*{{{*/
		PrDbg("%x-Check case (EQ_CNT==1 & C_det)\n", chanAddr);

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if(!curStVidStatusReg.b.det_chroma)
		{/*{{{*/
			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		} /*}}}*/
		else 
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/

				PrDbg("%x-find lock chroma\n", chanAddr);
				pstIsr->stJobProc.cntChromaLockTunn = 0;

#ifdef SUPPORT_PVIRX_CEQ_MONITOR
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 82;
				pstIsr->stJobProc.cntJobTry = 0;

				bitIrq = IRQ_TIMER1UP;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);

#else

				pstIsr->stJobProc.AC_GAIN_ADJ = 0;
				pstIsr->stJobProc.AC_GAIN_HOLD = 0;
				pstIsr->stJobProc.EQ_CNT = 2;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);

#endif // SUPPORT_PVIRX_CEQ_MONITOR
			}/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 81;
				pstIsr->stJobProc.cntJobTry = 0;

				bitIrq = IRQ_TIMER1UP;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}/*}}}*/
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 81) ) //EQ_CNT==1 & (C_det & !C_lock check)
	{/*{{{*/
		PrDbg("%x-C_det & !C_lock check. tunn chroma phase.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);
			tunnChromaPhase = pstIsr->stJobProc.defChromaPhase;

			stReg.reg = 0x0146;
			u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x00; //0x00: set auto 
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0147;
			u8RegData = (tunnChromaPhase>>8)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0148;
			u8RegData = (tunnChromaPhase)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

			/* Apply Set register */
			{/*{{{*/

				stReg.reg = 0x0154;
				u8RegData = 0x0E;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				stReg.reg = 0x0154;
				u8RegData = 0x0F;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		} /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				PrDbg("%x-find lock chroma\n", chanAddr);

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 82;
				pstIsr->stJobProc.cntJobTry = 0;

				bitIrq = IRQ_TIMER1UP;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 81;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 82 ) ) //EQ Fine Tunning-1 (from numJob(80) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-1 from numJob(80)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 82;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 83;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 83;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 82;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 83;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
                }
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 83;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(80)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 83;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
            //return(result);
		}/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 83 ) ) //EQ Fine Tunning-2 (from numJob(80) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-2 from numJob(80)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("%x-atten2:0x%04x, acGain:0x%04x\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.atten2 COMMA pstIsr->stJobProc.stDetGainStatus.acGain);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 83;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}
		}/*}}}*/
		else 
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 83;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 50;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        //go directly next step. no return.
						//return(result);
					}
					else
					{
                        result = eSUCCESS;
                        return(result);
					}
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
					//return(result);
				}
			}/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(80)\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER0UP) && (pstIsr->stJobProc.numJobStep == 100) ) //Monitoring EQ gain
	{/*{{{*/
		PrDbgString("Monitoring EQ gain\n");
#ifdef SUPPORT_AUTODETECT_PVI
# ifdef SUPPORT_PVIRX_CEQ_MONITOR
        PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
        if(curStVidStatusReg.b.det_chroma)
        {
            /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
            pstIsr->stJobProc.cntWait300MsecTime = 0;
            pstIsr->stJobProc.numJobStep = 110;
            pstIsr->stJobProc.cntJobTry = 0;

            bitIrq = IRQ_TIMER1UP;

            result = eSUCCESS;
            //go directly next step. no return.
            //return(result);
        }
        else
        {
            /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
            pstIsr->stJobProc.cntWait300MsecTime = 0;
            pstIsr->stJobProc.numJobStep = 50;
            pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
        }
# else
        /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
        pstIsr->stJobProc.cntWait300MsecTime = 0;
        pstIsr->stJobProc.numJobStep = 50;
        pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);

# endif // SUPPORT_PVIRX_CEQ_MONITOR
#else
        /* Set wait time next loop. 300msec unit. 1:wait 600msec. */
        pstIsr->stJobProc.cntWait300MsecTime = 0;
        pstIsr->stJobProc.numJobStep = 50;
        pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);
#endif // SUPPORT_AUTODETECT_PVI
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 110) ) //(C_det & !C_lock) check
	{/*{{{*/
		PrDbg("%x-(C_det & !C_lock) check. tunn chroma phase.\n", chanAddr);
		PrDbg("%x-defChromaPhase:0x%08lx, cntTunn:%x\n", chanAddr COMMA pstIsr->stJobProc.defChromaPhase COMMA pstIsr->stJobProc.cntChromaLockTunn); 

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (!curStVidStatusReg.b.det_chroma) || (pstIsr->stJobProc.cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
		{/*{{{*/
			PrDbg("%x-Can't Det chroma(%d) or Over check time(%d) chroma lock.\n", chanAddr COMMA curStVidStatusReg.b.det_chroma COMMA pstIsr->stJobProc.cntChromaLockTunn);
			tunnChromaPhase = pstIsr->stJobProc.defChromaPhase;

			stReg.reg = 0x0146;
			u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x00; //0x00: set auto 
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0147;
			u8RegData = (tunnChromaPhase>>8)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			stReg.reg = 0x0148;
			u8RegData = (tunnChromaPhase)&0xFF;
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
			PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

			/* Apply Set register */
			{/*{{{*/

				stReg.reg = 0x0154;
				u8RegData = 0x0E;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				stReg.reg = 0x0154;
				u8RegData = 0x0F;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			pstIsr->stJobProc.cntChromaLockTunn = 0;

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 50;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            return(result);
		} /*}}}*/
		else if(pstIsr->stJobProc.cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
		{
			if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				PrDbg("%x-find lock chroma\n", chanAddr);

				pstIsr->stJobProc.cntChromaLockTunn = 0;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 111;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}/*}}}*/
			else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
			{/*{{{*/
				tunnChromaPhase = (pstIsr->stJobProc.cntChromaLockTunn >>1) + 1;
				tunnChromaPhase *= 40;
				if(pstIsr->stJobProc.cntChromaLockTunn & 1) // - tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase - tunnChromaPhase;
				}
				else // + tunning
				{
					tunnChromaPhase = pstIsr->stJobProc.defChromaPhase + tunnChromaPhase;
				}

				stReg.reg = 0x0146;
				u8RegData = ((tunnChromaPhase>>16)&0x1F) | 0x20; //0x20: set manual 
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0147;
				u8RegData = (tunnChromaPhase>>8)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0148;
				u8RegData = (tunnChromaPhase)&0xFF;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				PrDbg("%x-Tunn CPhase:0x%08x\n", chanAddr COMMA tunnChromaPhase);

				/* Apply Set register */
				{/*{{{*/

					stReg.reg = 0x0154;
					u8RegData = 0x0E;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					stReg.reg = 0x0154;
					u8RegData = 0x0F;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				pstIsr->stJobProc.cntChromaLockTunn++;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 1;
				pstIsr->stJobProc.numJobStep = 110;

                result = eSUCCESS;
                return(result);
			}/*}}}*/
		}
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 111 ) ) //EQ Fine Tunning-1 (from numJob(110) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-1 from numJob(110)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);
		PrDbg("%x-dcGain:0x%04x\n", pstIsr->stJobProc.stDetGainStatus.dcGain);
		if(pstIsr->stJobProc.stDetGainStatus.dcGain == 0xFFFF)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_GetTunnEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain = 0xFFFF, decrease Eq step.\n", chanAddr);

				PPDRV_PVIRX_SetEQGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stMeasureEqInfo);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 111;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 112;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
                }
                else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 112;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
                //return(result);
			}
		}/*}}}*/
		else if(pstIsr->stJobProc.stDetGainStatus.dcGain < 0x1300)
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnVADCGain(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-1 dc_gain < 0x1300, decrease Eq vadcgain.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 111;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 112;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    //go directly next step. no return.
                    //return(result);
				}
				else
				{
                    result = eSUCCESS;
                    return(result);
				}
			}/*}}}*/
			else
			{
				/* MIN or MAX or fail. */
				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 112;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                //go directly next step. no return.
				//return(result);
			}
		}/*}}}*/
		else
		{/*{{{*/
			PrDbg("%x-End EQ Fine Tunning-1 from numJob(110)\n", chanAddr);

			/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
			pstIsr->stJobProc.cntWait300MsecTime = 0;
			pstIsr->stJobProc.numJobStep = 112;
			pstIsr->stJobProc.cntJobTry = 0;

            result = eSUCCESS;
            //go directly next step. no return.
            //return(result);
        }/*}}}*/
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

		result = eSUCCESS;
		return(result);

#endif
	}/*}}}*/

	if( (bitIrq == IRQ_TIMER1UP) && (pstIsr->stJobProc.numJobStep == 112 ) ) //EQ Fine Tunning-2 (from numJob(110) )
	{/*{{{*/
#ifdef SUPPORT_PVIRX_CEQ_MONITOR
		PrDbg("%x-EQ Fine Tunning-2 from numJob(110)\n", chanAddr);

		PPDRV_PVIRX_GetDetGainStatus(cid, chanAddr, &pstIsr->stJobProc.stDetGainStatus);
		PPDRV_PVIRX_GetStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution, &reJudge);
		PrDbg("%x-atten2:0x%04x, acGain:0x%04x\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.atten2 COMMA pstIsr->stJobProc.stDetGainStatus.acGain);
		if( (pstIsr->stJobProc.stDetGainStatus.atten2 < 0x0500) || (pstIsr->stJobProc.stDetGainStatus.acGain < 0x0700) )
		{/*{{{*/
			pstIsr->stJobProc.stTunnFactor.stepDir = -1;
			if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
			{/*{{{*/
				PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
				PrDbg("%x-  Eq fine tunning-2 atten2 < 0x0500 or acGain < 0x0700, decrease Eq band.\n", chanAddr);

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 112;
				if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
				{
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    return(result);
				}

                result = eSUCCESS;
                return(result);
            }/*}}}*/
            else
			{
				/* MIN or MAX or fail. */
				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }
        }/*}}}*/
        else 
		{
			PP_U16 tmpAtten2 = 0;

			if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1200) tmpAtten2 = 0x1D00;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 1000) tmpAtten2 = 0x1800;
			else if(pstIsr->stJobProc.stMeasureEqInfo.distComp > 800) tmpAtten2 = 0x1600;
			else if( (pstIsr->stJobProc.stMeasureEqInfo.distComp > 600) || (cameraStandard == CVBS) ) tmpAtten2 = 0x1400;
			else tmpAtten2 = 0x13F8;

			if(pstIsr->stJobProc.stDetGainStatus.atten2 > tmpAtten2)
			{/*{{{*/
				pstIsr->stJobProc.stTunnFactor.stepDir = 1;
				if( (PPDRV_PVIRX_SetTunnEQBand(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &pstIsr->stJobProc.stDetGainStatus, &pstIsr->stJobProc.stTunnFactor, &pstIsr->stJobProc.stMeasureEqInfo, &s32Ret) == eSUCCESS) && (s32Ret > 0) )
				{/*{{{*/
					PrDbg("%x-  [syncLevel:%02x, dc:%04x, ac:%04x, comp[%04x, %04x], atten[%04x, %04x]]\n", chanAddr COMMA pstIsr->stJobProc.stDetGainStatus.syncLevel COMMA pstIsr->stJobProc.stDetGainStatus.dcGain COMMA pstIsr->stJobProc.stDetGainStatus.acGain COMMA pstIsr->stJobProc.stDetGainStatus.comp1 COMMA pstIsr->stJobProc.stDetGainStatus.comp2 COMMA pstIsr->stJobProc.stDetGainStatus.atten1 COMMA pstIsr->stJobProc.stDetGainStatus.atten2);
					PrDbg("%x-  Eq fine tunning-2 (distComp:%d,%s) && (atten2 > 0x%04x), increase Eq band.\n", chanAddr COMMA pstIsr->stJobProc.stMeasureEqInfo.distComp COMMA (cameraStandard == CVBS)?"SD":"HD" COMMA tmpAtten2);

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 112;
					if(pstIsr->stJobProc.cntJobTry++ > MAX_PVIRX_CEQ_COMP_TBL_NUM)
					{
						pstIsr->stJobProc.EQ_CNT = 2;
						pstIsr->stJobProc.C_LOCK_CNT = 1;

						/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
						pstIsr->stJobProc.numJobStep = 50;
						pstIsr->stJobProc.cntJobTry = 0;

                        result = eSUCCESS;
                        return(result);
                    }

                    result = eSUCCESS;
                    return(result);
				}/*}}}*/
				else
				{
					/* MIN or MAX or fail. */
					pstIsr->stJobProc.EQ_CNT = 2;
					pstIsr->stJobProc.C_LOCK_CNT = 1;

					/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
					pstIsr->stJobProc.cntWait300MsecTime = 0;
					pstIsr->stJobProc.numJobStep = 50;
					pstIsr->stJobProc.cntJobTry = 0;

                    result = eSUCCESS;
                    return(result);
                }
            }/*}}}*/
			else
			{/*{{{*/
				PrDbg("%x-End EQ Fine Tunning-2 from numJob(110)\n", chanAddr);

				pstIsr->stJobProc.EQ_CNT = 2;
				pstIsr->stJobProc.C_LOCK_CNT = 1;

				/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
				pstIsr->stJobProc.cntWait300MsecTime = 0;
				pstIsr->stJobProc.numJobStep = 50;
				pstIsr->stJobProc.cntJobTry = 0;

                result = eSUCCESS;
                return(result);
            }/*}}}*/
		}
#else
		pstIsr->stJobProc.EQ_CNT = 2;

		/* Set wait time next loop. 300msec unit. 1:wait 600msec. */
		pstIsr->stJobProc.cntWait300MsecTime = 0;
		pstIsr->stJobProc.numJobStep = 50;
		pstIsr->stJobProc.cntJobTry = 0;

        result = eSUCCESS;
        return(result);

#endif
	}/*}}}*/

	return(result);
}



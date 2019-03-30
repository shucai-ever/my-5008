#include "pvirx_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system.h"
#include "osal.h"
#include "sub_intr.h"
#include "proc.h"
#include "common.h"
#include "api_ipc.h"

#include "pvirx.h"
#include "pvirx_table.h"
#include "pvirx_func.h"
#include "pvirx_ceqfunc.h"
#include "pvirx_user_config.h"

#include "task_manager.h" //for event send to task.

/* pi5008 driver header */
#include "system.h"
#define PVIRX_GetRegValue(reg)          (*((volatile unsigned char *)(PVIRX0_BASE_ADDR+reg)))
#define PVIRX_SetRegValue(reg, data)  	((*((volatile unsigned char *)(PVIRX0_BASE_ADDR+reg))) = (unsigned char)(data))
#define PVIRX_BITCLR(reg, mask)         ((*((volatile unsigned char *)(PVIRX0_BASE_ADDR+reg))) &= ~(unsigned char)(mask))
#define PVIRX_BITSET(reg, mask)         ((*((volatile unsigned char *)(PVIRX0_BASE_ADDR+reg))) |= (unsigned char)(mask))

PP_RESULT_E PVIRX_SetTableIRQ(const PP_S32 cid, const PP_U8 chanAddr);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern PP_U32 gu32PviRxIntcCnt[4];
/*** PVIRX Intf Funcion ***/
volatile PP_S32 spinLockINTF = 0;
////////////////////////////////////////
/*** use PI5008 interface function ***/
////////////////////////////////////////
PP_RESULT_E PPDRV_PVIRX_Read(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_U16 IN addr, PP_U8 OUT *pData)
{
	PP_RESULT_E ret = eSUCCESS;

	*pData = PVIRX_GetRegValue(PVIRX_CHN_ADDR(chanAddr, addr));

	return(ret);
}

PP_RESULT_E PPDRV_PVIRX_Write(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_U16 IN addr, PP_U8 IN data)
{
	PP_RESULT_E ret = eSUCCESS;

	PVIRX_SetRegValue(PVIRX_CHN_ADDR(chanAddr, addr), data);

	return(ret);
}

PP_RESULT_E PPDRV_PVIRX_ReadMaskBit(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_U16 IN addr, const PP_U8 IN maskBit, PP_U8 OUT *pData)
{
	PP_RESULT_E ret = eSUCCESS;
	PP_U8 tmp = 0;

	tmp = PVIRX_GetRegValue(PVIRX_CHN_ADDR(chanAddr, addr));
	tmp &= maskBit;
	*pData = tmp;

	return(ret);
}

PP_RESULT_E PPDRV_PVIRX_WriteMaskBit(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_U16 IN addr, const PP_U8 IN maskBit, const PP_U8 IN data)
{
	PP_RESULT_E ret = eSUCCESS;
	PP_U8 tmp = 0;

	tmp = PVIRX_GetRegValue(PVIRX_CHN_ADDR(chanAddr, addr));
	tmp &= ~maskBit;
	tmp |= (data & maskBit);

	PVIRX_SetRegValue(PVIRX_CHN_ADDR(chanAddr, addr), tmp);

	return(ret);
}

PP_RESULT_E PPDRV_PVIRX_ReadBurst(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_U16 IN addr, const PP_U8 IN length, PP_U8 OUT *pData)
{
	PP_RESULT_E ret = eSUCCESS;
	PP_S32 i;

	for(i=0; i<length; i++) 
	{
		pData[i] = PVIRX_GetRegValue(PVIRX_CHN_ADDR(chanAddr, addr)); //<<-- "don't increase addr"
	}

	return(ret);
}

PP_RESULT_E PPDRV_PVIRX_WriteBurst(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_U16 IN addr, const PP_U8 IN length, PP_U8 IN *pData)
{
	PP_RESULT_E ret = eSUCCESS;
	PP_S32 i;

	for(i=0; i<length; i++) 
	{
		PVIRX_SetRegValue(PVIRX_CHN_ADDR(chanAddr, addr), pData[i]); //<<-- "don't increase addr"
	}

	return(ret);
}

/*******************/
static PP_RESULT_E PPDRV_PVIRX_CheckINTFReadWrite(const PP_S32 cid, const PP_U8 chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	PP_S32 retValue = -1;

	_stPVIRX_Reg stReg;
	PP_S32 i;

	PP_U8 u8RegData;
	const PP_U8 testRWData[8] = { 0x00, 0xFF, 0xAA, 0x55, 0x0A, 0x50, 0xFA, 0xCE };
	PP_U8 u8Temp[8] = {0, };
	PP_S32 bWaitStableStatus = 0;

	/* Check Single Read - Write */
	{
		for(i = 0; i < 8; i++)
		{
			stReg.reg = 0x0020 + i;
			retValue = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, testRWData[i]);
		}
		for(i = 0; i < 8; i++) { u8Temp[i] = 0xFF; }
		for(i = 0; i < 8; i++)
		{
			stReg.reg = 0x0020 + i;
			retValue = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8Temp[i]);
		}
		for(i = 0; i < 8; i++)
		{
			if(u8Temp[i] != testRWData[i])
			{
				PrError("Invalid data[Wr:0x%02x, Rd:0x%02x]\n", testRWData[i] COMMA u8Temp[i]);
				retValue = -1;
				break;
			}
		}
	}
	if(retValue < 0) return(eERROR_FAILURE);
	else PrPrint("%x-INTF Single R/W Ok..\n", chanAddr);

	/* Should do write common registers. */
	if( (result = PPDRV_PVIRX_SetTableStdResol(cid, chanAddr, DEFAULT_CAMERA_STANDARD, DEFAULT_CAMERA_RESOLUTION, (enum _eVideoResolution)stDefaultCameraVideoRelation[DEFAULT_CAMERA_RESOLUTION].eVideoResolution, bWaitStableStatus)) != eSUCCESS)
	{
		PrErrorString("SetTableStResol\n");
		return(result);
	}

	/* Check Burst UTC Tx buffer Read - Write */
    retValue = 0;
	{
		stReg.reg = 0x0060;
		u8RegData = 0x80;
		result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData);
		stReg.reg = 0x0050;
		u8RegData = 0x80;
		result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData);
		stReg.reg = 0x004D;
		u8RegData = 0x40;
		result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData);

		stReg.reg = 0x0051;
		result = PPDRV_PVIRX_WriteBurst(cid, chanAddr, stReg.b.addr, 8, (PP_U8 *)testRWData);

		stReg.reg = 0x0052;
		result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData);

		if(u8RegData != 8)
		{
			PrError("Invalid data[ u8RegData:0x%02x]\n", u8RegData);
			retValue = -1;
		}

		stReg.reg = 0x0060;
		u8RegData = 0x00;
		PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData);
		stReg.reg = 0x0050;
		u8RegData = 0x00;
		PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData);

	}
	if(retValue < 0) return(eERROR_FAILURE);
	else PrPrint("%x-INTF Burst UTC Tx buffer R/W Ok..\n", chanAddr);

	return(eSUCCESS);
}

PP_RESULT_E PPDRV_PVIRX_CheckChipID(const PP_S32 IN cid, const PP_U8 IN chanAddr, PP_U16 OUT *pRetChipID, PP_U8 OUT *pRetRevID, PP_S32 OUT *pRetRWVerify)
{
	PP_U8 u8Temp = 0;
	PP_U32 chipID = 0;
	PP_U8 u8RevID = 0;
	_stPVIRX_Reg stReg;

	if( (pRetChipID == NULL) || (pRetRevID == NULL) || (pRetRWVerify == NULL) ) return(eERROR_FAILURE);

	stReg.reg = PVIRX_ADDR_CHIPID_MSB;
	if( (PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, (PP_U8 *)&u8Temp)) != eSUCCESS)
	{
		return(eERROR_FAILURE);
	}
	else
	{
		chipID = ((PP_U32)u8Temp)<<8;
		stReg.reg = PVIRX_ADDR_CHIPID_LSB;
		if( (PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, (PP_U8 *)&u8Temp)) != eSUCCESS)
		{
			return(eERROR_FAILURE);
		}
		else
		{
			chipID |= (PP_U32)u8Temp;

			stReg.reg = PVIRX_ADDR_REVID;
			if( (PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, (PP_U8 *)&u8RevID)) != eSUCCESS)
			{
				return(eERROR_FAILURE);
			}
			else
			{

				LOG_NOTICE("%x-PVIRX ChipID:0x%04x RevID:0x%02x... %s\n", chanAddr, chipID, u8RevID, ((chipID&0xFF00) == 0x2000) ? "Ok.":"Fail.");

				if( (chipID&0xFF00) != 0x2000) 
				{
					return(eERROR_FAILURE);
				}
			}
		}
	}

	{
		PrPrint("%x-Check INTF R/W test\n", chanAddr);

		*pRetChipID = chipID;
		*pRetRevID = u8RevID;

		/* check interface read & write */
		if( (PPDRV_PVIRX_CheckINTFReadWrite(cid, chanAddr)) == eSUCCESS)
		{
			*pRetRWVerify = TRUE;
#ifdef SUPPORT_PVIRX_FPGA
			/* forcely set 0. When fpga no power down, don't reset pvirx register. forcely disable interrupt. */
			stReg.reg = 0x0089;
			if( (PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, 0x00)) != eSUCCESS)
			{
				return(eERROR_FAILURE);
			}
#endif // SUPPORT_PVIRX_FPGA
		}
	}

	return(eSUCCESS);
}

PP_RESULT_E PPDRV_PVIRX_SetAttrChip(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stAttrChip IN *pstPviRxAttrChip)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	PrDbg("%x-Set Chip attribute register\n", chanAddr);

	/* set chip attribute */
	if(pstPviRxAttrChip->chanAddr == chanAddr)
	{
		{    
			PrDbgString("  [ENV] Chip Attribute:\n");
			PrDbg("    chanAddr(0x%02X)\n", pstPviRxAttrChip->chanAddr);
			PrDbg("    vinMode(0:[Differential|VinPN], 1:VinP, 3:VinN): %d\n", pstPviRxAttrChip->vinMode);
		}            

		//PP_U8 vinMode;
		stReg.reg = 0x0015;
		u8RegData = (pstPviRxAttrChip->vinMode & 0x3)<<0;
		u8RegMask = 0x03;
		result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData);
	}
	else
	{
		PrErrorString("Invalid interface channel addr\n");
		result = eERROR_FAILURE;
	}

	return(result);
}

PP_RESULT_E PVIRX_SetTableIRQ(const PP_S32 cid, const PP_U8 chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	const _stPVIRX_Table_Common *pTblIRQ = NULL;
	_stPVIRX_Reg stReg;
	PP_S32 regInx = 0;
	PP_U8 u8RegData;

	PrDbg("%x-Set irq register\n", chanAddr);

	/* get current novid status. and set novid level. */
	{/*{{{*/
		PP_U8 novidStatus[3];
		PP_U8 tryCnt = 0;
		PP_U8 i = 0;

		do
		{
			for(i = 0; i < 3; i++)
			{
				stReg.reg = 0x008D;
				u8RegData = 0;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				novidStatus[i] = u8RegData&0x10;
                //LOOPDELAY_MSEC(30);
			}
			tryCnt++;
		} while( (tryCnt < 4) && ((novidStatus[0] != novidStatus[1]) || (novidStatus[0] != novidStatus[2]) || (novidStatus[1] != novidStatus[2])) );
		PrDbg("%x-tryCnt:%d, current novid status:%d, invert level.\n", chanAddr COMMA tryCnt COMMA (u8RegData>>4)&1);

		//LEVEL MODE, VALUE VFD/NOVID
		{
			PP_U8 u8Temp = 0;
			stReg.reg = 0x0089;
			if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8Temp)) != eSUCCESS)
			{
				PrErrorString("invalid read\n");
				return(result);
			}
			PrDbg("%x-0x%02x\n", chanAddr COMMA u8Temp);
			if( (u8Temp & 0x10) == 0) //disabled novid interrupt.(== is first setting time)
			{
				//u8RegData |= 0x10; //forcely novideo status. make camera connect start.
				u8RegData &= ~0x10; //forcely video status.
			}

			stReg.reg = 0x0084; 
			u8RegData = 0x30 | ((0x08|((u8RegData&0x10)?0x00:0x04))&0x0F);
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
		}
	}/*}}}*/

        pTblIRQ = (const _stPVIRX_Table_Common *)stPVIRX_Table_IRQ;
        regInx = 0; 
        stReg.reg = pTblIRQ[regInx].stReg.reg;
        while(stReg.reg != 0xFFFF)
        {    
                u8RegData = pTblIRQ[regInx].u8Data;
                PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);

                if(stReg.reg == 0x0084) // novid level.
                {    
                        regInx++;
                        stReg.reg = pTblIRQ[regInx].stReg.reg;
                        continue;
                }    

                if(stReg.reg == 0x0089) // disable vfd.
                {    
#ifdef SUPPORT_AUTODETECT_PVI
                    u8RegData &= 0xDF;
#else
                    u8RegData &= 0x1F; //disable WAKEUP(TIMER1/0)
#endif // SUPPORT_AUTODETECT_PVI
                }    

                if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS) 
                {    
                        PrErrorString("invalid write\n");
                        return(result);
                }

                regInx++;
                stReg.reg = pTblIRQ[regInx].stReg.reg;
        }

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_SetTableStdResol(const PP_S32 IN cid, const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution, const enum _eVideoResolution IN videoResolution, const PP_S32 IN bWaitStableStatus)
{
	PP_RESULT_E result = eERROR_FAILURE;
	const _stPVIRX_Table_STD_HDResolution *pTblSTDHDResolution = NULL;
	const _stPVIRX_Table_ETC_STDResolution *pTblETCSTDResolution = NULL;
	const _stPVIRX_Table_EXT_HDResolution *pTblEXTHDResolution = NULL;
	const _stPVIRX_Table_ETC_EXTResolution *pTblETCEXTResolution = NULL;
	const _stPVIRX_Table_SDResolution *pTblSDResolution = NULL;
	_stPVIRX_Reg stReg;
	PP_S32 regInx = 0;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;
	PP_S32 chipInx = 0;

    PrDbg("%x-Set vdec [Camera:%d(%s), videoResolution:%d(%s)]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);

	if(cameraStandard >= max_camera_standard)
	{
		PrError("Invalid standard:%d\n", cameraStandard);
		return(eERROR_FAILURE);
	}
	if(videoResolution >= max_video_resolution)
	{
		PrError("Invalid video resolution:%d\n", videoResolution);
		return(eERROR_FAILURE);
	}

	/* get chip attribute */
	for(chipInx = 0; chipInx < MAX_PVIRX_CHANCNT; chipInx++)
	{
		if(PVIRX_ATTR_CHIP[chipInx].chanAddr == chanAddr)
		{
			break;
		}
	}

	if(videoResolution > video_1920x1080p25)
	{
		pTblETCEXTResolution = (const _stPVIRX_Table_ETC_EXTResolution *)stPVIRX_Table_ETC_EXTResol;
	}
	else
	{
		pTblETCSTDResolution = (const _stPVIRX_Table_ETC_STDResolution *)stPVIRX_Table_ETC_STDResol;
	}

	// Write etc setting for standard
	if(videoResolution <= video_1920x1080p25)
	{/*{{{*/
		regInx = 0;
		stReg.reg = pTblETCSTDResolution[regInx].stReg.reg;
		PrDbg("%x-Write etc setting for standard\n", chanAddr);
		while(stReg.reg != 0xFFFF)
		{
			u8RegData = pTblETCSTDResolution[regInx].u8Data[videoResolution];
			PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);

			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}

			regInx++;
			stReg.reg = pTblETCSTDResolution[regInx].stReg.reg;
		}
	}/*}}}*/
	else
	{/*{{{*/
		regInx = 0;
		stReg.reg = pTblETCEXTResolution[regInx].stReg.reg;
		PrDbg("%x-Write etc setting for extend\n", chanAddr);
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

	if( (cameraStandard == CVBS) && (pTblSDResolution != NULL) )
	{/*{{{*/
		regInx = 0;
		stReg.reg = pTblSDResolution[regInx].stReg.reg;
		while(stReg.reg != 0xFFFF)
		{
			u8RegData = pTblSDResolution[regInx].u8Data[videoResolution];
			PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);
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
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}

			regInx++;
			stReg.reg = pTblSDResolution[regInx].stReg.reg;
		}
	}/*}}}*/
	else
	{
		if( (pTblSTDHDResolution != NULL) )
		{/*{{{*/
			regInx = 0;
			stReg.reg = pTblSTDHDResolution[regInx].stReg.reg;
			while(stReg.reg != 0xFFFF)
			{
				u8RegData = pTblSTDHDResolution[regInx].u8Data[videoResolution-video_1280x720p60]; 
				PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);
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
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}

				regInx++;
				stReg.reg = pTblSTDHDResolution[regInx].stReg.reg;
			}
		}/*}}}*/
		if( (pTblEXTHDResolution != NULL) )
		{/*{{{*/
			regInx = 0;
			stReg.reg = pTblEXTHDResolution[regInx].stReg.reg;
			while(stReg.reg != 0xFFFF)
			{
				u8RegData = pTblEXTHDResolution[regInx].u8Data[videoResolution-video_1280x960p30]; 
				PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);
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
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}

				regInx++;
				stReg.reg = pTblEXTHDResolution[regInx].stReg.reg;
			}
		}/*}}}*/
	}

#ifdef DEFAULT_CEQ_GAIN_DIST
    if(DEFAULT_CEQ_GAIN_DIST != 0)
    {
        _stPVIRX_Table_CEQ_VADC *pTblVadcHD = NULL;
        PP_U8 step = 0;
        PP_S32 tblReolution = 0;
        PP_U8 vadcGain = 0;
        PP_U8 gainfit = 0;
        PP_U8 y_out_gain = 0;

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

        if(DEFAULT_CEQ_GAIN_DIST > 1000) step = 10;
        else step = DEFAULT_CEQ_GAIN_DIST/100;
        PrDbg("%x-step(%d)\n", chanAddr COMMA step);

        vadcGain = pTblVadcHD[step].pData[tblReolution][0] & 0xFF;
        gainfit = (pTblVadcHD[step].pData[tblReolution][1]>>8) & 0xFF;
        y_out_gain = pTblVadcHD[step].pData[tblReolution][1] & 0xFF;
        PrDbg("%x-find vadc param vadcGain:%02x\n", chanAddr COMMA vadcGain);

        PrPrint("DEFAULT_CEQ_GAIN_DIST:%d, vadc:0x%02x\n", DEFAULT_CEQ_GAIN_DIST COMMA vadcGain);

        /* vadc_gain_sel */
        stReg.reg = 0x0011;
        u8RegMask = 0x70;
        u8RegData = (vadcGain&0x7)<<4;
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
    }

#endif //DEFAULT_CEQ_GAIN_DIST

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

	if(bWaitStableStatus)
	{
		/* should wait 400msec. for stable detection of std&resol. */
		LOOPDELAY_MSEC(400);
	}

	return(result);

}

PP_RESULT_E PPDRV_PVIRX_SetNovidInitIRQ(const PP_S32 cid, const PP_U8 chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	PrDbg("%x-Set Novid Init irq register\n", chanAddr);

	/* get current novid status. and set novid level. */
	{/*{{{*/
		PP_U8 novidStatus[3];
		PP_U8 tryCnt = 0;
		PP_U8 i = 0;

		do
		{
			for(i = 0; i < 3; i++)
			{
				stReg.reg = 0x008D;
				u8RegData = 0;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				novidStatus[i] = u8RegData&0x10;
                //LOOPDELAY_MSEC(30);
			}
			tryCnt++;
		} while( (tryCnt < 4) && ((novidStatus[0] != novidStatus[1]) || (novidStatus[0] != novidStatus[2]) || (novidStatus[1] != novidStatus[2])) );
		PrDbg("%x-tryCnt:%d, current novid status:%d, invert level.\n", chanAddr COMMA tryCnt COMMA (u8RegData>>4)&1);

		//LEVEL MODE, VALUE VFD/NOVID
		{
			stReg.reg = 0x0084; 
			u8RegData = 0x30 | ((0x08|((u8RegData&0x10)?0x04:0x00))&0x0F);
			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}
		}
	}/*}}}*/

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_SetInit(const PP_S32 IN cid, const PP_U8 IN chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;

	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;

	PP_S32 chipInx = 0;
	PP_S32 bWaitStableStatus = 0;

	_stPVIRX_Reg stReg;
	PP_U8 u8Temp = 0;

	/* get chip number. */
	for(chipInx = 0; chipInx < MAX_PVIRX_CHANCNT; chipInx++)
	{
		if(PVIRX_ATTR_CHIP[chipInx].chanAddr == chanAddr)
		{
			break;
		}
	}

	if(gpPviRxDrvHost[chipInx] == NULL)
	{
		memset(&gPviRxDrvHost[chipInx], 0, sizeof(_PviRxDrvHost));
		gpPviRxDrvHost[chipInx] = (_PviRxDrvHost *)&gPviRxDrvHost[chipInx];

		gpPviRxDrvHost[chipInx]->stPrRxMode.standard = (enum _eCameraStandard)DEFAULT_CAMERA_STANDARD;
		gpPviRxDrvHost[chipInx]->stPrRxMode.cameraResolution = (enum _eCameraResolution)DEFAULT_CAMERA_RESOLUTION;
		gpPviRxDrvHost[chipInx]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[DEFAULT_CAMERA_RESOLUTION].eVideoResolution;
		if( (DEFAULT_CAMERA_RESOLUTION == camera_ntsc) || (DEFAULT_CAMERA_RESOLUTION == camera_pal) )
		{
			if(gbPviRxSelSD960H)
			{
				gpPviRxDrvHost[chipInx]->stPrRxMode.vidOutResolution += video_960x480i60;
			}
		}
	}

	if( (gpPviRxDrvHost[chipInx]->stPrRxMode.cameraResolution == camera_ntsc) || (gpPviRxDrvHost[chipInx]->stPrRxMode.cameraResolution == camera_pal) )
	{
		gpPviRxDrvHost[chipInx]->stPrRxMode.standard = CVBS;
	}

	stReg.reg = PVIRX_ADDR_CHIPID_MSB;
	if( (PPDRV_PVIRX_Read(0, chanAddr, stReg.b.addr, (PP_U8 *)&u8Temp)) != eSUCCESS)
	{
		return(eERROR_FAILURE);
	}
	gpPviRxDrvHost[chanAddr]->chipID = ((PP_U32)u8Temp)<<8;

	stReg.reg = PVIRX_ADDR_CHIPID_LSB;
	if( (PPDRV_PVIRX_Read(0, chanAddr, stReg.b.addr, (PP_U8 *)&u8Temp)) != eSUCCESS)
	{
		return(eERROR_FAILURE);
	}
	gpPviRxDrvHost[chanAddr]->chipID |= (PP_U32)u8Temp;

	stReg.reg = PVIRX_ADDR_REVID;
	if( (PPDRV_PVIRX_Read(0, chanAddr, stReg.b.addr, (PP_U8 *)&gpPviRxDrvHost[chanAddr]->u8RevID)) != eSUCCESS)
	{
		return(eERROR_FAILURE);
	}

	if(gpPviRxDrvHost[chanAddr]->chipID == 0x2000)
	{
		gpPviRxDrvHost[chanAddr]->u8Initialized = TRUE;
	}
	else
	{
		gpPviRxDrvHost[chanAddr]->u8Initialized = FALSE;
		return(eERROR_FAILURE);
	}

	cameraStandard = gpPviRxDrvHost[chipInx]->stPrRxMode.standard;
	cameraResolution = gpPviRxDrvHost[chipInx]->stPrRxMode.cameraResolution;
	videoResolution = gpPviRxDrvHost[chipInx]->stPrRxMode.vidOutResolution;

	PrDbg("### PVIRX Driver Version : v%s ### chipinx:%d\n", _VER_PVIRX COMMA chipInx);
	PrPrint("%x-Init Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s)\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution]);
	if( (result = PPDRV_PVIRX_SetTableStdResol(cid, chanAddr, cameraStandard, cameraResolution, videoResolution, bWaitStableStatus)) != eSUCCESS)
	{
		PrErrorString("SetTableStResol\n");
		return(result);
	}

	if( (result = PPDRV_PVIRX_SetAttrChip(cid, chanAddr, &PVIRX_ATTR_CHIP[chipInx])) != eSUCCESS)
	{
		PrErrorString("SetAttrChip\n");
		return(result);
	}
	if( (result = PVIRX_SetTableIRQ(cid, chanAddr)) != eSUCCESS)
	{
		PrErrorString("SetTableIRQ\n");
		return(result);
	}
#ifdef SUPPORT_PVIRX_UTC
	if( (result = PPDRV_PVIRX_UTC_SetTable(cid, chanAddr, cameraStandard, cameraResolution)) < 0)
	{
		PrErrorString("UTC_SetTable\n");
		return(result);
	}
#endif // SUPPORT_PVIRX_UTC
#ifdef SUPPORT_PVIRX_FPGA
	if( (result = PPDRV_PVIRX_FPGA_SetTable(chanAddr)) < 0)
	{
		PrErrorString("FPGA_SetTable\n");
		return(result);
	}
#endif // SUPPORT_PVIRX_FPGA

    /* C_det & !C_lock check */
	{/*{{{*/
        _stPVIRX_VidStatusReg curStVidStatusReg;
        PP_U8 u8RegData;
        PP_U32 defChromaPhase = 0;
        PP_U8 cntChromaLockTunn = 0;
        PP_U32 tunnChromaPhase = 0;

        PrDbg("%x-Check C_det & !C_lock\n", chanAddr);

		PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
		if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
		{/*{{{*/
			PrDbg("%x-chroma lock\n", chanAddr);
		}/*}}}*/
		else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
		{/*{{{*/
			/* get current chromaphase register value */
			{/*{{{*/
				stReg.reg = 0x0146;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				defChromaPhase = (PP_U32)(u8RegData & 0x1F)<<16;

				stReg.reg = 0x0147;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				defChromaPhase |= (PP_U32)(u8RegData & 0xFF)<<8;

				stReg.reg = 0x0148;
				if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid read\n");
					return(result);
				}
				defChromaPhase |= (PP_U32)(u8RegData & 0xFF);

				PrDbg("%x-base CPhaseRefBase:0x%08lx\n", chanAddr COMMA defChromaPhase);
            }/*}}}*/
            while(cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
            {
                LOOPDELAY_MSEC(30);

                PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
                if( (!curStVidStatusReg.b.det_chroma) || (cntChromaLockTunn >= MAX_CNT_TUNN_CHROMALOCK) )
                {/*{{{*/
                    PrPrint("%x-Can't Det chroma or chroma lock.\n", chanAddr);
                    tunnChromaPhase = defChromaPhase;

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
                    return(result);
                } /*}}}*/
                else if(cntChromaLockTunn < MAX_CNT_TUNN_CHROMALOCK)
                {
                    if( (curStVidStatusReg.b.det_chroma) && (curStVidStatusReg.b.lock_chroma) )
                    {/*{{{*/
                        PrPrint("%x-chroma lock\n", chanAddr);

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

                    }/*}}}*/
                    else if( (curStVidStatusReg.b.det_chroma) && (!curStVidStatusReg.b.lock_chroma) )
                    {/*{{{*/
                        tunnChromaPhase = (cntChromaLockTunn >>1) + 1;
                        tunnChromaPhase *= 40;
                        if(cntChromaLockTunn & 1) // - tunning
                        {
                            tunnChromaPhase = defChromaPhase - tunnChromaPhase;
                        }
                        else // + tunning
                        {
                            tunnChromaPhase = defChromaPhase + tunnChromaPhase;
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

                        cntChromaLockTunn++;
                    }/*}}}*/
                }
            }
        }/*}}}*/
    }/*}}}*/

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_Isr(const PP_S32 IN cid, const PP_U8 IN chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_PviRxDrvHost *pHost = (_PviRxDrvHost *)gpPviRxDrvHost[chanAddr];
	_stPVIRX_Isr *pstIsr = (_stPVIRX_Isr *)&pHost->stPVIRX_Isr;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegMask;
	PP_U8 u8RegData = 0;
	PP_S32 i;
	PP_S32 regInx = 0;
	enum _eVideoResolution videoResolution = 0;
	_stPVIRX_VidStatusReg curStVidStatusReg;

	const _stPVIRX_Table_ETC_EXTResolution *pTblETCEXTResolution = NULL;

	/* get irq clr & status */
	{/*{{{*/
		stReg.reg = 0x008A;
		if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
		{
			PrErrorString("can't read\n");
			return(result);
		}
		pstIsr->stIrqClr.reg[0] = u8RegData;
		stReg.reg = 0x008B;
		if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
		{
			PrErrorString("can't read\n");
			return(result);
		}
		pstIsr->stIrqClr.reg[1] = u8RegData;

		stReg.reg = 0x008C;
		if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
		{
			PrErrorString("can't read\n");
			return(result);
		}
		pstIsr->stIrqStatus.reg[0] = u8RegData;
		stReg.reg = 0x008D;
		if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
		{
			PrErrorString("can't read\n");
			return(result);
		}
		pstIsr->stIrqStatus.reg[1] = u8RegData;
	}/*}}}*/
	PrDbg("%x-Irq:0x%02x/0x%02x\n", chanAddr COMMA pstIsr->stIrqClr.reg[0] COMMA pstIsr->stIrqClr.reg[1]);

	if(pstIsr->stIrqClr.b.novid)
	{/*{{{*/
		PrDbg("%x-NOVID:%d\n", chanAddr COMMA pstIsr->stIrqStatus.b.novid);
		if(pstIsr->stIrqStatus.b.novid) //Video->Novideo
		{
			/* stop VFD irq */
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

#if defined(SUPPORT_PVIRX_RECALL_PREV_CAM)
			/* stop monitoring camera */
			_SET_BIT(JOB_PAUSE_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);

			/* initialize novide->video check flag. */
			pstIsr->stJobProc.camPlugInCheckCnt = 0;
			pstIsr->stJobProc.cntJobTry = 0;
#else
			/* Init CEQ gain register. */
			{/*{{{*/
				stReg.reg = 0x0011;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
                stReg.reg = 0x0012;
                u8RegMask = 0x1F;
                u8RegData = 0x00;
                if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
                {
                    PrErrorString("invalid write\n");
                    return(result);
                }
				stReg.reg = 0x0014;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/

			/* initialize argument */
			memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));
#endif //!defined(SUPPORT_PVIRX_RECALL_PREV_CAM)

			/* set next camera plug in level interrupt */
			stReg.reg = 0x0084;
			u8RegMask = 0x04;
			u8RegData = (PP_U8)~(pstIsr->stIrqStatus.b.novid<<2);
			if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}

			PrDbg("%x-Video->Novideo interrupt\n", chanAddr);
			if(!_TEST_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrqStatus))
            {
                PrDbg("%x-Video->Novideo event. alarm event to task.\n", chanAddr);
                //Send event to task.
                {/*{{{*/
                    PP_VID_S stVidStatus;
                    stVidStatus.s8VidPort = chanAddr;
                    stVidStatus.s8Status = -1;

                    if( (AppTask_SendCmdFromISR(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, &stVidStatus, sizeof(stVidStatus))) != eSUCCESS)
                    {
                        LOG_DEBUG_ISR("Error!!! Fail send command\n");
                    }
                }/*}}}*/                            
            }
			_SET_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrq);
			_SET_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrqStatus);

		}
		else //Novideo->Video
		{
#if defined(SUPPORT_PVIRX_RECALL_PREV_CAM)
			if( pstIsr->stJobProc.EQ_CNT >= 2 ) // previous process done completed to end sequence.
			{
				enum _eCameraStandard cameraStandard = 0;
				enum _eCameraResolution cameraResolution = 0;
				enum _eVideoResolution videoResolution = 0;

				PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
				PrDbg("%x,%x,%x\n", curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1] COMMA curStVidStatusReg.reg[2]);
				if( (curStVidStatusReg.b.lock_gain) && (curStVidStatusReg.b.lock_clamp) && (curStVidStatusReg.b.lock_hperiod) )
				{
					pstIsr->stJobProc.camPlugInCheckCnt++;
				}
				else
				{
					/* wait stable status */
					pstIsr->stJobProc.camPlugInCheckCnt = 0;
					pstIsr->stJobProc.cntJobTry++;
				}
				PrDbg("%x-camPlugInCheckCnt:%d, recall tryCnt:%d\n", chanAddr COMMA pstIsr->stJobProc.camPlugInCheckCnt COMMA pstIsr->stJobProc.cntJobTry);

				/* stable camera plugin(camPlugInCheckCnt>=2) or unstable(cntJobTry>=4). if unstable, forcely set detect CVI. */
				if( (pstIsr->stJobProc.camPlugInCheckCnt == 4) || (pstIsr->stJobProc.cntJobTry >= 5) )
				{/*{{{*/
					PrDbg("%x-Novideo->Video interrupt\n", chanAddr);
					_SET_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrq);
					_CLEAR_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrqStatus);

					PrDbg("%x-Novideo->Video event. alarm event to task.\n", chanAddr);
                    //Send event to task.
                    {/*{{{*/
                        PP_VID_S stVidStatus;
                        stVidStatus.s8VidPort = chanAddr;
                        stVidStatus.s8Status = chanAddr;

                        if( (AppTask_SendCmdFromISR(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, &stVidStatus, sizeof(stVidStatus))) != eSUCCESS)
                        {
                            LOG_DEBUG_ISR("Error!!! Fail send command\n");
                        }
                    }/*}}}*/

					PrDbg("%x-check lock status reg1:0x%02x\n", chanAddr COMMA curStVidStatusReg.reg[1]);
					if( (!curStVidStatusReg.b.lock_chroma) )
					{
						PrDbg("%x-detect changed camera on Recall mode\n", chanAddr);
						PrDbgString("In recall mode, initialize JobProc\n");

						/* Init CEQ gain register. */
						{/*{{{*/
							stReg.reg = 0x0011;
							u8RegData = 0x00;
							if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid write\n");
								return(result);
							}
                            stReg.reg = 0x0012;
                            u8RegMask = 0x1F;
                            u8RegData = 0x00;
                            if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
                            {
                                PrErrorString("invalid write\n");
                                return(result);
                            }
                            stReg.reg = 0x0014;
                            u8RegData = 0x00;
                            if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid write\n");
								return(result);
							}
						}/*}}}*/
						/* initialize argument */
						memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));
					}
					else
					{/*{{{*/
						/* set next novideo level interrupt */
						stReg.reg = 0x0084;
						u8RegMask = 0x04;
						u8RegData = (PP_U8)~(pstIsr->stIrqStatus.b.novid<<2);
						if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						PPDRV_PVIRX_ReadStdResol(cid, chanAddr, &pstIsr->stJobProc.stVidStatusReg, &cameraStandard, &cameraResolution, &videoResolution);
						PrDbg("Recall prev cam. bitJobProcess:0x%lx\n", pstIsr->stJobProc.bitJobProcess);
						PrPrint("%x-Recall final vdec format[Camera:%d(%s), cameraResolution:%d(%s), videoResolution:%d(%s) reJudge:%d]\n", chanAddr COMMA cameraStandard COMMA _strCameraStandard[cameraStandard] COMMA cameraResolution COMMA _strCameraResolution[cameraResolution] COMMA videoResolution COMMA _strVideoResolution[videoResolution] COMMA pstIsr->stJobProc.reJudgeStdResol);

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

						pHost->stPrRxMode.standard = cameraStandard;
						pHost->stPrRxMode.cameraResolution = cameraResolution;
						pHost->stPrRxMode.vidOutResolution = videoResolution;

						_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrq);
						_SET_BIT(IRQ_SET_RXMODE, &pstIsr->stUserPoll.bitIrqStatus);

						_CLEAR_BIT(JOB_PAUSE_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess);
					}/*}}}*/
				}/*}}}*/
			}
			else // previous process do not completed sequence.
			{
				if(_TEST_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess))
				{
					if(pstIsr->stJobProc.numJobStep != 1)
					{
						PrDbgString("In recall mode, initialize JobProc\n");
						/* Init CEQ gain register. */
						{/*{{{*/
							stReg.reg = 0x0011;
							u8RegData = 0x00;
							if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid write\n");
								return(result);
							}
                            stReg.reg = 0x0012;
                            u8RegMask = 0x1F;
                            u8RegData = 0x00;
                            if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
                            {
                                PrErrorString("invalid write\n");
                                return(result);
                            }
                            stReg.reg = 0x0014;
							u8RegData = 0x00;
							if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid write\n");
								return(result);
							}
						}/*}}}*/
						/* initialize argument */
						memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));
					}
				}
			}

#endif //defined(SUPPORT_PVIRX_RECALL_PREV_CAM)

			if(!_TEST_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess))
			{
				PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
				PrDbg("%x,%x,%x\n", curStVidStatusReg.reg[0] COMMA curStVidStatusReg.reg[1] COMMA curStVidStatusReg.reg[2]);
				if( (curStVidStatusReg.b.lock_gain) && (curStVidStatusReg.b.lock_clamp) && (curStVidStatusReg.b.lock_hperiod) )
				{
					pstIsr->stJobProc.camPlugInCheckCnt++;
				}
				else
				{
					/* wait stable status */
					pstIsr->stJobProc.camPlugInCheckCnt = 0;
					pstIsr->stJobProc.cntJobTry++;
				}
				PrDbg("%x-camPlugInCheckCnt:%d, tryCnt:%d\n", chanAddr COMMA pstIsr->stJobProc.camPlugInCheckCnt COMMA pstIsr->stJobProc.cntJobTry);

				/* stable camera plugin(camPlugInCheckCnt>=2) or unstable(cntJobTry>=4). if unstable, forcely set detect CVI. */
				if( (pstIsr->stJobProc.camPlugInCheckCnt >= 3) || (pstIsr->stJobProc.cntJobTry >= 4) )
				{/*{{{*/
					PrDbg("%x-Novideo->Video interrupt\n", chanAddr);
					_SET_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrq);
					_CLEAR_BIT(IRQ_NOVID, &pstIsr->stUserPoll.bitIrqStatus);

					PrDbg("%x-Novideo->Video event. alarm event to task.\n", chanAddr);
                    //Send event to task.
                    {/*{{{*/
                        PP_VID_S stVidStatus;
                        stVidStatus.s8VidPort = chanAddr;
                        stVidStatus.s8Status = chanAddr;

                        if( (AppTask_SendCmdFromISR(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, &stVidStatus, sizeof(stVidStatus))) != eSUCCESS)
                        {
                            LOG_DEBUG_ISR("Error!!! Fail send command\n");
                        }
                    }/*}}}*/

					/* Write pre setting for 900p. For valid operating 900p */
					if( curStVidStatusReg.b.det_ifmt_res == DET_HD960p)
					{/*{{{*/
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

					/* set next novideo level interrupt */
					stReg.reg = 0x0084;
					u8RegMask = 0x04;
					u8RegData = (PP_U8)~(pstIsr->stIrqStatus.b.novid<<2);
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					/* save cur video status registers */
					memcpy(&pstIsr->stJobProc.stStartVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
					memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
					if( (pstIsr->stJobProc.cntJobTry >= 4) )
					{
						PrDbg("%x-Forcely Set abnormal [CVI 1080p25]\n", chanAddr);
						pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;
						pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
						pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD1080p;
					}
					pstIsr->stJobProc.cntJobTry = 0;

					/* set start camera in flag */
					_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
					pstIsr->stJobProc.numJobStep = 1;
					/* do timer1 process next isr. */
					if(pstIsr->stIrqClr.b.timer1) 
					{
						/* if current timer1 interrupt set, wait next timer1 interrupt. */
						pstIsr->stJobProc.cntWait300MsecTime = 1;
					}
					else
					{
						/* if not current timer1 interrupt set, dont' wait next timer1 interrupt. */
						pstIsr->stJobProc.cntWait300MsecTime = 0;
					}

                    /* next processing in timer1 isr. *///periodic job 300msec
#ifndef SUPPORT_AUTODETECT_PVI
                    {
							stReg.reg = 0x0089;
                            if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
                            {
                                PrErrorString("invalid read\n");
                                return(result);
                            }

                            u8RegData |= 0x80; //enable WAKEUP(TIMER1)

                            if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS) 
                            {    
                                PrErrorString("invalid write\n");
                                return(result);
                            }
                    }
#endif // SUPPORT_AUTODETECT_PVI

				}/*}}}*/
			}
		}
	}/*}}}*/
	if(pstIsr->stIrqClr.b.vfd)
	{/*{{{*/
		PrDbg("%x-VFD:%d(novideo:%d)\n", chanAddr COMMA pstIsr->stIrqStatus.b.vfd COMMA pstIsr->stIrqStatus.b.novid);

		if(pstIsr->stIrqStatus.b.novid == 0)
		{
			pstIsr->stIrqClr.b.vfd = 0; //Don't clear irq

			if(!_TEST_BIT(JOB_START_VFD_CAMERA, &pstIsr->stJobProc.bitJobProcess) && !_TEST_BIT(JOB_DONE_VFD_CAMERA, &pstIsr->stJobProc.bitJobProcess))
			{
				/* initialize argument */
				memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));
				PrDbg("%x-Start VFD interrupt\n", chanAddr);
				_SET_BIT(IRQ_VFD, &pstIsr->stUserPoll.bitIrq);
				_SET_BIT(IRQ_VFD, &pstIsr->stUserPoll.bitIrqStatus);

				_SET_BIT(JOB_START_VFD_CAMERA, &pstIsr->stJobProc.bitJobProcess);

			}
			else
			{
				PPDRV_PVIRX_ReadVidStatusReg(cid, chanAddr, &curStVidStatusReg);
				if( (curStVidStatusReg.b.lock_gain) && (curStVidStatusReg.b.lock_clamp) && (curStVidStatusReg.b.lock_hperiod) )
				{
					pstIsr->stJobProc.camPlugInCheckCnt++;
				}
				else
				{
					/* wait stable status */
					pstIsr->stJobProc.camPlugInCheckCnt = 0;
					pstIsr->stJobProc.cntJobTry++;
				}
				PrDbg("%x-VFD camPlugInCheckCnt:%d, tryCnt:%d\n", chanAddr COMMA pstIsr->stJobProc.camPlugInCheckCnt COMMA pstIsr->stJobProc.cntJobTry);

				if( (pstIsr->stJobProc.camPlugInCheckCnt >= 2) || (pstIsr->stJobProc.cntJobTry >= 4) )
				{/*{{{*/

					pstIsr->stIrqClr.b.vfd = 1; //Clear irq
					_SET_BIT(IRQ_VFD, &pstIsr->stUserPoll.bitIrq);
					_CLEAR_BIT(IRQ_VFD, &pstIsr->stUserPoll.bitIrqStatus);

					/* Write pre setting for 900p. For valid operating 900p */
					if( curStVidStatusReg.b.det_ifmt_res == DET_HD960p)
					{/*{{{*/
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

					/* stop VFD irq */
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
					/* initialize argument */
					memset(&pstIsr->stJobProc, 0, sizeof(_stJobProc));

					/* save cur video status registers */
					memcpy(&pstIsr->stJobProc.stStartVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
					memcpy(&pstIsr->stJobProc.stVidStatusReg, &curStVidStatusReg, sizeof(_stPVIRX_VidStatusReg));
					if( (pstIsr->stJobProc.cntJobTry >= 4) )
					{
						PrDbg("%x-Forcely Set abnormal [CVI 1080p25]\n", chanAddr);
						pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_std = DET_CVI;
						pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_ref = DET_25Hz;
						pstIsr->stJobProc.stVidStatusReg.b.det_ifmt_res = DET_HD1080p;
					}
					pstIsr->stJobProc.cntJobTry = 0;

					/* set start camera in flag */
					_SET_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess);
					_SET_BIT(JOB_DONE_VFD_CAMERA, &pstIsr->stJobProc.bitJobProcess);
					pstIsr->stJobProc.numJobStep = 1;
					/* do timer1 process next isr */
					if(pstIsr->stIrqClr.b.timer1) 
					{
						pstIsr->stJobProc.cntWait300MsecTime = 1;
					}
					else
					{
						pstIsr->stJobProc.cntWait300MsecTime = 0;
					}
				}/*}}}*/
			}
		}
	}/*}}}*/
#ifdef SUPPORT_PVIRX_UTC
	if(pstIsr->stIrqClr.reg[0] != 0) //ptz
	{
		PrDbg("%x-PTZ:0x%02x\n", chanAddr COMMA pstIsr->stIrqClr.reg[0]);
		_SET_BIT(IRQ_UTC, &pstIsr->stUserPoll.bitIrq);
		if(pstIsr->stIrqClr.b.ptz0) //tx done
		{/*{{{*/
			PrDbgString("Ptz0. Tx Done.\n");
			_SET_BIT(IRQ_UTC_TX, &pstIsr->stUserPoll.bitIrq);
		}/*}}}*/
		if(pstIsr->stIrqClr.b.ptz1) //tx fifo empty
		{/*{{{*/
			PrDbgString("Ptz1. Tx fifo empty.\n");
			/* reset tx fifo */
			{/*{{{*/
				stReg.reg = 0x0060;
				u8RegMask = 0x80;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0050;
				u8RegMask = 0x87;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0060;
				u8RegMask = 0x80;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
		}/*}}}*/
		if(pstIsr->stIrqClr.b.ptz2) //tx fifo overflow
		{/*{{{*/
			PrDbgString("Ptz2. Tx fifo overflow.\n");
			/* reset tx fifo */
			{/*{{{*/
				stReg.reg = 0x0060;
				u8RegMask = 0x80;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0050;
				u8RegMask = 0x87;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0060;
				u8RegMask = 0x80;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
		}/*}}}*/
		if(pstIsr->stIrqClr.b.ptz3) //rx fifo received data
		{/*{{{*/
			PP_U8 u8RxFifoSize = 0;
			PP_U8 pu8TempRecvBuffer[MAX_PVIRX_UTC_BUF_SIZE] = {0, };

			PrDbgString("Ptz3.\n");
			/* get size of rx fifo */
			stReg.reg = 0x0055;
			if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RxFifoSize)) != eSUCCESS)
			{
				PrErrorString("invalid read\n");
				return(result);
			}
			PrDbg("UTC Rx size:%d\n", u8RxFifoSize);
			if(u8RxFifoSize > MAX_PVIRX_UTC_BUF_SIZE)
			{
				/* reset rx fifo */
				{/*{{{*/
					stReg.reg = 0x0040;
					u8RegMask = 0x80;
					u8RegData = 0x00;
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					stReg.reg = 0x0054;
					u8RegMask = 0x87;
					u8RegData = 0x83;
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					stReg.reg = 0x0040;
					u8RegMask = 0x80;
					u8RegData = 0x80;
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/
			}
			else
			{
				if(u8RxFifoSize)
				{
					/* ready read fifo */
					stReg.reg = 0x0054;
					u8RegMask = 0x17; 
					u8RegData = 0x13; // 1'st set pre-loaddata bit before read fifo.
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					for(i = 0; i < u8RxFifoSize; i++)
					{
						stReg.reg = 0x0056;
						if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &pu8TempRecvBuffer[i])) != eSUCCESS)
						{
							PrErrorString("invalid read\n");
							return(result);
						}
					}
					PrDbgString("UTC Recv:");
					for(i = 0; i < u8RxFifoSize; i++)
					{
#if 1  //TODO kiki
						printf(" 0x%02x,", pu8TempRecvBuffer[i]);
#endif
						pHost->stUTCRecvCmd.utcCmd[i] = pu8TempRecvBuffer[i];
					}
#if 1  //TODO kiki
					printf("\r\n");
#endif
					pHost->stUTCRecvCmd.utcCmdLength = u8RxFifoSize;

					_SET_BIT(IRQ_UTC_RX, &pstIsr->stUserPoll.bitIrq);
				}
				else
				{
					PrErrorString("invalid rxfifosize\n");
				}
			}

		}/*}}}*/
		if(pstIsr->stIrqClr.b.ptz4) //rx fifo empty
		{/*{{{*/
			PrDbgString("Ptz4. Rx fifo empty.\n");
			/* reset rx fifo */
			{/*{{{*/
				stReg.reg = 0x0040;
				u8RegMask = 0x80;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0054;
				u8RegMask = 0x87;
				u8RegData = 0x83;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0040;
				u8RegMask = 0x80;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
		}/*}}}*/
		if(pstIsr->stIrqClr.b.ptz5) //rx fifo overflow
		{/*{{{*/
			PrDbgString("Ptz5. Rx fifo overflow.\n");
			/* reset rx fifo */
			{/*{{{*/
				stReg.reg = 0x0040;
				u8RegMask = 0x80;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0054;
				u8RegMask = 0x87;
				u8RegData = 0x83;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0040;
				u8RegMask = 0x80;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
		}/*}}}*/
		if(pstIsr->stIrqClr.b.ptz6) //rx sync
		{
			PrDbgString("Ptz6. Rx sync.\n");
		}
		if(pstIsr->stIrqClr.b.ptz7) //rx data error
		{/*{{{*/
			PrDbgString("Ptz7. Rx data error.\n");
			/* stop rx */
			{/*{{{*/
				stReg.reg = 0x0040;
				u8RegMask = 0x40;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
			/* reset rx fifo */
			{/*{{{*/
				stReg.reg = 0x0040;
				u8RegMask = 0x80;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0054;
				u8RegMask = 0x87;
				u8RegData = 0x83;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				stReg.reg = 0x0040;
				u8RegMask = 0x80;
				u8RegData = 0x80;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
			/* start rx */
			{/*{{{*/
				stReg.reg = 0x0040;
				u8RegMask = 0xC0;
				u8RegData = 0xC0;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
		}/*}}}*/
	}
#endif // SUPPORT_PVIRX_UTC
	if(pstIsr->stIrqClr.b.timer0) //periodic job 4sec
	{/*{{{*/
		PrDbg("%x-TIMER0:%d\n", chanAddr COMMA pstIsr->stIrqStatus.b.timer0);
		//ignore. don't poll user
		//_SET_BIT(IRQ_TIMER0UP, &pstIsr->stUserPoll.bitIrq);

		if(pstIsr->stJobProc.numJobStep)
		{
			if( (_TEST_BIT(JOB_DOING_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess)) )
			{
				PrDbgString("Do TIMER0(4sec) Process camera monitoring.\n");

                //don't do deferring. do immediately because of fast video display.
                {
                    _stMonitorParam stMonitorParam;

                    stMonitorParam.cid = cid;
                    stMonitorParam.chanAddr = chanAddr;
                    stMonitorParam.eBitIrq = IRQ_TIMER0UP;
                    stMonitorParam.pHost = (void *)pHost;

                    PPDRV_PVIRX_ProcessCameraMonitor((PP_VOID *)&stMonitorParam, 0);
                }
			}
		}
	}/*}}}*/
	if(pstIsr->stIrqClr.b.timer1) //periodic job 300msec
	{/*{{{*/
		PrDbg("%x-TIMER1:%d\n", chanAddr COMMA pstIsr->stIrqStatus.b.timer1);
		//ignore. don't poll user
		//_SET_BIT(IRQ_TIMER1UP, &pstIsr->stUserPoll.bitIrq);

		if(pstIsr->stJobProc.cntWait300MsecTime != 0)
		{
			pstIsr->stJobProc.cntWait300MsecTime--;
		}
		else
		{
            if(pstIsr->stJobProc.numJobStep)
            {
                if( ((_TEST_BIT(JOB_START_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess)) && (!_TEST_BIT(JOB_DONE_CAMERA_IN, &pstIsr->stJobProc.bitJobProcess))) )
                {
                    PrDbgString("Do TIMER1(300msec) Process camera plug in.\n");

                    //don't do deferring. do immediately because of fast video display.
                    {
                        _stPlugInParam stPlugInParam;

                        stPlugInParam.cid = cid;
                        stPlugInParam.chanAddr = chanAddr;
                        stPlugInParam.pHost = (void *)pHost;

                        PPDRV_PVIRX_ProcessCameraPlugIn((PP_VOID *)&stPlugInParam, 0);
                    }
                }

				if( ((_TEST_BIT(JOB_START_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess)) || (_TEST_BIT(JOB_DOING_CAMERA_MONITOR, &pstIsr->stJobProc.bitJobProcess))) )
				{
					PrDbgString("Do TIMER1(300msec) Process camera monitoring.\n");

                    //don't do deferring. do immediately because of fast video display.
                    {
                        _stMonitorParam stMonitorParam;

                        stMonitorParam.cid = cid;
                        stMonitorParam.chanAddr = chanAddr;
                        stMonitorParam.eBitIrq = IRQ_TIMER1UP;
                        stMonitorParam.pHost = (void *)pHost;

                        PPDRV_PVIRX_ProcessCameraMonitor((PP_VOID *)&stMonitorParam, 0);
                    }
				}
			}
		}
	}/*}}}*/

	/* clr irq */
	{/*{{{*/
		stReg.reg = 0x008A;
		u8RegData = pstIsr->stIrqClr.reg[0];
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			return(result);
		}
		stReg.reg = 0x008B;
		u8RegData = pstIsr->stIrqClr.reg[1];
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			return(result);
		}
	}/*}}}*/

	if(pstIsr->stUserPoll.bitIrq != 0)
	{
		WAKE_UP_INTERRUPTIBLE(&pHost->wqPoll);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_ReadVidStatusReg(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg OUT *pstVidStatusReg)
{
	PP_RESULT_E result = eERROR_FAILURE;

	PP_S32 i;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstVidStatusReg == NULL)
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	for(i = 0; i < sizeof(_stPVIRX_VidStatusReg); i++)
	{
		stReg.reg = 0x0000 + i;
		u8RegData = 0;
		if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid read\n");
			return(result);
		}
		pstVidStatusReg->reg[i] = u8RegData;
	}

	PrDbg("%d- 0x%02x[std:%d,ref:%d,video:%d,res:%d]\n", chanAddr COMMA pstVidStatusReg->reg[0] COMMA 
		pstVidStatusReg->b.det_ifmt_std COMMA
		pstVidStatusReg->b.det_ifmt_ref COMMA
		pstVidStatusReg->b.det_video COMMA
		pstVidStatusReg->b.det_ifmt_res);
#if 0
	PrDbgString("Det->");
	switch( pstVidStatusReg->b.det_ifmt_std )
	{
		case DET_PVI: printf("PVI/"); break;
		case DET_CVI: printf("CVI/"); break;
		case DET_HDA: printf("HDA/"); break;
		case DET_HDT: printf("HDT/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_ref )
	{
		case DET_25Hz: printf("25Hz/"); break;
		case DET_30Hz: printf("30Hz/"); break;
		case DET_50Hz: printf("50Hz/"); break;
		case DET_60Hz: printf("60Hz/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_res )
	{
		case DET_SD480i: printf("SD480i"); break;
		case DET_SD576i: printf("SD576i"); break;
		case DET_HD720p: printf("HD720p"); break;
		case DET_HD1080p: printf("HD1080p"); break;
		case DET_HD960p: printf("HD960p"); break;
		default: break;
	}
	printf("\r\n");
#endif

	PrDbg("%d- 0x%02x[lock(std:%d,gain:%d,clamp:%d,hperiod:%d,hpll:%d,c_fine:%d,chroma:%d),det_chroma:%d]\n", chanAddr COMMA pstVidStatusReg->reg[1] COMMA
                pstVidStatusReg->b.lock_std COMMA
                pstVidStatusReg->b.lock_gain COMMA
                pstVidStatusReg->b.lock_clamp COMMA
                pstVidStatusReg->b.lock_hperiod COMMA
                pstVidStatusReg->b.lock_hpll COMMA
                pstVidStatusReg->b.lock_c_fine COMMA
                pstVidStatusReg->b.lock_chroma COMMA
                pstVidStatusReg->b.det_chroma);

	PrDbg("%d- 0x%02x[det(hdtv:%d,hdth1:%d,hdth0:%d,hda:%d)]\n", chanAddr COMMA pstVidStatusReg->reg[2] COMMA
                pstVidStatusReg->b.det_std_hdt_v COMMA
                pstVidStatusReg->b.det_std_hdt_h1 COMMA
                pstVidStatusReg->b.det_std_hdt_h0 COMMA
                pstVidStatusReg->b.det_std_hda);

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_MonitorCurVidStatusReg(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg OUT *pstVidStatusReg)
{
	PP_RESULT_E result = eERROR_FAILURE;

	PP_S32 i;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstVidStatusReg == NULL)
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	for(i = 0; i < sizeof(_stPVIRX_VidStatusReg); i++)
	{
		stReg.reg = 0x0000 + i;
		u8RegData = 0;
		if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid read\n");
			return(result);
		}
		pstVidStatusReg->reg[i] = u8RegData;
	}

	PrDbg("%d- 0x%02x[std:%d,ref:%d,video:%d,res:%d]\n", chanAddr COMMA pstVidStatusReg->reg[0] COMMA 
		pstVidStatusReg->b.det_ifmt_std COMMA
		pstVidStatusReg->b.det_ifmt_ref COMMA
		pstVidStatusReg->b.det_video COMMA
		pstVidStatusReg->b.det_ifmt_res);
#if 1
	printf("%d-Det-> ", chanAddr);
	switch( pstVidStatusReg->b.det_ifmt_std )
	{
		case DET_PVI: printf("PVI/"); break;
		case DET_CVI: printf("CVI/"); break;
		case DET_HDA: printf("HDA/"); break;
		case DET_HDT: printf("HDT/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_ref )
	{
		case DET_25Hz: printf("25Hz/"); break;
		case DET_30Hz: printf("30Hz/"); break;
		case DET_50Hz: printf("50Hz/"); break;
		case DET_60Hz: printf("60Hz/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_res )
	{
		case DET_SD480i: printf("SD480i"); break;
		case DET_SD576i: printf("SD576i"); break;
		case DET_HD720p: printf("HD720p"); break;
		case DET_HD1080p: printf("HD1080p"); break;
		case DET_HD960p: printf("HD960p"); break;
		default: break;
	}
	printf("\r\n");
#endif

	PrDbg("%d- 0x%02x[lock(std:%d,gain:%d,clamp:%d,hperiod:%d,hpll:%d,c_fine:%d,chroma:%d),det_chroma:%d]\n", chanAddr COMMA pstVidStatusReg->reg[1] COMMA
                pstVidStatusReg->b.lock_std COMMA
                pstVidStatusReg->b.lock_gain COMMA
                pstVidStatusReg->b.lock_clamp COMMA
                pstVidStatusReg->b.lock_hperiod COMMA
                pstVidStatusReg->b.lock_hpll COMMA
                pstVidStatusReg->b.lock_c_fine COMMA
                pstVidStatusReg->b.lock_chroma COMMA
                pstVidStatusReg->b.det_chroma);

	PrDbg("%d- 0x%02x[det(hdtv:%d,hdth1:%d,hdth0:%d,hda:%d)]\n", chanAddr COMMA pstVidStatusReg->reg[2] COMMA
                pstVidStatusReg->b.det_std_hdt_v COMMA
                pstVidStatusReg->b.det_std_hdt_h1 COMMA
                pstVidStatusReg->b.det_std_hdt_h0 COMMA
                pstVidStatusReg->b.det_std_hda);

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_ReadStdResol(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stPVIRX_VidStatusReg IN *pstVidStatusReg, enum _eCameraStandard OUT *pCameraStandard, enum _eCameraResolution OUT *pCameraResolution, enum _eVideoResolution OUT *pVideoResolution)
{
	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;

	if(pstVidStatusReg == NULL)
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	PrDbg("0x%02x[std:%d,ref:%d,video:%d,res:%d]\n", pstVidStatusReg->reg[0] COMMA 
		pstVidStatusReg->b.det_ifmt_std COMMA
		pstVidStatusReg->b.det_ifmt_ref COMMA
		pstVidStatusReg->b.det_video COMMA
		pstVidStatusReg->b.det_ifmt_res);
#if 0
	PrDbgString("Det->");
	switch( pstVidStatusReg->b.det_ifmt_std )
	{
		case DET_PVI: printf("PVI/"); break;
		case DET_CVI: printf("CVI/"); break;
		case DET_HDA: printf("HDA/"); break;
		case DET_HDT: printf("HDT/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_ref )
	{
		case DET_25Hz: printf("25Hz/"); break;
		case DET_30Hz: printf("30Hz/"); break;
		case DET_50Hz: printf("50Hz/"); break;
		case DET_60Hz: printf("60Hz/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_res )
	{
		case DET_SD480i: printf("SD480i"); break;
		case DET_SD576i: printf("SD576i"); break;
		case DET_HD720p: printf("HD720p"); break;
		case DET_HD1080p: printf("HD1080p"); break;
		case DET_HD960p: printf("HD960p"); break;
		default: break;
	}
	printf("\r\n");
#endif
	PrDbg("0x%02x[lock(std:%d,gain:%d,clamp:%d,hperiod:%d,hpll:%d,c_fine:%d,chroma:%d),det_chroma:%d]\n", pstVidStatusReg->reg[1] COMMA
                pstVidStatusReg->b.lock_std COMMA
                pstVidStatusReg->b.lock_gain COMMA
                pstVidStatusReg->b.lock_clamp COMMA
                pstVidStatusReg->b.lock_hperiod COMMA
                pstVidStatusReg->b.lock_hpll COMMA
                pstVidStatusReg->b.lock_c_fine COMMA
                pstVidStatusReg->b.lock_chroma COMMA
                pstVidStatusReg->b.det_chroma);

	PrDbg("0x%02x[det(hdtv:%d,hdth1:%d,hdth0:%d,hda:%d)]\n", pstVidStatusReg->reg[2] COMMA
                pstVidStatusReg->b.det_std_hdt_v COMMA
                pstVidStatusReg->b.det_std_hdt_h1 COMMA
                pstVidStatusReg->b.det_std_hdt_h0 COMMA
                pstVidStatusReg->b.det_std_hda);

	switch( pstVidStatusReg->b.det_ifmt_res )
	{/*{{{*/
		case DET_SD480i: 
			{
				cameraStandard = CVBS; 
				cameraResolution = camera_ntsc; 
			}
			break;
		case DET_SD576i: 
			{
				cameraStandard = CVBS; 
				cameraResolution = camera_pal; 
			}
			break;
		case DET_HD720p:
		case DET_HD1080p:
			{
				switch( pstVidStatusReg->b.det_ifmt_std )
				{
					case DET_PVI: cameraStandard = PVI; break;
					case DET_CVI: cameraStandard = CVI; break;
					case DET_HDA: cameraStandard = HDA; break;
					case DET_HDT: cameraStandard = HDT_NEW; break;
					default: cameraStandard = PVI; break;
				}
			}
			break;
		case DET_HD960p:
			{
				switch( pstVidStatusReg->b.det_ifmt_std )
				{
					case DET_PVI: cameraStandard = PVI; break;
					case DET_CVI: cameraStandard = CVI; break;
					case DET_HDA: cameraStandard = HDA; break;
					case DET_HDT: cameraStandard = HDT_NEW; break;
					default: cameraStandard = PVI; break;
				}
			}
			break;
		default:
			{
				cameraStandard = PVI; 
			}
			break;
	}/*}}}*/

	if(cameraStandard != CVBS)
	{/*{{{*/
		switch( pstVidStatusReg->b.det_ifmt_res )
		{
			case DET_HD720p:
				{
					switch( pstVidStatusReg->b.det_ifmt_ref )
					{
						case DET_25Hz: cameraResolution = camera_1280x720p25; break;
						case DET_30Hz: cameraResolution = camera_1280x720p30; break;
						case DET_50Hz: cameraResolution = camera_1280x720p50; break;
						case DET_60Hz: cameraResolution = camera_1280x720p60; break;
						default: cameraResolution = camera_1280x720p25; break;
					}
				}
				break;
			case DET_HD1080p:
				{
					switch( pstVidStatusReg->b.det_ifmt_ref )
					{
						case DET_25Hz: cameraResolution = camera_1920x1080p25; break;
						case DET_30Hz: cameraResolution = camera_1920x1080p30; break;
						case DET_50Hz:
						case DET_60Hz:
						default: cameraResolution = camera_1920x1080p25; break;
					}
				}
				break;
			case DET_HD960p: 
				{
					switch( pstVidStatusReg->b.det_ifmt_ref )
					{
						case DET_25Hz: cameraResolution = camera_1280x960p25; break;
						case DET_30Hz: cameraResolution = camera_1280x960p30; break;
						default: cameraResolution = camera_1280x960p25; break;
					}
				}
				break;
			default:
				{
					cameraResolution = camera_1280x720p25;
				}
				break;
		}
	}/*}}}*/

	if(pCameraStandard) *pCameraStandard = cameraStandard;
	if(pCameraResolution) *pCameraResolution = cameraResolution;
	videoResolution = stDefaultCameraVideoRelation[cameraResolution].eVideoResolution;
	if( (cameraResolution == camera_ntsc) || (cameraResolution == camera_pal) )
	{
		if(gbPviRxSelSD960H)
		{
			videoResolution += video_960x480i60;
		}
	}
	if(pVideoResolution) *pVideoResolution = videoResolution;

	return(eSUCCESS);
}

PP_RESULT_E PPDRV_PVIRX_GetStdResol(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, enum _eCameraStandard OUT *pCameraStandard, enum _eCameraResolution OUT *pCameraResolution, enum _eVideoResolution OUT *pVideoResolution, int OUT *pReJudge)
{
	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;
	PP_S32 reJudge = 0;

	if(pstVidStatusReg == NULL)
	{
		PrErrorString("invalid argu\n");
		return(eERROR_FAILURE);
	}

	PrDbg("0x%02x[std:%d,ref:%d,video:%d,res:%d]\n", pstVidStatusReg->reg[0] COMMA 
		pstVidStatusReg->b.det_ifmt_std COMMA
		pstVidStatusReg->b.det_ifmt_ref COMMA
		pstVidStatusReg->b.det_video COMMA
		pstVidStatusReg->b.det_ifmt_res);
#if 0
	PrDbgString("Det->");
	switch( pstVidStatusReg->b.det_ifmt_std )
	{
		case DET_PVI: printf("PVI/"); break;
		case DET_CVI: printf("CVI/"); break;
		case DET_HDA: printf("HDA/"); break;
		case DET_HDT: printf("HDT/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_ref )
	{
		case DET_25Hz: printf("25Hz/"); break;
		case DET_30Hz: printf("30Hz/"); break;
		case DET_50Hz: printf("50Hz/"); break;
		case DET_60Hz: printf("60Hz/"); break;
		default: break;
	}
	switch( pstVidStatusReg->b.det_ifmt_res )
	{
		case DET_SD480i: printf("SD480i"); break;
		case DET_SD576i: printf("SD576i"); break;
		case DET_HD720p: printf("HD720p"); break;
		case DET_HD1080p: printf("HD1080p"); break;
		case DET_HD960p: printf("HD960p"); break;
		default: break;
	}
	printf("\r\n");
#endif
	PrDbg("0x%02x[lock(std:%d,gain:%d,clamp:%d,hperiod:%d,hpll:%d,c_fine:%d,chroma:%d),det_chroma:%d]\n", pstVidStatusReg->reg[1] COMMA
                pstVidStatusReg->b.lock_std COMMA
                pstVidStatusReg->b.lock_gain COMMA
                pstVidStatusReg->b.lock_clamp COMMA
                pstVidStatusReg->b.lock_hperiod COMMA
                pstVidStatusReg->b.lock_hpll COMMA
                pstVidStatusReg->b.lock_c_fine COMMA
                pstVidStatusReg->b.lock_chroma COMMA
                pstVidStatusReg->b.det_chroma);

	PrDbg("0x%02x[det(hdtv:%d,hdth1:%d,hdth0:%d,hda:%d)]\n", pstVidStatusReg->reg[2] COMMA
                pstVidStatusReg->b.det_std_hdt_v COMMA
                pstVidStatusReg->b.det_std_hdt_h1 COMMA
                pstVidStatusReg->b.det_std_hdt_h0 COMMA
                pstVidStatusReg->b.det_std_hda);

	switch( pstVidStatusReg->b.det_ifmt_res )
	{/*{{{*/
		case DET_SD480i: 
			{
				cameraStandard = CVBS; 
				cameraResolution = camera_ntsc; 
			}
			break;
		case DET_SD576i: 
			{
				cameraStandard = CVBS; 
				cameraResolution = camera_pal; 
			}
			break;
		case DET_HD720p:
		case DET_HD1080p:
		case DET_HD960p:
			{
				switch( pstVidStatusReg->b.det_ifmt_std )
				{
					case DET_PVI: cameraStandard = PVI; break;
					case DET_CVI: cameraStandard = CVI; break;
					case DET_HDA: cameraStandard = HDA; break;
					case DET_HDT: cameraStandard = HDT_NEW; break;
					default: cameraStandard = PVI; break;
				}
			}
			break;
		default:
			{
				cameraStandard = PVI; 
			}
			break;
	}/*}}}*/

	if(cameraStandard != CVBS)
	{/*{{{*/
		switch( pstVidStatusReg->b.det_ifmt_res )
		{
			case DET_HD720p:
				{
					switch( pstVidStatusReg->b.det_ifmt_ref )
					{
						case DET_25Hz: cameraResolution = camera_1280x720p25; break;
						case DET_30Hz: cameraResolution = camera_1280x720p30; break;
						case DET_50Hz: cameraResolution = camera_1280x720p50; break;
						case DET_60Hz: cameraResolution = camera_1280x720p60; break;
						default: cameraResolution = camera_1280x720p25; break;
					}
				}
				break;
			case DET_HD1080p:
				{
					switch( pstVidStatusReg->b.det_ifmt_ref )
					{
						case DET_25Hz: cameraResolution = camera_1920x1080p25; break;
						case DET_30Hz: cameraResolution = camera_1920x1080p30; break;
						case DET_50Hz:
						case DET_60Hz:
						default: cameraResolution = camera_1920x1080p25; break;
					}
				}
				break;
			case DET_HD960p: 
				{
					switch( pstVidStatusReg->b.det_ifmt_ref )
					{
						case DET_25Hz: cameraResolution = camera_1280x960p25; break;
						case DET_30Hz: cameraResolution = camera_1280x960p30; break;
						default: cameraResolution = camera_1280x960p25; break;
					}
				}
				break;
			default:
				{
					cameraResolution = camera_1280x720p25;
				}
				break;
		}
	}/*}}}*/

	/* check hdt - 720p old/new standard */
	if( (cameraStandard == HDT_NEW) && ((cameraResolution == camera_1280x720p25) || (cameraResolution == camera_1280x720p30)) )
	{
		if( (pstVidStatusReg->b.det_std_hdt_v == 1) && (pstVidStatusReg->b.det_std_hdt_h1 == 1) && (pstVidStatusReg->b.det_std_hdt_h0 == 1) )
		{
			cameraStandard = HDT_NEW;
		}
		else if( (pstVidStatusReg->b.det_std_hdt_v == 1) && (pstVidStatusReg->b.det_std_hdt_h1 == 0) && (pstVidStatusReg->b.det_std_hdt_h0 == 1) )
		{
			cameraStandard = HDT_OLD;
		}
		else
		{
			PrError("invalid hdt flag(%02x)\n", pstVidStatusReg->reg[2]);
			cameraStandard = HDT_NEW;
			return(eERROR_FAILURE);
		}
	}
	/* check hda - 1080p standard */
	else if( (cameraStandard == HDA) )
	{
		if( ((cameraResolution == camera_1920x1080p25) || (cameraResolution == camera_1920x1080p30)) )
		{
			if( (pstVidStatusReg->b.det_std_hda == 1) )
			{
				cameraStandard = HDA;
			}
			else
			{
				cameraStandard = CVI; // reJudge CVI
				reJudge = 1;
			}
		}
	}

	if(pCameraStandard) *pCameraStandard = cameraStandard;
	if(pCameraResolution) *pCameraResolution = cameraResolution;
	videoResolution = stDefaultCameraVideoRelation[cameraResolution].eVideoResolution;
	if( (cameraResolution == camera_ntsc) || (cameraResolution == camera_pal) )
	{
		if(gbPviRxSelSD960H)
		{
			videoResolution += video_960x480i60;
		}
	}

	if(pVideoResolution) *pVideoResolution = videoResolution;
	if(pReJudge) *pReJudge = reJudge;

	return(eSUCCESS);
}

#ifdef SUPPORT_PVIRX_VID
PP_RESULT_E PPDRV_PVIRX_VID_SetChnAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stChnAttr IN *pstChnAttr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	if(pstChnAttr == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//uint16_t u16HActive; //b[12:0]
	stReg.reg = 0x0112;
	u8RegData = (pstChnAttr->u16HActive >> 8)&0x1F;
	u8RegMask = 0x1F;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	stReg.reg = 0x0114;
	u8RegData = pstChnAttr->u16HActive & 0xFF;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	//uint16_t u16HDelay; //b[12:0]
	stReg.reg = 0x0111;
	u8RegData = (pstChnAttr->u16HDelay >> 8)&0x1F;
	u8RegMask = 0x1F;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	stReg.reg = 0x0113;
	u8RegData = pstChnAttr->u16HDelay & 0xFF;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	//uint16_t u16VActive; //b[10:0]
	stReg.reg = 0x0112;
	u8RegData = ((pstChnAttr->u16VActive >> 8)&0x07) << 5;
	u8RegMask = 0xE0;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	stReg.reg = 0x0116;
	u8RegData = pstChnAttr->u16VActive & 0xFF;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	//uint16_t u16VDelay; //b[10:0]
	stReg.reg = 0x0111;
	u8RegData = ((pstChnAttr->u16VDelay >> 8)&0x07) << 5;
	u8RegMask = 0xE0;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	stReg.reg = 0x0115;
	u8RegData = pstChnAttr->u16VDelay & 0xFF;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	if(pstChnAttr->u16HSCLRatio != 0)
	{
		stReg.reg = 0x0129;
		u8RegData = pstChnAttr->u16HSCLRatio>>8;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("Write reg.\n");
		    return(result);
		}

		stReg.reg = 0x012A;
		u8RegData = pstChnAttr->u16HSCLRatio & 0xFF;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("Write reg.\n");
		    return(result);
		}
	}

    return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetChnAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stChnAttr OUT *pstChnAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData = 0;
	PP_U8 u8RegMask;

	if(pstChnAttr == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//uint16_t u16HActive; //b[12:0]
	stReg.reg = 0x0112;
	u8RegMask = 0x1F;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16HActive = (u8RegData&0x1F)<<8;

	stReg.reg = 0x0114;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16HActive |= u8RegData;

	//uint16_t u16HDelay; //b[12:0]
	stReg.reg = 0x0111;
	u8RegMask = 0x1F;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16HDelay = (u8RegData&0x1F)<<8;

	stReg.reg = 0x0113;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16HDelay |= u8RegData;

	//uint16_t u16VActive; //b[10:0]
	stReg.reg = 0x0112;
	u8RegMask = 0xE0;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16VActive = (u8RegData>>5)<<8;

	stReg.reg = 0x0116;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16VActive |= u8RegData;

	//uint16_t u16VDelay; //b[10:0]
	stReg.reg = 0x0111;
	u8RegMask = 0x1F;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16VDelay = (u8RegData>>5)<<8;

	stReg.reg = 0x0115;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16VDelay |= u8RegData;

	stReg.reg = 0x0129;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16HSCLRatio = u8RegData<<8;

	stReg.reg = 0x012A;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstChnAttr->u16HSCLRatio |= u8RegData;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetCscAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stCscAttr IN *pstCscAttr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstCscAttr == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//u8CbGain;
	stReg.reg = 0x0124;
	u8RegData = pstCscAttr->u8CbGain;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	//u8CrGain;
	stReg.reg = 0x0125;
	u8RegData = pstCscAttr->u8CrGain;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	//u8CbOffset;
	stReg.reg = 0x0126;
	u8RegData = pstCscAttr->u8CbOffset;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	//u8CrOffset;
	stReg.reg = 0x0127;
	u8RegData = pstCscAttr->u8CrOffset;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetCscAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stCscAttr OUT *pstCscAttr)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstCscAttr == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//u8CbGain;
	stReg.reg = 0x0124;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstCscAttr->u8CbGain = u8RegData;

	//u8CrGain;
	stReg.reg = 0x0125;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstCscAttr->u8CrGain = u8RegData;

	//u8CbOffset;
	stReg.reg = 0x0126;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstCscAttr->u8CbOffset = u8RegData;

	//u8CrOffset;
	stReg.reg = 0x0127;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstCscAttr->u8CrOffset = u8RegData;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetContrast(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stContrast IN *pstContrast)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstContrast == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Contrast
	stReg.reg = 0x0120;
	u8RegData = pstContrast->u8Contrast;
	if( (result = PPDRV_PVIRX_Write(0, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetContrast(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stContrast OUT *pstContrast)
{

	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstContrast == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Contrast
	stReg.reg = 0x0120;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(0, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstContrast->u8Contrast = u8RegData;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetBright(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stBright IN *pstBright)
{

	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstBright == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Bright
	stReg.reg = 0x0121;
	u8RegData = pstBright->u8Bright;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetBright(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stBright OUT *pstBright)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstBright == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Bright
	stReg.reg = 0x0121;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstBright->u8Bright = u8RegData;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetSaturation(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stSaturation IN *pstSaturation)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstSaturation == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Saturation
	stReg.reg = 0x0122;
	u8RegData = pstSaturation->u8Saturation;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetSaturation(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stSaturation OUT *pstSaturation)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstSaturation == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Saturation
	stReg.reg = 0x0122;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstSaturation->u8Saturation = u8RegData;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetHue(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stHue IN *pstHue)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstHue == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Hue
	stReg.reg = 0x0123;
	u8RegData = pstHue->u8Hue;
	if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetHue(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stHue OUT *pstHue)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	if(pstHue == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Hue
	stReg.reg = 0x0123;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	pstHue->u8Hue = u8RegData;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetSharpness(const PP_S32 IN cid, const PP_U8 IN chanAddr, const _stSharpness IN *pstSharpness)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	if(pstSharpness == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Sharpness
	stReg.reg = 0x0139;
	u8RegMask = 0x0F;
	u8RegData = pstSharpness->u8Sharpness & 0xF;
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_GetSharpness(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stSharpness OUT *pstSharpness)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData = 0;
	PP_U8 u8RegMask;

	if(pstSharpness == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	//Sharpness
	stReg.reg = 0x0139;
	u8RegMask = 0x0F;
	if( (result = PPDRV_PVIRX_ReadMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}

	pstSharpness->u8Sharpness = u8RegData&0xF;

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_VID_SetBlank(const PP_S32 IN cid, const PP_U8 IN chanAddr, const PP_S32 IN bEnable, const PP_S32 IN blankColor)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData = 0;
	PP_U8 u8RegMask;

	/* blank */
	stReg.reg = 0x0100;
	u8RegMask = 0x03;
	u8RegData = ((bEnable&1)<<1) | (blankColor&1); //0:normal video output, 1:manual background mode
	if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		PrErrorString("Write reg.\n");
		return(result);
	}

	return(result);
}
#endif // SUPPORT_PVIRX_VID

PP_RESULT_E PPDRV_PVIRX_GetNovidStatus(const PP_S32 IN cid, const PP_U8 IN chanAddr, PP_U8 OUT *pStatus)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData = 0;

	if(pStatus == NULL)
	{
		PrErrorString("Invalid argu.\n");
		return(eERROR_FAILURE);
	}

	/* IRQ Status */
	stReg.reg = 0x008D;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}
	*pStatus = (u8RegData>>4)&1;

	return(result);
}


#ifdef SUPPORT_PVIRX_UTC
static PP_RESULT_E PPDRV_PVIRX_UTC_WriteFlagFifo(const PP_S32 cid, const PP_U8 chanAddr, const PP_S32 sizeArray, const PP_U8 *pData)
{
	PP_RESULT_E result = eERROR_FAILURE;

	PP_S32 i;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;

	for(i = 0; i < sizeArray; i++)
	{
		PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
		stReg.reg = 0x0051;
		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
		{
			PrErrorString("invalid write\n");
			return(result);
		}
	}

	/* verify utc data count */
	stReg.reg = 0x0053;
	u8RegData = 0;
	if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
	{
		PrErrorString("invalid read\n");
		return(result);
	}
	if(sizeArray != u8RegData+1)
	{
		PrError("invalid utc data count(%d-%d)\n", sizeArray COMMA u8RegData);
		return(eERROR_FAILURE);
	}
	return(eSUCCESS);
}

static PP_RESULT_E PPDRV_PVIRX_UTC_SendTxFifo(const PP_S32 cid, const PP_U8 chanAddr, const enum _eCameraStandard cameraStandard, const enum _eCameraResolution cameraResolution, const PP_S32 dataSize, const PP_U8 *pData)
{
	PP_RESULT_E result = eERROR_FAILURE;

	PP_S32 i;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegMask;
	PP_U8 u8RegData;

	PrDbg("%x-standrad:%d, size:%d, data0:0x%02x\n", chanAddr COMMA cameraStandard COMMA dataSize COMMA pData[0]);
	PrDbg("%x-senddata[size:%d]:\n", chanAddr COMMA dataSize);
#if 0  //TODO kiki
	for(i = 0; i < dataSize; i++)
	{
		printf("0x%02x ", pData[i]);
	}
	printf("\r\n");
#endif

	switch(cameraStandard)
	{
		case CVBS:
			{/*{{{*/
				PrDbgString("utc CVBS\n");
				for(i = 0; i < dataSize; i++)
				{
					PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
					stReg.reg = 0x0051;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					if( (i != 0) && (((i+1) % PVIRX_UTC_SD_TXCMD_BASE_BYTE_CNT)==0) )
					{
						/* verify utc data count */
						stReg.reg = 0x0052;
						u8RegData = 0;
						if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid read\n");
							return(result);
						}
						if((PVIRX_UTC_SD_TXCMD_BASE_BYTE_CNT) != (u8RegData))
						{
							PrError("invalid utc data count(%d-%d)\n", PVIRX_UTC_SD_TXCMD_BASE_BYTE_CNT COMMA u8RegData);
							return(eERROR_FAILURE);
						}

						PrDbg("Send & Wait %dByte\n", PVIRX_UTC_SD_TXCMD_BASE_BYTE_CNT);
						/* start utc send */
						stReg.reg = 0x0060;
						u8RegMask = 0x40;
						u8RegData = 0x40;
						if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						/* wait complete sending */
						{/*{{{*/
							PP_S32 retryCnt = 10;
							PP_U8 u8TxFifoCnt = 0;
							PP_U8 u8TxBusy = 0;
							do
							{/*{{{*/
								stReg.reg = 0x0052;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxFifoCnt);

								stReg.reg = 0x008C;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxBusy);
								u8TxBusy &= 0x01;
								PrDbg("txBusy:%x\n", u8TxBusy);

								if( (u8TxFifoCnt == 0) && (u8TxBusy == 0) ) break;
								else
								{
		                            LOOPDELAY_MSEC(100);
									if(retryCnt <= 0) 
									{
										PrError("txBusy:%x\n", u8TxBusy);
										return(eERROR_FAILURE);
									}
								}
							} while(retryCnt--);/*}}}*/
						}/*}}}*/
					}
				}
			}/*}}}*/
			break;
#if defined(SUPPORT_PVIRX_STD_PVI)
		case PVI:
			{/*{{{*/
				PrDbgString("utc PVI\n");
				for(i = 0; i < dataSize; i++)
				{
					PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
					stReg.reg = 0x0051;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					if( (i != 0) && (((i+1) % PVIRX_UTC_PVI_TXCMD_BASE_BYTE_CNT)==0) )
					{
						/* verify utc data count */
						stReg.reg = 0x0052;
						u8RegData = 0;
						if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid read\n");
							return(result);
						}
						if((PVIRX_UTC_PVI_TXCMD_BASE_BYTE_CNT) != (u8RegData))
						{
							PrError("invalid utc data count(%d-%d)\n", PVIRX_UTC_PVI_TXCMD_BASE_BYTE_CNT COMMA u8RegData);
							return(eERROR_FAILURE);
						}

						PrDbg("Send & Wait %dByte\n", PVIRX_UTC_PVI_TXCMD_BASE_BYTE_CNT);
						/* start utc send */
						stReg.reg = 0x0060;
						u8RegMask = 0x40;
						u8RegData = 0x40;
						if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						/* wait complete sending */
						{/*{{{*/
							PP_S32 retryCnt = 10;
							PP_U8 u8TxFifoCnt = 0;
							PP_U8 u8TxBusy = 0;
							do
							{/*{{{*/
								stReg.reg = 0x0052;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxFifoCnt);

								stReg.reg = 0x008C;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxBusy);
								u8TxBusy &= 0x01;
								PrDbg("txBusy:%x\n", u8TxBusy);

								if( (u8TxFifoCnt == 0) && (u8TxBusy == 0) ) break;
								else
								{
		                            LOOPDELAY_MSEC(100);
									if(retryCnt <= 0) 
									{
										PrError("txBusy:%x\n", u8TxBusy);
										return(eERROR_FAILURE);
									}
								}
							} while(retryCnt--);/*}}}*/
						}/*}}}*/
					}
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
		case HDA:
			{
				PrDbgString("utc HDA\n");
				if(cameraResolution >= camera_1920x1080p30)
				{/*{{{*/
					for(i = 0; i < dataSize; i++)
					{
						PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
						stReg.reg = 0x0051;
						if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						if( (i != 0) && (((i+1) % PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_1080p)==0) )
						{
							/* verify utc data count */
							stReg.reg = 0x0052;
							u8RegData = 0;
							if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid read\n");
								return(result);
							}
							if((PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_1080p) != (u8RegData))
							{
								PrError("invalid utc data count(%d-%d)\n", PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_1080p COMMA u8RegData);
								return(eERROR_FAILURE);
							}

							PrDbg("Send & Wait %dByte\n", PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_1080p);
							/* start utc send */
							stReg.reg = 0x0060;
							u8RegMask = 0x40;
							u8RegData = 0x40;
							if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid write\n");
								return(result);
							}

							/* wait complete sending */
							{/*{{{*/
								PP_S32 retryCnt = 10;
								PP_U8 u8TxFifoCnt = 0;
								PP_U8 u8TxBusy = 0;
								do
								{/*{{{*/
									stReg.reg = 0x0052;
									PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxFifoCnt);

									stReg.reg = 0x008C;
									PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxBusy);
									u8TxBusy &= 0x01;
									PrDbg("txBusy:%x\n", u8TxBusy);

									if( (u8TxFifoCnt == 0) && (u8TxBusy == 0) ) break;
									else
									{
		                                LOOPDELAY_MSEC(100);
										if(retryCnt <= 0) 
										{
											PrError("txBusy:%x\n", u8TxBusy);
											return(eERROR_FAILURE);
										}
									}
								} while(retryCnt--);/*}}}*/
							}/*}}}*/
						}
					}
				}/*}}}*/
				else
				{/*{{{*/
					for(i = 0; i < dataSize; i++)
					{
						PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
						stReg.reg = 0x0051;
						if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						if( (i != 0) && (((i+1) % PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_720p)==0) )
						{
							/* verify utc data count */
							stReg.reg = 0x0052;
							u8RegData = 0;
							if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid read\n");
								return(result);
							}
							if((PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_720p) != (u8RegData))
							{
								PrError("invalid utc data count(%d-%d)\n", PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_720p COMMA u8RegData);
								return(eERROR_FAILURE);
							}

							PrDbg("Send & Wait %dByte\n", PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_720p);
							/* start utc send */
							stReg.reg = 0x0060;
							u8RegMask = 0x40;
							u8RegData = 0x40;
							if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
							{
								PrErrorString("invalid write\n");
								return(result);
							}

							/* wait complete sending */
							{/*{{{*/
								PP_S32 retryCnt = 10;
								PP_U8 u8TxFifoCnt = 0;
								PP_U8 u8TxBusy = 0;
								do
								{/*{{{*/
									stReg.reg = 0x0052;
									PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxFifoCnt);

									stReg.reg = 0x008C;
									PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxBusy);
									u8TxBusy &= 0x01;
									PrDbg("txBusy:%x\n", u8TxBusy);

									if( (u8TxFifoCnt == 0) && (u8TxBusy == 0) ) break;
									else
									{
		                                LOOPDELAY_MSEC(100);
										if(retryCnt <= 0) 
										{
											PrError("txBusy:%x\n", u8TxBusy);
											return(eERROR_FAILURE);
										}
									}
								} while(retryCnt--);/*}}}*/
							}/*}}}*/
						}
					}
				}/*}}}*/
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
		case CVI:
			{/*{{{*/
                        	PP_U8 bitCnt = 0;
	                        PP_U8 evenParity = 0;

				PrDbgString("utc CVI\n");

				for(i = 0; i < dataSize; i++)
				{
					PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
					stReg.reg = 0x0051;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					/* append parity data */
					for(evenParity = 0, bitCnt = 0; bitCnt < 8; bitCnt++)
					{     
						if(pData[i] & (1<<bitCnt)) evenParity++;
					}     
					u8RegData = ((evenParity&0x1))|0xFE;

					stReg.reg = 0x0051;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					if( (i != 0) && (((i+1) % PVIRX_UTC_CVI_TXCMD_BASE_BYTE_CNT)==0) )
					{
						/* verify utc data count */
						stReg.reg = 0x0052;
						u8RegData = 0;
						if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid read\n");
							return(result);
						}
						if((PVIRX_UTC_CVI_TXCMD_BASE_BYTE_CNT) != (u8RegData/2))
						{
							PrError("invalid utc data count(%d-%d)\n", PVIRX_UTC_CVI_TXCMD_BASE_BYTE_CNT COMMA u8RegData);
							return(eERROR_FAILURE);
						}

						PrDbg("Send & Wait %dByte\n", PVIRX_UTC_CVI_TXCMD_BASE_BYTE_CNT);
						/* start utc send */
						stReg.reg = 0x0060;
						u8RegMask = 0x40;
						u8RegData = 0x40;
						if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						/* wait complete sending */
						{/*{{{*/
							PP_S32 retryCnt = 10;
							PP_U8 u8TxFifoCnt = 0;
							PP_U8 u8TxBusy = 0;

							do
							{/*{{{*/
								stReg.reg = 0x0052;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxFifoCnt);

								stReg.reg = 0x008C;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxBusy);
								u8TxBusy &= 0x01;
								PrDbg("txBusy:%x\n", u8TxBusy);

								if( (u8TxFifoCnt == 0) && (u8TxBusy == 0) ) break;
								else
								{
		                            LOOPDELAY_MSEC(100);
									if(retryCnt <= 0) 
									{
										PrError("txBusy:%x\n", u8TxBusy);
										return(eERROR_FAILURE);
									}
								}
							} while(retryCnt--);/*}}}*/
						}/*}}}*/
					}
				}

			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
		case HDT_OLD:
		case HDT_NEW:
			{/*{{{*/
				PrDbgString("utc HDT\n");
				for(i = 0; i < dataSize; i++)
				{
					PrDbg("i:%d, data:0x%02x\n", i COMMA pData[i]);
					stReg.reg = 0x0051;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, pData[i])) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}

					if( (i != 0) && (((i+1) % PVIRX_UTC_HDT_TXCMD_BASE_BYTE_CNT)==0) )
					{
						/* verify utc data count */
						stReg.reg = 0x0052;
						u8RegData = 0;
						if( (result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid read\n");
							return(result);
						}
						if((PVIRX_UTC_HDT_TXCMD_BASE_BYTE_CNT) != (u8RegData))
						{
							PrError("invalid utc data count(%d-%d)\n", PVIRX_UTC_HDT_TXCMD_BASE_BYTE_CNT COMMA u8RegData);
							return(eERROR_FAILURE);
						}

						PrDbg("Send & Wait %dByte\n", PVIRX_UTC_HDT_TXCMD_BASE_BYTE_CNT);
						/* start utc send */
						stReg.reg = 0x0060;
						u8RegMask = 0x40;
						u8RegData = 0x40;
						if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
						{
							PrErrorString("invalid write\n");
							return(result);
						}

						/* wait complete sending */
						{/*{{{*/
							PP_S32 retryCnt = 10;
							PP_U8 u8TxFifoCnt = 0;
							PP_U8 u8TxBusy = 0;
							do
							{/*{{{*/
								stReg.reg = 0x0052;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxFifoCnt);

								stReg.reg = 0x008C;
								PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, &u8TxBusy);
								u8TxBusy &= 0x01;
								PrDbg("txBusy:%x\n", u8TxBusy);

								if( (u8TxFifoCnt == 0) && (u8TxBusy == 0) ) break;
								else
								{
		                            LOOPDELAY_MSEC(100);
									if(retryCnt <= 0) 
									{
										PrError("txBusy:%x\n", u8TxBusy);
										return(eERROR_FAILURE);
									}
								}
							} while(retryCnt--);/*}}}*/
						}/*}}}*/
					}
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
		default:
			{
				PrErrorString("Invalid standard\n");
				result = eERROR_FAILURE;
			}
			break;
	}


	return(result);
}

PP_RESULT_E PPDRV_PVIRX_UTC_SetTable(const PP_S32 IN cid, const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution)
{
	PP_RESULT_E result = eERROR_FAILURE;
	const _stPVIRX_Table_STD_HD_CameraResolution *pTblSTD_UTCHD = NULL;
	const _stPVIRX_Table_EXT_HD_CameraResolution *pTblEXT_UTCHD = NULL;
	const _stPVIRX_Table_SD_CameraResolution *pTblUTCSD = NULL;
	_stPVIRX_Reg stReg;
	PP_S32 regInx = 0;
	PP_U8 u8RegData;
	PP_S32 sizeArray = 0;
	PP_U8 *pData = NULL;

	PrDbg("%x-Set UTC Standard:%d, cameraResolution:%d\n", chanAddr COMMA cameraStandard COMMA cameraResolution);

	if(cameraStandard >= max_camera_standard)
	{
		PrError("Invalid standard:%d\n", cameraStandard);
		return(eERROR_FAILURE);
	}
	if(cameraResolution >= max_camera_resolution)
	{
		PrError("Invalid camera resolution:%d\n", cameraResolution);
		return(eERROR_FAILURE);
	}

	if(cameraResolution > camera_1920x1080p25)
	{/*{{{*/
		switch(cameraStandard)
		{/*{{{*/
			case CVBS:
				{
					pTblUTCSD = (const _stPVIRX_Table_SD_CameraResolution *)stPVIRX_Table_UTC_SD;
				}
				break;
#if defined(SUPPORT_PVIRX_STD_PVI)
			case PVI:
				{
					pTblEXT_UTCHD = (const _stPVIRX_Table_EXT_HD_CameraResolution *)stPVIRX_Table_EXT_UTC_PVI;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
			case HDA:
				{
					pTblEXT_UTCHD = (const _stPVIRX_Table_EXT_HD_CameraResolution *)stPVIRX_Table_EXT_UTC_HDA;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
			case CVI:
				{
					pTblEXT_UTCHD = (const _stPVIRX_Table_EXT_HD_CameraResolution *)stPVIRX_Table_EXT_UTC_CVI;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
			case HDT_OLD:
			case HDT_NEW:
				{
					pTblEXT_UTCHD = (const _stPVIRX_Table_EXT_HD_CameraResolution *)stPVIRX_Table_EXT_UTC_HDT;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
			default:
				{
					PrError("Invalid camera Standard(%d)\n", cameraStandard);
					return(eERROR_FAILURE);
				}
		}/*}}}*/
	}/*}}}*/
	else
	{/*{{{*/
		switch(cameraStandard)
		{/*{{{*/
			case CVBS:
				{
					pTblUTCSD = (const _stPVIRX_Table_SD_CameraResolution *)stPVIRX_Table_UTC_SD;
				}
				break;
#if defined(SUPPORT_PVIRX_STD_PVI)
			case PVI:
				{
					pTblSTD_UTCHD = (const _stPVIRX_Table_STD_HD_CameraResolution *)stPVIRX_Table_STD_UTC_PVI;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
			case HDA:
				{
					pTblSTD_UTCHD = (const _stPVIRX_Table_STD_HD_CameraResolution *)stPVIRX_Table_STD_UTC_HDA;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
			case CVI:
				{
					pTblSTD_UTCHD = (const _stPVIRX_Table_STD_HD_CameraResolution *)stPVIRX_Table_STD_UTC_CVI;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
			case HDT_OLD:
			case HDT_NEW:
				{
					pTblSTD_UTCHD = (const _stPVIRX_Table_STD_HD_CameraResolution *)stPVIRX_Table_STD_UTC_HDT;
				}
				break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
			default:
				{
					PrError("Invalid camera Standard(%d)\n", cameraStandard);
					return(eERROR_FAILURE);
				}
		}/*}}}*/
	}/*}}}*/

	if( (cameraStandard == CVBS) && (pTblUTCSD != NULL) )
	{/*{{{*/
		regInx = 0;
		stReg.reg = pTblUTCSD[regInx].stReg.reg;
		while(stReg.reg != 0xFFFF)
		{
			u8RegData = pTblUTCSD[regInx].u8Data[cameraResolution];
			PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);

			if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
			{
				PrErrorString("invalid write\n");
				return(result);
			}

			regInx++;
			stReg.reg = pTblUTCSD[regInx].stReg.reg;
		}
	}/*}}}*/
	else
	{
		if( (pTblSTD_UTCHD != NULL) )
		{/*{{{*/
			regInx = 0;
			stReg.reg = pTblSTD_UTCHD[regInx].stReg.reg;
			while(stReg.reg != 0xFFFF)
			{
				u8RegData = pTblSTD_UTCHD[regInx].u8Data[cameraResolution-camera_1280x720p60]; 
				PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);

				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				regInx++;
				stReg.reg = pTblSTD_UTCHD[regInx].stReg.reg;
			}
		}/*}}}*/
		if( (pTblEXT_UTCHD != NULL) )
		{/*{{{*/
			regInx = 0;
			stReg.reg = pTblEXT_UTCHD[regInx].stReg.reg;
			while(stReg.reg != 0xFFFF)
			{
				u8RegData = pTblEXT_UTCHD[regInx].u8Data[cameraResolution-camera_1280x960p30]; 
				PrDbg("inx:%d, reg:0x%02x, data:0x%02x\n", regInx COMMA stReg.b.addr COMMA u8RegData);

				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				regInx++;
				stReg.reg = pTblEXT_UTCHD[regInx].stReg.reg;
			}
		}/*}}}*/
	}

	/* set pattern */
	switch(cameraStandard)
	{/*{{{*/
		case CVBS:
			{/*{{{*/
				/* set tx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_sd_tx_pat_format)/sizeof(pvirx_ptz_table_sd_tx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_sd_tx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x81;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set tx pattern data */
				sizeArray = sizeof(pvirx_ptz_table_sd_tx_pat_data)/sizeof(pvirx_ptz_table_sd_tx_pat_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_sd_tx_pat_data;
				stReg.reg = 0x0050;
				u8RegData = 0x82;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_sd_rx_pat_format)/sizeof(pvirx_ptz_table_sd_rx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_sd_rx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x84;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start format */
				sizeArray = sizeof(pvirx_ptz_table_sd_rx_pat_start_format)/sizeof(pvirx_ptz_table_sd_rx_pat_start_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_sd_rx_pat_start_format;
				stReg.reg = 0x0050;
				u8RegData = 0x85;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start data */
				sizeArray = sizeof(pvirx_ptz_table_sd_rx_pat_start_data)/sizeof(pvirx_ptz_table_sd_rx_pat_start_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_sd_rx_pat_start_data;
				stReg.reg = 0x0050;
				u8RegData = 0x86;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#if defined(SUPPORT_PVIRX_STD_PVI)
		case PVI:
			{/*{{{*/
				/* set tx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_std_pvi_tx_pat_format)/sizeof(pvirx_ptz_table_std_pvi_tx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_pvi_tx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x81;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set tx pattern data */
				sizeArray = sizeof(pvirx_ptz_table_std_pvi_tx_pat_data)/sizeof(pvirx_ptz_table_std_pvi_tx_pat_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_pvi_tx_pat_data;
				stReg.reg = 0x0050;
				u8RegData = 0x82;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_std_pvi_rx_pat_format)/sizeof(pvirx_ptz_table_std_pvi_rx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_pvi_rx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x84;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start format */
				sizeArray = sizeof(pvirx_ptz_table_std_pvi_rx_pat_start_format)/sizeof(pvirx_ptz_table_std_pvi_rx_pat_start_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_pvi_rx_pat_start_format;
				stReg.reg = 0x0050;
				u8RegData = 0x85;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start data */
				sizeArray = sizeof(pvirx_ptz_table_std_pvi_rx_pat_start_data)/sizeof(pvirx_ptz_table_std_pvi_rx_pat_start_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_pvi_rx_pat_start_data;
				stReg.reg = 0x0050;
				u8RegData = 0x86;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
		case HDA:
			{
				if(cameraResolution >= camera_1920x1080p30)
				{/*{{{*/
					/* set tx pattern format */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_tx_pat_format_1080p)/sizeof(pvirx_ptz_table_std_hda_tx_pat_format_1080p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_tx_pat_format_1080p;
					stReg.reg = 0x0050;
					u8RegData = 0x81;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set tx pattern data */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_tx_pat_data_1080p)/sizeof(pvirx_ptz_table_std_hda_tx_pat_data_1080p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_tx_pat_data_1080p;
					stReg.reg = 0x0050;
					u8RegData = 0x82;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set rx pattern format */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_rx_pat_format_1080p)/sizeof(pvirx_ptz_table_std_hda_rx_pat_format_1080p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_rx_pat_format_1080p;
					stReg.reg = 0x0050;
					u8RegData = 0x84;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}


					/* set rx pattern start format */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_rx_pat_start_format_1080p)/sizeof(pvirx_ptz_table_std_hda_rx_pat_start_format_1080p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_rx_pat_start_format_1080p;
					stReg.reg = 0x0050;
					u8RegData = 0x85;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set rx pattern start data */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_rx_pat_start_data_1080p)/sizeof(pvirx_ptz_table_std_hda_rx_pat_start_data_1080p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_rx_pat_start_data_1080p;
					stReg.reg = 0x0050;
					u8RegData = 0x86;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}
				}/*}}}*/
				else
				{/*{{{*/
					/* set tx pattern format */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_tx_pat_format_720p)/sizeof(pvirx_ptz_table_std_hda_tx_pat_format_720p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_tx_pat_format_720p;
					stReg.reg = 0x0050;
					u8RegData = 0x81;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set tx pattern data */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_tx_pat_data_720p)/sizeof(pvirx_ptz_table_std_hda_tx_pat_data_720p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_tx_pat_data_720p;
					stReg.reg = 0x0050;
					u8RegData = 0x82;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set rx pattern format */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_rx_pat_format_720p)/sizeof(pvirx_ptz_table_std_hda_rx_pat_format_720p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_rx_pat_format_720p;
					stReg.reg = 0x0050;
					u8RegData = 0x84;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set rx pattern start format */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_rx_pat_start_format_720p)/sizeof(pvirx_ptz_table_std_hda_rx_pat_start_format_720p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_rx_pat_start_format_720p;
					stReg.reg = 0x0050;
					u8RegData = 0x85;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}

					/* set rx pattern start data */
					sizeArray = sizeof(pvirx_ptz_table_std_hda_rx_pat_start_data_720p)/sizeof(pvirx_ptz_table_std_hda_rx_pat_start_data_720p[0]);
					pData = (PP_U8 *)pvirx_ptz_table_std_hda_rx_pat_start_data_720p;
					stReg.reg = 0x0050;
					u8RegData = 0x86;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
					if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
					{
						PrErrorString("invalid fifo count\n");
						return(result);
					}
				}/*}}}*/
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
		case CVI:
			{/*{{{*/
				/* set tx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_std_cvi_tx_pat_format)/sizeof(pvirx_ptz_table_std_cvi_tx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_cvi_tx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x81;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set tx pattern data */
				sizeArray = sizeof(pvirx_ptz_table_std_cvi_tx_pat_data)/sizeof(pvirx_ptz_table_std_cvi_tx_pat_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_cvi_tx_pat_data;
				stReg.reg = 0x0050;
				u8RegData = 0x82;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_std_cvi_rx_pat_format)/sizeof(pvirx_ptz_table_std_cvi_rx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_cvi_rx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x84;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start format */
				sizeArray = sizeof(pvirx_ptz_table_std_cvi_rx_pat_start_format)/sizeof(pvirx_ptz_table_std_cvi_rx_pat_start_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_cvi_rx_pat_start_format;
				stReg.reg = 0x0050;
				u8RegData = 0x85;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start data */
				sizeArray = sizeof(pvirx_ptz_table_std_cvi_rx_pat_start_data)/sizeof(pvirx_ptz_table_std_cvi_rx_pat_start_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_cvi_rx_pat_start_data;
				stReg.reg = 0x0050;
				u8RegData = 0x86;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
		case HDT_OLD:
		case HDT_NEW:
			{/*{{{*/
				/* set tx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_std_hdt_tx_pat_format)/sizeof(pvirx_ptz_table_std_hdt_tx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_hdt_tx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x81;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set tx pattern data */
				sizeArray = sizeof(pvirx_ptz_table_std_hdt_tx_pat_data)/sizeof(pvirx_ptz_table_std_hdt_tx_pat_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_hdt_tx_pat_data;
				stReg.reg = 0x0050;
				u8RegData = 0x82;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern format */
				sizeArray = sizeof(pvirx_ptz_table_std_hdt_rx_pat_format)/sizeof(pvirx_ptz_table_std_hdt_rx_pat_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_hdt_rx_pat_format;
				stReg.reg = 0x0050;
				u8RegData = 0x84;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start format */
				sizeArray = sizeof(pvirx_ptz_table_std_hdt_rx_pat_start_format)/sizeof(pvirx_ptz_table_std_hdt_rx_pat_start_format[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_hdt_rx_pat_start_format;
				stReg.reg = 0x0050;
				u8RegData = 0x85;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}

				/* set rx pattern start data */
				sizeArray = sizeof(pvirx_ptz_table_std_hdt_rx_pat_start_data)/sizeof(pvirx_ptz_table_std_hdt_rx_pat_start_data[0]);
				pData = (PP_U8 *)pvirx_ptz_table_std_hdt_rx_pat_start_data;
				stReg.reg = 0x0050;
				u8RegData = 0x86;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
				if( (result = PPDRV_PVIRX_UTC_WriteFlagFifo(cid, chanAddr, sizeArray, pData)) < 0)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
		default:
			{
				PrError("Invalid camera Standard(%d)\n", cameraStandard);
				return(eERROR_FAILURE);
			}
	}/*}}}*/

	/* set rx fifo configuration. do last */
	switch(cameraStandard)
	{/*{{{*/
		case CVBS:
			{/*{{{*/
				stReg.reg = 0x0054;
				u8RegData = 0x93;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
			break;
#if defined(SUPPORT_PVIRX_STD_PVI)
		case PVI:
			{/*{{{*/
				stReg.reg = 0x0054;
				u8RegData = 0x93;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
		case HDA:
			{
				if(cameraResolution >= camera_1920x1080p30)
				{/*{{{*/
					stReg.reg = 0x0054;
					u8RegData = 0x93;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/
				else
				{/*{{{*/
					stReg.reg = 0x0054;
					u8RegData = 0xD3;
					if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/
			}
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
		case CVI:
			{/*{{{*/
				stReg.reg = 0x0054;
				u8RegData = 0xD3;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
		case HDT_OLD:
		case HDT_NEW:
			{/*{{{*/
				stReg.reg = 0x0054;
				u8RegData = 0x93;
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
		default:
			{
				PrError("Invalid camera Standard(%d)\n", cameraStandard);
				return(eERROR_FAILURE);
			}
	}/*}}}*/

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_UTC_SendData(const PP_S32 IN cid, const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution, const PP_S32 IN dataSize, const PP_U8 IN *pData)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegMask;
	PP_U8 u8RegData = 0;

	PrDbg("%x-Send Standard:%d, cameraResolution:%d\n", chanAddr COMMA cameraStandard COMMA cameraResolution);

	if(cameraStandard >= max_camera_standard)
	{
		PrError("Invalid standard:%d\n", cameraStandard);
		return(eERROR_FAILURE);
	}
	if(cameraResolution >= max_camera_resolution)
	{
		PrError("Invalid camera resolution:%d\n", cameraResolution);
		return(eERROR_FAILURE);
	}

	if( (dataSize <= 0) || (pData == NULL) )
	{
		PrErrorString("Invalid utc data\n");
		return(eERROR_FAILURE);
	}

	/* check current tx fifo size */
	stReg.reg = 0x0052;
	result = PPDRV_PVIRX_Read(cid, chanAddr, stReg.b.addr, (PP_U8 *)&u8RegData);
	if(u8RegData != 0)
	{
		PrErrorString("Not empty\n");
		return(eERROR_FAILURE);
	}

	/* send utc data */
	switch(cameraStandard)
	{/*{{{*/
		case CVBS:
			{/*{{{*/
				/* init data reg */
				stReg.reg = 0x0050;
				u8RegMask = 0xC7;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				/* write utc data */
				if( (result = PPDRV_PVIRX_UTC_SendTxFifo(cid, chanAddr, cameraStandard, cameraResolution, dataSize, pData)) != eSUCCESS)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#if defined(SUPPORT_PVIRX_STD_PVI)
		case PVI:
			{/*{{{*/
				/* init data reg */
				stReg.reg = 0x0050;
				u8RegMask = 0xC7;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				/* write utc data */
				if( (result = PPDRV_PVIRX_UTC_SendTxFifo(cid, chanAddr, cameraStandard, cameraResolution, dataSize, pData)) != eSUCCESS)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_PVI)
#if defined(SUPPORT_PVIRX_STD_HDA)
		case HDA:
			{/*{{{*/
				/* init data reg */
				if(cameraResolution >= camera_1920x1080p30)
				{/*{{{*/
					stReg.reg = 0x0050;
					u8RegMask = 0xC7;
					u8RegData = 0x00;
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/
				else
				{/*{{{*/
					stReg.reg = 0x0050;
					u8RegMask = 0xC7;
					u8RegData = 0x40;
					if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
					{
						PrErrorString("invalid write\n");
						return(result);
					}
				}/*}}}*/

				/* write utc data */
				if( (result = PPDRV_PVIRX_UTC_SendTxFifo(cid, chanAddr, cameraStandard, cameraResolution, dataSize, pData)) != eSUCCESS)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDA)
#if defined(SUPPORT_PVIRX_STD_CVI)
		case CVI:
			{/*{{{*/
				/* init data reg */
				stReg.reg = 0x0050;
				u8RegMask = 0xC7;
				u8RegData = 0x40;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				/* write utc data */
				if( (result = PPDRV_PVIRX_UTC_SendTxFifo(cid, chanAddr, cameraStandard, cameraResolution, dataSize, pData)) != eSUCCESS)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_CVI)
#if defined(SUPPORT_PVIRX_STD_HDT)
		case HDT_OLD:
		case HDT_NEW:
			{/*{{{*/
				/* init data reg */
				stReg.reg = 0x0050;
				u8RegMask = 0xC7;
				u8RegData = 0x00;
				if( (result = PPDRV_PVIRX_WriteMaskBit(cid, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

				/* write utc data */
				if( (result = PPDRV_PVIRX_UTC_SendTxFifo(cid, chanAddr, cameraStandard, cameraResolution, dataSize, pData)) != eSUCCESS)
				{
					PrErrorString("invalid fifo count\n");
					return(result);
				}
			}/*}}}*/
			break;
#endif //defined(SUPPORT_PVIRX_STD_HDT)
		default:
			{
				PrError("Invalid camera Standard(%d)\n", cameraStandard);
				return(eERROR_FAILURE);
			}
	}/*}}}*/

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_UTC_GetRxAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stUTCRxAttr OUT *pstUTCRxAttr)
{
	PP_RESULT_E result = eERROR_FAILURE;

	_stPVIRX_Reg stReg;

	if(pstUTCRxAttr == NULL)
	{
		PrErrorString("Invalid argu\n");
		return(eERROR_FAILURE);
	}

	stReg.reg = 0x0040;
	PrDbgString("UTC GetRxRegs\n");
	if( (result = PPDRV_PVIRX_ReadBurst(cid, chanAddr, stReg.b.addr, sizeof(_stUTCRxAttr), (PP_U8 *)pstUTCRxAttr)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_UTC_GetTxAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stUTCTxAttr OUT *pstUTCTxAttr)
{
	PP_RESULT_E result = eERROR_FAILURE;

	_stPVIRX_Reg stReg;

	if(pstUTCTxAttr == NULL)
	{
		PrErrorString("Invalid argu\n");
		return(eERROR_FAILURE);
	}

	stReg.reg = 0x0060;
	PrDbgString("PTZ GetTxRegs\n");
	if( (result = PPDRV_PVIRX_ReadBurst(cid, chanAddr, stReg.b.addr, sizeof(_stUTCTxAttr), (PP_U8 *)pstUTCTxAttr)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPDRV_PVIRX_UTC_GetHVStartAttr(const PP_S32 IN cid, const PP_U8 IN chanAddr, _stUTCHVStartAttr OUT *pstUTCHVStartAttr)
{
	PP_RESULT_E result = eERROR_FAILURE;

	_stPVIRX_Reg stReg;

	if(pstUTCHVStartAttr == NULL)
	{
		PrErrorString("Invalid argu\n");
		return(eERROR_FAILURE);
	}

	stReg.reg = 0x0058;
	PrDbgString("PTZ GetHVStartRegs\n");
	if( (result = PPDRV_PVIRX_ReadBurst(cid, chanAddr, stReg.b.addr, sizeof(_stUTCHVStartAttr), (PP_U8 *)pstUTCHVStartAttr)) != eSUCCESS)
	{
		PrErrorString("Read reg.\n");
		return(result);
	}

	return(result);
}

#endif //SUPPORT_PVIRX_UTC

#ifdef SUPPORT_PVIRX_FPGA

PP_RESULT_E PPDRV_PVIRX_FPGA_SetTable(const PP_U8 IN chanAddr)
{/*{{{*/
	PP_RESULT_E result = eERROR_FAILURE;
	const _stPVIRX_Table_Common *pTblFPGA = NULL;
	_stPVIRX_Reg stReg;
	PP_S32 regInx = 0;
	PP_U8 u8RegData;

	const _stPVIRX_Table_Common stPVIRX_Table_FPGA_ch0[] = 
	{/*{{{*/
		/* reg, data */
		//{  {0x0015}, 0x81}, //VADC_CLK_SEL
		{  {0x0015}, 0x21}, //VADC_CLK_SEL

		/* End */
		{  {0xFFFF}, 0xFF}, //end
	};/*}}}*/
	const _stPVIRX_Table_Common stPVIRX_Table_FPGA_ch1[] = 
	{/*{{{*/
		/* reg, data */
		//{  {0x0015}, 0x81}, //VADC_CLK_SEL
		{  {0x0015}, 0x21}, //VADC_CLK_SEL

		/* End */
		{  {0xFFFF}, 0xFF}, //end
	};/*}}}*/
	const _stPVIRX_Table_Common stPVIRX_Table_FPGA_ch2[] = 
	{/*{{{*/
		/* reg, data */
		//{  {0x0015}, 0x81}, //VADC_CLK_SEL
		{  {0x0015}, 0x21}, //VADC_CLK_SEL

		/* End */
		{  {0xFFFF}, 0xFF}, //end
	};/*}}}*/
	const _stPVIRX_Table_Common stPVIRX_Table_FPGA_ch3[] = 
	{/*{{{*/
		/* reg, data */
		{  {0x0015}, 0x81}, //VADC_CLK_SEL
		//{  {0x0015}, 0x21}, //VADC_CLK_SEL

		/* End */
		{  {0xFFFF}, 0xFF}, //end
	};/*}}}*/

	PrPrint("%x-Set fpga register\n", chanAddr);

	if(chanAddr == 0) pTblFPGA = (const _stPVIRX_Table_Common *)stPVIRX_Table_FPGA_ch0;
	else if(chanAddr == 1) pTblFPGA = (const _stPVIRX_Table_Common *)stPVIRX_Table_FPGA_ch1;
	else if(chanAddr == 2) pTblFPGA = (const _stPVIRX_Table_Common *)stPVIRX_Table_FPGA_ch2;
	else if(chanAddr == 3) pTblFPGA = (const _stPVIRX_Table_Common *)stPVIRX_Table_FPGA_ch3;
	regInx = 0;
	stReg.reg = pTblFPGA[regInx].stReg.reg;
	while(stReg.reg != 0xFFFF)
	{
		u8RegData = pTblFPGA[regInx].u8Data;
		PrDbg("%x-inx:%d, reg:0x%04x, data:0x%02x\n", chanAddr COMMA regInx COMMA stReg.b.addr COMMA u8RegData);

		if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
		{
			PrErrorString("invalid write\n");
			return(result);
		}

		regInx++;
		stReg.reg = pTblFPGA[regInx].stReg.reg;
	}

#if 0 //distinguish channel by brightness
	switch(chanAddr)
	{
		case 0:
			{
				stReg.reg = 0x0121; //BRIGHTNESS
				u8RegData = 0x80;
				PrDbg("%x-Test brightness reg:0x%04x, data:0x%02x\n", chanAddr COMMA stReg.b.addr COMMA u8RegData);
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}
			}
			break;
		case 1:
			{
				stReg.reg = 0x0121; //BRIGHTNESS
				u8RegData = 0xA0;
				PrDbg("%x-Test brightness reg:0x%04x, data:0x%02x\n", chanAddr COMMA stReg.b.addr COMMA u8RegData);
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

			}
			break;
		case 2:
			{
				stReg.reg = 0x0121; //BRIGHTNESS
				u8RegData = 0x60;
				PrDbg("%x-Test brightness reg:0x%04x, data:0x%02x\n", chanAddr COMMA stReg.b.addr COMMA u8RegData);
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

			}
			break;
		case 3:
			{
				stReg.reg = 0x0121; //BRIGHTNESS
				u8RegData = 0xC0;
				PrDbg("%x-Test brightness reg:0x%04x, data:0x%02x\n", chanAddr COMMA stReg.b.addr COMMA u8RegData);
				if( (result = PPDRV_PVIRX_Write(cid, chanAddr, stReg.b.addr, u8RegData)) != eSUCCESS)
				{
					PrErrorString("invalid write\n");
					return(result);
				}

			}
			break;
		default:
			break;
	}
#endif

	return(result);
}/*}}}*/

#endif // SUPPORT_PVIRX_FPGA

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROC irq
PP_RESULT_E PVIRX_DEV0_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stPVIRX0_PROC_irqs[] = {
	    { .fn = PVIRX_DEV0_PROC_IRQ,	.irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_0,     .next = (void*)0, },
};
PP_RESULT_E PVIRX_DEV1_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stPVIRX1_PROC_irqs[] = {
	    { .fn = PVIRX_DEV1_PROC_IRQ,	.irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_1,     .next = (void*)0, },
};
PP_RESULT_E PVIRX_DEV2_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stPVIRX2_PROC_irqs[] = {
	    { .fn = PVIRX_DEV2_PROC_IRQ,	.irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_2,     .next = (void*)0, },
};
PP_RESULT_E PVIRX_DEV3_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stPVIRX3_PROC_irqs[] = {
	    { .fn = PVIRX_DEV3_PROC_IRQ,	.irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_3,     .next = (void*)0, },
};

PP_RESULT_E PVIRX_DEV0_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PP_S32 irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_0;

//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("PVIRX0", irqNum, gu32PviRxIntcCnt[0]);

	return(eSUCCESS);
}
PP_RESULT_E PVIRX_DEV1_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PP_S32 irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_1;

//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("PVIRX1", irqNum, gu32PviRxIntcCnt[1]);

	return(eSUCCESS);
}
PP_RESULT_E PVIRX_DEV2_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PP_S32 irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_2;

//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("PVIRX2", irqNum, gu32PviRxIntcCnt[2]);

	return(eSUCCESS);
}
PP_RESULT_E PVIRX_DEV3_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PP_S32 irqNum = 100/*IRQ_0_VECTOR*/+IRQ_PVI_RX_3;

//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("PVIRX3", irqNum, gu32PviRxIntcCnt[3]);

	return(eSUCCESS);
}

//PROC device
PP_RESULT_E PVIRX_PROC_DEVICE(PP_S32 argc, const char **argv);
struct proc_device_struct stPVIRX_PROC_devices[] = {
	    { .pName = "PVIRX",  .fn = PVIRX_PROC_DEVICE,    .next = (void*)0, },
};

PP_RESULT_E PVIRX_PROC_DEVICE(PP_S32 argc, const char **argv)
{
	if( (argc) && (strcmp(argv[0], stPVIRX_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_FAILURE);
	}
	printf("\n%s Device Info -------------\n", stPVIRX_PROC_devices[0].pName);

	PVIRX_proc();

	return(eSUCCESS);
}

void PPDRV_PVIRX_Initialize(void)
{
	PP_S32 chanAddr;

	{
        /* get standard */
        if( (BD_CAMERA_IN_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_CVBS)
        {
            if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
            {
                gbPviRxSelSD960H = FALSE;
                DEFAULT_CAMERA_STANDARD = CVBS;
            }
            else if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
            {
                gbPviRxSelSD960H = TRUE;
                DEFAULT_CAMERA_STANDARD = CVBS;
            }
		}
		else if( (BD_CAMERA_IN_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_PVI)
		{
			DEFAULT_CAMERA_STANDARD = PVI;
		}
		else if( (BD_CAMERA_IN_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_CVI)
		{
			DEFAULT_CAMERA_STANDARD = CVI;
		}
		else if( (BD_CAMERA_IN_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_HDA)
		{
			DEFAULT_CAMERA_STANDARD = HDA;
		}
		else if( (BD_CAMERA_IN_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_HDT)
		{
			DEFAULT_CAMERA_STANDARD = HDT_NEW;
		}
		else
		{
			DEFAULT_CAMERA_STANDARD = CVBS;
		}

		/* get resolution */
		if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
		{
			if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_ntsc;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_pal;
			}
			else
			{
				printf("Error. Invalid vinResol720H\n");
			}
		}
		else if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
		{
			if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_ntsc;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_pal;
			}
			else
			{
				printf("Error. Invalid vinResol960H\n");
			}
		}
		else if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
		{
			if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1280x720p30;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1280x720p25;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1280x720p60;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1280x720p50;
			}
			else
			{
				printf("Error. Invalid vinResol720p\n");
			}
		}
		else if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
		{
			if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1280x960p30;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1280x960p25;
			}
			else
			{
				printf("Error. Invalid vinResol960p\n");
			}
		}
		else if( (BD_CAMERA_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
		{
			if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1920x1080p30;
			}
			else if( (BD_CAMERA_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
			{
				DEFAULT_CAMERA_RESOLUTION = camera_1920x1080p25;
			}
			else
			{
				printf("Error. Invalid vinResol1080p\n");
			}
		}
		else
		{
			printf("Error. Invalid vinResol\n");
		}

		/* Initialize PVI RX */
	    PrDbgString("Support PVIRX interrupt.\n");
		for(chanAddr = 0; chanAddr < MAX_PVIRX_CHANCNT; chanAddr++)
		{
			if(PPDRV_PVIRX_SetInit(0, chanAddr) != eSUCCESS)
			{
				printf("Can't initialzie pvirx %d\n", chanAddr);
				continue;
			}

			switch(chanAddr)
			{
				case 0:
					{
						//register proc infomation.
						SYS_PROC_addIrq(stPVIRX0_PROC_irqs);
					}
					break;
				case 1:
					{
						//register proc infomation.
						SYS_PROC_addIrq(stPVIRX1_PROC_irqs);
					}
					break;

				case 2:
					{
						//register proc infomation.
						SYS_PROC_addIrq(stPVIRX2_PROC_irqs);
					}
					break;

				case 3:
					{
						//register proc infomation.
						SYS_PROC_addIrq(stPVIRX3_PROC_irqs);
					}
					break;
				default:
					break;
			}
		}
		SYS_PROC_addDevice(stPVIRX_PROC_devices);
	}

	return;
}


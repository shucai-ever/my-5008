#include "pvitx_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proc.h"
#include "pvitx.h"
#include "pvitx_table.h"


/* pi5008 driver header */
#include "system.h"
#define PVITX_GetRegValue(reg)          (*((volatile unsigned char *)(PVITX_BASE_ADDR+reg)))
#define PVITX_SetRegValue(reg, data)  	((*((volatile unsigned char *)(PVITX_BASE_ADDR+reg))) = (unsigned char)(data))
#define PVITX_BITCLR(reg, mask)         ((*((volatile unsigned char *)(PVITX_BASE_ADDR+reg))) &= ~(unsigned char)(mask))
#define PVITX_BITSET(reg, mask)         ((*((volatile unsigned char *)(PVITX_BASE_ADDR+reg))) |= (unsigned char)(mask))


PP_RESULT_E PPDRV_PVITX_Read(const PP_S32 IN cid, const PP_U16 IN addr, PP_U8 OUT *pData)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_S32 temp_addr=0;
    temp_addr=addr*4;

    *pData = PVITX_GetRegValue(temp_addr);

    return(ret);
}

PP_RESULT_E PPDRV_PVITX_Write(const PP_S32 IN cid, const PP_U16 IN addr, PP_U8 IN data)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_S32 temp_addr=0;
    temp_addr=addr*4;

    PVITX_SetRegValue( temp_addr, data);

    return(ret);
}

PP_RESULT_E PPDRV_PVITX_ReadMaskBit(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN maskBit, PP_U8 OUT *pData)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_U8 tmp_data = 0;
    PP_S32 temp_addr=0;
    temp_addr=addr*4;

    tmp_data = PVITX_GetRegValue(temp_addr);
    tmp_data &= maskBit;
    *pData = tmp_data;

    return(ret);
}

PP_RESULT_E PPDRV_PVITX_WriteMaskBit(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN maskBit, const PP_U8 IN data)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_U8 tmp_data = 0;
    PP_S32 temp_addr=0;
    temp_addr=addr*4;

    tmp_data = PVITX_GetRegValue(temp_addr);
    tmp_data &= ~maskBit;
    tmp_data |= (data & maskBit);

    PVITX_SetRegValue(temp_addr, tmp_data);

    return(ret);
}

PP_RESULT_E PPDRV_PVITX_ReadBurst(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN length, PP_U8 OUT *pData)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_S32 i;
    PP_S32 temp_addr=0;
    temp_addr=addr*4;

    for(i=0; i<length; i++) 
    {
        pData[i] = PVITX_GetRegValue(temp_addr); //<<-- "don't increase addr"
    }

    return(ret);
}

PP_RESULT_E PPDRV_PVITX_WriteBurst(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN length, PP_U8 IN *pData)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_S32 i;
    PP_S32 temp_addr=0;
    temp_addr=addr*4;

    for(i=0; i<length; i++) 
    {
        PVITX_SetRegValue(temp_addr, pData[i]); //<<-- "don't increase addr"
    }

    return(ret);
}


///////////////////////////////////////////////////////////////////////////
static PP_RESULT_E PPDRV_PVITX_TestInfeface(void)
{
    PP_U8 addr = 0x10;
    PP_U8 wr = 0xff;
    PP_U8 rd = 0xff;
    PP_U8 i;

    for(i = 0; i < 0x10; i++)
    {
        wr = 0x80 + i;
        PPDRV_PVITX_Write(0, addr, wr);

        PPDRV_PVITX_Read(0, addr, &rd);

        if(wr != rd)
        {
            PtDbg("rw test(addr:0x%x (w:0x%x-r:0x%x)\n", addr COMMA wr COMMA rd);
            return(eERROR_FAILURE);
        }
    }

    PtPrint("INTF Single R/W ... %s\n", (i>=0x10) ? "Ok.":"Fail.");

    PPDRV_PVITX_Write(0, addr, 0x00); //default

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_PVITX_CheckChipID(PP_U16 OUT *pRetChipID, PP_S32 OUT *pRetRWVerify)
{
    PP_U8 u8Temp = 0;
    PP_U32 chipID = 0;

    if( (pRetChipID == NULL) || (pRetRWVerify == NULL) ) return(eERROR_FAILURE);

    PPDRV_PVITX_Read(0, 0x00, &u8Temp);
    chipID = ((PP_U32)u8Temp)<<8;
    PPDRV_PVITX_Read(0, 0x01, &u8Temp);
    chipID |= (PP_U32)u8Temp;

    PtPrint("PVI_TX ChipID:0x%04x ... %s\n", chipID COMMA ((chipID&0xFF00) == 0x1000) ? "Ok.":"Fail.");

    if( (chipID&0xFF00) != 0x1000) 
    {
        return(eERROR_FAILURE);
    }

    {
        PtPrintString("Check INTF R/W test\n");

        *pRetChipID = chipID;

        /* check interface read & write */
        if( PPDRV_PVITX_TestInfeface() == eSUCCESS)
        {
            *pRetRWVerify = TRUE;
        }
    }

    return(eSUCCESS);
}

void PPDRV_PVITX_Set(const enum _pvi_tx_table_type_format IN typeFormat, const enum _pvi_tx_table_resol_format IN resolFormat)
{
    PP_U16 u16Reg = 0;
    PP_U16 u16Data = 0;
    PP_U8 resolFormatType = 0;
    PP_U8 inx = 0;
    PP_U8 chn = 0;
    PP_U8 maxChn = 1;
    PP_U8 regInx = 0;
    _SD_PVI_TX_TABLE_T *pSdPviTxTable = NULL;
    _HD_PVI_TX_TABLE_T *pHdPviTxTable = NULL;

    if( (typeFormat >= max_pvi_tx_table_type_format) || (resolFormat >= max_pvi_tx_table_resol_format) )
    {
        PtErrorString("invalid format\n");
        return;
    }

    for(inx = chn; inx < maxChn; inx++)
    {
        PtPrint("Set TX type:%s resol:%s\n", _STR_PVI_TX_TYPE_FORMAT[typeFormat] COMMA _STR_PVI_TX_RESOL_FORMAT[resolFormat]);

        /* loading default table register */
        {/*{{{*/
            if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
            {/*{{{*/
                PtDbgString("Set table:pvi_tx_table_format_SD\n");
                pSdPviTxTable = (_SD_PVI_TX_TABLE_T *)pvi_tx_table_sd;

                regInx = 0;
                while(pSdPviTxTable[regInx].addr != 0xFF)
                {
                    u16Reg = pSdPviTxTable[regInx].addr;
                    u16Data = pSdPviTxTable[regInx].data[resolFormat];
                    regInx++;
                    PtDbg("TX Write 0x%x [0x%x]-0x%x\n", u16Reg COMMA u16Reg*4 COMMA  u16Data);
                    PPDRV_PVITX_Write(0, u16Reg,(PP_U8) u16Data);
                }
            }/*}}}*/
            else //HD
            {
                resolFormatType = resolFormat - pvi_tx_table_format_1280x720p60; //HD

                switch(typeFormat)
                {
#ifdef SUPPORT_PVITX_PVI
                    case pvi_tx_table_format_PVI:
                        {
                            PtDbgString("Set table:pvi_tx_table_format_PVI\n");
                            pHdPviTxTable = (_HD_PVI_TX_TABLE_T *)pvi_tx_table_pvi;
                        }
                        break;
#endif /* SUPPORT_PVITX_PVI */
#ifdef SUPPORT_PVITX_HDA
                    case pvi_tx_table_format_HDA:
                        {
                            PtDbgString("Set table:pvi_tx_table_format_HDA\n");
                            pHdPviTxTable = (_HD_PVI_TX_TABLE_T *)pvi_tx_table_hda;
                        }
                        break;
#endif /* SUPPORT_PVITX_HDA */
#ifdef SUPPORT_PVITX_CVI
                    case pvi_tx_table_format_CVI:
                        {
                            PtDbgString("Set table:pvi_tx_table_format_CVI\n");
                            pHdPviTxTable = (_HD_PVI_TX_TABLE_T *)pvi_tx_table_cvi;
                        }
                        break;
#endif /* SUPPORT_PVITX_CVI */
#ifdef SUPPORT_PVITX_HDT
                    case pvi_tx_table_format_HDT:
                        {
                            PtDbgString("Set table:pvi_tx_table_format_HDT\n");
                            pHdPviTxTable = (_HD_PVI_TX_TABLE_T *)pvi_tx_table_hdt;
                        }
                        break;
#endif /* SUPPORT_PVITX_HDT*/
                    default:
                        {
                            PtErrorString("Set table:unknown->pvi\n");
                            pHdPviTxTable = NULL;
                            return;
                        }
                        break;
                }

                regInx = 0;
                while(pHdPviTxTable[regInx].addr != 0xFF)
                {
                    u16Reg = pHdPviTxTable[regInx].addr;
                    u16Data = pHdPviTxTable[regInx].data[resolFormatType];
                    regInx++;
                    PtDbg("TX Write 0x%x-0x%x\n", u16Reg*4 COMMA u16Data);
                    PPDRV_PVITX_Write(0, u16Reg, (PP_U8)u16Data);
                }
            }
        }/*}}}*/

    }

    gpPviTxDrvHost->pviTxType = typeFormat;
    gpPviTxDrvHost->pviTxResol = resolFormat;

    return;
}

/* PVI_TX initialize function */
PP_RESULT_E PPDRV_PVITX_SetInit(const enum _pvi_tx_table_type_format IN pviTxType, const enum _pvi_tx_table_resol_format IN pviTxResol)
{
    PP_U8 u8Temp = 0;
    PP_U32 chipID = 0;

    PPDRV_PVITX_Read(0, 0x00, &u8Temp);
    chipID = ((PP_U32)u8Temp)<<8;
    PPDRV_PVITX_Read(0, 0x01, &u8Temp);
    chipID |= (PP_U32)u8Temp;

    PPDRV_PVITX_Read(0, 0x0F, &u8Temp);

    gpPviTxDrvHost->u16ChipID = (PP_U16)chipID;
    gpPviTxDrvHost->u8RevID = (PP_U8)u8Temp;

    if(gpPviTxDrvHost->u16ChipID == 0x1000)
    {
        gpPviTxDrvHost->u8Initialized = TRUE;
    }
    else
    {
        gpPviTxDrvHost->u8Initialized = FALSE;
        return(eERROR_FAILURE);
    }

    PtDbg("### PVITX Driver Version : v%s ### \n", _VER_PVITX);

    PPDRV_PVITX_Set(pviTxType, pviTxResol);
    return(eSUCCESS);
}

///////////////////////////////////////////////////////////////
//PROC device
PP_RESULT_E PVITX_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stPVITX_PROC_devices[] = {
    { .pName = "PVITX",  .fn = PVITX_PROC_DEVICE,    .next = (void*)0, },
};

PP_RESULT_E PVITX_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
    if( (argc) && (strcmp(argv[0], stPVITX_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
    {
        return(eERROR_FAILURE);
    }
    printf("\n%s Device Info -------------\n", stPVITX_PROC_devices[0].pName);

    PVITX_proc();

    return(eSUCCESS);
}

void PPDRV_PVITX_Initialize(void)
{
    enum _pvi_tx_table_type_format pviTxType = 0;
    enum _pvi_tx_table_resol_format pviTxResol = 0;

    /* get standard */
    if( (BD_PVITX_OUT_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_CVBS)
    {
        if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
        {
            pviTxType = pvi_tx_table_format_SD720;
        }
        else if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
        {
            pviTxType = pvi_tx_table_format_SD960;
        }
    }
    else if( (BD_PVITX_OUT_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_PVI)
    {
        pviTxType = pvi_tx_table_format_PVI;
    }
    else if( (BD_PVITX_OUT_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_CVI)
    {
        pviTxType = pvi_tx_table_format_CVI;
    }
    else if( (BD_PVITX_OUT_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_HDA)
    {
        pviTxType = pvi_tx_table_format_HDA;
    }
    else if( (BD_PVITX_OUT_FMT & VID_STANDARD_MASKBIT) == VID_STANDARD_HDT)
    {
        pviTxType = pvi_tx_table_format_HDT;
    }
    else
    {
        pviTxType = pvi_tx_table_format_PVI;
    }


    /* get resolution */
    if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
    {
        if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
        {
            pviTxResol = pvi_tx_table_format_720x480i60;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
        {
            pviTxResol = pvi_tx_table_format_720x576i50;
        }
        else
        {
            printf("Error. Invalid voutResol720H\n");
        }
    }
    else if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
    {
        if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
        {
            pviTxResol = pvi_tx_table_format_960x480i60;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
        {
            pviTxResol = pvi_tx_table_format_960x576i50;
        }
        else
        {
            printf("Error. Invalid voutResol960H\n");
        }
    }
    else if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
    {
        if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
        {
            pviTxResol = pvi_tx_table_format_1280x720p30;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
        {
            pviTxResol = pvi_tx_table_format_1280x720p25;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60)
        {
            pviTxResol = pvi_tx_table_format_1280x720p60;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50)
        {
            pviTxResol = pvi_tx_table_format_1280x720p50;
        }
        else
        {
            printf("Error. Invalid voutResol720p\n");
        }
    }
    else if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
    {
        if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
        {
            pviTxResol = pvi_tx_table_format_1280x960p30;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
        {
            pviTxResol = pvi_tx_table_format_1280x960p25;
        }
        else
        {
            printf("Error. Invalid voutResol960p\n");
        }
    }
    else if( (BD_PVITX_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
    {
        if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
        {
            pviTxResol = pvi_tx_table_format_1920x1080p30;
        }
        else if( (BD_PVITX_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)
        {
            pviTxResol = pvi_tx_table_format_1920x1080p25;
        }
        else
        {
            printf("Error. Invalid voutResol1080p\n");
        }
    }
    else
    {
        printf("Error. Invalid voutResol\n");
    }

    //Disable DAC PowerDown
    *(vuint32 *)(MISC_BASE_ADDR + 0x70) &= (vuint32)~(1<<20);

    /* Initialize PVI TX */
    PPDRV_PVITX_SetInit(pviTxType, pviTxResol);

    //register proc infomation.
    SYS_PROC_addIrq(NULL);
    SYS_PROC_addDevice(stPVITX_PROC_devices);

    return;
}



#include "pvirx_support.h"

#include <stdio.h>
#include <stdlib.h>

#include "pvirx_drvcommon.h"
#include "pvirx.h"
#include "pvirx_func.h"
#include "pvirx_support.h"
#include "pvirx_user_config.h"

_PviRxDrvHost gPviRxDrvHost[MAX_PVIRX_CHANCNT];
_PviRxDrvHost *gpPviRxDrvHost[MAX_PVIRX_CHANCNT] = {NULL, };

PP_U8 gbPviRxSelSD960H = FALSE; //default: 720h

PP_U32 PVIRX_poll(PP_S32 chipInx)
{
    PP_U32 mask = 0; 

    PrDbg("(chip:%d)\n", chipInx);

    if(chipInx >= MAX_PVIRX_CHANCNT)  return(mask);

    if(gPviRxDrvHost[chipInx].wqPoll == 0)  return(mask);

    if( (gPviRxDrvHost[chipInx].stPVIRX_Isr.stUserPoll.bitIrq != 0) )
    {    
        mask = POLLIN | POLLRDNORM;
    }    

        return mask;
}

PP_RESULT_E PVIRX_proc(void)
{/*{{{*/
    PP_S32 fd = 0;
    PP_S32 i, j;
    _stPVIRX_Reg stReg;
    PP_U8 u8Data[20] = {0, };

#ifdef SUPPORT_PVIRX_VID
    _stChnAttr stChnAttr;
    _stCscAttr stCscAttr;
    _stContrast stContrast;
    _stBright stBright;
    _stSaturation stSaturation;
    _stHue stHue;
    _stSharpness stSharpness;
#endif // SUPPORT_PVIRX_VID

#ifdef SUPPORT_PVIRX_UTC
    _stUTCRxAttr stUTCRxAttr;
    _stUTCHVStartAttr stUTCHVStartAttr;
    _stUTCTxAttr stUTCTxAttr;
#endif // SUPPORT_PVIRX_UTC

    _stPVIRX_Irq stPVIRX_Irq;
    _stPVIRX_VidStatusReg stPVIRX_VidStatusReg;
    _stPVIRX_ManIfmtReg stPVIRX_ManIfmtReg;

    //loff_t *spos = (loff_t *) v;

    printf("### PVIRX info(Driver Version : v%s) ### \n", _VER_PVIRX);

    printf("---------- Driver loading environment(pvirx_user_config.c) param ------------------\n");
    printf("[ENV] CHAN COUNT: %d\n", MAX_PVIRX_CHANCNT);

    printf("[ENV] Chan Attribute:\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++)
    {
        printf("  [CHAN%d]: ChanAddr(0x%02X)\n", i, PVIRX_ATTR_CHIP[i].chanAddr);

        printf("    vinMode(0:[Differential|VinPN], 1:VinP, 3:VinN): %d\n", PVIRX_ATTR_CHIP[i].vinMode);
    }

    printf("---------------------- PVIRX chan info ---------------------\n");
    printf("Chan status.\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++)
    {
        printf("  [CHAN%d]:\n", i);
        printf("    ChipID:0x%04X, RevID:0x%02X (Initialized:%s)\n", gpPviRxDrvHost[i]->chipID, gpPviRxDrvHost[i]->u8RevID, ((gpPviRxDrvHost[i]->u8Initialized == 0)?"Fail":"Success")); 
    }

    printf("------------------ irq proc info -------------------------\n");
    printf("Irq Attribute: \n");
    printf("    Chan IrqCtrl SyncPrd Wake0Prd Wake1Prd IRQNovidMd IRQENA IRQCLR IRQSTATUS\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        /* Read irq status. */
        stReg.reg = 0x0080;
        for(j = 0; j < 14; j++)
        {
            PPDRV_PVIRX_Read(fd, i, stReg.b.addr+j, (PP_U8 *)&u8Data[j]);
        }
        printf("    %4d      %02x      %02x       %02x       %02x         %02x  %02x/%02x  %02x/%02x  %02x/%02x\n", i,
                u8Data[0], u8Data[1], u8Data[2], u8Data[3], u8Data[4], 
                u8Data[8], u8Data[9], u8Data[10], u8Data[11], u8Data[12], u8Data[13]);
    }

#ifdef SUPPORT_PVIRX_VID
    printf("-------------------- vid proc info -------------------------\n");
    printf("Vid Active/Delay attribute: \n");
    printf("    Chan Hactive Hdelay Vactive Vdelay\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        if( PPDRV_PVIRX_VID_GetChnAttr(fd, i, &stChnAttr) >= 0)
        {
            printf("    %4d %7d %6d %7d %6d\n", i,
                    stChnAttr.u16HActive,
                    stChnAttr.u16HDelay,
                    stChnAttr.u16VActive,
                    stChnAttr.u16VDelay);
        }
    }

    printf("CSC attribute: \n");
    printf("    Chan CbGain CrGain CbOff CrOff \n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        if(PPDRV_PVIRX_VID_GetCscAttr(fd, i, &stCscAttr) >= 0)
        {
            printf("    %4d %6x %6x %5x %5x\n", i,
                    stCscAttr.u8CbGain,
                    stCscAttr.u8CrGain,
                    stCscAttr.u8CbOffset,
                    stCscAttr.u8CrOffset);
        }
    }
    printf("Video setup: \n");
    printf("    Chan Contrast Bright Saturation Hue Sharpness\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        printf("    %4d", i);
        if(PPDRV_PVIRX_VID_GetContrast(fd, i, &stContrast) >= 0)
        {
            printf(" %8x", stContrast.u8Contrast);
        }
        if(PPDRV_PVIRX_VID_GetBright(fd, i, &stBright) >= 0)
        {
            printf(" %6x", stBright.u8Bright);
        }
        if(PPDRV_PVIRX_VID_GetSaturation(fd, i, &stSaturation) >= 0)
        {
            printf(" %10x", stSaturation.u8Saturation);
        }
        if(PPDRV_PVIRX_VID_GetHue(fd, i, &stHue) >= 0)
        {
            printf(" %3x", stHue.u8Hue);
        }
        if(PPDRV_PVIRX_VID_GetSharpness(fd, i, &stSharpness) >= 0)
        {
            printf(" %9x", stSharpness.u8Sharpness);
        }
        printf("\n");
    }
#endif // SUPPORT_PVIRX_VID

#if defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 
    printf("------------------ ceq proc info -------------------------\n");
    printf("CEQ Step: \n");
    printf("     Chan (C_Lock/AcGainAdj/AcGainHold/EqCnt) Steps\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {    
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        printf("     %4d (%d/%d/%d/%d)                           ", i, 
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.C_LOCK_CNT,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.AC_GAIN_ADJ,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.AC_GAIN_HOLD,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.EQ_CNT);
        printf("\n");
    }    
    printf("CEQ FirstGainStatus: \n");
    printf("    Chan level dcGain acGain  comp1  comp2 atten1 atten2\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {    
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        printf("    %4d  0x%02x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", i,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.syncLevel,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.dcGain,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.acGain,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.comp1,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.comp2,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.atten1,
                gpPviRxDrvHost[i]->stPVIRX_Isr.stJobProc.stFirstDetGainStatus.atten2);
    }

#endif //defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 

    printf("Current Set: \n");
    printf("     Chan CameraType Resolution\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {    
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        if( (gpPviRxDrvHost[i]->stPrRxMode.standard < max_camera_standard) && (gpPviRxDrvHost[i]->stPrRxMode.cameraResolution < max_camera_resolution) )
        {
#ifdef SUPPORT_PVIRX_HELP_STRING 
            printf("     %4d %-10s %s\n", i, 
                    _strCameraStandard[(gpPviRxDrvHost[i]->stPrRxMode.standard)],
                    _strCameraResolution[(gpPviRxDrvHost[i]->stPrRxMode.cameraResolution)]);
#else // SUPPORT_PVIRX_HELP_STRING 
            printf("     %4d %-10d %d\n", i, 
                    gpPviRxDrvHost[i]->stPrRxMode.standard,
                    gpPviRxDrvHost[i]->stPrRxMode.cameraResolution);
#endif // SUPPORT_PVIRX_HELP_STRING 
        }
    }    

#ifdef SUPPORT_PVIRX_UTC
    printf("-------------------- utc proc info -------------------------\n");
    printf("UTC Tx attribute: \n");
    printf("    Chan fType fPol start pEn hstOs lCnt hst dPol freqFirst freqency   hpst lLen aDLen lRNum rptEn cGNum tpSel gEn\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        if(PPDRV_PVIRX_UTC_GetTxAttr(fd, i, &stUTCTxAttr) >= 0)
        {
            printf("    %4d %5d %4d %5d %3d %5d %4d %3d %4d 0x%07x 0x%06x 0x%04x %4d %5d %5d %5d %5d %5d %3d\n", i,
                    stUTCTxAttr.b.fieldType,
                    stUTCTxAttr.b.fieldPol,
                    stUTCTxAttr.b.start,
                    stUTCTxAttr.b.pathEn,
                    stUTCTxAttr.b.hstOs,
                    stUTCTxAttr.b.lineCnt,
                    stUTCTxAttr.b.hst,
                    stUTCTxAttr.b.dataPol,
                    (stUTCTxAttr.b.freqFirst23<<16)| (stUTCTxAttr.b.freqFirst15<<8) | stUTCTxAttr.b.freqFirst07,
                    (stUTCTxAttr.b.freq23<<16) | (stUTCTxAttr.b.freq15<<8) | stUTCTxAttr.b.freq07,
                    (stUTCTxAttr.b.hpst12<<8) | stUTCTxAttr.b.hpst07,
                    stUTCTxAttr.b.lineLen,
                    stUTCTxAttr.b.allDataLen,
                    stUTCTxAttr.b.lastRptNum,
                    stUTCTxAttr.b.rptEn,
                    stUTCTxAttr.b.cmdGrpNum,
                    stUTCTxAttr.b.tpSel,
                    stUTCTxAttr.b.grpEn);
        }
    }
    printf("UTC Rx attribute: \n");
    printf("    Chan fType fPol iFrmEn iLEn start pEn hstOs lCnt hst dPol freqFirst freqency lpfLen pOff lLen vCnt tpSel aHEn tEn\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        if(PPDRV_PVIRX_UTC_GetRxAttr(fd, i, &stUTCRxAttr) >= 0)
        {
            printf("    %4d %5d %4d %6d %4d %5d %3d %5d %4d %3d %4d 0x%07x 0x%06x %6d %4d %4d %4d %5d %4d %3d\n", i,
                    stUTCRxAttr.b.fieldType,
                    stUTCRxAttr.b.fieldPol,
                    stUTCRxAttr.b.ignoreFrmEn,
                    stUTCRxAttr.b.ignoreLineEn,
                    stUTCRxAttr.b.start,
                    stUTCRxAttr.b.pathEn,
                    stUTCRxAttr.b.hstOs,
                    stUTCRxAttr.b.lineCnt,
                    stUTCRxAttr.b.hst,
                    stUTCRxAttr.b.dataPol,
                    (stUTCRxAttr.b.freqFirst23<<16) | (stUTCRxAttr.b.freqFirst15<<8) | stUTCRxAttr.b.freqFirst07,
                    (stUTCRxAttr.b.freq23<<16) | (stUTCRxAttr.b.freq15<<8) | stUTCRxAttr.b.freq07,
                    stUTCRxAttr.b.lpfLen,
                    stUTCRxAttr.b.pixOffset,
                    stUTCRxAttr.b.lineLen,
                    stUTCRxAttr.b.validCnt,
                    stUTCRxAttr.b.tpSel,
                    stUTCRxAttr.b.addrHoldEn,
                    stUTCRxAttr.b.testEn);
        }
    }
    printf("UTC HV Start attribute: \n");
    printf("    Chan rxHstartOs rxHSyncPol rxVstartOs rxVsyncPol txHstartOs txHSyncPol txVstartOs txVsyncPol\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        if(PPDRV_PVIRX_UTC_GetHVStartAttr(fd, i, &stUTCHVStartAttr) >= 0)
        {
            printf("    %4d %10d %10d %10d %10d %10d %10d %10d %10d\n", i,
                    (stUTCHVStartAttr.b.rxHstrtOs13<<8) | stUTCHVStartAttr.b.rxHstrtOs07,
                    stUTCHVStartAttr.b.rxHsyncPol,
                    (stUTCHVStartAttr.b.rxVstrtOs10<<8) | stUTCHVStartAttr.b.rxVstrtOs07,
                    stUTCHVStartAttr.b.rxVsyncPol,
                    (stUTCHVStartAttr.b.txHstrtOs13<<8) | stUTCHVStartAttr.b.txHstrtOs07,
                    stUTCHVStartAttr.b.txHsyncPol,
                    (stUTCHVStartAttr.b.txVstrtOs10<<8) | stUTCHVStartAttr.b.txVstrtOs07,
                    stUTCHVStartAttr.b.txVsyncPol);

        }
    }
#endif // SUPPORT_PVIRX_UTC

    printf("------------------ current irq status -------------------------\n");
    printf("Chan UTC VFD|NOVID\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {    
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        /* Read irq status. */
        stReg.reg = 0x008C;
        for(j = 0; j < sizeof(_stPVIRX_Irq); j++)
        {
            PPDRV_PVIRX_Read(fd, i, stReg.b.addr+j, (PP_U8 *)&u8Data[j]);
        }

        memcpy(&stPVIRX_Irq, u8Data, sizeof(_stPVIRX_Irq));

        printf("%4d  %02x        %02x\n", i,
                stPVIRX_Irq.reg[0],
                stPVIRX_Irq.reg[1]);
    }            

    printf("-------------------- current detect format -------------------------\n");
    for(i = 0; i < MAX_PVIRX_CHANCNT; i++) 
    {    
        if(gpPviRxDrvHost[i]->chipID != 0x2000) break;

        stReg.reg = 0x0000;
        for(j = 0; j < 20; j++)
        {
            PPDRV_PVIRX_Read(fd, i, stReg.b.addr+j, (PP_U8 *)&u8Data[j]);
        }
        {     
            printf("Chan%d Lock[0x00..0x03] [%02x,%02x,%02x,%02x]\n", i, u8Data[0], u8Data[1], u8Data[2], u8Data[3]);
            stPVIRX_VidStatusReg.reg[0] = u8Data[0];
            stPVIRX_VidStatusReg.reg[1] = u8Data[1];
            stPVIRX_VidStatusReg.reg[2] = u8Data[2];
            stPVIRX_ManIfmtReg.reg[0] = u8Data[0x10];

            if( (u8Data[0]|u8Data[1]|u8Data[2]) != 0)
            {    
                printf("   Detect[std:%d(PVI/CVI/HDA/HDT), ref:%d(25/30/50/60), res:%d(480i/576i/720p/1080p/960p)]\n",
                        stPVIRX_VidStatusReg.b.det_ifmt_std, 
                        stPVIRX_VidStatusReg.b.det_ifmt_ref, 
                        stPVIRX_VidStatusReg.b.det_ifmt_res);
                printf("   CurSet[std:%d(PVI/CVI/HDA/HDT), ref:%d(25/30/50/60), res:%d(480i/576i/720p/1080p/960p)]\n",
                        stPVIRX_ManIfmtReg.b.man_ifmt_std, 
                        stPVIRX_ManIfmtReg.b.man_ifmt_ref, 
                        stPVIRX_ManIfmtReg.b.man_ifmt_res);
                printf("   EQ_Gain:DC(%04x),AC(%04x)\n", u8Data[4]<<8|u8Data[5], u8Data[6]<<8|u8Data[7]);
                printf("   EQ_Comp:C1(%04x),C2(%04x)\n", u8Data[8]<<8|u8Data[9], u8Data[0xA]<<8|u8Data[0xB]);
                printf("   EQ_Atten:A1(%04x),A2(%04x)\n", u8Data[0xC]<<8|u8Data[0xD], u8Data[0xE]<<8|u8Data[0xF]);
                printf("   MAN_IF:IFMT(%04x),MAN_EQ(%04x)\n", u8Data[0x10]<<8|u8Data[0x11], u8Data[0x12]<<8|u8Data[0x13]);
            }
        }
    }
    printf("--------------------------------------------------------------\n");


    return(eSUCCESS);
}/*}}}*/

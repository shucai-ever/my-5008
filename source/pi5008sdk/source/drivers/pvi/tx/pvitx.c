#include <stdio.h>
#include <stdlib.h>

#include "pvitx.h"
#include "pvitx_func.h"
#include "pvitx_support.h"

_PviTxDrvHost gPviTxDrvHost;
_PviTxDrvHost *gpPviTxDrvHost = &gPviTxDrvHost;

PP_RESULT_E PVITX_proc(void)
{/*{{{*/
    PP_S32 i;
    PP_U16 u16Addr;
    PP_U8 u8Data[20] = {0, };

    printf("### PVITX info(Driver Version : v%s) ### \n", _VER_PVITX);

    printf("---------------------- PVITX info ---------------------\n");
    {
        printf("  ChipID:0x%04X, RevID:0x%02X (Initialized:%s)\n", gpPviTxDrvHost->u16ChipID, gpPviTxDrvHost->u8RevID, ((gpPviTxDrvHost->u8Initialized == 0)?"Fail":"Success")); 
    }

    printf("------------------ detect proc info -------------------------\n");

    u16Addr = 0x08;
    for(i = 0; i < 4; i++)
    {
        PPDRV_PVITX_Read(0, u16Addr+i, &u8Data[i]);
    }
    printf("Detect: \n");
    printf("    Status Hsize Vsize\n");
    printf("        %02x  %04x  %04x\n", u8Data[0], (u8Data[1]<<3)|(u8Data[3]>>5), (u8Data[2]<<4)|(u8Data[3]&0xF) );

    u16Addr = 0x10;
    for(i = 0; i < 3; i++)
    {
        PPDRV_PVITX_Read(0, u16Addr+i, &u8Data[i]);
    }
    printf("FMT: \n");
    printf("    IN_FMT HD_IN_SEL CSC_MD\n");
    printf("        %02x        %02x     %02x\n", u8Data[0], u8Data[1], u8Data[2] );

    printf("-------------------- vid proc info -------------------------\n");

    u16Addr = 0x1C;
    for(i = 0; i < 4; i++)
    {
        PPDRV_PVITX_Read(0, u16Addr+i, &u8Data[i]);
    }
    printf("CSC attribute: \n");
    printf("    YGain CbGain CrGain\n");
    printf("    %5x %6x %6x\n",
            (u8Data[0]<<2)|(u8Data[3]&0x3),
            (u8Data[1]<<2)|((u8Data[3]>>2)&0x3),
            (u8Data[2]<<2)|((u8Data[3]>>4)&0x3) );

    u16Addr = 0x13;
    for(i = 0; i < 5; i++)
    {
        PPDRV_PVITX_Read(0, u16Addr+i, &u8Data[i]);
    }
    printf("Video setup: \n");
    printf("    Contrast(en) Bright(en) Saturation(en) Hue(en)\n");
    printf("    %8x %2d ", u8Data[1], (u8Data[0]>>3)&1);
    printf(" %6x %2d ", u8Data[2], (u8Data[0]>>2)&1);
    printf(" %10x %2d ", u8Data[3], (u8Data[0]>>1)&1);
    printf(" %3x %2d ", u8Data[4], (u8Data[0]>>0)&1);
    printf("\n");

    u16Addr = 0x1A;
    for(i = 0; i < 2; i++)
    {
        PPDRV_PVITX_Read(0, u16Addr+i, &u8Data[i]);
    }
    printf("SYNC Start Delay: %04x\n", (u8Data[1]<<8)|(u8Data[0]) );

    u16Addr = 0x80;
    for(i = 0; i < 8; i++)
    {
        PPDRV_PVITX_Read(0, u16Addr+i, &u8Data[i]);
    }
    printf("Dac: \n");
    printf("    CTL0 HLOC0 CTL1 HLOC1 MSK_STRT MSK_END\n");
    printf("      %02x  %04x", u8Data[0], (u8Data[1]<<8)|(u8Data[2]));
    printf("   %02x  %04x", u8Data[3], (u8Data[4]<<8)|(u8Data[5]));
    printf("       %02x      %02x", u8Data[6], u8Data[7]);
    printf("\n");

    u16Addr = 0x10;
    PPDRV_PVITX_Read(0, u16Addr, &u8Data[0]);
    printf("Current Set: \n");
    printf("     CameraType   Resolution BT.xxxx\n");
    printf("     %-10s %12s %-7s\n",
            _STR_PVI_TX_TYPE_FORMAT[gpPviTxDrvHost->pviTxType], 
            _STR_PVI_TX_RESOL_FORMAT[gpPviTxDrvHost->pviTxResol],
            ((u8Data[0]&0x7) == 0)?"BT656":(((u8Data[0]&0x7) == 1)?"BT1120": "unknown") );

    printf("--------------------------------------------------------------\n");


    return(eSUCCESS);
}/*}}}*/

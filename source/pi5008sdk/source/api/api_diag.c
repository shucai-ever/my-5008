/*
 */

#include <stdio.h>
#include <api_diag.h>
#include <api_pvirx_func.h>

#include "osal.h"
#include "sub_intr.h"
#include "vin.h"
#include "vin_user_config.h"
#include "diag.h"

#include "api_vin.h"

PP_RESULT_E PPAPI_DIAG_Initialize(void)
{
	PPDRV_DIAG_Initialize();

	return eSUCCESS;
}

PP_VOID PPAPI_DIAG_GetFrameRate(const PP_U8 u8WaitSec, PP_U32 *pu32FrameCnt)
{
    PP_U32 u32VsyncCnt[2][IRQ_VSYNC_DU+1] = {{0, }, };
    int i;

    PPDRV_VSYNC_GetCurrentCnt((PP_U32 *)&u32VsyncCnt[0]);

    OSAL_sleep(u8WaitSec*1000);

    PPDRV_VSYNC_GetCurrentCnt((PP_U32 *)&u32VsyncCnt[1]);

    for(i = IRQ_VSYNC_QUAD; i <= IRQ_VSYNC_DU; i++)
    {
        if(u32VsyncCnt[1][i] < u32VsyncCnt[0][i])
        {
            pu32FrameCnt[i] = (0xFFFFFFFF - u32VsyncCnt[0][i]) + u32VsyncCnt[1][i];
        }
        else
        {
            pu32FrameCnt[i] = u32VsyncCnt[1][i] - u32VsyncCnt[0][i];
        }
    }
#if ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_PVI)
    {
        PP_U8 statusNoVideo = 0;
        for(i = 0; i < 4; i++)
        {
            statusNoVideo = 0;
            PPAPI_PVIRX_GetNovidStatus(i, &statusNoVideo);
            if(statusNoVideo) //Novideo. frame rate=>0
            {
                pu32FrameCnt[IRQ_VSYNC_VIN0+i] = 0;
            }
        }
    }
#else
    {
        VIN_SYNC_VIN_HSIZE_INFO_CONFIG_U *pHsizeInfo;
        PP_U32 inputInfo[3] = {0, };

        for(i = 0; i < 4; i++)
        {
            PPAPI_VIN_GetInputInfo(i, inputInfo);

            pHsizeInfo = (VIN_SYNC_VIN_HSIZE_INFO_CONFIG_U *)&inputInfo[0];
            if(pHsizeInfo->param.video_loss_det) //Novideo. frame rate=>0
            {
                pu32FrameCnt[IRQ_VSYNC_VIN0+i] = 0;
            }
        }
    }
#endif 

}

PP_VOID PPAPI_DIAG_GetDisplayFreezeStatus(PP_U8 pRetStatus[])
{
    PP_U8 stStatus[4] = {0, };
    int i;

    if(pRetStatus == NULL) return;

    PPDRV_DIAG_GetDisplayFreezeStatus(stStatus);

    for(i = 0; i < 4; i++)
    {
        pRetStatus[i] = stStatus[i];
    }
}

PP_VOID PPAPI_DIAG_GetCamInvalidStatus(PP_U8 pRetStatus[])
{
    PP_U8 stStatus[4] = {0, };
    int i;

    if(pRetStatus == NULL) return;

    PPDRV_DIAG_GetCamInvalidStatus(stStatus);

    for(i = 0; i < 4; i++)
    {
        pRetStatus[i] = stStatus[i];
    }
}

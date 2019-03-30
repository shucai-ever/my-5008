#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api_pvitx_func.h"
#include "debug.h"

PP_RESULT_E PPAPI_PVITX_CheckChipID(PP_U16 OUT *pRetChipID, PP_S32 OUT *pRetRWVerify)
{
    PP_RESULT_E result;
    result = PPDRV_PVITX_CheckChipID(pRetChipID, pRetRWVerify);
    return(result);
}

void PPAPI_PVITX_Set(const enum _pvi_tx_table_type_format IN typeFormat, const enum _pvi_tx_table_resol_format IN resolFormat)
{
    PPDRV_PVITX_Set(typeFormat, resolFormat);
    return;
}

PP_RESULT_E PPAPI_PVITX_SetInit(const enum _pvi_tx_table_type_format IN pviTxType, const enum _pvi_tx_table_resol_format IN pviTxResol)
{
    PP_RESULT_E result;
    result = PPDRV_PVITX_SetInit(pviTxType, pviTxResol);
    return(result);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PPAPI_PVITX_Initialize(void)
{
    PPDRV_PVITX_Initialize();

    return;
}


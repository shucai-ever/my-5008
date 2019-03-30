#ifndef __API_PVITX_FUNC_H__
#define __API_PVITX_FUNC_H__

#include "error.h"
#include "pvitx_func.h"

PP_RESULT_E PPAPI_PVITX_CheckChipID(PP_U16 OUT *pRetChipID, PP_S32 OUT *pRetRWVerify);
void PPAPI_PVITX_Set(const enum _pvi_tx_table_type_format IN typeFormat, const enum _pvi_tx_table_resol_format IN resolFormat);
PP_RESULT_E PPAPI_PVITX_SetInit(const enum _pvi_tx_table_type_format IN pviTxType, const enum _pvi_tx_table_resol_format IN pviTxResol);
void PPAPI_PVITX_Initialize(void);


#endif // __PVIRX_FUNC_H__

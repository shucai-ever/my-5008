#ifndef __PVITX_FUNC_H__
#define __PVITX_FUNC_H__

#include "type.h"
#include "error.h"

#include "pvitx_support.h"
#include "pvitx_table.h"

/* interface function */
PP_RESULT_E PPDRV_PVITX_Read(const PP_S32 IN cid, const PP_U16 IN addr, PP_U8 OUT *pData);
PP_RESULT_E PPDRV_PVITX_Write(const PP_S32 IN cid, const PP_U16 IN addr, PP_U8 IN data);
PP_RESULT_E PPDRV_PVITX_ReadMaskBit(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN maskBit, PP_U8 OUT *pData);
PP_RESULT_E PPDRV_PVITX_WriteMaskBit(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN maskBit, const PP_U8 IN data);
PP_RESULT_E PPDRV_PVITX_ReadBurst(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN length, PP_U8 OUT *pData);
PP_RESULT_E PPDRV_PVITX_WriteBurst(const PP_S32 IN cid,  const PP_U16 IN addr, const PP_U8 IN length, PP_U8 IN *pData);

PP_RESULT_E PPDRV_PVITX_CheckChipID(PP_U16 OUT *pRetChipID, PP_S32 OUT *pRetRWVerify);
void PPDRV_PVITX_Set(const enum _pvi_tx_table_type_format IN typeFormat, const enum _pvi_tx_table_resol_format IN resolFormat);
PP_RESULT_E PPDRV_PVITX_SetInit(const enum _pvi_tx_table_type_format IN pviTxType, const enum _pvi_tx_table_resol_format IN pviTxResol);
void PPDRV_PVITX_Initialize(void);


#endif // __PVIRX_FUNC_H__

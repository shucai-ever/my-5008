#ifndef _SYS_API_H_
#define _SYS_API_H_

#include "type.h"

PP_U32 PPAPI_SYS_GetChipID(PP_VOID);
PP_VOID PPAPI_SYS_SetDeviceID(PP_U32 IN u32ID);
PP_U32 PPAPI_SYS_GetDeviceID(PP_VOID);
PP_VOID PPAPI_SYS_Reset(PP_VOID);
PP_U32 PPAPI_SYS_GetBootSection(PP_VOID);

PP_VOID PPAPI_SYS_CACHE_Writeback(PP_U32 IN *pAddr, PP_S32 IN size);
PP_VOID PPAPI_SYS_CACHE_Invalidate(PP_U32 IN *pAddr, PP_S32 IN size);
PP_VOID PPAPI_SYS_CACHE_Invalidate_ALL(PP_VOID);
PP_VOID PPAPI_SYS_CACHE_Writeback_ALL(PP_VOID);

#endif /* _SYS_API_H_ */

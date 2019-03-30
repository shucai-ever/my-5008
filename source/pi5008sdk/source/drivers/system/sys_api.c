#include "system.h"
#include "debug.h"
#include "osal.h"
#include "cache.h"
#include "sys_api.h"

#define BOOT_RESULT_ADDR			0x13ff0

PP_U32 PPAPI_SYS_GetChipID(PP_VOID)
{
	return((PP_U32)(*(vuint32 *)(SCU_BASE_ADDR + 0x5C)));
}

PP_VOID PPAPI_SYS_SetDeviceID(PP_U32 IN u32ID)
{
	*(vuint32 *)(MISC_BASE_ADDR + 0x6C) = u32ID;
}

PP_U32 PPAPI_SYS_GetDeviceID(PP_VOID)
{
	return((PP_U32)(*(vuint32 *)(MISC_BASE_ADDR + 0x6C)));
}

PP_VOID PPAPI_SYS_Reset(PP_VOID)
{
	*(vuint32 *)(SCU_BASE_ADDR + 0x40) = 0x80;
}

PP_U32 PPAPI_SYS_GetBootSection(PP_VOID)
{
	return *(PP_U32 *)BOOT_RESULT_ADDR;
}
/********************************
 * CACHE
 ********************************/
PP_VOID PPAPI_SYS_CACHE_Writeback(PP_U32 IN *pAddr, PP_S32 IN size)
{
	cache_wb_range(pAddr, size);

	return;
}

PP_VOID PPAPI_SYS_CACHE_Invalidate(PP_U32 IN *pAddr, PP_S32 IN size)
{
	cache_inv_range(pAddr, size);

	return;
}
PP_VOID PPAPI_SYS_CACHE_Writeback_ALL(PP_VOID)
{
	cache_wb_all();
}

PP_VOID PPAPI_SYS_CACHE_Invalidate_ALL(PP_VOID)
{
	cache_inv_all();
}


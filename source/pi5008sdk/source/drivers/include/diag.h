#ifndef _PI5008_DIAG_H_
#define _PI5008_DIAG_H_

#include "system.h"
#include "type.h"
#include "error.h"

#ifndef _VER_DIAG
#define _VER_DIAG "1_000"
#endif //_VER_DIAG

void PPDRV_DIAG_GetDisplayFreezeStatus(unsigned char pRetStatus[]);
void PPDRV_DIAG_GetCamInvalidStatus(unsigned char pRetStatus[]);
void PPDRV_DIAG_Set(void);
void PPDRV_DIAG_Initialize(void);

#endif

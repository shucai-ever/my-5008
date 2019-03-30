#ifndef __APP_CALIBRATION_H__
#define __APP_CALIBRATION_H__
#ifdef CALIB_LIB_USE
#include "type.h"
#include "error.h"

#include "pcvOffLineCalib.h"
#include "pcvSvmView.h"
#include "viewmode_config.h"


PP_VOID PPAPP_Offcalib_Main(PP_U16  IN step);
PP_U32 PPAPP_Section_Viewgen_Main(PP_U16  IN step);
#if 0
PP_U32 PPAPP_Section_Viewgen_Main_Swing_Static_Section(PP_VOID);
#endif

PP_VOID PPAPP_Section_Viewgen_Get_Req_Core1_Update(PP_U32 section_num);
PP_VOID PPAPP_Section_Viewgen_Core1_Done(PP_VOID);


#else//CALIB_LIB_USE



#include "type.h"
#include "error.h"

#endif ////CALIB_LIB_USE

#endif // __APP_CALIBRATION_H__


#ifndef __TASK_CALIBRATION_H__
#define __TASK_CALIBRATION_H__
#ifdef CALIB_LIB_USE
#include "type.h"
#include "error.h"

#include "pcvOffLineCalib.h"
#include "pcvSvmView.h"
#include "viewmode_config.h"

PP_VOID vTaskCalibration(PP_VOID *pvData);



typedef enum {
	TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_UPDATE=0,	
	TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_DONE,
	MAX_TASK_CMDATTR_MULTICORE_VIEWGEN,   
}eTASK_CMDATTR;



#else//CALIB_LIB_USE



#include "type.h"
#include "error.h"
PP_VOID vTaskCalibration(PP_VOID *pvData);

#endif ////CALIB_LIB_USE

#endif // __TASK_CALIBRATION_H__


#ifndef PRJ_CONFIG_SUB_H
#define PRJ_CONFIG_SUB_H

////////////////////////////////////////////////////////////////////////////////
// DEBUG. Print log <= LOG_BUILD_LEVEL. LOG_LVL_NONE is no print any log.
#ifndef LOG_BUILD_LEVEL
#ifdef NDEBUG
#define LOG_BUILD_LEVEL			LOG_LVL_CRITICAL 
#else
#define LOG_BUILD_LEVEL			LOG_LVL_DEBUG //LOG_LVL_NONE: nothing message
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
// Common
#define CACHE_VIEW_USE


////////////////////////////////////////////////////////////////////////////////
// CLI
#define SUPPORT_DEBUG_CLI


////////////////////////////////////////////////////////////////////////////////
// VIN


////////////////////////////////////////////////////////////////////////////////
// VOUT


////////////////////////////////////////////////////////////////////////////////
// SVM


////////////////////////////////////////////////////////////////////////////////
// DU
#define USE_PP_GUI

#if defined(USE_PP_GUI)
#define USE_BOOTING_IMG
#define USE_GUI_MENU
//#define USE_16BIT_CAR

#define USE_CAR_WHEEL
#define USE_CAR_DOOR

//#define USE_SEPERATE_SHADOW
//#define USE_SEPERATE_PGL
#else
#undef CACHE_VIEW_USE
#endif


////////////////////////////////////////////////////////////////////////////////
// VPU


////////////////////////////////////////////////////////////////////////////////
// Calibration
#define CALIB_LIB_USE

////////////////////////////////////////////////////////////////////////////////
// SUPPORT SLT
#undef BD_SLT

#endif /* PRJ_CONFIG_SUB_H */

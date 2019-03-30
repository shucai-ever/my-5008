/*
 * =====================================================================================
 *
 *       Filename:  def_enum_name.h
 *
 * =====================================================================================
 */
#ifndef __DEF_ENUM_NAME_H__
#define  __DEF_ENUM_NAME_H__

#include "prj_config_def_enum.h"

extern const char *pNameVideoInType[VIDEO_IN_TYPE_PARALLEL+1];
extern const char *pNameMipiVidBit[(MIPI_VID_BIT_2XRAW10>>MIPI_VID_BIT_BITSHIFT)+1];
extern const char *pNameVidType[(VID_TYPE_BAYER_10BIT>>VID_TYPE_BITSHIFT)+1];
extern const char *pNameVidFrame[(VID_FRAME_PAL_50>>VID_FRAME_BITSHIFT)+1];
extern const char *pNameVidResol[(VID_RESOL_HD1024_600P>>VID_RESOL_BITSHIFT)+1];
extern const char *pNameVidStandard[(VID_STANDARD_HDA>>VID_STANDARD_BITSHIFT)+1];
extern const char *pNamePviTxSrc[(PVITX_SRC_QUAD>>PVITX_SRC_BITSHIFT)+1];
extern const char *pNameChipOption[4];
extern const char *pNameBootMode[7];

#endif // __DEF_ENUM_NAME_H__

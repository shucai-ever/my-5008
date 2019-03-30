/*
 * def_enum_name.c
 */

#include <stdio.h>

#include "type.h"
#include "def_enum_name.h"

const char *pNameVideoInType[VIDEO_IN_TYPE_PARALLEL+1] = 
{
    "VIDEO_IN_TYPE_MIPI_BAYER",
    "VIDEO_IN_TYPE_MIPI_YUV",
    "VIDEO_IN_TYPE_PVI",
    "VIDEO_IN_TYPE_PARALLEL",
};

const char *pNameMipiVidBit[(MIPI_VID_BIT_2XRAW10>>MIPI_VID_BIT_BITSHIFT)+1] =
{
    "MIPI_VID_BIT_RAW6",
    "MIPI_VID_BIT_RAW7",
    "MIPI_VID_BIT_RAW8",
    "MIPI_VID_BIT_RAW10",
    "MIPI_VID_BIT_RAW12",
    "MIPI_VID_BIT_RAW14",
    "MIPI_VID_BIT_YUV8_2XRAW8",
    "MIPI_VID_BIT_2XRAW10",
};

const char *pNameVidType[(VID_TYPE_BAYER_10BIT>>VID_TYPE_BITSHIFT)+1] =
{
    "VID_TYPE_YC8_EMB",
    "VID_TYPE_YC8_EXT",
    "VID_TYPE_YC16_EMB",
    "VID_TYPE_YC16_EXT",
    "VID_TYPE_RGB24",
    "VID_TYPE_BAYER_8BIT",
    "VID_TYPE_BAYER_10BIT",
};

const char *pNameVidFrame[(VID_FRAME_PAL_50>>VID_FRAME_BITSHIFT)+1] =
{
    "VID_FRAME_NTSC_30",
    "VID_FRAME_PAL_25",
    "VID_FRAME_NTSC_60",
    "VID_FRAME_PAL_50",
};

const char *pNameVidResol[(VID_RESOL_HD1024_600P>>VID_RESOL_BITSHIFT)+1] = 
{
    "VID_RESOL_SD720H",
    "VID_RESOL_SD960H",
    "VID_RESOL_SDH720",
    "VID_RESOL_SDH960",
    "VID_RESOL_HD720P",
    "VID_RESOL_HD960P",
    "VID_RESOL_HD1080P",
    "VID_RESOL_HD800_480P",
    "VID_RESOL_HD1024_600P",
};

const char *pNameVidStandard[(VID_STANDARD_HDA>>VID_STANDARD_BITSHIFT)+1] =
{
    "VID_STANDARD_CVBS",
    "VID_STANDARD_SDH",
    "VID_STANDARD_PVI",
    "VID_STANDARD_CVI",
    "VID_STANDARD_HDT",
    "VID_STANDARD_HDA",
};

const char *pNamePviTxSrc[(PVITX_SRC_QUAD>>PVITX_SRC_BITSHIFT)+1] = 
{
    "PVITX_SRC_NONE",
    "PVITX_SRC_DU",
    "PVITX_SRC_QUAD",
};

const char *pNameChipOption[4] = 
{
    "HD_NoISP",
    "HD_ISP",
    "FHD_NoISP",
    "FHD_ISP",
};

const char *pNameBootMode[7] = 
{
    "SPI_NOR",
    "SDCARD",
    "EXT SPI",
    "EXT UART",
    "SPI_NAND",
    "Reserved",
    "Reserved",
};


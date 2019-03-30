#ifndef __PVIRX_SUPPORT_H__
#define __PVIRX_SUPPORT_H__

/* fpga test function. */
//#define SUPPORT_PVIRX_FPGA

/* Support camera standard format. */
#define SUPPORT_PVIRX_STD_PVI
#define SUPPORT_PVIRX_STD_HDA
#define SUPPORT_PVIRX_STD_CVI
#define SUPPORT_PVIRX_STD_HDT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define PVIRX support //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* enable autodetect. If undef, forcely set user defined standard & resolution. */
#undef SUPPORT_AUTODETECT_PVI

/* In driver, auto set register as detected standard & resolution. */
#define SUPPORT_PVIRX_AUTOSET_VIDEO_INDRIVER

/* When fixed camera & cable type, support recall previous camera setting. */
/* This case you see camera image directly. If different camera, start autodetect preocessing. */
#ifdef SUPPORT_PVIRX_AUTOSET_VIDEO_INDRIVER
#define SUPPORT_PVIRX_RECALL_PREV_CAM
#endif // SUPPORT_PVIRX_AUTOSET_VIDEO_INDRIVER

/* select SD video resolution(720H or 960H) function. If "define", select SD 960H resolution. */
#undef SUPPORT_PVIRX_SD_VIDEO_960H_RESOLUTION

/* define default cable EQ gain distance. (0:default, 100, 200, .. , 1000) */
#define DEFAULT_CEQ_GAIN_DIST    (0)

/* video enhancement function. */
#define SUPPORT_PVIRX_VID

/* utc function. */
#define SUPPORT_PVIRX_UTC

#ifdef SUPPORT_AUTODETECT_PVI
/* cable EQ function on plugin. */
#undef SUPPORT_PVIRX_CEQ_PLUGIN
/* cable EQ function on monitoring. */
#undef SUPPORT_PVIRX_CEQ_MONITOR
#endif //SUPPORT_AUTODETECT_PVI

/* dbg function. */
#undef SUPPORT_PVIRX_DBG_PRINT

/* Support print help string. */
#define SUPPORT_PVIRX_HELP_STRING


#endif // __PVIRX_SUPPORT_H__

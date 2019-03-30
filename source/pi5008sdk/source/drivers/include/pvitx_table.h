#ifndef __PVITX_TABLE_H__
#define __PVITX_TABLE_H__

#include "pvitx_support.h"


#define PVI_TX_REG_ADDR_PTZ_RX_EN		(0x80)	
#define PVI_TX_REG_ADDR_PTZ_TX_EN		(0xB0)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_WR_INIT	(0xA0)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_WR_DATA	(0xA1)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_WR_SIZE	(0xA2)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_WR_ADDR	(0xA3)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_RD_INIT	(0xA4)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_RD_DATA	(0xA6)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_RD_SIZE	(0xA5)	
#define PVI_TX_REG_ADDR_PTZ_FIFO_RD_ADDR	(0xA7)	

#define PVI_TX_REG_ADDR_PTZ_TX_LINE_CNT		(0xB1)	
#define PVI_TX_REG_ADDR_PTZ_TX_LINE_BIT_LENGTH	(0xBB)	
#define PVI_TX_REG_ADDR_PTZ_TX_ALL_DATA_LENGTH	(0xBC)	
#define PVI_TX_REG_ADDR_PTZ_TX_GROUP_CNT	(0xBE)	
#define PVI_TX_REG_ADDR_PTZ_RX_LINE_CNT		(0x81)	
#define PVI_TX_REG_ADDR_PTZ_RX_LINE_BIT_LENGTH	(0x8B)	
#define PVI_TX_REG_ADDR_PTZ_RX_ALL_DATA_LENGTH	(0x8C)	

#define PVI_TX_REG_ADDR_FSC_OPT_CTRL		(0x58)	

#define PVI_TX_REG_ADDR_PTZ_PTZ_CMD_MD		(0x95)	
#define PVI_TX_REG_ADDR_PTZ_PTZ_RZ_EN		(0x96)	
#define PVI_TX_REG_ADDR_IRQEN			(0xF0)	
#define PVI_TX_REG_ADDR_IRQEN_OP		(0xF3)	
#define PVI_TX_REG_ADDR_IRQCLR			(0xF5)	
#define PVI_TX_REG_ADDR_IRQOUT			(0xF7)	

enum _pvi_tx_table_type_format {
	pvi_tx_table_format_SD720 = 0,
	pvi_tx_table_format_SD960,
	pvi_tx_table_format_PVI,
	pvi_tx_table_format_HDA,
	pvi_tx_table_format_CVI,
	pvi_tx_table_format_HDT,
	max_pvi_tx_table_type_format
};

enum _pvi_tx_table_resol_format {
	pvi_tx_table_format_720x480i60 = 0,
	pvi_tx_table_format_720x576i50,
	pvi_tx_table_format_960x480i60,
	pvi_tx_table_format_960x576i50,
	pvi_tx_table_format_1280x720p60,
	pvi_tx_table_format_1280x720p50,
	pvi_tx_table_format_1280x720p30,
	pvi_tx_table_format_1280x720p25,
	pvi_tx_table_format_1280x960p30,
	pvi_tx_table_format_1280x960p25,
	pvi_tx_table_format_1920x1080p30,
	pvi_tx_table_format_1920x1080p25,
	max_pvi_tx_table_resol_format
};

enum _pvi_tx_table_sd_pal_type {
	SDPAL_BDG = 0,
	SDPAL_M,
	SDPAL_N,
};


#define MAX_PVI_TX_SD_RESOL 	(4)
#define MAX_PVI_TX_SDH_RESOL 	(4)
#define MAX_PVI_TX_HD_RESOL 	(8)

typedef struct
{
	unsigned char addr;
	unsigned char data[MAX_PVI_TX_SD_RESOL];
}_SD_PVI_TX_TABLE_T;
typedef struct
{
	unsigned char addr;
	unsigned char data[MAX_PVI_TX_HD_RESOL];
}_HD_PVI_TX_TABLE_T;

extern const char _STR_PVI_TX_TYPE_FORMAT[max_pvi_tx_table_type_format][8];
extern const char _STR_PVI_TX_RESOL_FORMAT[max_pvi_tx_table_resol_format][16];
extern const int _PVI_TX_1FRAME_MSEC[max_pvi_tx_table_resol_format];

#if defined(SUPPORT_PVITX_720H) || defined(SUPPORT_PVITX_960H)
extern const _SD_PVI_TX_TABLE_T pvi_tx_table_sd[];
#endif /* defined(SUPPORT_PVITX_720H) || defined(SUPPORT_PVITX_960H) */

#ifdef SUPPORT_PVITX_PVI
extern const _HD_PVI_TX_TABLE_T pvi_tx_table_pvi[];
#endif /* SUPPORT_PVITX_PVI */

#ifdef SUPPORT_PVITX_HDA
extern const _HD_PVI_TX_TABLE_T pvi_tx_table_hda[];
#endif /* SUPPORT_PVITX_HDA */

#ifdef SUPPORT_PVITX_CVI
extern const _HD_PVI_TX_TABLE_T pvi_tx_table_cvi[];
#endif /* SUPPORT_PVITX_CVI */

#ifdef SUPPORT_PVITX_HDT
extern const _HD_PVI_TX_TABLE_T pvi_tx_table_hdt[];
#endif /* SUPPORT_PVITX_HDT*/

#endif // __PVITX_TABLE_H__

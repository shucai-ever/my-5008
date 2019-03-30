#ifndef __PVIRX_TABLE_H__
#define __PVIRX_TABLE_H__

#include "pvirx_support.h"
#include "type.h"

/* PI5008 pvirx address offset */
#define PVIRX_CHN_OFFSET			(0x1000)
#define PVIRX_CHN_ADDR(chn, addr)	((chn*PVIRX_CHN_OFFSET)+(addr<<2))

enum _eCameraStandard {
        CVBS = 0,
        PVI,
        CVI,
        HDA,
        HDT_OLD,
        HDT_NEW,
        max_camera_standard
};

enum _eCameraResolution {
        camera_ntsc = 0,
        camera_pal,

        camera_1280x720p60,
        camera_1280x720p50,
        camera_1280x720p30,
        camera_1280x720p25,
        camera_1920x1080p30,
        camera_1920x1080p25,

        camera_1280x960p30,
        camera_1280x960p25,

        max_camera_resolution
};

enum _eVideoResolution {
        video_720x480i60 = 0,
        video_720x576i50,
        video_960x480i60,
        video_960x576i50,

        video_1280x720p60,
        video_1280x720p50,
        video_1280x720p30,
        video_1280x720p25,
        video_1920x1080p30,
        video_1920x1080p25,

        video_1280x960p30,
        video_1280x960p25,

        max_video_resolution
};

enum _eDetIfmtStd{
        DET_PVI = 0,
        DET_CVI,
        DET_HDA,
        DET_HDT,
        max_DetIfmtStd
};
enum _eDetIfmtRef{
        DET_25Hz = 0,
        DET_30Hz,
        DET_50Hz,
        DET_60Hz,
        max_DetIfmtRef
};
enum _eDetIfmtRes{
        DET_SD480i = 0,
        DET_SD576i,
        DET_HD720p,
        DET_HD1080p,
        DET_HD960p,
        max_DetIfmtRes
};

enum _eBitIrq{
        IRQ_NOVID = 0,
        IRQ_VFD,
        IRQ_SET_RXMODE,
        IRQ_TIMER0UP,
        IRQ_TIMER1UP,
        IRQ_UTC,
        IRQ_UTC_TX,
        IRQ_UTC_RX,
        max_BitIrq
};

//////////////////////////////////////////////////////////
typedef union 
{
        uint16_t reg;
        struct
        {    
                uint16_t addr:16;
        }b;    
}_stPVIRX_Reg;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data; 
}_stPVIRX_Table_Common;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[video_960x576i50+1]; 
}_stPVIRX_Table_SDResolution;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[video_1920x1080p25-video_1280x720p60+1]; 
}_stPVIRX_Table_STD_HDResolution;

typedef struct
{
	uint16_t distance;
	uint16_t pData[2][3]; //rangeMin, rangeMax, factor
}_stPVIRX_Table_CEQ_SDResolution;
typedef struct
{
	uint16_t distance;
	uint16_t pData[video_1920x1080p25-video_1280x720p60+1][3]; //rangeMin, rangeMax, factor
}_stPVIRX_Table_CEQ_STD_HDResolution;

typedef struct
{
	uint16_t distance;
	uint16_t pData[camera_1920x1080p25+1][2]; //[vadc(H|L)],[gfit|ygan]
}_stPVIRX_Table_CEQ_VADC;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[video_1280x960p25-video_1280x960p30+1]; 
}_stPVIRX_Table_EXT_HDResolution;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[video_1920x1080p25+1]; 
}_stPVIRX_Table_ETC_STDResolution;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[video_1280x960p25-video_1280x960p30+1]; 
}_stPVIRX_Table_ETC_EXTResolution;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[camera_pal+1]; 
}_stPVIRX_Table_SD_CameraResolution;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[camera_1920x1080p25-camera_1280x720p60+1]; 
}_stPVIRX_Table_STD_HD_CameraResolution;

typedef struct
{
	_stPVIRX_Reg stReg;
	uint8_t u8Data[camera_1280x960p25-camera_1280x960p30+1]; 
}_stPVIRX_Table_EXT_HD_CameraResolution;

typedef struct
{
	enum _eCameraResolution eCameraResolution;
	enum _eVideoResolution eVideoResolution;
}_stCameraVideoRelation;

////////////// PVIRX REG ADDR //////////////////////
#define PVIRX_ADDR_VID_STATUS		(0x0000)
#define PVIRX_ADDR_CHIPID_MSB		(0x00FC)
#define PVIRX_ADDR_CHIPID_LSB		(0x00FD)
#define PVIRX_ADDR_REVID		(0x00FE)

/* read vid(std/resol) status registers */
typedef union 
{
        uint8_t reg[3];
        struct
        {    
                uint8_t det_ifmt_res:3;
                uint8_t det_video:1;
                uint8_t det_ifmt_ref:2;
                uint8_t det_ifmt_std:2;

                uint8_t det_chroma:1;
                uint8_t lock_chroma:1;
                uint8_t lock_c_fine:1;
                uint8_t lock_hpll:1;
                uint8_t lock_hperiod:1;
                uint8_t lock_clamp:1;
                uint8_t lock_gain:1;
                uint8_t lock_std:1;

                uint8_t reserved0:2;
                uint8_t det_std_hda:1;
                uint8_t det_std_hdt_h0:1;
                uint8_t det_std_hdt_h1:1;
                uint8_t det_std_hdt_v:1;
                uint8_t reserved1:2;
        }b;    
}_stPVIRX_VidStatusReg;

typedef union 
{
        uint8_t reg[1];
        struct
        {    
                uint8_t man_ifmt_res:3;
                uint8_t man_video:1;
                uint8_t man_ifmt_ref:2;
                uint8_t man_ifmt_std:2;
        }b;    
}_stPVIRX_ManIfmtReg;

#if defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 

typedef struct 
{
        int stepDir; //> 0:++, <0:--
}_stPVIRX_TunnFactor;
#endif //defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 
typedef struct 
{
        uint8_t syncLevel;
        uint16_t dcGain;
        uint16_t acGain;
        uint16_t comp1;
        uint16_t comp2;
        uint16_t atten1;
        uint16_t atten2;
}_stPVIRX_DetGainStatus;

typedef struct 
{
        uint8_t vadcGain;

	uint8_t vadcEqBandComp;
	uint16_t distComp;
	uint8_t eqStepCompNum;
	uint8_t vadcEqBandAtten;
	uint16_t distAtten;
	uint8_t eqStepAttenNum;

	uint8_t bTypeComp; //result eq from Comp or Atten value
	uint8_t vadcEqBand; //result confirm value
	uint8_t manEqAcGainMd; //result confirm value
	uint16_t dist; //result confirm value
	uint8_t eqStepNum; //result confirm value
}_stPVIRX_MeasureEqInfo;

/* irq clr register 0x94/0x95/0x96 */
typedef union 
{
        uint8_t reg[2];
        struct
        {    
                uint8_t ptz0:1;
                uint8_t ptz1:1;
                uint8_t ptz2:1;
                uint8_t ptz3:1;
                uint8_t ptz4:1;
                uint8_t ptz5:1;
                uint8_t ptz6:1;
                uint8_t ptz7:1;
                uint8_t reserved0:4;
                uint8_t novid:1;
                uint8_t vfd:1;
                uint8_t timer0:1;
                uint8_t timer1:1;
        }b;    
}_stPVIRX_Irq;

typedef union
{       
        uint8_t reg[14];
        struct  
        {       
                uint8_t fieldType:2;
                uint8_t fieldPol:1;
                uint8_t ignoreFrmEn:1;
                uint8_t ignoreLineEn:2;
                uint8_t start:1;
                uint8_t pathEn:1;       //reg0
                uint8_t hstOs:3;
                uint8_t lineCnt:5; //reg1
                uint8_t hst:7;
                uint8_t dataPol:1; //reg2
                uint8_t freqFirst23:8; //reg3
                uint8_t freqFirst15:8; //reg4
                uint8_t freqFirst07:8; //reg5
                uint8_t freq23:8; //reg6
                uint8_t freq15:8; //reg7
                uint8_t freq07:8; //reg8
                uint8_t lpfLen:6;
                uint8_t reserved0:2; //reg9
                uint8_t pixOffset:8; //reg10
                uint8_t lineLen:6;
                uint8_t reserved1:2; //reg11
                uint8_t validCnt:8; //reg12
                uint8_t tpSel:3;
                uint8_t reserved2:3;
                uint8_t addrHoldEn:1;
                uint8_t testEn:1; //reg13
        }b;
}_stUTCRxAttr;

typedef union
{
        uint8_t reg[8];
        struct
        {
                uint8_t rxHstrtOs13:6;
                uint8_t reserved0:1;
                uint8_t rxHsyncPol:1; //reg0
                uint8_t rxHstrtOs07:8; //reg1
                uint8_t rxVstrtOs10:3;
                uint8_t reserved1:4;
                uint8_t rxVsyncPol:1; //reg2
                uint8_t rxVstrtOs07:8; //reg3
                uint8_t txHstrtOs13:6;
                uint8_t reserved2:1;
                uint8_t txHsyncPol:1; //reg4
                uint8_t txHstrtOs07:8; //reg5
                uint8_t txVstrtOs10:3;
                uint8_t reserved3:4;
                uint8_t txVsyncPol:1; //reg6
                uint8_t txVstrtOs07:8; //reg7
        }b;
}_stUTCHVStartAttr;

typedef union
{
        uint8_t reg[15];
        struct
        {
                uint8_t fieldType:2;
                uint8_t fieldPol:1;
                uint8_t reserved0:3;
                uint8_t start:1;
                uint8_t pathEn:1;       //reg0
                uint8_t hstOs:3;
                uint8_t lineCnt:5; //reg1
                uint8_t hst:7;
                uint8_t dataPol:1; //reg2
                uint8_t freqFirst23:8; //reg3
                uint8_t freqFirst15:8; //reg4
                uint8_t freqFirst07:8; //reg5
                uint8_t freq23:8; //reg6
                uint8_t freq15:8; //reg7
                uint8_t freq07:8; //reg8
                uint8_t hpst12:5;
                uint8_t reserved1:3; //reg9
                uint8_t hpst07:8; //reg10
                uint8_t lineLen:6;
                uint8_t reserved2:2; //reg11
                uint8_t allDataLen:8; //reg12
                uint8_t lastRptNum:7;
                uint8_t rptEn:1; //reg13
                uint8_t cmdGrpNum:4;
                uint8_t tpSel:3;
                uint8_t grpEn:1; //reg14
        }b;
}_stUTCTxAttr;

enum _eBitJobProcess{
        JOB_START_CAMERA_IN,
        JOB_DONE_CAMERA_IN,
        JOB_START_VFD_CAMERA,
        JOB_DONE_VFD_CAMERA,
        JOB_START_CAMERA_MONITOR,
        JOB_DOING_CAMERA_MONITOR,
        JOB_PAUSE_CAMERA_MONITOR,

	JOB_GET_CAMERA_STDRESOL,
        max_BitJobProcess
};

enum _eBitJobProcessTimer0{
        JOB_CHECK_VFD_MANUAL,
        max_BitJobProcessTimer0
};

enum _eBitJobProcessTimer1{
        JOB_TUNNING_CHROMALOCK,
        max_BitJobProcessTimer1
};

typedef struct
{
	unsigned long bitJobProcess;  //processing job bit --> enum _eBitJobProcess

	uint8_t camPlugInCheckCnt;
	uint8_t cntWait300MsecTime; //300msec unit.

	uint8_t numJobStep;
	uint8_t cntJobTry;
	int reJudgeStdResol;

	uint32_t defChromaPhase;
	uint8_t cntChromaLockTunn;

	uint8_t C_LOCK_CNT;
	uint8_t AC_GAIN_ADJ;
	uint8_t AC_GAIN_HOLD;
	uint8_t EQ_CNT;

	_stPVIRX_VidStatusReg stStartVidStatusReg;
	_stPVIRX_VidStatusReg stVidStatusReg;
#if defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 
	_stPVIRX_TunnFactor stTunnFactor;
#endif //defined(SUPPORT_PVIRX_CEQ_PLUGIN) || defined(SUPPORT_PVIRX_CEQ_MONITOR) 
	_stPVIRX_DetGainStatus stFirstDetGainStatus;
	_stPVIRX_DetGainStatus stDetGainStatus;
	_stPVIRX_MeasureEqInfo stMeasureEqInfo;

	unsigned long bitJobProcessTimer0;  //timer0 job bit --> enum _eBitJobProcessTimer0 , fixed 4000msec
	unsigned long bitJobProcessTimer1;  //timer1 job bit --> enum _eBitJobProcessTimer1 , fixed 300msec
}_stJobProc;

typedef struct
{
	enum _eCameraStandard standard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution vidOutResolution;
}_stPrRxMode;

typedef struct
{
	unsigned long bitIrq; 
	unsigned long bitIrqStatus; 
}_stUserPoll;

typedef struct
{
	_stPVIRX_Irq stIrqClr;
	_stPVIRX_Irq stIrqStatus;
	_stJobProc stJobProc;

	_stUserPoll stUserPoll;
}_stPVIRX_Isr;

typedef struct
{
    PP_S32 cid;
    PP_U8 chanAddr;
	void *pHost;
}_stPlugInParam;

typedef struct
{
    PP_S32 cid;
    PP_U8 chanAddr;
    enum _eBitIrq eBitIrq;
	void *pHost;
}_stMonitorParam;

////////////////////////////////////////////////////////
extern const _stPVIRX_Table_Common stPVIRX_Table_IRQ[];
extern const _stPVIRX_Table_SDResolution stPVIRX_Table_SDResol[];
extern const _stPVIRX_Table_STD_HDResolution stPVIRX_Table_STD_PVIResol[];
extern const _stPVIRX_Table_EXT_HDResolution stPVIRX_Table_EXT_PVIResol[];
extern const _stPVIRX_Table_STD_HDResolution stPVIRX_Table_STD_HDAResol[];
extern const _stPVIRX_Table_EXT_HDResolution stPVIRX_Table_EXT_HDAResol[];
extern const _stPVIRX_Table_STD_HDResolution stPVIRX_Table_STD_CVIResol[];
extern const _stPVIRX_Table_EXT_HDResolution stPVIRX_Table_EXT_CVIResol[];
extern const _stPVIRX_Table_STD_HDResolution stPVIRX_Table_STD_HDT_OLDResol[];
extern const _stPVIRX_Table_EXT_HDResolution stPVIRX_Table_EXT_HDT_OLDResol[];
extern const _stPVIRX_Table_STD_HDResolution stPVIRX_Table_STD_HDT_NEWResol[];
extern const _stPVIRX_Table_EXT_HDResolution stPVIRX_Table_EXT_HDT_NEWResol[];

extern const _stPVIRX_Table_ETC_STDResolution stPVIRX_Table_ETC_STDResol[];
extern const _stPVIRX_Table_ETC_EXTResolution stPVIRX_Table_ETC_EXTResol[];
extern const _stPVIRX_Table_ETC_EXTResolution stPVIRX_Table_ETC_EXTResol_SyncParallel[];

#ifdef SUPPORT_PVIRX_UTC
extern const _stPVIRX_Table_SD_CameraResolution stPVIRX_Table_UTC_SD[];
extern const _stPVIRX_Table_STD_HD_CameraResolution stPVIRX_Table_STD_UTC_PVI[];
extern const _stPVIRX_Table_EXT_HD_CameraResolution stPVIRX_Table_EXT_UTC_PVI[];
extern const _stPVIRX_Table_STD_HD_CameraResolution stPVIRX_Table_STD_UTC_HDA[];
extern const _stPVIRX_Table_EXT_HD_CameraResolution stPVIRX_Table_EXT_UTC_HDA[];
extern const _stPVIRX_Table_STD_HD_CameraResolution stPVIRX_Table_STD_UTC_CVI[];
extern const _stPVIRX_Table_EXT_HD_CameraResolution stPVIRX_Table_EXT_UTC_CVI[];
extern const _stPVIRX_Table_STD_HD_CameraResolution stPVIRX_Table_STD_UTC_HDT[];
extern const _stPVIRX_Table_EXT_HD_CameraResolution stPVIRX_Table_EXT_UTC_HDT[];

#define PVIRX_UTC_SD_TXCMD_BASE_BYTE_CNT               (4)
extern const uint8_t pvirx_ptz_table_sd_tx_pat_format[((6)*2)];
extern const uint8_t pvirx_ptz_table_sd_tx_pat_data[((6)*2)];
extern const uint8_t pvirx_ptz_table_sd_rx_pat_format[((6)*2)];
extern const uint8_t pvirx_ptz_table_sd_rx_pat_start_format[(6)];
extern const uint8_t pvirx_ptz_table_sd_rx_pat_start_data[(6)];
#define PVIRX_UTC_PVI_TXCMD_BASE_BYTE_CNT              (7)
extern const uint8_t pvirx_ptz_table_std_pvi_tx_pat_format[((3*4)*2)];
extern const uint8_t pvirx_ptz_table_std_pvi_tx_pat_data[((3*4)*2)];
extern const uint8_t pvirx_ptz_table_std_pvi_rx_pat_format[((3*4)*2)];
extern const uint8_t pvirx_ptz_table_std_pvi_rx_pat_start_format[(3*1)];
extern const uint8_t pvirx_ptz_table_std_pvi_rx_pat_start_data[(3*1)];
#define PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_1080p                (24)
#define PVIRX_UTC_HDA_TXCMD_BASE_BYTE_CNT_720p        		(4)
extern const uint8_t pvirx_ptz_table_std_hda_tx_pat_format_1080p[((3*4)*6)];
extern const uint8_t pvirx_ptz_table_std_hda_tx_pat_format_720p[((3*2)*2)];
extern const uint8_t pvirx_ptz_table_std_hda_tx_pat_data_1080p[((3*4)*6)];
extern const uint8_t pvirx_ptz_table_std_hda_tx_pat_data_720p[((3*2)*2)];
extern const uint8_t pvirx_ptz_table_std_hda_rx_pat_format_1080p[((3*4)*6)];
extern const uint8_t pvirx_ptz_table_std_hda_rx_pat_format_720p[((3*2)*2)];
extern const uint8_t pvirx_ptz_table_std_hda_rx_pat_start_format_1080p[(3*1)];
extern const uint8_t pvirx_ptz_table_std_hda_rx_pat_start_format_720p[(3*2)];
extern const uint8_t pvirx_ptz_table_std_hda_rx_pat_start_data_1080p[(3*1)];
extern const uint8_t pvirx_ptz_table_std_hda_rx_pat_start_data_720p[(3*2)];
#define PVIRX_UTC_CVI_TXCMD_BASE_BYTE_CNT              (7)
extern const uint8_t pvirx_ptz_table_std_cvi_tx_pat_format[((4*6)*3)];
extern const uint8_t pvirx_ptz_table_std_cvi_tx_pat_data[((4*6)*3)];
extern const uint8_t pvirx_ptz_table_std_cvi_rx_pat_format[((4*6)*3)];
extern const uint8_t pvirx_ptz_table_std_cvi_rx_pat_start_format[(4*1)];
extern const uint8_t pvirx_ptz_table_std_cvi_rx_pat_start_data[(4*1)];
#define PVIRX_UTC_HDT_TXCMD_BASE_BYTE_CNT              (8)
extern const uint8_t pvirx_ptz_table_std_hdt_tx_pat_format[((5*1)*2)];
extern const uint8_t pvirx_ptz_table_std_hdt_tx_pat_data[((5*1)*2)];
extern const uint8_t pvirx_ptz_table_std_hdt_rx_pat_format[((5*1)*2)];
extern const uint8_t pvirx_ptz_table_std_hdt_rx_pat_start_format[(5*1)];
extern const uint8_t pvirx_ptz_table_std_hdt_rx_pat_start_data[(5*1)];
#endif //SUPPORT_PVIRX_UTC

extern const _stCameraVideoRelation stDefaultCameraVideoRelation[max_camera_resolution];

#ifdef SUPPORT_PVIRX_HELP_STRING
extern const char _strCameraStandard[max_camera_standard][8];
extern const char _strCameraResolution[max_camera_resolution][20];
extern const char _strVideoResolution[max_video_resolution][20];
#else
extern const char _strCameraStandard[max_camera_standard][1];
extern const char _strCameraResolution[max_camera_resolution][1];
extern const char _strVideoResolution[max_video_resolution][1];
#endif // SUPPORT_PVIRX_HELP_STRING

#endif // __PVIRX_TABLE_H__

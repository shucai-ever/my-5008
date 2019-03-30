/**
 *@file			task_manager.h
 *@author		dcson
 *@section		DESCRIPTION
 *				task_manager.c header
 */


#ifndef APP_TASK_H
#define APP_TASK_H

/*---------------------------------------------------------------------------*/
/* Includes */
/*---------------------------------------------------------------------------*/
#include "prj_config.h"
#include "osal.h"
#include "ring_buffer.h"


/*---------------------------------------------------------------------------*/
/* Defines */
/*---------------------------------------------------------------------------*/
typedef enum {
	TASK_FWDN = 0,
	TASK_EMERGENCY,
	TASK_MONITOR,
	TASK_AUDIO,
	TASK_UARTCON,
	TASK_DISPLAY,
	TASK_UI,
	TASK_CALIBRATION,
#ifdef DB_LIB_USE
	TASK_DYNBLEND,
#endif // DB_LIB_USE
	TASK_FS,
#ifdef CACHE_VIEW_USE
	TASK_CACHE,
#endif
#ifdef SUPPORT_DEBUG_CLI
	TASK_CLI, 
#endif // SUPPORT_DEBUG_CLI
	MAX_TASK
}eTASK_INDEX;

extern const char *psTaskName[MAX_TASK];

typedef struct stHandleCfg
{
    TaskHandle_t phTask;
    QueueHandle_t phQueue;
    EventGroupHandle_t phEventGroup;
} HandleCfg_t;

typedef struct stQueueCfg
{
    uint8 maxNumItem;
    char* name;
    uint32 sizeItem;
    void *pvParameters;
} QueueCfg_t;

typedef struct stEventGroupCfg
{
    uint32 maxNumBit;
    char* name;
    void *pvParameters;
} EventGroupCfg_t;

#define TASK(name)      extern void (name) (void* arg)
typedef void (*task_t)(void* arg);
typedef struct stTaskCfg
{
    eTASK_INDEX index;
    uint8 priority;
    char* taskName;
    task_t taskFn;
    uint32 stackSize;
    void *pvParameters;
    const QueueCfg_t *pQueueCfg;
    const EventGroupCfg_t *pEventGroupCfg;
} TaskCfg_t;

typedef struct {
	uint32 u32Cmd;	//32bit command ID.
	uint16 u16Sender;	//Sender Queue ID. one of eQUEUE_CMDOWNER.
	uint16 u16Attr; //eQUEUE_CMDATTR bit combination.
	uint32 u32Length; //length of pData. If 0 is NULL.
	void *pData;
}stQueueItem;

typedef enum {
	QUEUE_CMDATTR_BIT_REQACK = 0,
	QUEUE_CMDATTR_BIT_RESERVED1,
	QUEUE_CMDATTR_BIT_RESERVED2,
	QUEUE_CMDATTR_BIT_RESERVED3,
	QUEUE_CMDATTR_BIT_RESERVED4,
	QUEUE_CMDATTR_BIT_RESERVED5,
	QUEUE_CMDATTR_BIT_RESERVED6,
	QUEUE_CMDATTR_BIT_RESERVED7,
	QUEUE_CMDATTR_BIT_RESERVED8,
	QUEUE_CMDATTR_BIT_RESERVED9,
	QUEUE_CMDATTR_BIT_RESERVED10,
	QUEUE_CMDATTR_BIT_RESERVED11,
	QUEUE_CMDATTR_BIT_RESERVED12,
	QUEUE_CMDATTR_BIT_RESERVED13,
	QUEUE_CMDATTR_BIT_RESERVED14,
	QUEUE_CMDATTR_BIT_RESERVED15,
	MAX_QUEUE_CMDATTR   // < bit16
}eQUEUE_CMDATTR;

typedef struct stTaskParam
{
	void *pvParameters;
	PP_U32 u32TaskLoopCnt;
} TaskParam_t;

///////////////////////////// task cmd define /////////////////////////////////
typedef enum {
	CMD_ACK = 0,
	CMD_NACK,
	MAX_CMD_COMMON
}eCMD_SET_COMMON;

typedef enum {
	CMD_FWDN_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_FWDN_FLASH_UPDATE,
	MAX_CMD_FWDN
}eCMD_SET_FWDN;

typedef enum {
	CMD_EMERGENCY_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_EMERGENCY_CAMERA_PLUG,
	CMD_EMERGENCY_DISPLAY_FREEZE,
	CMD_EMERGENCY_INVALID_CAMERA,
	MAX_CMD_EMERGENCY
}eCMD_SET_EMERGENCY;

typedef enum {
	CMD_MONITOR_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_MONITOR_NUM1,	
	CMD_GET_REMOCON,
	CMD_GET_WELTREND,
	CMD_DBG_PRINT_MSG_ON,
	CMD_DBG_PRINT_MSG_OFF,
	CMD_MONITOR_CAMERA_PLUG,
	CMD_MONITOR_STANDBY,
	MAX_CMD_MONITOR
}eCMD_SET_MONITOR;

typedef enum {
	CMD_AUDIO_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_AUDIO_NUM1,
	MAX_CMD_AUDIO
}eCMD_SET_AUDIO;

typedef enum {
	CMD_UARTCON_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_UARTCON_DEBUG_PRINT,
	CMD_UARTCON_NUM1,
	MAX_CMD_UARTCON
}eCMD_SET_UARTCON;

typedef enum {
	CMD_DISPLAY_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_DISPLAY_POPUP_TIMER,
	CMD_DISPLAY_POPUP_ON,
	CMD_DISPLAY_POPUP_OFF,
	CMD_DISPLAY_PROGRESSBAR_TIMER_ON,
	CMD_DISPLAY_PROGRESSBAR_TIMER_OFF,
	CMD_DISPLAY_PROGRESSBAR_ON,
	CMD_DISPLAY_PROGRESSBAR_OFF,

	CMD_DISPLAY_STEER_ANGLE,
	CMD_DISPLAY_PGL_ANGLE_TEST,	// for auto test

	MAX_CMD_DISPLAY
}eCMD_SET_DISPLAY;

typedef enum {
	CMD_UI_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_UI_FWDN,
	CMD_UI_KEY_UP,
	CMD_UI_KEY_UP_LONG,
	CMD_UI_KEY_DOWN,
	CMD_UI_KEY_DOWN_LONG,
	CMD_UI_KEY_LEFT,
	CMD_UI_KEY_LEFT_LONG,
	CMD_UI_KEY_RIGHT,
	CMD_UI_KEY_RIGHT_LONG,
	CMD_UI_KEY_CENTER,
	CMD_UI_KEY_CENTER_LONG,	
	CMD_UI_KEY_MENU,
	CMD_UI_KEY_MENU_LONG,

	CMD_UI_KEY_3D_ANGLE,

	CMD_UI_INIT_SCENE,
	CMD_UI_NEXT_SCENE,
	CMD_UI_PREV_SCENE,
	CMD_UI_DIALOG,
	CMD_UI_TRIGGER_REVERSE,
	CMD_UI_TRIGGER_TURN,
	CMD_UI_TRIGGER_EMERGENCY,
	CMD_UI_TRIGGER_DRIVE,
	CMD_UI_CAMERA_PLUG,
	CMD_UI_DISPLAY_FREEZE,
	CMD_UI_CAMERA_INVALID,
#ifdef CACHE_VIEW_USE
	CMD_UI_CACHE_TASK_DONE,
#endif
	CMD_UI_CAR_DOOR,
	
	MAX_CMD_UI
}eCMD_SET_UI;

typedef enum {
    CMD_CALIBRATION_ALIVE = MAX_CMD_COMMON, //fix value 0
    CMD_OFF_CALIB_START,
    CMD_VIEWGEN_START,
    CMD_MULTICORE_VIEWGEN_START,
    CMD_CALIB_TEST,
	MAX_CMD_CALIBRATION
}eCMD_SET_CALIBRATION;

typedef enum {
	CMD_DYNBLEND_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_DYNBLEND_OPER,      //enable/disable command with param.
	MAX_CMD_DYNBLEND
}eCMD_SET_DYNBLEND;

typedef enum {
	CMD_FS_ALIVE = MAX_CMD_COMMON, //fix value 0
	CMD_FS_NUM1,
	MAX_CMD_FS
}eCMD_SET_FS;

#ifdef CACHE_VIEW_USE
typedef enum {
	CMD_CACHE_LOAD = MAX_CMD_COMMON, //fix value 0
	MAX_CMD_CACHE
}eCMD_SET_CACHE;
#endif


///////////////////////////// task cmd define /////////////////////////////////



///////////////////////////// event bit define /////////////////////////////////
typedef enum {
	EVENT_FWDN_INIT = 0,    //bit0
	EVENT_FWDN_MSG,         //bit1
	                        //....
	MAX_EVENT_FWDN          // <= bit23
}eEVENTGROUP_FWDN;

extern const char *psEventNameFWDN[MAX_EVENT_FWDN];

typedef enum {
	EVENT_EMERGENCY_INIT = 0,   //bit0
	EVENT_EMERGENCY_MSG,        //bit1
	                            //....
	MAX_EVENT_EMERGENCY         // <= bit23
}eEVENTGROUP_EMERGENCY;

extern const char *psEventNameEmergency[MAX_EVENT_EMERGENCY];

typedef enum {
	EVENT_MONITOR_INIT = 0,		//bit0
	EVENT_MONITOR_MSG,		    //bit1
	EVENT_MONITOR_TIMER_WAKEUP,	//bit2
	EVENT_MONITOR_UART_KEY_WAKEUP,	//bit3
	                        //....
	MAX_EVENT_MONITOR        	// <= bit23 (end bit)
}eEVENTGROUP_MONITOR;

extern const char *psEventNameMonitor[MAX_EVENT_MONITOR];

typedef enum {
	EVENT_AUDIO_INIT = 0,   //bit0
	EVENT_AUDIO_MSG,        //bit1
	                        //....
	MAX_EVENT_AUDIO         // <= bit23
}eEVENTGROUP_AUDIO;

extern const char *psEventNameAudio[MAX_EVENT_AUDIO];

typedef enum {
	EVENT_UARTCON_INIT = 0, //bit0
	EVENT_UARTCON_MSG,      //bit1
	EVENT_UARTCON_PRINT,	//bit2
							//....
	MAX_EVENT_UARTCON       // <= bit23
}eEVENTGROUP_UARTCON;

extern const char *psEventNameUartCon[MAX_EVENT_UARTCON];

typedef enum {
	EVENT_DISPLAY_INIT = 0, //bit0
	EVENT_DISPLAY_MSG,      //bit1
	                        //....
	MAX_EVENT_DISPLAY       // <= bit23
}eEVENTGROUP_DISPLAY;

extern const char *psEventNameDisplay[MAX_EVENT_DISPLAY];

typedef enum {
	EVENT_UI_INIT = 0,	//bit0
	EVENT_UI_MSG,       //bit1
	                    //....
	MAX_EVENT_UI        // <= bit23
}eEVENTGROUP_UI;

extern const char *psEventNameUI[MAX_EVENT_UI];

typedef enum {
	EVENT_CALIBRATION_INIT = 0, //bit0
	EVENT_CALIBRATION_MSG,      //bit1
	                            //....
	MAX_EVENT_CALIBRATION       // <= bit23
}eEVENTGROUP_CALIBRATION;

extern const char *psEventNameCalibration[MAX_EVENT_CALIBRATION];
    
typedef enum {
	EVENT_DYNBLEND_INIT = 0,    //bit0
	EVENT_DYNBLEND_MSG,         //bit1
	EVENT_DYNBLEND_OPER,        //bit2
	                            //....
	MAX_EVENT_DYNBLEND          // <= bit23
}eEVENTGROUP_DYNBLEND;

extern const char *psEventNameDynBlend[MAX_EVENT_DYNBLEND];
    
typedef enum {
	EVENT_FS_INIT = 0,  //bit0
	EVENT_FS_MSG,       //bit1
	                    //....
	MAX_EVENT_FS        // <= bit23
}eEVENTGROUP_FS;

extern const char *psEventNameFS[MAX_EVENT_FS];

#ifdef CACHE_VIEW_USE
typedef enum {
	EVENT_CACHE_INIT = 0,  //bit0
	EVENT_CACHE_MSG,       //bit1
	                       //....
	MAX_EVENT_CACHE        // <= bit23
}eEVENTGROUP_CACHE;

extern const char *psEventNameCache[MAX_EVENT_CACHE];
#endif

#ifdef SUPPORT_DEBUG_CLI
typedef enum {
	EVENT_CLI_INIT = 0, //bit0
	EVENT_CLI_MSG,      //bit1
	                    //....
	MAX_EVENT_CLI       // <= bit23
}eEVENTGROUP_CLI;

extern const char *psEventNameCLI[MAX_EVENT_CLI];
#endif // SUPPORT_DEBUG_CLI
///////////////////////////// event bit define /////////////////////////////////

extern HandleCfg_t gHandle[MAX_TASK];
extern const PP_U32 gu32TaskMsgEvent[MAX_TASK];

#define ARRY_ELEMS(arr)     (sizeof (arr) / sizeof ((arr)[0]))

/*---------------------------------------------------------------------------*/
/* Interfaces */
/*---------------------------------------------------------------------------*/
#if (configUSE_IDLE_HOOK == 1U)
PP_VOID vTaskBackground(PP_VOID *pvData);
#endif /* (configUSE_IDLE_HOOK == 1U) */

extern void AppTask_Init(void);
extern PP_RESULT_E AppTask_SendCmd(PP_U32 u32Cmd, PP_U16 u16SendTask, PP_U16 u16RecvTask, PP_U16 u16Attr, PP_VOID *pvData, PP_U32 u32DataLen, const PP_S32 s32TimeOut);
extern PP_RESULT_E AppTask_SendCmdFromISR(PP_U32 u32Cmd, PP_U16 u16SendTask, PP_U16 u16RecvTask, PP_U16 u16Attr,PP_VOID *pvData,PP_U32 u32DataLen);
extern void AppTask_Deferred_handler(circ_bbuf_t *pCirc_Buf, PP_U32 circ_buf_size);

#endif /* APP_TASK_H */
/*---------------------------------------------------------------------------*/
/* End Of File */
/*---------------------------------------------------------------------------*/








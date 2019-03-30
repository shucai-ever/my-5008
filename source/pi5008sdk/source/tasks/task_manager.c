/*---------------------------------------------------------------------------*/
/* Includes */
/*---------------------------------------------------------------------------*/
#include "type.h"
#include "error.h"

#include "FreeRTOSConfig.h"
#include "pi5008_config.h"
#include "task_manager.h"

#include "debug.h"
#include "osal.h"

#include "video_config.h"
#include "uart_drv.h"
#include "api_vpu.h"
#include "api_vin.h"

#include "Audio/task_audio.h"
#include "Calibration/task_calibration.h"
#include "Display/task_display.h"
#ifdef DB_LIB_USE
#include "DynBlend/task_dynblend.h"
#endif // DB_LIB_USE
#include "Emergency/task_emergency.h"
#include "FS/task_fs.h"
#include "FWDN/task_fwdn.h"
#include "Monitor/task_monitor.h"
#include "UARTCon/task_uartcon.h"
#include "UI/task_ui.h"
#ifdef CACHE_VIEW_USE
#include "Cache/task_cache.h"
#endif
#ifdef SUPPORT_DEBUG_CLI
#include "CLI/cli.h"
#endif //SUPPORT_DEBUG_CLI

/*---------------------------------------------------------------------------*/
/* Defines */
/*---------------------------------------------------------------------------*/
const char *psTaskName[MAX_TASK] = {
	"TASK_FWDN",
	"TASK_EMERGENCY",
	"TASK_MONITOR",
	"TASK_AUDIO",
	"TASK_UARTCON",
	"TASK_DISPLAY",
	"TASK_UI",
	"TASK_CALIBRATION",
	"TASK_DYNBLEND",
	"TASK_FS",
#ifdef CACHE_VIEW_USE
	"TASK_CACHE",
#endif
#ifdef SUPPORT_DEBUG_CLI
	"TASK_CLI", 
#endif // SUPPORT_DEBUG_CLI
};
const char *psEventNameFWDN[MAX_EVENT_FWDN] = {
	"EVENT_FWDN_INIT",          //bit0
	"EVENT_FWDN_MSG",           //bit1
};
const char *psEventNameEmergency[MAX_EVENT_EMERGENCY] = {
	"EVENT_EMERGENCY_INIT",   //bit0
	"EVENT_EMERGENCY_MSG",        //bit1
};
const char *psEventNameMonitor[MAX_EVENT_MONITOR] = {
	"EVENT_MONITOR_INIT",		//bit0
	"EVENT_MONITOR_MSG",		    //bit1
	"EVENT_MONITOR_TIMER_WAKEUP",	//bit2
	"EVENT_MONITOR_UART_KEY_WAKEUP",	//bit3
};
const char *psEventNameAudio[MAX_EVENT_AUDIO] = {
	"EVENT_AUDIO_INIT",   //bit0
	"EVENT_AUDIO_MSG",        //bit1
};
const char *psEventNameUartCon[MAX_EVENT_UARTCON] = {
	"EVENT_UARTCON_INIT", //bit0
	"EVENT_UARTCON_MSG",      //bit1
};
const char *psEventNameDisplay[MAX_EVENT_DISPLAY] = {
	"EVENT_DISPLAY_INIT", //bit0
	"EVENT_DISPLAY_MSG",      //bit1
};
const char *psEventNameUI[MAX_EVENT_UI] = {
	"EVENT_UI_INIT",	//bit0
	"EVENT_UI_MSG",       //bit1
};
const char *psEventNameCalibration[MAX_EVENT_CALIBRATION] = {
	"EVENT_CALIBRATION_INIT", //bit0
	"EVENT_CALIBRATION_MSG",      //bit1
};
const char *psEventNameDynBlend[MAX_EVENT_DYNBLEND] = {
	"EVENT_DYNBLEND_INIT",    //bit0
	"EVENT_DYNBLEND_MSG",         //bit1
};
const char *psEventNameFS[MAX_EVENT_FS] = {
	"EVENT_FS_INIT",  //bit0
	"EVENT_FS_MSG",       //bit1
};
#ifdef CACHE_VIEW_USE
const char *psEventNameCache[MAX_EVENT_CACHE] = {
	"EVENT_CACHE_INIT",  //bit0
	"EVENT_CACHE_MSG",       //bit1
};
#endif
#ifdef SUPPORT_DEBUG_CLI
const char *psEventNameCLI[MAX_EVENT_CLI] = {
	"EVENT_CLI_INIT", //bit0
	"EVENT_CLI_MSG",      //bit1
};
#endif // SUPPORT_DEBUG_CLI
/*---------------------------------------------------------------------------*/
/* Global Variables */
/*---------------------------------------------------------------------------*/
HandleCfg_t gHandle[MAX_TASK];
const PP_U32 gu32TaskMsgEvent[MAX_TASK] = {
		EVENT_FWDN_MSG,
		EVENT_EMERGENCY_MSG,
		EVENT_MONITOR_MSG,
		EVENT_AUDIO_MSG,
		EVENT_UARTCON_MSG,
		EVENT_DISPLAY_MSG,
		EVENT_UI_MSG,
		EVENT_CALIBRATION_MSG,
		EVENT_DYNBLEND_MSG,
		EVENT_FS_MSG,
#ifdef CACHE_VIEW_USE
		EVENT_CACHE_MSG,
#endif
#ifdef SUPPORT_DEBUG_CLI
		EVENT_CLI_MSG,
#endif // SUPPORT_DEBUG_CLI
};


/*---------------------------------------------------------------------------*/
/* Function Prototypes */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Implementations */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const QueueCfg_t gstQueueCfg[MAX_TASK] =
{
	{10,	"QueueFWDN",		sizeof(stQueueItem),	NULL},
	{30,	"QueueEmergency",	sizeof(stQueueItem),	NULL},
	{10,	"QueueMonitor",	    sizeof(stQueueItem),	NULL},
	{10,	"QueueAudio",	    sizeof(stQueueItem),	NULL},
	{10,	"QueueUartCon",	    sizeof(stQueueItem),	NULL},
	{10,	"QueueDisplay",	    sizeof(stQueueItem),	NULL},
	{10,	"QueueUI",	        sizeof(stQueueItem),	NULL},
	{10,	"QueueCalibration",	sizeof(stQueueItem),	NULL},
#ifdef DB_LIB_USE
	{10,	"QueueDynBlend",	sizeof(stQueueItem),	NULL},
#endif // DB_LIB_USE
	{10,	"QueueFS",	        sizeof(stQueueItem),	NULL},
#ifdef CACHE_VIEW_USE
	{10,	"QueueCache",       sizeof(stQueueItem),	NULL},
#endif
# ifdef SUPPORT_DEBUG_CLI
	{10,	"QueueCLI",		    sizeof(stQueueItem),	NULL},
# endif // SUPPORT_DEBUG_CLI
};

static const EventGroupCfg_t gstEventGroupCfg[MAX_TASK] =
{
	{MAX_EVENT_FWDN,	    "EventFWDN",		NULL},
	{MAX_EVENT_EMERGENCY,   "EventEmergency",	NULL},
	{MAX_EVENT_MONITOR,     "EventMonitor",		NULL},
	{MAX_EVENT_AUDIO,       "EventAudio",		NULL},
	{MAX_EVENT_UARTCON,     "EventUartCon",		NULL},
	{MAX_EVENT_DISPLAY,     "EventDisplay",		NULL},
	{MAX_EVENT_UI,          "EventUI",		    NULL},
	{MAX_EVENT_CALIBRATION, "EventCalibration",	NULL},
#ifdef DB_LIB_USE
	{MAX_EVENT_DYNBLEND,    "EventDynBlend",	NULL},
#endif // DB_LIB_USE
	{MAX_EVENT_FS,          "EventFS",		    NULL},
#ifdef CACHE_VIEW_USE
	{MAX_EVENT_CACHE,       "EventCache",	    NULL},
#endif
# ifdef SUPPORT_DEBUG_CLI
	{MAX_EVENT_CLI,	        "EventCLI",		    NULL},
# endif // SUPPORT_DEBUG_CLI
};

TaskParam_t gstTaskParam[MAX_TASK] =
{
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
#ifdef DB_LIB_USE
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
#endif
#ifdef CACHE_VIEW_USE
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
#endif
#ifdef SUPPORT_DEBUG_CLI
		{.pvParameters = NULL, .u32TaskLoopCnt = 0},
#endif
};

// Higher : High Priority
static const TaskCfg_t gstTaskCfg[MAX_TASK] =
{
	/* priority(max:18), task name, task, stack size, params */
	{TASK_FWDN,     17U,	"TaskFWDN",	    vTaskFWDN,	    CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_FWDN], 			&gstQueueCfg[TASK_FWDN],      &gstEventGroupCfg[TASK_FWDN]},
	{TASK_EMERGENCY, 16U,	"TaskEmergency",vTaskEmergency,	CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_EMERGENCY], 		&gstQueueCfg[TASK_EMERGENCY], &gstEventGroupCfg[TASK_EMERGENCY]},
	{TASK_MONITOR,  15U,	"TaskMonitor",	vTaskMonitor,	CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_MONITOR], 		&gstQueueCfg[TASK_MONITOR],   &gstEventGroupCfg[TASK_MONITOR]},
	{TASK_AUDIO,    14U,	"TaskAudio",	vTaskAudio,	    CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_AUDIO], 			&gstQueueCfg[TASK_AUDIO],     &gstEventGroupCfg[TASK_AUDIO]},
	{TASK_UARTCON,  12U,	"TaskUartCon",	vTaskUARTCon,	CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_UARTCON], 		&gstQueueCfg[TASK_UARTCON],   &gstEventGroupCfg[TASK_UARTCON]},
	{TASK_DISPLAY,  10U,	"TaskDisplay",	vTaskDisplay,	CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_DISPLAY], 		&gstQueueCfg[TASK_DISPLAY],   &gstEventGroupCfg[TASK_DISPLAY]},
	{TASK_UI,       9U,	    "TaskUI",	    vTaskUI,	    CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_UI], 			&gstQueueCfg[TASK_UI],        &gstEventGroupCfg[TASK_UI]},
	{TASK_CALIBRATION, 8U,	"TaskCalibration", vTaskCalibration, CONFIG_MINIMAL_STACK_SIZE * 2, &gstTaskParam[TASK_CALIBRATION],	&gstQueueCfg[TASK_CALIBRATION], &gstEventGroupCfg[TASK_CALIBRATION]},
#ifdef DB_LIB_USE
	{TASK_DYNBLEND, 7U,	    "TaskDynBlend",	vTaskDynBlend,	CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_DYNBLEND], 		&gstQueueCfg[TASK_DYNBLEND],  &gstEventGroupCfg[TASK_DYNBLEND]},
#endif // DB_LIB_USE
	{TASK_FS,       6U,	    "TaskFS",	    vTaskFS,	    CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_FS], 			&gstQueueCfg[TASK_FS],        &gstEventGroupCfg[TASK_FS]},
#ifdef CACHE_VIEW_USE
	{TASK_CACHE,    4U,	    "TaskCache",	vTaskCache,	    CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_CACHE], 			&gstQueueCfg[TASK_CACHE],     &gstEventGroupCfg[TASK_CACHE]},
#endif
# ifdef SUPPORT_DEBUG_CLI
	{TASK_CLI,      4U,	    "TaskCLI",		vTaskCLI,	    CONFIG_MINIMAL_STACK_SIZE * 2,      &gstTaskParam[TASK_CLI], 			&gstQueueCfg[TASK_CLI],       &gstEventGroupCfg[TASK_CLI]},
# endif // SUPPORT_DEBUG_CLI
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*---------------------------------------------------------------------------*/
/* Implementations */
/*---------------------------------------------------------------------------*/
void AppTask_Init(void)
{
    sys_os_thread_t thread;
    uint8 taskIndex = 0U;

    // CmdQueue, EventGroup should be created before task running.
    LOG_DEBUG("Create Queue & EventGroup\n");
    for (taskIndex = 0U; taskIndex < MAX_TASK; taskIndex++)
    {
        if(gstTaskCfg[taskIndex].pQueueCfg != NULL)
        {
            QueueCfg_t *pQueueCfg  = (QueueCfg_t *)(gstTaskCfg[taskIndex].pQueueCfg);

            //LOG_DEBUG("Create Queue:%s\n", pQueueCfg->name);
            if( (gHandle[taskIndex].phQueue = OSAL_QUEUE_CMD_Create(pQueueCfg->maxNumItem, pQueueCfg->sizeItem)) == NULL)
            {
                PANIC("Can't create Queue\n");
            }
        }

        if(gstTaskCfg[taskIndex].pEventGroupCfg != NULL)
        {
            //EventGroupCfg_t *pEventGroupCfg  = (EventGroupCfg_t *)(gstTaskCfg[taskIndex].pEventGroupCfg);

            //LOG_DEBUG("Create EventGroup:%s\n", pEventGroupCfg->name);
            if( (gHandle[taskIndex].phEventGroup = OSAL_EVENTGROUP_CMD_Create()) == NULL)
            {
                PANIC("Can't create EventGroup\n");
            }
        }
    }

    // Send Init Event to all task.
    for (taskIndex = 0U; taskIndex < MAX_TASK; taskIndex++)
    {
#ifdef SUPPORT_DEBUG_CLI
        if(taskIndex == TASK_CLI) continue;
#endif //SUPPORT_DEBUG_CLI

        OSAL_EVENTGROUP_CMD_SetBits(gHandle[taskIndex].phEventGroup, 1<<0);
    }

    //LOG_DEBUG("Create Task\n");
    for (taskIndex = 0U; taskIndex < MAX_TASK; taskIndex++)
    {
        thread.fn           = gstTaskCfg[taskIndex].taskFn;
        thread.name         = gstTaskCfg[taskIndex].taskName;
        thread.stack_size   = gstTaskCfg[taskIndex].stackSize;
    	thread.pvParameters = (void*)gstTaskCfg[taskIndex].pvParameters;
    	thread.prio 		= gstTaskCfg[taskIndex].priority;
    	thread.phTask	    = (void*)&gHandle[taskIndex].phTask;
		
		//LOG_DEBUG("Create Task:%s, Stack:%d, proirity:%d\n", thread.name, thread.stack_size, thread.prio);
        if (eSUCCESS != OSAL_create_thread(&thread))
        {
            OSAL_system_panic(eERROR_FAILURE);
        }
    }
}

#if (configUSE_IDLE_HOOK == 1U)
void vApplicationIdleHook (void) //resreved function name "vApplicationIdleHook()" by OS
{

    for(;;)
    {
        //LOG_DEBUG("idle task\n");
    }

    return;
}
#endif

PP_RESULT_E AppTask_SendCmd(PP_U32 u32Cmd, PP_U16 u16SendTask, PP_U16 u16RecvTask, PP_U16 u16Attr, PP_VOID *pvData, PP_U32 u32DataLen, const PP_S32 s32TimeOut)
{
	stQueueItem queueItem;
	PP_RESULT_E ret = eERROR_FAILURE;

	memset(&queueItem, 0, sizeof(stQueueItem));
	queueItem.u32Cmd = u32Cmd;
	queueItem.u16Sender = u16SendTask;
	queueItem.u16Attr = u16Attr;
	queueItem.u32Length = u32DataLen;

	if(pvData && u32DataLen > 0){
		queueItem.pData = OSAL_malloc(queueItem.u32Length);
		if(queueItem.pData){
			memcpy(queueItem.pData, pvData, queueItem.u32Length);
		}else{
			return ret;
		}
	}

	if((ret = OSAL_QUEUE_CMD_Send(gHandle[u16RecvTask].phQueue, &queueItem, s32TimeOut)) == eSUCCESS){
		OSAL_EVENTGROUP_CMD_SetBits(gHandle[u16RecvTask].phEventGroup, (1<<gu32TaskMsgEvent[u16RecvTask]));
	}


	return ret;
}
PP_RESULT_E AppTask_SendCmdFromISR(PP_U32 u32Cmd, PP_U16 u16SendTask, PP_U16 u16RecvTask, PP_U16 u16Attr,PP_VOID *pvData,PP_U32 u32DataLen)
{
	BaseType_t xHigherPriorityTaskWoken;
	stCIRCBufItemForCmdSend cirBufItem;
	PP_U32 circ_buf_size= u32DataLen + sizeof(stCIRCBufItemForCmdSend);
	PP_U32 i;
	PP_U8 *pBuf;
	PP_RESULT_E ret = eERROR_FAILURE;

	/* check empty circ buf size*/


	if(PPUTIL_CIRCBUF_GETEMPTYSIZE(&global_circ_buf)<circ_buf_size)
	{
		//if dont have memory space for push, return error
		ret=eERROR_FAILURE;
		return ret;
	}
		
	cirBufItem.u32Cmd=u32Cmd;
	cirBufItem.u16Sender=u16SendTask;
	cirBufItem.u16RecvTask=u16RecvTask;
	cirBufItem.u16Attr=u16Attr;
	cirBufItem.u32Length=u32DataLen;
	
	pBuf=&cirBufItem;
	
	for(i=0;i<sizeof(cirBufItem);i++)
	{
		PPUTIL_CIRCBUF_PUSH(&global_circ_buf, pBuf[i]);						
	}

	pBuf=(PP_U8 *)pvData;

	for(i=0;i<u32DataLen;i++)
	{
		PPUTIL_CIRCBUF_PUSH(&global_circ_buf,pBuf[i]);						
	}


	xHigherPriorityTaskWoken = pdFALSE;
	xTimerPendFunctionCallFromISR( AppTask_Deferred_handler, &global_circ_buf, circ_buf_size, &xHigherPriorityTaskWoken );
	if(xHigherPriorityTaskWoken == pdTRUE){
		portYIELD_FROM_ISR();
	}
	
	ret=eSUCCESS;
	
	return ret;
}


PP_VOID AppTask_Deferred_handler(circ_bbuf_t *pCirc_Buf, PP_U32 circ_buf_size)
{
	stCIRCBufItemForCmdSend *pCirBufItem;
	PP_U32 i;
	stQueueItem queueItem;
	PP_RESULT_E ret = eERROR_FAILURE;
	BaseType_t xHigherPriorityTaskWoken;
	PP_U8 *pCirc_Buf_Temp;

	memset(&queueItem, 0, sizeof(stQueueItem));

	if(circ_buf_size > 0){
		pCirc_Buf_Temp = OSAL_malloc(circ_buf_size);

		for(i=0;i<circ_buf_size;i++)
		{
			PPUTIL_CIRCBUF_POP(pCirc_Buf,&pCirc_Buf_Temp[i]);
		}
	}
	else
	{
		ret = eERROR_FAILURE;
		return ;
	}
	
	
	pCirBufItem = (stCIRCBufItemForCmdSend *)pCirc_Buf_Temp;




	queueItem.u32Cmd = pCirBufItem->u32Cmd;
	queueItem.u16Sender = pCirBufItem->u16Sender;
	queueItem.u16Attr = pCirBufItem->u16Attr;
	queueItem.u32Length = pCirBufItem->u32Length;

	if(queueItem.u32Length > 0){
		queueItem.pData = OSAL_malloc(queueItem.u32Length);

	
		if(queueItem.pData){		
			memcpy(queueItem.pData, (PP_U8 *)pCirc_Buf_Temp + sizeof(stCIRCBufItemForCmdSend), queueItem.u32Length);
		}else{
			OSAL_free(pCirc_Buf_Temp);
			return;
		}
	}

	xHigherPriorityTaskWoken = pdFALSE;
    ret = OSAL_QUEUE_CMD_SendFromISR(gHandle[pCirBufItem->u16RecvTask].phQueue, &queueItem, &xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
	if(ret == eSUCCESS)
	{
	    xHigherPriorityTaskWoken = pdFALSE;
		OSAL_EVENTGROUP_CMD_SetBitsFromISR(gHandle[pCirBufItem->u16RecvTask].phEventGroup, (1<<gu32TaskMsgEvent[pCirBufItem->u16RecvTask]), &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
	}

	OSAL_free(pCirc_Buf_Temp);
	return ;



	
	

}












/*---------------------------------------------------------------------------*/
/* End Of File */
/*---------------------------------------------------------------------------*/

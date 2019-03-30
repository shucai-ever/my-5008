#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"
#include "task_ui.h"
#include "task_display.h"
#include "api_FAT_FTL.h"
#include "api_display.h"

#if defined(USE_PP_GUI)
#include "application.h"
#include "menu_data.h"

#define POPUP_TIME_MS			1000
#define PROGRESSBAR_TIME_MS		500
#define PGL_TEST_TIME_MS		100
#define WHEEL_UPDATE_TIME_MS	100

extern PP_SCENE_S *currScene;	// define "app_ui.c.h"


STATIC PP_VOID PopUpTimerHandler(TimerHandle_t IN timer)
{
    configASSERT(timer);

	PPAPI_DISPLAY_POPUP_Off();
}

STATIC PP_VOID SetPopUpTimer(PP_POPUP_E IN msg)
{
    TimerHandle_t timer = NULL;

	PPAPI_DISPLAY_POPUP_On(msg);
	
    timer = xTimerCreate("POP-UP Timer", OSAL_ms2ticks(POPUP_TIME_MS), pdFALSE, (void*)NULL, PopUpTimerHandler);
    configASSERT(timer);
    configASSERT(xTimerStart(timer, 0) == pdPASS);

    LOG_DEBUG("POP-UP timer created. handler: 0x%x, id: %d\n", (uint32)timer, (int)pvTimerGetTimerID(timer));
}

STATIC PP_U32 progLevel = 0;
PP_VOID SetProgressBarLevel (PP_U32 IN level)
{
	progLevel = level;	// range : 0 ~ 10
}

STATIC PP_VOID ProgressBarTimerHandler(TimerHandle_t IN timer)
{
	STATIC PP_U32 flicker = 0;
	
    configASSERT(timer);

	flicker = (flicker + 1) % 2;
	PPAPI_DISPLAY_PROGRESSBAR_On(progLevel, flicker);
}

STATIC TimerHandle_t EnableProgressBarTimer (PP_VOID)
{
	TimerHandle_t timer = NULL;
	
	progLevel = 0;

    timer = xTimerCreate("ProgressBar Timer Enable", OSAL_ms2ticks(PROGRESSBAR_TIME_MS), pdTRUE, (void*)NULL, ProgressBarTimerHandler);
    configASSERT(timer);
    configASSERT(xTimerStart(timer, 0) == pdPASS);

    LOG_DEBUG("Progress Bar timer created. handler: 0x%x, id: %d\n", (uint32)timer, (int)pvTimerGetTimerID(timer));
	return timer;
}

STATIC PP_VOID DisableProgressBarTimer (TimerHandle_t IN timer)
{
	configASSERT(timer);
	configASSERT(xTimerStop(timer, 0) == pdPASS);
	
	xTimerDelete(timer, 0);
	PPAPI_DISPLAY_PROGRESSBAR_Off();
}

STATIC PP_VOID PglTestTimerHandler(TimerHandle_t IN timer)
{
	STATIC PP_S16 angle = 0;
	STATIC PP_BOOL plusDir = PP_TRUE;
	PP_U16 attr;

//	configASSERT(timer);

	if(angle >= 35)			plusDir = PP_FALSE;
	else if(angle <= -35)	plusDir = PP_TRUE;

	if(plusDir)				angle += 1;
	else					angle -= 1;

	attr = angle;
	AppTask_SendCmd(CMD_DISPLAY_STEER_ANGLE, TASK_DISPLAY, TASK_DISPLAY, attr, NULL, sizeof(attr), 1000);
}

STATIC TimerHandle_t EnablePglTestTimer (PP_VOID)
{
	TimerHandle_t timer = NULL;
	timer = xTimerCreate("Pgl Test Timer Enable", OSAL_ms2ticks(PGL_TEST_TIME_MS), pdTRUE, (void*)NULL, PglTestTimerHandler);
    configASSERT(timer);
    configASSERT(xTimerStart(timer, 0) == pdPASS);

    LOG_DEBUG("Pgl Test timer created. handler: 0x%x, id: %d\n", (uint32)timer, (int)pvTimerGetTimerID(timer));
	return timer;
}

STATIC PP_VOID DisablePglTestTimer (TimerHandle_t IN timer)
{
	configASSERT(timer);
	configASSERT(xTimerStop(timer, 0) == pdPASS);
	
	xTimerDelete(timer, 0);
}

static PP_VOID WheelUpdateTimerHeandler(TimerHandle_t timer)
{
	STATIC PP_BOOL isRun = PP_FALSE;
    configASSERT(timer);

	if(currScene->elem->id == eScene_LiveView)
	{
		switch(currScene->elem->selIdx)
		{
			case eViewMode_Top3D:
				isRun = PP_TRUE;
				break;
			default:
				isRun = PP_FALSE;
				break;
		}
	}
	else
	{
		isRun = PP_FALSE;
	}

	if (isRun)
		PPAPI_DISPLAY_VIEW_UpdateWheel();

    return;
}

STATIC PP_VOID SetWheelUpdateTimer (PP_VOID)
{
    TimerHandle_t timer = NULL;

    timer = xTimerCreate("Display Task Wheel Update Timer", OSAL_ms2ticks(WHEEL_UPDATE_TIME_MS), pdTRUE, (void*)NULL, WheelUpdateTimerHeandler);
    configASSERT(timer);
    configASSERT(xTimerStart(timer, 0) == pdPASS);

    LOG_DEBUG("display task wheel update timer created. handler: 0x%x, id: %d\n", (uint32)timer, (int)pvTimerGetTimerID(timer));
}

#endif

PP_VOID vTaskDisplay(PP_VOID* IN pvData)
{
    PP_S32 myHandleNum = TASK_DISPLAY;
    PP_S32 timeOut = 1000; //msec
    EventBits_t eventWaitBits;
    EventBits_t eventResultBits;
	TaskParam_t *pstParam = (TaskParam_t *)pvData;
	
	TimerHandle_t progTimer = NULL;
	TimerHandle_t pglTestTimer = NULL;

	PP_S32 i;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));
	PPAPI_FATFS_EnterFS();

#if defined(USE_CAR_WHEEL)
	SetWheelUpdateTimer();
#endif
    for(;;)
    {
    	pstParam->u32TaskLoopCnt++;

	    //LOG_DEBUG("WaitBits(%s)\n", __FUNCTION__);
        if(gHandle[myHandleNum].phEventGroup)
        {
            eventWaitBits = 0x00FFFFFF; //0x00FFFFFF all bit
            eventResultBits = OSAL_EVENTGROUP_CMD_WaitBits(gHandle[myHandleNum].phEventGroup, eventWaitBits, pdTRUE, pdFALSE, timeOut);

            if ( eventResultBits == 0 )
            {
                //LOG_DEBUG("timeout\n");
                continue;
            }
            else
            {
                // process event bit
                if(eventResultBits & (1<<EVENT_DISPLAY_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_DISPLAY_INIT, psEventNameDisplay[EVENT_DISPLAY_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_DISPLAY_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_DISPLAY_MSG, psEventNameDisplay[EVENT_DISPLAY_MSG]);
                    {/*{{{*/
                        if(gHandle[myHandleNum].phQueue)
                        {
                            PP_S32 queueCnt = 0;
                            stQueueItem queueItem;
                            if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                            {
                                //LOG_DEBUG("get Queue cnt:%d\n", queueCnt);
                                for(i = 0; i < queueCnt; i++)
                                {
                                    if( OSAL_QUEUE_CMD_Receive(gHandle[myHandleNum].phQueue, &queueItem, 0) == eSUCCESS )
                                    {
                                        /*
                                           LOG_DEBUG("%s:Rcv cmd:%08x, sender:%d(%s), attr:%x, length:%d\n", psTaskName[myHandleNum],
                                           queueItem.u32Cmd, queueItem.u16Sender, psTaskName[queueItem.u16Sender], queueItem.u16Attr, queueItem.u32Length);
                                           */

                                        if(queueItem.u32Cmd == CMD_DISPLAY_ALIVE)
                                        {						
                                        }

                                        //--- EVENT Process -----------------------------------------------
                                        switch(queueItem.u32Cmd)
                                        {
										#if defined(USE_PP_GUI)
                                            case CMD_DISPLAY_POPUP_TIMER:
                                                SetPopUpTimer(queueItem.u16Attr);
                                                break;

                                            case CMD_DISPLAY_POPUP_ON:
                                                PPAPI_DISPLAY_POPUP_On(queueItem.u16Attr);
                                                break;

                                            case CMD_DISPLAY_POPUP_OFF:
                                                PPAPI_DISPLAY_POPUP_Off();
                                                break;

                                            case CMD_DISPLAY_PROGRESSBAR_TIMER_ON:
                                                if(progTimer == NULL)
                                                {
                                                    progTimer = EnableProgressBarTimer();
                                                }
                                                break;

                                            case CMD_DISPLAY_PROGRESSBAR_TIMER_OFF:
                                                if(progTimer != NULL)
                                                {
                                                    DisableProgressBarTimer(progTimer);
                                                    progTimer = NULL;
                                                }
                                                break;

                                            case CMD_DISPLAY_PROGRESSBAR_ON:
                                                PPAPI_DISPLAY_PROGRESSBAR_On(queueItem.u16Attr, 0);
                                                break;

                                            case CMD_DISPLAY_PROGRESSBAR_OFF:
                                                PPAPI_DISPLAY_PROGRESSBAR_Off();
                                                break;

											case CMD_DISPLAY_STEER_ANGLE:
												gPglAngle = queueItem.u16Attr;
												
												if( (gMenuData.u32PGL != ePgl_Off) && (currScene->elem->id == eScene_LiveView) )
												{
													PPAPI_DISPLAY_VIEW_PGL_SetAngle(gPglAngle);
												}
												break;

											case CMD_DISPLAY_PGL_ANGLE_TEST:
												#if 1
												{
													STATIC PP_BOOL isRun = PP_FALSE;

													isRun = !isRun;

													if(isRun)
													{
														if(pglTestTimer == NULL)
															pglTestTimer = EnablePglTestTimer();
	  												}
													else
													{
														 if(pglTestTimer != NULL)
		                                                {
		                                                    DisablePglTestTimer(pglTestTimer);
		                                                    pglTestTimer = NULL;
		                                                }
													}
												}
												#else
												PglTestTimerHandler(0);
												#endif
												break;
										#endif // USE_PP_GUI
                                        }
									
                                        //-----------------------------------------------------------------

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_DISPLAY_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_DISPLAY, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_DISPLAY, queueItem.u16Sender, 0, NULL_PTR, 0, 1000);
                                                }
                                            }
                                        }

                                        if( (queueItem.u32Length > 0) && (queueItem.pData != NULL_PTR) )
                                        {
                                            OSAL_free(queueItem.pData);
                                        }

                                    }
                                }
                            }
                            else
                            {
                                //ignore
                                //LOG_DEBUG("Don't remain queue.\n");
                            }
                        }
                        else
                        {
                            LOG_CRITICAL("Invalid handle.\n");
                        }

                    }/*}}}*/
                }/*}}}*/
            }
        }
    }
    PPAPI_FATFS_ReleaseFS();

    return;
}


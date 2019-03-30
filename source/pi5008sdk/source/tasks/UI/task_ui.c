#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"
#include "task_fwdn.h"
#include "task_ui.h"
#include "board_config.h"
#include "api_FAT_FTL.h"
#include "api_svm.h"
#include "api_display.h"
#include "api_vin.h"
#include "sys_api.h"
#if defined(USE_PP_GUI)
#include "application.h"
PP_SCENE_S *currScene = NULL_PTR;
PP_BOOL gUiLoadingDone = PP_FALSE;
#endif

PP_VOID vTaskUI(PP_VOID* IN pvData)
{
    PP_S32 myHandleNum = TASK_UI;
    PP_S32 timeOut = 1000; //msec
    EventBits_t eventWaitBits;
    EventBits_t eventResultBits;
	TaskParam_t *pstParam = (TaskParam_t *)pvData;
	
	#if defined(USE_PP_GUI)
	PP_SCENE_S *prevScene = NULL_PTR;
	#endif
	PP_S32 i;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

	#if defined(USE_PP_GUI)
	currScene = PPAPP_UI_Initialize();
	if(currScene == NULL_PTR)
	{
		configASSERT(currScene);
	}
	#endif //USE_PP_GUI
	
	PPAPI_FATFS_EnterFS();
    for(;;)
    {
    	pstParam->u32TaskLoopCnt++;

	   // LOG_DEBUG("WaitBits(%s)\n", __FUNCTION__);
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
                if(eventResultBits & (1<<EVENT_UI_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_UI_INIT, psEventNameUI[EVENT_UI_INIT]);

#if defined(USE_PP_GUI) && defined(CACHE_VIEW_USE)
                    currScene->elem->uiFunc(currScene, (PP_U32)NULL);
#endif
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_UI_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_UI_MSG, psEventNameUI[EVENT_UI_MSG]);
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
                                        if(queueItem.u32Cmd == CMD_UI_ALIVE)
                                        {						
                                        }
                                        // TODO: temp
                                        else if(queueItem.u32Cmd==CMD_UI_FWDN)
                                        {
                                            LOG_DEBUG("!!!!!!!!!!!!!!!!!! CMD_UI_FWDN\n");
                                            if(AppTask_SendCmd(CMD_FWDN_FLASH_UPDATE, TASK_UI, TASK_FWDN, (1<<QUEUE_CMDATTR_BIT_REQACK), (PP_VOID *)queueItem.pData, queueItem.u32Length, 3000) != eSUCCESS){
                                                LOG_CRITICAL("UI TASK. Send FWDN CMD Fail !!!!\n");
                                            }

                                        }
                                        else if(queueItem.u32Cmd == CMD_UI_CAMERA_PLUG)
                                        {
                                            PP_U32 u32StatusPlug = *(PP_U32*)queueItem.pData;
                                            LOG_DEBUG("chan:%d, plug:%s\n", u32StatusPlug>>16, (u32StatusPlug&1)?"In":"Out");

#if 0 //TODO!!
#if defined(USE_PP_GUI)
											if(gUiLoadingDone)
											{
												if(u32StatusPlug&1)
													AppTask_SendCmd(CMD_DISPLAY_POPUP_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
												else
													AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_NoInput, NULL_PTR, 0, 1000);
											}
#endif
#endif
                                        }   
                                        else if(queueItem.u32Cmd == CMD_UI_DISPLAY_FREEZE)
                                        {
                                            PP_U32 u32StatusPlug = *(PP_U32*)queueItem.pData;
                                            LOG_DEBUG("chan:%d, freeze:%s\n", u32StatusPlug>>16, (u32StatusPlug&1)?"Normal":"Freeze");

#if 0 //TODO!!
#if defined(USE_PP_GUI)
											if(gUiLoadingDone)
											{
												if(u32StatusPlug&1)
													AppTask_SendCmd(CMD_DISPLAY_POPUP_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
												else
													AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_NoVideo, NULL_PTR, 0, 1000);
											}
#endif
#endif
                                        }
                                        else if(queueItem.u32Cmd == CMD_UI_CAMERA_INVALID)
                                        {
                                            PP_U32 u32StatusPlug = *(PP_U32*)queueItem.pData;
                                            LOG_DEBUG("chan:%d, camera format valid:%s\n", u32StatusPlug>>16, (u32StatusPlug&1)?"Valid":"Invalid");
#if 0 //TODO!!
#if defined(USE_PP_GUI)
											if(gUiLoadingDone)
											{
												if(u32StatusPlug&1)
													AppTask_SendCmd(CMD_DISPLAY_POPUP_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
												else
													AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_NoFormat, NULL_PTR, 0, 1000);
											}
#endif
#endif
                                        }
										else
										{
	                                        //--- EVENT Process -----------------------------------------------
	                                        switch(queueItem.u32Cmd)
	                                        {
											#if defined(USE_PP_GUI)
	                                            case CMD_UI_INIT_SCENE:
	                                            case CMD_UI_KEY_CENTER:
	                                            case CMD_UI_KEY_UP:
	                                            case CMD_UI_KEY_DOWN:
	                                            case CMD_UI_KEY_LEFT:
	                                            case CMD_UI_KEY_RIGHT:
	                                            case CMD_UI_KEY_MENU:
	                                            case CMD_UI_TRIGGER_REVERSE:
	                                            case CMD_UI_TRIGGER_TURN:
	                                            case CMD_UI_TRIGGER_EMERGENCY:
	                                            case CMD_UI_TRIGGER_DRIVE:
	                                                currScene->elem->uiFunc(currScene, queueItem.u32Cmd);
	                                                break;
	                                            case CMD_UI_NEXT_SCENE:
	                                                currScene = currScene->next[queueItem.u16Attr];
	                                                //currScene->elem->uiFunc(currScene, CMD_UI_INIT_SCENE);
	                                                break;
	                                            case CMD_UI_KEY_3D_ANGLE:
													*currScene->elem->subElem[currScene->elem->selIdx].value = queueItem.u16Attr;
	                                            	currScene->elem->uiFunc(currScene, queueItem.u32Cmd);
	                                            	break;
	                                            case CMD_UI_PREV_SCENE:
	                                                currScene = currScene->prev;
	                                                //currScene->elem->uiFunc(currScene, CMD_UI_INIT_SCENE);
	                                                break;

	                                            case CMD_UI_DIALOG:
	                                                if((currScene->elem->id != eScene_Dialog) && (currScene->elem->id != eScene_ViewGen))
	                                                    prevScene = currScene;
	                                                currScene = PPAPP_UI_GetScene(eScene_Dialog);
	                                                currScene->prev = prevScene;
	                                                currScene->elem->selIdx = queueItem.u16Attr;
	                                                currScene->elem->uiFunc(currScene, CMD_UI_INIT_SCENE);
	                                                break;
#ifdef CACHE_VIEW_USE
												case CMD_UI_CACHE_TASK_DONE:
													printf("!!!!!!!!!!!!!!!!!!CACHE_TASK_DONE!!!!!!!!!!!\n");
													PPAPI_DISPlAY_UpdateLUT();
													gUiLoadingDone = PP_TRUE;
													switch(*currScene->elem->subElem[currScene->elem->selIdx].value)
													{
														case eViewMode_TopCam_Front:	PPAPP_View_ChangeImage(eVIEWMODE_LOAD_TOP2D_FRONTSD);		break;
														case eViewMode_TopCam_Left:		PPAPP_View_ChangeImage(eVIEWMODE_LOAD_TOP2D_LEFTSD);		break;
														case eViewMode_TopCam_Right:	PPAPP_View_ChangeImage(eVIEWMODE_LOAD_TOP2D_RIGHTSD);		break;
														case eViewMode_TopCam_Rear:		PPAPP_View_ChangeImage(eVIEWMODE_LOAD_TOP2D_REARSD);		break;
														default:						PPAPP_View_ChangeImage(eVIEWMODE_LOAD_TOP2D_REARSD);		break;
													}
													break;
#endif

#if defined(USE_CAR_DOOR)
												case CMD_UI_CAR_DOOR:
													{
														#if 0
														gCarOpenDoor = *(PP_U32*)queueItem.pData;
														#else	// for test
														gCarOpenDoor++;
														if(gCarOpenDoor >= eCar_OpenDoor_Max)
															gCarOpenDoor = 0;
														#endif
														PPAPI_DISPLAY_VIEW_SetCarDoor (gCarOpenDoor);
													}
													break;
#endif

											#endif // USE_PP_GUI

	                                            case CMD_ACK:
	                                                if(queueItem.u16Sender == TASK_FWDN){
	                                                    LOG_DEBUG("Firmware upgrade success\n");
														#if defined(USE_PP_GUI)
														AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_Done, NULL_PTR, 0, 1000);
														OSAL_sleep(1000);
														AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_ShutDown, NULL_PTR, 0, 1000);
														OSAL_sleep(3000);
														#endif
	                                                	PPAPI_SYS_Reset();
	                                                }
	                                                break;
	                                            case CMD_NACK:
	                                                if(queueItem.u16Sender == TASK_FWDN){
	                                                    if(queueItem.u32Length){
		                                                	PP_RESULT_E enRet;

	                                                    	memcpy(&enRet, queueItem.pData, sizeof(enRet));
	                                                    	if(enRet == eERROR_UPGRADE_REJECTED){
	                                                    		LOG_DEBUG("Can not upgrade firmware\n");
	                                                    	}else{
	                                                    		LOG_DEBUG("Firmware upgrade fail\n");
	                                                    	}
															#if defined(USE_PP_GUI)
	                            							AppTask_SendCmd(CMD_DISPLAY_POPUP_TIMER, TASK_UI, TASK_DISPLAY, ePopUp_NoUpdate, NULL_PTR, 0, 1000);
															#endif
							    							AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	                                                    }

	                                                }

	                                                break;
	                                        }
										}
                                        //-----------------------------------------------------------------

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_UI_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_UI, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_UI, queueItem.u16Sender, 0, NULL_PTR, 0, 1000);
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


#include "system.h"
#include "debug.h"
#include "osal.h"
#include "common.h"
#include "task_manager.h"

#include "api_diag.h"
#include "api_ipc.h"
#include "api_vin.h"
#include "api_svm.h"
#include "api_pvirx_func.h"

#include "task_emergency.h"

#undef EMERGENCY_SAMPLE //sample processing for emergency.

#ifdef EMERGENCY_SAMPLE                                            
#if defined(USE_PP_GUI)
extern PP_BOOL gUiLoadingDone;
#endif //defined(USE_PP_GUI)
#endif // EMERGENCY_SAMPLE                                            

PP_VOID vTaskEmergency(PP_VOID *pvData)
{
	int myHandleNum = TASK_EMERGENCY;
	int timeOut = 1000; //msec
	EventBits_t eventWaitBits;
	EventBits_t eventResultBits;
	TaskParam_t *pstParam = (TaskParam_t *)pvData;

	int i;
    PP_S8 s8CurCamChStatus[4] = {0, };
    PP_U8 u8CurDisplayFreeze[4] = {0, };
    PP_U8 u8CurInvalidCamCh[4] = {0, };

    PP_VID_S stCurVidStatus[4];

    int queueCnt = 0;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
    {
        int ch;
        for(ch = 0; ch < 4; ch++)
        {
            PPAPI_PVIRX_SetNovidInitIRQ(ch);
        }
    }
#endif //(VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)

    /* check camera plug status */
    {/*{{{*/
        if( (PPAPI_VIN_GetCameraPlugStatus(s8CurCamChStatus) == eSUCCESS) )
        {
            //Send event to task.
            AppTask_SendCmd(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, s8CurCamChStatus, sizeof(PP_S8)*4, 100);
        }
    }/*}}}*/

    //Get camera input information 
    for(i = 0; i < 4; i++)
    {/*{{{*/
        PPAPI_VIN_DiagCameraInput(i);
    }/*}}}*/

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
				    
                /* check camera plug status */
                {/*{{{*/
                    PP_U8 u8FlagChange = FALSE;
                    PP_S8 s8TempCamStatus[4];

                    if( (PPAPI_VIN_GetCameraPlugStatus(s8TempCamStatus) == eSUCCESS) )
                    {
                        PP_U8 portNum;

                        for(portNum = 0; portNum < 4; portNum++)
                        {
                            if(s8TempCamStatus[portNum] != s8CurCamChStatus[portNum])
                            {
                                u8FlagChange = TRUE;
                                break;
                            }
                        }

                        if(u8FlagChange == TRUE)
                        {
                            LOG_DEBUG("Some camera state changed!\n");

                            //Send event to task.
                            AppTask_SendCmd(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, s8TempCamStatus, sizeof(PP_S8)*4, 100);
                        }
                    }
                }/*}}}*/

                /* check display freeze status */
                {/*{{{*/
                    PP_U8 u8FlagChange = FALSE;
                    PP_U8 u8TempStatus[4] = {0, };
                    PP_U8 portNum;

                    PPAPI_DIAG_GetDisplayFreezeStatus(u8TempStatus);

                    for(portNum = 0; portNum < 4; portNum++)
                    {
                        if(u8TempStatus[portNum] != u8CurDisplayFreeze[portNum])
                        {
                            u8FlagChange = TRUE;
                            break;
                        }
                    }

                    if(u8FlagChange == TRUE)
                    {/*{{{*/
                        LOG_DEBUG("Some display freeze state changed!\n");

                        //Send event to task.
                        AppTask_SendCmd(CMD_EMERGENCY_DISPLAY_FREEZE, (PP_U16)-1, TASK_EMERGENCY, 0, u8TempStatus, sizeof(PP_U8)*4, 100);
                    }/*}}}*/
                }/*}}}*/

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
                /* check camera format is invalid */
                {/*{{{*/
                    PP_U8 u8FlagChange = FALSE;
                    PP_U8 u8TempStatus[4] = {0, };
                    PP_U8 portNum;
                    PP_VID_S stVidStatus;

                    PPAPI_DIAG_GetCamInvalidStatus(u8TempStatus);

                    for(portNum = 0; portNum < 4; portNum++)
                    {
                        if(u8TempStatus[portNum] != u8CurInvalidCamCh[portNum])
                        {
                            u8FlagChange = TRUE;
                            break;
                        }
                    }

                    if(u8FlagChange == TRUE)
                    {/*{{{*/
                        LOG_DEBUG("Some camera format state changed!\n");

                        //Send event to task.
                        AppTask_SendCmd(CMD_EMERGENCY_INVALID_CAMERA, (PP_U16)-1, TASK_EMERGENCY, 0, u8TempStatus, sizeof(PP_U8)*4, 100);
                    }/*}}}*/
                }/*}}}*/
#endif //(VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)

                continue;
			}
			else
			{
				// process event bit
				if(eventResultBits & (1<<EVENT_EMERGENCY_INIT))
				{/*{{{*/
					//Called once when start task.
					//No message Queue.
					//printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_EMERGENCY_INIT, psEventNameEmergency[EVENT_EMERGENCY_INIT]);
				}/*}}}*/
				if(eventResultBits & (1<<EVENT_EMERGENCY_MSG))
				{/*{{{*/
					//LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_EMERGENCY_MSG, psEventNameEmergency[EVENT_EMERGENCY_MSG]);
					if(gHandle[myHandleNum].phQueue)
					{
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
									if(queueItem.u32Cmd == CMD_EMERGENCY_ALIVE)
									{						
                                    }
                                    else if(queueItem.u32Cmd == CMD_EMERGENCY_CAMERA_PLUG)
                                    {/*{{{*/
                                        if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
                                        {
                                            do
                                            {
                                                PP_S8 s8CamCh[4] = { -1, };
                                                PP_S8 s8UserCamCh[4] = { -1, };

                                                PP_U32 u32Status = 0;
                                                PP_U8 portNum;

                                                memcpy(s8CamCh, queueItem.pData, sizeof(PP_S8)*4);

                                                //save.
                                                memcpy(s8CurCamChStatus, s8CamCh, sizeof(PP_S8)*4);

                                                //LOG_DEBUG("[Emergency CAMERA PLUG IN/OUT]:%d/%d/%d/%d\n", s8CamCh[0], s8CamCh[1], s8CamCh[2], s8CamCh[3]); 

                                                if( (PPAPI_VIN_GetVIDPortFromUserDefined(s8CamCh, s8UserCamCh)) != eSUCCESS)
                                                {
                                                    LOG_DEBUG("Error!!! GetVidportFromUserDefined Fail\n");
                                                    break;
                                                }

                                                if( (PPAPI_VIN_SetVIDPort(s8UserCamCh[0], s8UserCamCh[1],  s8UserCamCh[2], s8UserCamCh[3], -1)) != eSUCCESS)
                                                {
                                                    LOG_DEBUG("Error!!! SetVidport Fail\n");
                                                    break;
                                                }

                                                for(portNum = 0; portNum < 4; portNum++)
                                                {
                                                    if( s8UserCamCh[portNum] < 0 )
                                                    {
                                                        PPAPI_VIN_SetSVMChannel(portNum, 1, 2); //Masking Black pattern 
                                                    }
                                                    else
                                                    {
                                                        PPAPI_VIN_SetSVMChannel(portNum, -1, -1); //Normal camera image. //[2018.12.17] keep svm port init value.
                                                    }

                                                    stCurVidStatus[portNum].s8VidPort = portNum;
                                                    stCurVidStatus[portNum].s8Status = s8UserCamCh[portNum];

                                                    //call svm function that control camera plug in-out. 
                                                    //If camera plug-out, mask enable to black color(tp).
                                                    PPAPI_SVM_SetInputReplaceColorOnOff(stCurVidStatus[portNum].s8VidPort, (stCurVidStatus[portNum].s8Status >= 0)?FALSE:TRUE);
                                                }

                                                if( (stCurVidStatus[0].s8Status<0) || (stCurVidStatus[1].s8Status<0) || (stCurVidStatus[2].s8Status<0) || (stCurVidStatus[3].s8Status<0) )
                                                {
                                                    u32Status = 0; //0:plug out some camera.
                                                }
                                                else
                                                {
                                                    u32Status = 1; //1:plug in all camera.
                                                }

                                                //TODO Processing Camera Plug.
                                                if(u32Status == 0) //plug out some camera.
                                                {
                                                    LOG_DEBUG("[Emergency CAMERA PLUG-OUT]\n");
                                                }

#ifdef EMERGENCY_SAMPLE                                            
                                                // sample> display Diaglog Box
                                                {/*{{{*/
#if defined(USE_PP_GUI)
                                                    if(gUiLoadingDone <= 0)
                                                    {
                                                        //retry until gUiLoadingDone == TRUE.
                                                        //LOG_DEBUG("retry until gUiLoadingDone == TRUE\n");
                                                        AppTask_SendCmd(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, s8CurCamChStatus, sizeof(PP_S8)*4, 100);
                                                        OSAL_sleep(1000);
                                                    }
                                                    else
                                                    {

                                                        AppTask_SendCmd(CMD_UI_CAMERA_PLUG, TASK_EMERGENCY, TASK_UI, 0, &u32Status, sizeof(u32Status), 1000);
                                                    }
#else
                                                    {

                                                        AppTask_SendCmd(CMD_UI_CAMERA_PLUG, TASK_EMERGENCY, TASK_UI, 0, &u32Status, sizeof(u32Status), 1000);
                                                    }
#endif //defined(USE_PP_GUI)
                                                }/*}}}*/
#endif // EMERGENCY_SAMPLE                                            
                                            }while(0);
                                        }
                                    }/*}}}*/
                                    else if(queueItem.u32Cmd == CMD_EMERGENCY_DISPLAY_FREEZE)
                                    {/*{{{*/
                                        if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
                                        {
                                            PP_U8 u8Status[4];
                                            PP_U32 u32Status = 0;

                                            memcpy(u8Status, queueItem.pData, sizeof(PP_U8)*4);
                                            //LOG_DEBUG("[Emergency Display freeze]:%d/%d/%d/%d\n", s8Status[0], s8Status[1], s8Status[2], s8Status[3]);
                                                
                                            //save.
                                            memcpy(u8CurDisplayFreeze, u8Status, sizeof(PP_U8)*4);

                                            //only check all camera is plug-in status.
                                            if( (stCurVidStatus[0].s8Status >= 0) && (stCurVidStatus[1].s8Status >= 0) && (stCurVidStatus[2].s8Status >= 0) && (stCurVidStatus[3].s8Status >= 0) )
                                            {
                                                if( (u8CurDisplayFreeze[0]>0) && (u8CurDisplayFreeze[1]>0) && (u8CurDisplayFreeze[2]>0) && (u8CurDisplayFreeze[3]>0) )
                                                {
                                                    u32Status = 0; //0:freeze all channel on all camera plug-in.
                                                }
                                                else
                                                {
                                                    u32Status = 1; //1:normal some channel on all camera plug-in.
                                                }

                                                //TODO Processing Display freeze.
                                                if(u32Status == 0) //freeze status
                                                {
                                                    LOG_DEBUG("[Emergency Display Freeze] TODO!! something.\n");
                                                }

#ifdef EMERGENCY_SAMPLE                                            
#endif // EMERGENCY_SAMPLE                                            

                                            }
                                        }
                                    }/*}}}*/
                                    else if(queueItem.u32Cmd == CMD_EMERGENCY_INVALID_CAMERA)
                                    {/*{{{*/
                                        if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
                                        {
                                            PP_U8 u8Status[4];
                                            PP_U32 u32Status = 0;

                                            memcpy(u8Status, queueItem.pData, sizeof(PP_U8)*4);

                                            //save.
                                            memcpy(u8CurInvalidCamCh, u8Status, sizeof(PP_U8)*4);

                                            //LOG_DEBUG("[Emergency Invalid Camera]:%d/%d/%d/%d\n", s8Status[0], s8Status[1], s8Status[2], s8Status[3]);

                                            //call svm function that control camera plug in-out. 
                                            //If camera plug-out, mask enable to black color(tp).
                                            //PPAPI_SVM_SetInputReplaceColorOnOff(stVidStatus.s8VidPort, (stVidStatus.s8Status == 0)?TRUE:FALSE);

                                            if( (stCurVidStatus[0].s8Status >= 0) && (stCurVidStatus[1].s8Status >= 0) && (stCurVidStatus[2].s8Status >= 0) && (stCurVidStatus[3].s8Status >= 0) )
                                            {
                                                if( (u8CurInvalidCamCh[0]>0) || (u8CurInvalidCamCh[1]>0) || (u8CurInvalidCamCh[2]>0) || (u8CurInvalidCamCh[3]>0) )
                                                {
                                                    u32Status = 0; //0:Invalid some camera on all camera plug-in.
                                                }
                                                else
                                                {
                                                    u32Status = 1; //1:valid all camera on all camera plug-in.
                                                }

                                                //TODO Processing Invalid camera.
                                                if(u32Status == 0) //invalid camera.
                                                {
                                                    LOG_DEBUG("[Emergency Invalid Camera]\n");
                                                }

#ifdef EMERGENCY_SAMPLE                                            
                                                // sample> display Diaglog Box
                                                {/*{{{*/
#if defined(USE_PP_GUI)
                                                    if(gUiLoadingDone <= 0)
                                                    {
                                                        //retry until gUiLoadingDone == TRUE.
                                                        //LOG_DEBUG("retry until gUiLoadingDone == TRUE\n");
                                                        AppTask_SendCmd(CMD_EMERGENCY_INVALID_CAMERA, (PP_U16)-1, TASK_EMERGENCY, 0, &stVidStatus, sizeof(PP_VID_S), 100);
                                                        OSAL_sleep(1000);
                                                    }
                                                    else
                                                    {
                                                        AppTask_SendCmd(CMD_UI_CAMERA_INVALID, TASK_EMERGENCY, TASK_UI, 0, &u32Status, sizeof(u32Status), 1000);
                                                    }
#else
                                                    {
                                                        AppTask_SendCmd(CMD_UI_CAMERA_INVALID, TASK_EMERGENCY, TASK_UI, 0, &u32Status, sizeof(u32Status), 1000);
                                                    }
#endif //defined(USE_PP_GUI)
                                                }/*}}}*/
#endif // EMERGENCY_SAMPLE                                            
                                            }
                                        }
                                    }/*}}}*/

									if(queueItem.u16Attr)
									{
										if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
										{
											if(queueItem.u32Cmd == CMD_EMERGENCY_ALIVE)
											{						
												AppTask_SendCmd(CMD_ACK, TASK_EMERGENCY, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
											}
											else
											{
												AppTask_SendCmd(CMD_ACK, TASK_EMERGENCY, queueItem.u16Sender, 0, NULL, 0, 1000);
											}
										}
									}
									if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
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
			        
                // recheck queue is empty.    
                if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                {
                    OSAL_EVENTGROUP_CMD_SetBits(gHandle[myHandleNum].phEventGroup, (1<<gu32TaskMsgEvent[myHandleNum]));
                }
            }
		}
	}

	return;
}

#include "type.h"
#include "error.h"

#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"
#include "task_calibration.h"
#include "api_FAT_FTL.h"
#include "api_svm.h"
#include "api_display.h"

#include "api_vpu.h"
#include "api_vin.h"

#include "dram.h"
#include"api_flash.h"
#if defined(USE_PP_GUI)
#include "application.h"
#endif

#ifdef CALIB_LIB_USE

#include "api_calibration.h"
#include "app_calibration.h"
#include "api_ipc.h"

extern PP_U32 multicore_calib_flag;

extern PP_U32 Core1_SectionNum;


extern PP_U8 gMulticore_Viewgen_Flag;
extern PP_U32 gMulticore_Viewgen_core1_done_Flag;

PP_VOID vTaskCalibration(PP_VOID *pvData)
{
	PP_S32 myHandleNum = TASK_CALIBRATION;
	PP_S32 timeOut = 1000; //msec
	EventBits_t eventWaitBits;
	EventBits_t eventResultBits;
	TaskParam_t *pstParam = (TaskParam_t *)pvData;

	PP_U32 i;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (PP_S32)uxTaskPriorityGet(NULL));

    PPAPI_FATFS_EnterFS();
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
                if(eventResultBits & (1<<EVENT_CALIBRATION_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_CALIBRATION_INIT, psEventNameCalibration[EVENT_CALIBRATION_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_CALIBRATION_MSG))
                {/*{{{*/
                    //LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_CALIBRATION_MSG, psEventNameCalibration[EVENT_CALIBRATION_MSG]);
                    {/*{{{*/
                        if(gHandle[myHandleNum].phQueue)
                        {
                            PP_S32 queueCnt = 0;
                            stQueueItem queueItem;
                            if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                            {
                                // LOG_DEBUG("get Queue cnt:%d\n", queueCnt);
                                for(i = 0; i < queueCnt; i++)
                                {
                                    if( OSAL_QUEUE_CMD_Receive(gHandle[myHandleNum].phQueue, &queueItem, 0) == eSUCCESS )
                                    {
                                        /*
                                           LOG_DEBUG("%s:Rcv cmd:%08x, sender:%d(%s), attr:0x%x, length:%d\n", psTaskName[myHandleNum],
                                           queueItem.u32Cmd, queueItem.u16Sender, psTaskName[queueItem.u16Sender], queueItem.u16Attr, queueItem.u32Length);
                                           */
                                        if(queueItem.u32Cmd == CMD_CALIBRATION_ALIVE)
                                        {						
                                        }
                                        else if(queueItem.u32Cmd == CMD_OFF_CALIB_START)
                                        {												
                                            PPAPP_Offcalib_Main(queueItem.u16Attr);		
                                            //PPAPP_Offcalib_Test_Code();
                                        }
                                        else if(queueItem.u32Cmd == CMD_VIEWGEN_START)
                                        {	                                      
											PPAPP_Section_Viewgen_Main(queueItem.u16Attr);											
                                        }
                                        
                                        else if(queueItem.u32Cmd == CMD_MULTICORE_VIEWGEN_START)
                                        {         
                                        	if(queueItem.u16Attr ==TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_UPDATE)
                                        	{	
                                        		PPAPP_Section_Viewgen_Get_Req_Core1_Update(Core1_SectionNum);
												PPAPI_IPC_Core0_Viewgen_Update_Ack(0xffffffff);
                                        	}
                                        	else if(queueItem.u16Attr ==TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_DONE)
                                        	{	
                                        		PPAPP_Section_Viewgen_Core1_Done();
                                        	}
                                        	
                                        }
                                        else if(queueItem.u32Cmd == CMD_CALIB_TEST)
                                        {               
											LOG_DEBUG("test calib multicore test\n");
                                        	multicore_calib_flag=1;
											PPAPI_Offcalib_Save_Cam_Ch(eCALIB_CAMERA_ALL);	
											PPAPP_Offcalib_Main(eOFFCALIB_START);
                                        }

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_CALIBRATION_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_CALIBRATION, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_CALIBRATION, queueItem.u16Sender, 0, NULL, 0, 1000);
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
                }/*}}}*/
            }
        }
    }
    PPAPI_FATFS_ReleaseFS();
    return;
}

#else // //CALIB_LIB_USE


#include "app_calibration.h"


PP_VOID vTaskCalibration(PP_VOID *pvData)
{
	PP_S32 myHandleNum = TASK_CALIBRATION;
	PP_S32 timeOut = 1000; //msec
	EventBits_t eventWaitBits;
	EventBits_t eventResultBits;
	TaskParam_t *pstParam = (TaskParam_t *)pvData;

	PP_U32 i;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (PP_S32)uxTaskPriorityGet(NULL));

    PPAPI_FATFS_EnterFS();
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
                if(eventResultBits & (1<<EVENT_CALIBRATION_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_CALIBRATION_INIT, psEventNameCalibration[EVENT_CALIBRATION_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_CALIBRATION_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_CALIBRATION_MSG, psEventNameCalibration[EVENT_CALIBRATION_MSG]);
                    {/*{{{*/
                        if(gHandle[myHandleNum].phQueue)
                        {
                            PP_S32 queueCnt = 0;
                            stQueueItem queueItem;
                            if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                            {
                                // LOG_DEBUG("get Queue cnt:%d\n", queueCnt);
                                for(i = 0; i < queueCnt; i++)
                                {
                                    if( OSAL_QUEUE_CMD_Receive(gHandle[myHandleNum].phQueue, &queueItem, 0) == eSUCCESS )
                                    {
                                        /*
                                           LOG_DEBUG("%s:Rcv cmd:%08x, sender:%d(%s), attr:0x%x, length:%d\n", psTaskName[myHandleNum],
                                           queueItem.u32Cmd, queueItem.u16Sender, psTaskName[queueItem.u16Sender], queueItem.u16Attr, queueItem.u32Length);
                                           */
                                        if(queueItem.u32Cmd == CMD_CALIBRATION_ALIVE)
                                        {						
                                        }

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_CALIBRATION_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_CALIBRATION, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_CALIBRATION, queueItem.u16Sender, 0, NULL, 0, 1000);
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
                }/*}}}*/
            }
        }
    }
    PPAPI_FATFS_ReleaseFS();
    return;
}




















#endif //CALIB_LIB_USE



#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"

#include "api_FAT_FTL.h"

#include "task_fs.h"

PP_VOID vTaskFS(PP_VOID *pvData)
{
    int myHandleNum = TASK_FS;
    int timeOut = 1000; //msec
    EventBits_t eventWaitBits;
    EventBits_t eventResultBits;
    TaskParam_t *pstParam = (TaskParam_t *)pvData;

	int i;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));
    
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
                if(eventResultBits & (1<<EVENT_FS_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_FS_INIT, psEventNameFS[EVENT_FS_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_FS_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_FS_MSG, psEventNameFS[EVENT_FS_MSG]);
                    {/*{{{*/
                        if(gHandle[myHandleNum].phQueue)
                        {
                            int queueCnt = 0;
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
                                        if(queueItem.u32Cmd == CMD_FS_ALIVE)
                                        {						
                                        }

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_FS_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_FS, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_FS, queueItem.u16Sender, 0, NULL, 0, 1000);
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

    return;
}

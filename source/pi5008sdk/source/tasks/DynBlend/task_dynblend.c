#ifdef DB_LIB_USE

#include "system.h"
#include "debug.h"
#include "osal.h"
#include "board_config.h"
#include "task_manager.h"

#include "task_dynblend.h"


#include "api_vpu.h"
#include "api_svm.h"
#include "ObjectDetector.h"

PP_VOID vTaskDynBlend(PP_VOID *pvData)
{
    int i;

    int myHandleNum = TASK_DYNBLEND;
    int timeOut = 1000; //msec
    EventBits_t eventWaitBits;
    EventBits_t eventResultBits;
    TaskParam_t *pstParam = (TaskParam_t *)pvData;

    //-------------------------------------------------------------------
    // variables for dynamic blending
    //-------------------------------------------------------------------
    ObjectDetectParam_t		odParam[TOTAL_ZONE_NUM];

    OptFlowPairPos_t * 		currMotionPair[TOTAL_ZONE_NUM];
    uint16_t				currNumMotion[TOTAL_ZONE_NUM] = { 0 };
    uint8_t					blendCorner[4] = { 255, 255, 255, 255 };

    uint16_t				outTotalNumMotion[TOTAL_ZONE_NUM] = { 0 };
    OptFlowPairPos_t * 		outMotionPair[TOTAL_ZONE_NUM];
    int iter;

    //-------------------------------------------------------------------
    _VPUStatus stVPUStatus;
    int runMax = 0;
    USER_VPU_FB_CONFIG_U stVPUFBConfig;
    uint32 u32TimeOut;
    int zoneNum = 0;

    uint32_t bDBLoop = 0;

    LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

    for (zoneNum = 0; zoneNum < eVPU_ZONE_MAX; zoneNum++)
    {
        if( (currMotionPair[zoneNum] = (OptFlowPairPos_t *)OSAL_malloc(sizeof(OptFlowPairPos_t)*VPU_MAX_HDMATCH)) == NULL)
        {
            LOG_CRITICAL("Error! fail allocate memory\n");
        }
        if( (outMotionPair[zoneNum] = (OptFlowPairPos_t *)OSAL_malloc(sizeof(OptFlowPairPos_t)*VPU_MAX_HDMATCH)) == NULL)
        {
            LOG_CRITICAL("Error! fail allocate memory\n");
        }
    }

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
                if(eventResultBits & (1<<EVENT_DYNBLEND_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_DYNBLEND_INIT, psEventNameDynBlend[EVENT_DYNBLEND_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_DYNBLEND_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_DYNBLEND_MSG, psEventNameDynBlend[EVENT_DYNBLEND_MSG]);
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
                                        if(queueItem.u32Cmd == CMD_DYNBLEND_ALIVE)
                                        {						
                                        }
                                        else if(queueItem.u32Cmd == CMD_DYNBLEND_OPER)
                                        {						
                                            uint32_t bEnable = *(uint32_t *)queueItem.pData;
                                            if(bEnable)
                                            {
                                                bDBLoop = PP_TRUE;
                                                PPAPI_SVM_SetDynamicBlendingOnOff(PP_TRUE);

                                                // set fast config
                                                {
                                                    PPAPI_VPU_FAST_GetConfig(&stVPUFBConfig);
                                                    //stVPUFBConfig.fld.fast_n = FAST_PX_NUM;
                                                    //stVPUFBConfig.fld.ch_sel = eVpuChannel;
                                                    //stVPUFBConfig.fld.use_5x3_nms = FALSE;
                                                    //stVPUFBConfig.fld.brief_enable = TRUE;
                                                    //stVPUFBConfig.fld.roi_enable = FALSE;
                                                    //stVPUFBConfig.fld.scl_enable = bScaleEnable;
                                                    stVPUFBConfig.fld.dma2otf_enable = FALSE;
                                                    stVPUFBConfig.fld.otf2dma_enable = FALSE;
                                                    PPAPI_VPU_FAST_SetConfig(&stVPUFBConfig);
                                                }
                                                //-------------------------------------------------------------------
                                                {
                                                    uint32_t odVer = 0;
                                                    ObjectDetector_Version(&odVer);
                                                    LOG_DEBUG(" ObjectDetector Ver:%d.%d.%d\n", odVer>>24&0xFF, odVer>>16&0xFF, odVer&0xFFFF);
                                                }

                                                ObjectDetector_InitParam(odParam);

                                                OSAL_EVENTGROUP_CMD_SetBits(gHandle[myHandleNum].phEventGroup, (1<<EVENT_DYNBLEND_OPER)); //self send event
                                            }
                                            else
                                            {
                                                bDBLoop = PP_FALSE;
                                                PPAPI_SVM_SetDynamicBlendingOnOff(PP_FALSE);
                                                PPAPI_VPU_FAST_Stop();
                                            }
                                        }

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_DYNBLEND_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_DYNBLEND, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_DYNBLEND, queueItem.u16Sender, 0, NULL, 0, 1000);
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
                if(eventResultBits & (1<<EVENT_DYNBLEND_OPER))
                {/*{{{*/
                    if(bDBLoop)
                    {
                        runMax = 1;
                        u32TimeOut = 1000;
                        //LOG_DEBUG("VPU_FAST start(%d)\n", runMax);
                        PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.
                        // When manual, 1frame + alpha.

                        PPAPI_VPU_GetStatus(&stVPUStatus);

                        if( (PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000)) == eSUCCESS)  //2~3msec 
                        {

                            //LOG_DEBUG("Get HamminD result of VPU.\n");
                            {/*{{{*/
                                pVPU_MATCHING_RESULT_POS_T hw_HD;
                                for (zoneNum = 0; zoneNum < eVPU_ZONE_MAX; zoneNum++)
                                {
                                    outTotalNumMotion[zoneNum] = stVPUStatus.u32HDMatchResultCount[zoneNum];

                                    //printf("zone:%d-%d\n", zoneNum, outTotalNumMotion[zoneNum]);
                                    hw_HD = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[zoneNum];
                                    for (i = 0; i < stVPUStatus.u32HDMatchResultCount[zoneNum]; i++)
                                    {
                                        /*
                                           LOG_DEBUG ("HW (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
                                           LOG_DEBUG ("Draw (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
                                           */

                                        outMotionPair[zoneNum][i].first.x = hw_HD[i].x1;
                                        outMotionPair[zoneNum][i].first.y = hw_HD[i].y1;
                                        outMotionPair[zoneNum][i].second.x = hw_HD[i].x2;
                                        outMotionPair[zoneNum][i].second.y = hw_HD[i].y2;
                                    }
                                }
                            }/*}}}*/

                            // process libOD function.
                            {/*{{{*/
                                uint32_t totalNumMotion = 0;
                                // After VPU operation, pass matching pair position and matching pair number at each zone.
                                // first - position for previous frame
                                // second - position for current frame

                                for (i = 0, totalNumMotion = 0; i < TOTAL_ZONE_NUM; i++) {
                                    currNumMotion[i] = outTotalNumMotion[i];
                                    totalNumMotion += currNumMotion[i];

                                    for (iter = 0; iter < currNumMotion[i]; iter++)
                                    {
                                        currMotionPair[i][iter].first.x = outMotionPair[i][iter].first.x;
                                        currMotionPair[i][iter].first.y = outMotionPair[i][iter].first.y;
                                        currMotionPair[i][iter].second.x = outMotionPair[i][iter].second.x;
                                        currMotionPair[i][iter].second.y = outMotionPair[i][iter].second.y;		
                                    }
                                }

                                odDynamicBlend (1, currMotionPair, currNumMotion, odParam, blendCorner);
                                //             IN      IN              IN           IN        OUT
                                // VPU??å¯ƒê³Œ??ï¿?odParam??è«›ì†ë¸?Top view ??corner??alpha mode ?°ì’•??(0, 64, 128, 192, 255(default)) --> blendCorner (SVM??alpha mode??è¹‚ï¿½å¯ƒì?ë¸????????

                                LOG_DEBUG("blendCorner(%04d)[LF(%3d),RF(%3d),LR(%3d),RR(%3d)]\n", totalNumMotion, blendCorner[0], blendCorner[1], blendCorner[2], blendCorner[3]);

                                PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_FRONTLEFT, blendCorner[0]);
                                PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_FRONTRIGHT, blendCorner[1]);
                                PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_REARLEFT, blendCorner[2]);
                                PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_REARRIGHT, blendCorner[3]);
                            }/*}}}*/
                        }

                        //OSAL_sleep(80); //check fastbrief time <= 2frame
                            
                        OSAL_EVENTGROUP_CMD_SetBits(gHandle[myHandleNum].phEventGroup, (1<<EVENT_DYNBLEND_OPER)); //self send event
                    }
                    else
                    {
                        LOG_DEBUG("Stop Dynmic Blending\n");
                    }

                }/*}}}*/
            }
        }
    }
    for (zoneNum = 0; zoneNum < eVPU_ZONE_MAX; zoneNum++)
    {
        OSAL_free(currMotionPair[zoneNum]);
        OSAL_free(outMotionPair[zoneNum]);
    }


    return;
}

#endif // DB_LIB_USE

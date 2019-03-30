#include <stdio.h>
#include <stdarg.h>
#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"
#include "dma.h"
#include "task_uartcon.h"
#include "task.h"
#include "ring_buffer.h"
#include "sys_api.h"
#include "common.h"

static SemaphoreHandle_t gUART_DMA_Lock = NULL;
static sys_os_mutex_t gstMutex;


#define UART_CONSOLE_RING_BUFFER	1
#define UART_CONSOLE_CMD_QUEUE		2
#define UART_CONSOLE_DIRECT			3

#define UART_CONSOLE_MODE	UART_CONSOLE_DIRECT


#if ( UART_CONSOLE_MODE == UART_CONSOLE_RING_BUFFER)
#define RING_BUFFER_SIZE			(32*1024)
#define PRINT_SIZE					(256)
HANDLE *gpBufHandle;
PP_U8 *gpu8PrnBuf;
#endif


void uart_dbg_tx_isr(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// uart fifo empty : transfer done -> irq disable
	PPDRV_UART_SetIER(eUART_CH_0, 0);
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(gUART_DMA_Lock, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE)
    {
		portYIELD_FROM_ISR();
    }
}

static PP_VOID DMA_ISR(PP_U8 ch, PP_U32 event)
{
#if 0
	if(ch == eDMA_CHANNEL_UART0){
		if(event == DMA_EVENT_DONE){
			// dma done -> enable uart fifo empty irq
			PPDRV_UART_SetIER(eUART_CH_0, UART_IER_ETHEI);
		}
	}
#endif
}

static PP_VOID UART_PutStrDMA(const PP_CHAR *pBuf, PP_U32 size)
{
	//dma_set_callback(eDMA_CHANNEL_UART0, DMA_ISR);
	PPAPI_SYS_CACHE_Writeback((uint32 *)pBuf, (size + 15) & (~0xf));
	if(PPDRV_UART_SetDmaTx (eUART_CH_0, eTX_LEVEL_2, (const PP_U8 *)pBuf, size, eDMA_CHANNEL_UART0, 3000) == eSUCCESS){
		// dma done -> enable uart fifo empty irq
		PPDRV_UART_SetIER(eUART_CH_0, UART_IER_ETHEI);

		if( xSemaphoreTake(gUART_DMA_Lock, 3000) == pdFALSE )
		{
			printf("Error!!! UART console put string fail. Timeout\n");
		}
	}else{
		printf("Error!!! UART dma fail\n");
	}
}

static PP_VOID UART_PutStr(const PP_CHAR *pBuf, PP_U32 size)
{
	PP_U32 i;

	for(i=0;i<size;i++)
		PPDRV_UART_OutByte(eUART_CH_0, pBuf[i]);
}

// Bottom half
void uartcon_deferred_handler(void *param1, uint32_t param2)
{
	eTaskState enState;
	PP_U32 len;
//	PP_RESULT_E result;
	PP_CHAR *pszBuf = (PP_CHAR *)param1;

	if(gHandle[TASK_UARTCON].phQueue == NULL){
		printf("%s", pszBuf);
		return;
	}

	if(gHandle[TASK_UARTCON].phTask){
		enState = eTaskGetState(gHandle[TASK_UARTCON].phTask);
		if((enState == eSuspended) || (enState == eDeleted)){
			printf("%s", pszBuf);
			return;
		}

	}

	len = strlen(pszBuf);
	if(len >0 && pszBuf[len-1] == '\n'){
		pszBuf[len-1] = '\r';
		pszBuf[len] = '\n';
		len++;
	}

#if ( UART_CONSOLE_MODE == UART_CONSOLE_RING_BUFFER)
	do{
		OSAL_wait_for_mutex(&gstMutex);
		result = PPUTIL_RINGBUFF_WriteData(gpBufHandle, pszBuf, len);
		OSAL_release_mutex(&gstMutex);
		if(result != eSUCCESS){
			// Write data fail ???
		}
	//}while(result != eSUCCESS);
	}while(0);
	OSAL_EVENTGROUP_CMD_SetBits(gHandle[TASK_UARTCON].phEventGroup, (1<<EVENT_UARTCON_PRINT));
#elif( UART_CONSOLE_MODE == UART_CONSOLE_CMD_QUEUE)
	AppTask_SendCmd(CMD_UARTCON_DEBUG_PRINT, (PP_U16)-1, TASK_UARTCON, 0, pszBuf, len, SYS_OS_SUSPEND);
#elif( UART_CONSOLE_MODE == UART_CONSOLE_DIRECT)
	OSAL_wait_for_mutex(&gstMutex);
	UART_PutStr(pszBuf, len);
	OSAL_release_mutex(&gstMutex);
#endif

}



PP_VOID vTaskUARTCon(PP_VOID *pvData)
{
    PP_S32 myHandleNum = TASK_UARTCON;
    PP_S32 timeOut = 1000; //msec
    EventBits_t eventWaitBits;
    EventBits_t eventResultBits;
//    PP_RESULT_E result;
//    PP_U32 u32Size;
	PP_S32 i;
	TaskParam_t *pstParam = (TaskParam_t *)pvData;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

    for(;;)
    {
    	pstParam->u32TaskLoopCnt++;

        //printf("WaitBits(%s)\n", __FUNCTION__);
        if(gHandle[myHandleNum].phEventGroup)
        {
            eventWaitBits = 0x00FFFFFF; //0x00FFFFFF all bit
            eventResultBits = OSAL_EVENTGROUP_CMD_WaitBits(gHandle[myHandleNum].phEventGroup, eventWaitBits, pdTRUE, pdFALSE, timeOut);

            if ( eventResultBits == 0 )
            {
                //printf("timeout\n");
                continue;
            }
            else
            {
                // process event bit
                if(eventResultBits & (1<<EVENT_UARTCON_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_UARTCON_INIT, psEventNameUartCon[EVENT_UARTCON_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_UARTCON_MSG))
                {/*{{{*/
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_UARTCON_MSG, psEventNameUartCon[EVENT_UARTCON_MSG]);
                    {/*{{{*/
                        if(gHandle[myHandleNum].phQueue)
                        {
                            int queueCnt = 0;
                            stQueueItem queueItem;
                            if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                            {
                                //printf("get Queue cnt:%d\n", queueCnt);
                                for(i = 0; i < queueCnt; i++)
                                {
                                    if( OSAL_QUEUE_CMD_Receive(gHandle[myHandleNum].phQueue, &queueItem, 0) == eSUCCESS )
                                    {
                                        /*
                                           printf("%s:Rcv cmd:%08x, sender:%d(%s), attr:%x, length:%d\n", psTaskName[myHandleNum],
                                           queueItem.u32Cmd, queueItem.u16Sender, psTaskName[queueItem.u16Sender], queueItem.u16Attr, queueItem.u32Length);
                                           */
                                        if(queueItem.u32Cmd == CMD_UARTCON_ALIVE)
                                        {						
                                        }
                                        else if(queueItem.u32Cmd == CMD_UARTCON_DEBUG_PRINT){
                                            if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) ){
                                                //printf("%s", queueItem.pData);
                                                UART_PutStrDMA(queueItem.pData, queueItem.u32Length);
                                            }
                                        }

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_UARTCON_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_UARTCON, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_UARTCON, queueItem.u16Sender, 0, NULL, 0, 1000);
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
                                //printf("Don't remain queue.\n");
                            }
                        }
                        else
                        {
                            printf("Invalid handle.\n");
                        }

                    }/*}}}*/
                }/*}}}*/
#if ( UART_CONSOLE_MODE == UART_CONSOLE_RING_BUFFER)
                if(eventResultBits & (1<<EVENT_UARTCON_PRINT))
                {/*{{{*/
                    u32Size = PPUTIL_RINGBUFF_GetBufferedSize(gpBufHandle);

                    if(u32Size > 0){
                        u32Size = (u32Size > PRINT_SIZE ? PRINT_SIZE : u32Size);
                        OSAL_wait_for_mutex(&gstMutex);
                        result = PPUTIL_RINGBUFF_ReadData(gpBufHandle, gpu8PrnBuf, u32Size);
                        OSAL_release_mutex(&gstMutex);
                        if( result != eSUCCESS){
                            printf("[%s]Get buffer fail\n", __FUNCTION__);
                        }else{
                            UART_PutStrDMA(gpu8PrnBuf, u32Size);
                            //UART_PutStr(gpu8PrnBuf, u32Size);
                        }

                        if(PPUTIL_RINGBUFF_GetBufferedSize(gpBufHandle) > 0){
                            OSAL_EVENTGROUP_CMD_SetBits(gHandle[TASK_UARTCON].phEventGroup, (1<<EVENT_UARTCON_PRINT));
                        }
                    }


                }/*}}}*/
#endif
            }
        }
    }

    return;
}


PP_VOID AppUARTCon_Initialize(PP_UART_BAUDRATE_E enBaudRate, PP_UART_DATABIT_E enDataBit, PP_UART_STOP_BIT_E enStopBit, PP_UART_PARITY_E enParity)
{
	SYSAPI_UART_initialize(eUART_CH_0, enBaudRate, enDataBit, enStopBit, enParity);
	if( (gUART_DMA_Lock = xSemaphoreCreateBinary()) == NULL)
	{
		printf("[%s]can't create lockDMA\n", __FUNCTION__);
	}

	if(OSAL_create_mutex(&gstMutex) != eSUCCESS){
		printf("[%s]can't create mutex\n", __FUNCTION__);
	}

#if ( UART_CONSOLE_MODE == UART_CONSOLE_RING_BUFFER)
	gpBufHandle = PPUTIL_RINGBUFF_Initialize(RING_BUFFER_SIZE);
	gpu8PrnBuf = (PP_U8 *)OSAL_malloc(PRINT_SIZE);
#endif

}

PP_VOID AppUARTCon_DebugPrint(PP_CHAR *pszFormat, ...)
{
	va_list ap;
	PP_CHAR szBuf[512];
	PP_U32 len;
	eTaskState enState;
//	PP_RESULT_E result;

	va_start(ap, pszFormat);
	vsprintf(szBuf, pszFormat, ap);
	va_end(ap);

#if 0 //debugging. because of sometime unstable.
    printf("%s", szBuf);
    return;
#endif

	if(gHandle[TASK_UARTCON].phQueue == NULL){
		printf("%s", szBuf);
		return;
	}

	if(gHandle[TASK_UARTCON].phTask){
		enState = eTaskGetState(gHandle[TASK_UARTCON].phTask);
		if((enState == eSuspended) || (enState == eDeleted)){
			printf("%s", szBuf);
			return;
		}

	}

	len = strlen(szBuf);
	if(len >0 && szBuf[len-1] == '\n'){
		szBuf[len-1] = '\r';
		szBuf[len] = '\n';
		len++;
	}

#if ( UART_CONSOLE_MODE == UART_CONSOLE_RING_BUFFER)
	do{
		OSAL_wait_for_mutex(&gstMutex);
		result = PPUTIL_RINGBUFF_WriteData(gpBufHandle, szBuf, len);
		OSAL_release_mutex(&gstMutex);
		if(result != eSUCCESS){
			// Write data fail ???
		}
	//}while(result != eSUCCESS);
	}while(0);
	OSAL_EVENTGROUP_CMD_SetBits(gHandle[TASK_UARTCON].phEventGroup, (1<<EVENT_UARTCON_PRINT));
#elif ( UART_CONSOLE_MODE == UART_CONSOLE_CMD_QUEUE)
	AppTask_SendCmd(CMD_UARTCON_DEBUG_PRINT, (PP_U16)-1, TASK_UARTCON, 0, szBuf, len, SYS_OS_SUSPEND);
#elif ( UART_CONSOLE_MODE == UART_CONSOLE_DIRECT)
	OSAL_wait_for_mutex(&gstMutex);
	UART_PutStr(szBuf, len);
	OSAL_release_mutex(&gstMutex);
#endif

}


PP_VOID AppUARTCon_DebugPrintFromISR(PP_CHAR *pszFormat, ...)
{
	va_list ap;
	PP_CHAR szBuf[512];
	BaseType_t xHigherPriorityTaskWoken;

	va_start(ap, pszFormat);
	vsprintf(szBuf, pszFormat, ap);
	va_end(ap);

#if 0 //debugging. because of sometime unstable.
    printf("%s", pszBuf);
    return;
#endif

	xHigherPriorityTaskWoken = pdFALSE;
    xTimerPendFunctionCallFromISR( uartcon_deferred_handler, (void *)szBuf, 0, &xHigherPriorityTaskWoken );
    if(xHigherPriorityTaskWoken == pdTRUE){
    	portYIELD_FROM_ISR();
    }

}



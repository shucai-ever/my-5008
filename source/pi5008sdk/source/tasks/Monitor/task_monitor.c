#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include <ctype.h>

#include "type.h"
#include "error.h"

#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"

#include "task_monitor.h"
#include "app_calibration.h"
#include "api_calibration.h"
#include "api_display.h"
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
#include "api_pvirx_func.h"
#endif //(VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
#include "task_calibration.h"
#include "wdt.h"
#include "standby.h"

#if MCU_CTRL_METHOD
#include "spi.h"
#include "task_fwdn.h"
#endif

#define MONITOR_TASK_TEST					0
#define PAR_VIN_TEST						0

#if MONITOR_TASK_TEST
#define MONITOR_TASK_WAKEUP_TIME_MS			100	//20
#else
#define MONITOR_TASK_WAKEUP_TIME_MS			20
#endif
char uart_temp[1024];
//static PP_S16 steerAngle = 0;

#if MCU_CTRL_METHOD
sint8 AppTask_SendCmd_To_UI_Remocon_Cmd_SPI(char *rCmd, uint32 rLen);
#else
sint8 AppTask_SendCmd_To_UI_Remocon_Cmd(char *rCmd, uint32 rLen);
#endif

static sint8 get_field(uint8 *data);
static sint8 rx_req_reg_action(void);
static sint8 rx_req_remote_action(void);
static sint8 iget_field(uint8 *data);
static uint8 get_byte(void);
inline static unsigned long simple_strtoul(const char *cp, unsigned int base);
sint8 tx_resp_remote(uint8 *action, sint8 result);
static void put_byte(uint8 c);
static void put_bytes(uint8 *buf, uint32 size);

#ifdef FWDN_TEST
PP_U32 gFWDN_TEST_FLAG = 1;
#endif


PP_U32 gDBG_PRINT_MSG = 0;
//extern PP_U32 gMulticore_Viewgen_core1_done_Flag;
//extern PP_U32 gMulticore_Viewgen_Flag;

#if PAR_VIN_TEST
PP_U32 gTempCnt = 0;

PP_VOID TempSetPARVINReg(PP_VOID)
{
    if(gTempCnt < 50){
    	gTempCnt++;
    }else if(gTempCnt == 50){
    	gTempCnt++;

    	// vin reg setting
		*(vuint32 *)0xf0f00000=0x04040404;
		*(vuint32 *)0xf0f00040=0x950002d0;
		*(vuint32 *)0xf0f00048=0x2ee;
		*(vuint32 *)0xf0f00080=0x950002d0;
		*(vuint32 *)0xf0f00088=0x2ee;

		// svm bypass
		*(vuint32 *)0xf0800000=0x202;

		// osd off
		*(vuint32 *)0xF090580C=0;
		*(vuint32 *)0xF090588C=0;
		*(vuint32 *)0xF090590C=0;
		*(vuint32 *)0xF090598C=0;
		*(vuint32 *)0xF090600C=0;
		*(vuint32 *)0xF090608C=0;
		*(vuint32 *)0xF090610C=0;
		*(vuint32 *)0xF090618C=0;
		*(vuint32 *)0xF090680C=0;
		*(vuint32 *)0xF090688C=0;
		*(vuint32 *)0xF090690C=0;
		*(vuint32 *)0xF090698C=0;
		*(vuint32 *)0xF090700C=0;
		*(vuint32 *)0xF090708C=0;
		*(vuint32 *)0xF090710C=0;
		*(vuint32 *)0xF090718C=0;
		*(vuint32 *)0xF090780C=0;
		*(vuint32 *)0xF090788C=0;
		*(vuint32 *)0xF090790C=0;
		*(vuint32 *)0xF090798C=0;

    }

}

#endif

static PP_VOID TaskMonitorTimerHandler(TimerHandle_t timer)
{

#if WATCHDFOG_ENABLE
	PPDRV_WDT_KeepAlive();
#endif

    configASSERT(timer);

    xEventGroupSetBits(gHandle[TASK_MONITOR].phEventGroup, (1<<EVENT_MONITOR_TIMER_WAKEUP));

#if PAR_VIN_TEST
    TempSetPARVINReg();
#endif

    return;
}

static PP_VOID SetTaskMonitorTimer(void)
{
    TimerHandle_t timer = NULL;

    timer = xTimerCreate("Monitor Task Timer", OSAL_ms2ticks(MONITOR_TASK_WAKEUP_TIME_MS), pdTRUE, (void*)NULL, TaskMonitorTimerHandler);
    configASSERT(timer);
    configASSERT(xTimerStart(timer, 0) == pdPASS);

    LOG_DEBUG("monitor task timer created. handler: 0x%x, id: %d\n", (uint32)timer, (int)pvTimerGetTimerID(timer));
}


#if MONITOR_TASK_TEST
static PP_VOID TempSendMsgTest(PP_U16 u32RecvTask)
{

	PP_U8 u8DataTemp[256];
	PP_U32 u32DataLen;

	sprintf((PP_CHAR *)u8DataTemp, "Test data from task%d to task%d\n", (PP_S32)TASK_MONITOR, (PP_S32)u32RecvTask);

	//u32DataLen = ((strlen(u8DataTemp) + (GetTickCount() & 0x3f) + 3) & (~3));
	u32DataLen = strlen((PP_CHAR*)u8DataTemp) + 1;
	AppTask_SendCmd(1, TASK_MONITOR, u32RecvTask, (1<<QUEUE_CMDATTR_BIT_REQACK), u8DataTemp, u32DataLen, 1000);

	LOG_DEBUG("Test msg sent to Task%d\n", u32RecvTask);
}
#endif

PP_VOID vTaskMonitor(PP_VOID *pvData)
{
    PP_S32 myHandleNum = TASK_MONITOR;//2
    //PP_S32 timeOut = 1000; //msec
    PP_S32 timeOut = SYS_OS_SUSPEND; //msec -1
    EventBits_t eventWaitBits;//无符号长整型
    EventBits_t eventResultBits;
    TaskParam_t *pstParam = (TaskParam_t *)pvData;
    //1.void* pvParameters; 2.usingned int(u32TaskLoopCnt)

    PP_S32 i;
#if MONITOR_TASK_TEST
    PP_S32 s32TaskCnt = 0;
#endif

    LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

    SetTaskMonitorTimer();

#if WATCHDFOG_ENABLE
    PPDRV_WDT_SetEnable(1);
#endif

    InitStandbyMode();//空函数

    for(;;)
    {
    	pstParam->u32TaskLoopCnt++;

	    //LOG_DEBUG("WaitBits(%s)\n", __FUNCTION__);
        if(gHandle[myHandleNum].phEventGroup)
        	//1.任务句柄 2.队列句柄 3.事件组句柄   void*
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
                if(eventResultBits & (1<<EVENT_MONITOR_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_MONITOR_INIT, psEventNameMonitor[EVENT_MONITOR_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_MONITOR_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_MONITOR_MSG, psEventNameMonitor[EVENT_MONITOR_MSG]);
                    {
                        if(gHandle[myHandleNum].phQueue)
                        {/*{{{*/
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
                                        if(queueItem.u32Cmd == CMD_MONITOR_ALIVE)
                                        {						
                                        }
                                        else if(queueItem.u32Cmd == CMD_GET_REMOCON)
                                        {/*{{{*/

#ifdef CALIB_LIB_USE
                                            if( (queueItem.u32Length > 0) && (queueItem.pData != NULL)&& (PPAPI_Offcalib_Get_Step_Send_Cmd()==eOFFCALIB_WAIT_CMD)&& (PPAPI_Section_Viewgen_Get_Step_Send_Cmd()==eSVIEWGEN_WAIT_CMD)){
#else
                                            	if( (queueItem.u32Length > 0) && (queueItem.pData != NULL)){
#endif
#if MCU_CTRL_METHOD
                                            		AppTask_SendCmd_To_UI_Remocon_Cmd_SPI(queueItem.pData,queueItem.u32Length);
#else
													LOG_DEBUG("[rem_cmd]:%s\n", (char *)queueItem.pData);
                                            		AppTask_SendCmd_To_UI_Remocon_Cmd(queueItem.pData,queueItem.u32Length);
                                            		memset(uart_temp,0,1024);
#endif
                                            }

                                        }/*}}}*/                                        
                                        else if(queueItem.u32Cmd == CMD_GET_WELTREND)
                                        {/*{{{*/
                                            if((queueItem.u32Length > 0) && (queueItem.pData != NULL))
											{
												LOG_DEBUG("[weltrend_cmd]:");
												for(i=0;i<queueItem.u32Length;i++)
												{
													//LOG_DEBUG(" %X",(char *)*queueItem.pData[i]);
												}

											}		
                                        }/*}}}*/
                                        else if(queueItem.u32Cmd == CMD_DBG_PRINT_MSG_ON)
                                        {/*{{{*/
                                            gDBG_PRINT_MSG = 1;
                                        }/*}}}*/
                                        else if(queueItem.u32Cmd == CMD_DBG_PRINT_MSG_OFF)
                                        {/*{{{*/
                                            gDBG_PRINT_MSG = 0;
                                        }/*}}}*/
                                        else if(queueItem.u32Cmd == CMD_MONITOR_CAMERA_PLUG)
                                        {/*{{{*/
                                            if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
                                            {
                                                PP_U32 u32StatusPlug=0;
                                                u32StatusPlug = *(PP_U32 *)queueItem.pData;
                                                LOG_DEBUG("TODO!. [rcv_cmd]:%08x\n", u32StatusPlug);
                                            }
                                        }/*}}}*/
                                        else if(queueItem.u32Cmd == CMD_MONITOR_STANDBY)
                                        {/*{{{*/
                                            EnterStandbyMode();
                                        }/*}}}*/

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_MONITOR_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_MONITOR, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    AppTask_SendCmd(CMD_ACK, TASK_MONITOR, queueItem.u16Sender, 0, NULL, 0, 1000);
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
                        }/*}}}*/
                        else
                        {
                            LOG_CRITICAL("Invalid handle.\n");
                        }
                    }
                }            /*}}}*/
                if(eventResultBits & (1<<EVENT_MONITOR_TIMER_WAKEUP))
                {		
#ifdef CALIB_LIB_USE
                	PPAPI_Lib_Progressbar_Processing();   
#endif

                    if(gDBG_PRINT_MSG)LOG_DEBUG("Event EVENT_MONITOR_WAKEUP\n");
                    {/*{{{*/

//#if MONITOR_TASK_TEST
//
//                        if(s32TaskCnt != TASK_MONITOR)TempSendMsgTest(s32TaskCnt);
//                        if(++s32TaskCnt >= MAX_TASK)s32TaskCnt = 0;
//#endif
//
//#ifdef FWDN_TEST
//                        if(gFWDN_TEST_FLAG > 0){
//                            strcpy(stFWDNParam.szFileName, "flash_image.bin");
//                            stFWDNParam.enFlashType = eFLASH_TYPE_NOR;
//                            stFWDNParam.u32PageSize = gstFlashNandID.u32PageSize;
//                            stFWDNParam.u32EraseBlockSize = gstFlashNandID.u32EraseBlockSize;
//                            if(AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_MONITOR, TASK_UI, 0, (PP_VOID *)&stFWDNParam, sizeof(stFWDNParam), 3000) != eSUCCESS){
//                                LOG_CRITICAL("MONITOR TASK. Send CMD_UI_KEY_CENTER Fail !!!!\n");
//                            }
//
//                            gFWDN_TEST_FLAG--;
//                        }
//#endif

                    }/*}}}*/
                }/*}}}*/
            }
        }
    }

    return;
}


#define SEPERATOR_SP			' '
#define SEPERATOR_LF			'\n'
#define SEPERATOR_US			0x1F
uint32 gRxCheckSum = 0;

//#if MCU_CTRL_METHOD
//
//static inline PP_U32 CalcChecksum(PP_U32 *pBuf, PP_U32 u32WordSize)
//{
//	PP_U32 u32CheckSum = 0;
//	PP_U32 i;
//
//	for(i=0;i<u32WordSize;i++){
//		u32CheckSum += pBuf[i];
//	}
//
//	return u32CheckSum;
//}
//
//void rx_req_remote_action_spi(PP_U8 *spi_buf)
//{
//	switch(spi_buf[2])
//	{
//		case REMOCON_UP:
//		case REMOCON_UPLONG:
//		case REMOCON_DOWN:
//		case REMOCON_DOWNLONG:
//		case REMOCON_LEFT:
//		case REMOCON_LEFTLONG:
//		case REMOCON_RIGHT:
//		case REMOCON_RIGHTLONG:
//		case REMOCON_CENTER:
//		case REMOCON_CENTERLONG:
//		case REMOCON_MENU:
//		case REMOCON_MENULONG:
//			LOG_DEBUG("%d\n", spi_buf[2] + 3);
//			AppTask_SendCmd(spi_buf[2] + 3, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
//			break;
//
//		case REMOCON_NAVI:
//			LOG_DEBUG("NAVI!!!\n");
//			AppTask_SendCmd(CMD_DISPLAY_PGL_ANGLE_TEST, TASK_MONITOR, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
//			break;
//
//		case REMOCON_NAVILONG:
//			LOG_DEBUG("NAVI LONG!!!\n");
//			break;
//		default:
//			LOG_DEBUG("Not Support!!\n");
//			break;
//	}
//}
//
//sint8 AppTask_SendCmd_To_UI_Remocon_Cmd_SPI(char *rCmd, uint32 rLen)
//{
//	int ret;
//	int i;
//
//	PP_U8 spi_buf[1024];
//	PP_U32 checksum;
//	PP_U32 checksum_calc;
//
//	memcpy(spi_buf,rCmd,rLen);
//	memcpy(&checksum, &spi_buf[8],4);
//
//#if 0
//	for( i = 0; i < rLen; i++ )
//		LOG_DEBUG("spi_buf[%d] = %02x\n", i, spi_buf[i]);
//#endif
//
//	checksum_calc = CalcChecksum((PP_U32 *)spi_buf, (( PACKET_HDR_SIZE ) >> 2));
//
//	if( spi_buf[0] != EXT_BOOT_SOF )
//	{
//		LOG_DEBUG("EXT_BOOT_SOF Error!!\n");
//		PPDRV_SPI_BufferClear(eSPI_CHANNEL_1);
//		ret = -1;
//		goto END_FT;
//	}
//
//	if( checksum_calc != checksum )
//	{
//		ret = -1;
//		LOG_DEBUG("Checksum Error!!! checksum : 0x%x, checksum_calc : 0x%x\n", checksum, checksum_calc);
//		goto END_FT;
//	}
//	else
//		LOG_DEBUG("Checksum OK!!\n");
//
//	if( spi_buf[1] == CMD_REMOCON_CONTROL )
//	{
//		rx_req_remote_action_spi( spi_buf );
//	}
//	else if( spi_buf[1] == CMD_UPGRADE_READY )
//	{
//		PPDRV_SPI_IRQClear(eSPI_CHANNEL_1, SPI_INTR_RXFIFO_THRD|SPI_INTR_TRANSFER_END|SPI_INTR_RXFIFO_OVER);
//#if(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
//		FlashUpdateSPI();
//#endif
//		//FlashUpdate( NULL, 0 );
//		PPDRV_SPI_BufferClear(eSPI_CHANNEL_1);
//		PPDRV_SPI_IRQEnable(eSPI_CHANNEL_1, SPI_INTR_RXFIFO_THRD|SPI_INTR_TRANSFER_END|SPI_INTR_RXFIFO_OVER);
//	}
//	else
//	{
//		LOG_DEBUG("Not Support!!\n");
//		ret = -1;
//	}
//
//END_FT:
//	memset(spi_buf, 0, 1024);
//	return ret;
//}
//#endif  // MCU_CTRL_METHOD

sint8 AppTask_SendCmd_To_UI_Remocon_Cmd(char *rCmd, uint32 rLen)
{
	uint8 field[1024];
	//uint32 checksum;
	sint8 ret;
	//sint8 sep;

	
	//temp copy
	memcpy(uart_temp,rCmd,rLen);

	
	gRxCheckSum = 0;

	//sep = get_field((uint8 *)field);
	get_field((uint8 *)field);

	if(field[0] != 't')goto END_FT;
	

	//sep = get_field((uint8 *)field);
	get_field((uint8 *)field);
    if(strcmp((const char *)field, "reg") == 0){
		ret = rx_req_reg_action();
	}else if(strcmp((sint8 *)field, "con") == 0){
		//sep = get_field((uint8 *)field);
		get_field((uint8 *)field);
		if(strcmp((sint8 *)field, "rem") == 0){
			rx_req_remote_action();
		}else if(strcmp((sint8 *)field, "tou") == 0){
			// rx_req_touch_action();
		}else{
			// error
			ret = -1;
		}
	}else if(strcmp((sint8 *)field, "file") == 0){	/// ?????

	}else{
		// error
		ret = -1;
	}


	END_FT:
		return ret;

	
}



static sint8 get_field(uint8 *data)
{
	sint8 sep;
	uint32 i;
	uint32 size;

	sep = iget_field(data);
	if(sep != SEPERATOR_LF){
		size = strlen((sint8 *)data);
		for(i=0;i<size;i++){
			gRxCheckSum += (uint32)data[i];
			//printf("%c(0x%x)\n", data[i], data[i]);
		}

		gRxCheckSum += sep;
		//printf("%c(0x%x)\n", sep);
	}


	return sep;
}


static sint8 iget_field(uint8 *data)
{
	uint32 i = 0;
	sint8 val;

	do{
		val = get_byte();

		if((val == SEPERATOR_SP) || (val == SEPERATOR_LF) || (val == SEPERATOR_US)){
			data[i] = 0;
			break;
		}

		data[i] = val;
		i++;
	}while(1);

	return val;

}

static uint8 get_byte()
{
	uint8 val;

	val = uart_temp[0];
	
	memmove(uart_temp,uart_temp+1,sizeof(char)*1023);

	return val;
}

int my_atoi(const char *src)
{

	int flag = 1;
	int ret = 0;
	while (*src == ' ')                           //跳过空格
	{
		src++;
	}
	if (*src == '\0')                           //如果这时候字符串已经完了，则直接返回
	{
		return  0;
	}
	if (*src == '+')                            //接下来判断正负
	{
		src++;
	}
	else if (*src == '-')
	{
		flag = -1;
		src++;
	}
	while (*src >= '0'&&*src <= '9')             //读取字符数字转换成整型
	{
		ret = ret * 10 + flag*(*src - '0');
		src++;
	}
	return (int)ret;
}



static sint8 rx_req_remote_action(void)//t con rem 2d         up    checksum "\n"
{                                      //t con rem 3d         value checksum "\n"
									   //t con rem steerangle value checksum "\n"



	sint8 field[16]="";
	sint8 action[16]="";
	sint8 sep=0;
	sint8 ret = 1;
	uint32 checksum;
	sint8 type[16]="";//存储类型信息

	sint16 angle_value = 0;//角度值初始化为0
	sint8 angle[16];//角度值

	get_field((uint8*)type);//获取2d/3d/steerangle

	if(strcmp(type,"2d")==0){
		sep = get_field((uint8 *)action);//获取上下左右按键
	}
	else {
		get_field((uint8*)angle);//读取角度字符串
		angle_value = (sint16)my_atoi(angle);//转换角度值
	}
	if(strcmp(action, "center") == 0){
		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem center cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			ret = -1;
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
		}

	}
	else if(strcmp(action, "centerlong") == 0){

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_CENTER_LONG, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem centerlong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;

		}

	}
	else if(strcmp(action, "up") == 0){

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_UP, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem up cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;

		}

	}

	else if(strcmp(action, "uplong") == 0){

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_UP_LONG, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem uplong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;

		}

	}
	else if(strcmp(action, "down") == 0){

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_DOWN, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem down cs:%s/0x%x(%s)\n",field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}

	}
	else if(strcmp(action, "downlong") == 0){

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_DOWN_LONG, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem downlong cs:%s/0x%x(%s)\n",field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}

	}
	else if(strcmp(action, "left") == 0){
	

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_LEFT, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem left cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}

	}
	else if(strcmp(action, "leftlong") == 0){
	
	
		sep = get_field((uint8 *)field);

		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);

			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_DISPLAY_STEER_ANGLE, TASK_MONITOR, TASK_DISPLAY, angle_value, NULL, 0, 1000);

			}
			LOG_DEBUG("Rx Req. cmd: con rem leftlong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}

	}
	else if(strcmp(action, "right") == 0){

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_RIGHT, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem right cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}

	}
	else if(strcmp(action, "rightlong") == 0){
		sep = get_field((uint8 *)field);

		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_DISPLAY_STEER_ANGLE, TASK_MONITOR, TASK_DISPLAY, angle_value, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem rightlong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}

	}
	else if(strcmp(action, "navi") == 0)
	{
		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				#if 0
				PPAPI_Offcalib_Save_Cam_Ch(eCALIB_CAMERA_ALL);
				AppTask_SendCmd(CMD_CALIB_TEST, TASK_MONITOR, TASK_CALIBRATION, eOFFCALIB_START, NULL_PTR, 0, 1000);
				#else
				// Auto dynamic PGL test for demo
				AppTask_SendCmd(CMD_DISPLAY_PGL_ANGLE_TEST, TASK_MONITOR, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
				#endif
			}
			LOG_DEBUG("Rx Req. cmd: con rem navi cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}
	}	
	else if(strcmp(action, "navilong") == 0)
	{
		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{	
				#ifdef CALIB_LIB_USE

				#endif //CALIB_LIB_USE
			}
			LOG_DEBUG("Rx Req. cmd: con rem navilong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}
	}	
	else if(strcmp(action, "menu") == 0)
	{

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_MENU, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem menu cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}
	}	
	else if(strcmp(action, "menulong") == 0)
	{

		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				AppTask_SendCmd(CMD_UI_KEY_MENU_LONG, TASK_MONITOR, TASK_UI, 0, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem menulong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}
	}

	else if(strcmp(type, "3d")==0){


		sep = get_field((uint8 *)field);
		if(sep == SEPERATOR_LF){
			checksum = simple_strtoul(field, 0);
			if(checksum != gRxCheckSum)
			{
				ret = -1;
			}
			else
			{
				if(angle_value<8)
					AppTask_SendCmd(CMD_UI_KEY_3D_ANGLE, TASK_MONITOR, TASK_UI, angle_value, NULL, 0, 1000);
			}
			LOG_DEBUG("Rx Req. cmd: con rem leftlong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
		}else{
			// error
			LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
			ret = -1;
		}
	}

	else if(strcmp(type, "steerangle")==0){


			sep = get_field((uint8 *)field);
			if(sep == SEPERATOR_LF){
				checksum = simple_strtoul(field, 0);
				if(checksum != gRxCheckSum)
				{
					ret = -1;
				}
				else
				{
					AppTask_SendCmd(CMD_DISPLAY_STEER_ANGLE, TASK_MONITOR, TASK_DISPLAY, angle_value-35, NULL, 0, 1000);
				}
				LOG_DEBUG("Rx Req. cmd: con rem leftlong cs:%s/0x%x(%s)\n", field, gRxCheckSum, checksum == gRxCheckSum ? "OK" : "Fail");
			}else{
				// error
				LOG_DEBUG("Error not LF(0x%x). Rx Req. cmd: con rem\n", sep);
				ret = -1;
			}
		}

	else
	{
		// error
		LOG_DEBUG("unknown packet(%s)\n", action);
		ret = -1;
	}

	//tx_resp_remote(action, ret);

	if(ret < 0){
		LOG_DEBUG("Rx remote controller fail\n");
	}
	return ret;
}

static sint8 rx_req_reg_action(void)
{
	sint8 field[16];
	uint32 addr;
	sint8 sep;
	uint32 data[256];
	sint8 ret = 1;
	uint32 checksum;

	sep = get_field((uint8 *)field);
	if(strcmp(field, "sw") == 0){
		sep = get_field((uint8 *)field);
		addr = simple_strtoul(field, 0);
		sep = get_field((uint8 *)field);
		data[0] = simple_strtoul(field, 0);
		sep = get_field((uint8 *)field);
		checksum = simple_strtoul(field, 0);

		// checksum check

		// do action

		LOG_DEBUG("Rx Req. cmd: reg sw, addr: 0x%x, data: 0x%x check sum %s\n", addr, data[0], checksum == gRxCheckSum ? "OK" : "Fail");

	}else if(strcmp(field, "cw") == 0){
		sep = get_field((uint8 *)field);
		//addr = strtoint(field);
		do{
			sep = get_field((uint8 *)field);
			//data[cnt] = strtoint(field);
		}while(sep != SEPERATOR_LF);

	}else{
		// error
		ret = -1;
	}

	return ret;
}



sint8 tx_resp_remote(uint8 *action, sint8 result)
{
	sint8 data[256];
	sint8 str_cs[16];
	sint8 *prefix = "r con rem ";
	uint32 str_size;
	uint32 checksum = 0;
	uint32 size;
	uint32 i;

	size = 0;
	str_size = strlen(prefix);
	for(i=0;i<str_size;i++){
		data[size++] = prefix[i];
		checksum += prefix[i];
	}

	str_size = strlen((sint8 *)action);
	for(i=0;i<str_size;i++){
		data[size++] = action[i];
		checksum += action[i];
	}

	data[size++] = SEPERATOR_SP;
	checksum += SEPERATOR_SP;

	sprintf(str_cs, "0x%x", checksum);
	str_size = strlen(str_cs);

	for(i=0;i<str_size;i++){
		data[size++] = str_cs[i];
	}
	data[size++] = SEPERATOR_LF;

	put_bytes((uint8 *)data, size);

	return 0;
}


inline static unsigned long simple_strtoul(const char *cp, unsigned int base)
{
	unsigned long result = 0,value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((toupper((int)*cp) == 'X') && isxdigit((int)cp[1])) {
				cp++;
				base = 16;
			}
		}
	} else if (base == 16) {
		if (cp[0] == '0' && toupper((int)cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit((int)*cp) &&
			(value = isdigit((int)*cp) ? *cp-'0' : toupper((int)*cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	return result;
}

static void put_byte(uint8 c)
{
	PPDRV_UART_OutByte (1, c); //ch 1 mcu channel
}

static void put_bytes(uint8 *buf, uint32 size)
{
	uint32 i;

	for(i=0;i<size;i++){
		put_byte(buf[i]);
	}
}

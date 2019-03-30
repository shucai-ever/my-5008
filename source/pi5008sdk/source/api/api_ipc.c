#include <stdio.h>
#include <stdarg.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "mbox.h"
#include "debug.h"
#include "system.h"
#include "api_ipc.h"
#include "api_vin.h"
#if (PI5008_CORE_ID == PI5008_CORE_1)
#include <string.h>
#include "cache.h"
#include "interrupt.h"
#include "intr.h"
#include "app_core1_viewgen.h"
#elif(PI5008_CORE_ID == PI5008_CORE_0)
#include "api_calibration.h"
#include "task_manager.h"
#include "task_calibration.h"
#include "sys_api.h"

#if defined(CALIB_LIB_USE)
extern PCV_SVM_VIEW_MD_VIEW_PARAM tMdViewParam;
#endif

#endif


#ifdef PI5008_FREE_RTOS
#include "osal.h"
#endif

#define CORE0_D0_IDX	0
#define CORE0_D1_IDX	1
#define CORE0_D2_IDX	2

#define CORE1_D0_IDX	3
#define CORE1_D1_IDX	4
#define CORE1_D2_IDX	5

#define ISP_READY_IDX	6
#define MAIN_READY_IDX	7

// COMMAND C0 -> C1
#define IPC_C0_CMD_PRINT_MEM_SET	0x01 // D0: memory address, D1: memory size
#define IPC_C0_CMD_MAIN_READY		0x02
#define IPC_C0_CMD_DATA_MEM_SET		0x03
#define IPC_C0_CMD_BRIGHT_CTRL		0x04
#define IPC_C0_CMD_CAM_CTRL			0x05
#define IPC_C0_CMD_CAM_STATUS_REQ	0x06
#define IPC_C0_CMD_VIEWGEN_SEND_START	0x08
#define IPC_C0_CMD_VIEWGEN_SEND_SVMCNF	0x09
#define IPC_C0_CMD_VIEWGEN_SEND_OFFCNF	0x0A
#define IPC_C0_CMD_VIEWGEN_UPDATE_ACK		0x0B

#define IPC_C0_CMD_MAX				0x0C
// Add cmd...

// COMMAND C1 -> C0
#define IPC_C1_CMD_DBG_PRINT		0x10
#define IPC_C1_CMD_ISP_READY		0x11 // D6: ready pattern
#define IPC_C1_CMD_SEND_DATA		0x12
#define IPC_C1_CMD_VIEWGEN_START_ACK	0x13
#define IPC_C1_CMD_VIEWGEN_UPDATE_SECTION	0x14
#define IPC_C1_CMD_VIEWGEN_DONE		0x15
#define IPC_C1_CMD_CAM_CTRL_RESP	0x16
#define IPC_C1_CMD_CAM_EVENT		0x17
// Add cmd...

// COMMAND Common
#define IPC_CMD_ACK					0x80
#define IPC_CMD_NACK				0x81

#define IPC_SENDER_POS				30
#define IPC_RECEIVER_POS			28
#define IPC_ATTR_POS				24
#define IPC_CMD_POS					16

#define IPC_ISP_READY_PATTERN		0x5008cafe
#define IPC_MAIN_READY_PATTERN		0x5008beef

typedef enum ppIPC_ATTR_E{
	eATTR_BIT_REQACK = 0,
	eATTR_BIT_RESERVED1,
	eATTR_BIT_RESERVED2,
	eATTR_BIT_RESERVED3,
	MAX_ATTR,
}IPC_ATTR_E;


typedef union ppIPC_CMD_U
{
	PP_U32 var;
	struct{
		PP_U32 	Rsv		:16,
				Cmd		:8,
				Attr	:4,
				RecvId	:2,
				SendId	:2;
	};
}IPC_CMD_U;

#if (PI5008_CORE_ID == PI5008_CORE_0)

static SemaphoreHandle_t ghLock[IPC_C0_CMD_MAX];

// test
PP_U8 gTestMem[32*1024];
PP_U32 gTestSTick;
PP_U32 gTestETick;
PP_U32 gTestCount = 0;

PP_U8 gRespReq[IPC_C0_CMD_MAX];
PP_U8 *gpu8CamConMem = NULL;

#elif (PI5008_CORE_ID == PI5008_CORE_1)
extern PP_U32 Multicore_Viewgen_Update_Ack_Flag;
#endif

PP_CHAR *gpszPrintBuf = NULL;
PP_U32 gu32PrintBufSize = 0;
PP_U8 *gpu8DataMem = NULL;
PP_U32 gu32DataMemSize = 0;

PP_U8 gIpcViewgenParamNum = 0;
PP_U8 *gIpcViewgenParamBuf = NULL;
PP_U32 gIpcViewgenParamBufSize = 0;

PP_U8 *gIpcViewgenOffcalibInfo = NULL;
PP_U32 gIpcViewgenOffcalibInfoSize = 0;


PP_U8 gReqACKCmdFlag[0xff];
PP_U8 gACKResult[0xff];
PP_U8 gViewgenStart = 0;

#if defined(CALIB_LIB_USE)

extern PP_U32 Core1_SectionNum;
extern PP_U8 *pCore1_FBLut;
extern PP_U8 *pCore1_LRLut;
extern PP_U32 gCore1_LutBufSize;
#endif

#if defined(CALIB_LIB_USE)
//extern PP_U32 gMulticore_Viewgen_core1_done_Flag;
extern PP_U8 gMulticore_Viewgen_Flag;
#endif

static PP_VOID IRQHandler(uint32 msg)
{
	IPC_CMD_U stCmd;
	PP_U32 u32Resp = 1;
	PP_U32 u32ReqAckCmd;

#if (PI5008_CORE_ID == PI5008_CORE_1)
	PP_U32 u32Value;
	PP_U32 ch, addr, data;
	PP_U32 enable;
	PP_U32 ret;
	PP_U32 u32Count;
#endif	


#if (PI5008_CORE_ID == PI5008_CORE_0)
	BaseType_t xHigherPriorityTaskWoken;
#endif

	stCmd.var = msg;

	switch(stCmd.Cmd){
#if (PI5008_CORE_ID == PI5008_CORE_1)
		case IPC_C0_CMD_PRINT_MEM_SET:
			gpszPrintBuf = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D0_IDX);
			gu32PrintBufSize = PPDRV_MBOX_GetData(CORE1_D1_IDX);
			break;
		case IPC_C0_CMD_MAIN_READY:
			break;
		case IPC_C0_CMD_DATA_MEM_SET:
			gpu8DataMem = (PP_U8 *)PPDRV_MBOX_GetData(CORE1_D0_IDX);
			gu32DataMemSize = PPDRV_MBOX_GetData(CORE1_D1_IDX);
			break;
		case IPC_C0_CMD_BRIGHT_CTRL:
			u32Value = PPDRV_MBOX_GetData(CORE1_D0_IDX);
			if(u32Value){
				SetBrightControl(1);	// test function in main
				dbg("bc on\n");
			}else{
				SetBrightControl(0);	// test function in main
				dbg("bc off\n");
			}
			break;
		case IPC_CMD_ACK:
			if(stCmd.RecvId == PI5008_CORE_1){
				u32ReqAckCmd = PPDRV_MBOX_GetData(CORE1_D0_IDX);
				gReqACKCmdFlag[u32ReqAckCmd] = 0;
				gACKResult[u32ReqAckCmd] = 0;
			}
			break;
		case IPC_CMD_NACK:
			if(stCmd.RecvId == PI5008_CORE_1){
				u32ReqAckCmd = PPDRV_MBOX_GetData(CORE1_D0_IDX);
				gReqACKCmdFlag[u32ReqAckCmd] = 0;
				gACKResult[u32ReqAckCmd] = 1;
				// NACK Action ???
			}
			break;
		case IPC_C0_CMD_VIEWGEN_START:
			gIpcViewgenParamNum = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D0_IDX);
			gViewgenStart=1;
			//VIEWGEN_LOG("gIpcViewgenParamNum = %d\n",gIpcViewgenParamNum);
				switch(gIpcViewgenParamNum)
				{
					case 0:
						gIpcViewgenParamBuf = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D1_IDX);						
						gIpcViewgenParamBufSize = PPDRV_MBOX_GetData(CORE1_D2_IDX);
						break;
					case 1:
						gIpcViewgenParamBuf = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D1_IDX);
						gIpcViewgenParamBufSize = PPDRV_MBOX_GetData(CORE1_D2_IDX);
						break;
					case 2:
						gIpcViewgenParamBuf = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D1_IDX);
						gIpcViewgenParamBufSize = PPDRV_MBOX_GetData(CORE1_D2_IDX);
						break;
					case 3:
						gIpcViewgenParamBuf = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D1_IDX);
						gIpcViewgenParamBufSize = PPDRV_MBOX_GetData(CORE1_D2_IDX);
						break;
					case 4:
						gIpcViewgenParamBuf = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D1_IDX);
						gIpcViewgenParamBufSize = PPDRV_MBOX_GetData(CORE1_D2_IDX);
						break;
				}
				u32Resp=1;	
			break;
		case IPC_C0_CMD_VIEWGEN_UPDATE_ACK:
				Multicore_Viewgen_Update_Ack_Flag = (PP_CHAR *)PPDRV_MBOX_GetData(CORE1_D0_IDX);
				u32Resp=1;	
			break;

		case IPC_C0_CMD_CAM_CTRL:
			u32Value = PPDRV_MBOX_GetData(CORE1_D0_IDX);
			u32Count = PPDRV_MBOX_GetData(CORE1_D1_IDX);
			SetCamControl((PP_CAM_CONTROL_S *)u32Value, u32Count);	// test function in main
			dbg("cam control\n");

			break;
#elif (PI5008_CORE_ID == PI5008_CORE_0)

		case IPC_C1_CMD_DBG_PRINT:
			LOG_DEBUG_ISR("%s", gpszPrintBuf);
			break;
		case IPC_C1_CMD_ISP_READY:
			break;
		case IPC_C1_CMD_SEND_DATA:
//========================================
// TEST CODE
//========================================
			if(gTestCount == 0){
				gTestSTick = GetTickCount();
			}

			memcpy(gTestMem, gpu8DataMem, 32*1024);

			if(++gTestCount == 100){
				gTestETick = GetTickCount();
				LOG_DEBUG_ISR("IPC data send time(%d): %d ms\n", gTestCount, gTestETick - gTestSTick);
				gTestCount = 0;
			}
//========================================
			break;
		case IPC_CMD_ACK:
			if(stCmd.RecvId == PI5008_CORE_0){
				u32ReqAckCmd = PPDRV_MBOX_GetData(CORE0_D0_IDX);
				gReqACKCmdFlag[u32ReqAckCmd] = 0;
				gACKResult[u32ReqAckCmd] = 0;

	            xHigherPriorityTaskWoken = pdFALSE;
	            xSemaphoreGiveFromISR(ghLock[u32ReqAckCmd], &xHigherPriorityTaskWoken);
	            if(xHigherPriorityTaskWoken == pdTRUE)
	            {
	                portYIELD_FROM_ISR();
	            }

			}
			break;
		case IPC_CMD_NACK:
			if(stCmd.RecvId == PI5008_CORE_0){
				u32ReqAckCmd = PPDRV_MBOX_GetData(CORE0_D0_IDX);
				gReqACKCmdFlag[u32ReqAckCmd] = 0;
				gACKResult[u32ReqAckCmd] = 1;
				// NACK Action ???
	            xHigherPriorityTaskWoken = pdFALSE;
	            xSemaphoreGiveFromISR(ghLock[u32ReqAckCmd], &xHigherPriorityTaskWoken);
	            if(xHigherPriorityTaskWoken == pdTRUE)
	            {
	                portYIELD_FROM_ISR();
	            }


			}
			break;
		case IPC_C1_CMD_VIEWGEN_UPDATE_SECTION:
			if(stCmd.RecvId == PI5008_CORE_0){
		
				#if defined(CALIB_LIB_USE)

				Core1_SectionNum = PPDRV_MBOX_GetData(CORE0_D0_IDX);
				pCore1_FBLut = (PP_U8 *)PPDRV_MBOX_GetData(CORE0_D1_IDX);
				pCore1_LRLut = (PP_U8 *)PPDRV_MBOX_GetData(CORE0_D2_IDX);

				gCore1_LutBufSize = PPDRV_MBOX_GetData(CORE1_D2_IDX);

				PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pCore1_FBLut, gCore1_LutBufSize);
				PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pCore1_LRLut, gCore1_LutBufSize);

				AppTask_SendCmdFromISR(CMD_MULTICORE_VIEWGEN_START, TASK_MONITOR, TASK_CALIBRATION, TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_UPDATE, 0, 0);
				#endif
				u32Resp = 1;
				
			}
			break;
						
		case IPC_C1_CMD_VIEWGEN_DONE:
			if(stCmd.RecvId == PI5008_CORE_0){				
				if(0xFEEDCAFE==PPDRV_MBOX_GetData(CORE0_D0_IDX))
				{
					//gMulticore_Viewgen_core1_done_Flag = 1;
				}
				
				#if defined(CALIB_LIB_USE)
				AppTask_SendCmdFromISR(CMD_MULTICORE_VIEWGEN_START, TASK_MONITOR, TASK_CALIBRATION, TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_DONE, 0, 0);
				#endif
				u32Resp = 1;
				
			}
			break;
		case IPC_C1_CMD_VIEWGEN_START_ACK:
				if(stCmd.RecvId == PI5008_CORE_0){				

					#if defined(CALIB_LIB_USE)
					if(0xFEEDCAFE==PPDRV_MBOX_GetData(CORE0_D0_IDX))
					{
						gMulticore_Viewgen_Flag=1;
						//gMulticore_Viewgen_core1_done_Flag = 1;
					}
					
					
					//AppTask_SendCmdFromISR(CMD_MULTICORE_VIEWGEN_START, TASK_MONITOR, TASK_CALIBRATION, TASK_CMDATTR_MULTICORE_VIEWGEN_CORE1_DONE, 0, 0);
					#endif
					u32Resp = 1;
					
				}
			break;

		case IPC_C1_CMD_CAM_CTRL_RESP:
			if(stCmd.RecvId == PI5008_CORE_0 && gRespReq[IPC_C0_CMD_CAM_CTRL]){
				gRespReq[IPC_C0_CMD_CAM_CTRL] = 0;
				xHigherPriorityTaskWoken = pdFALSE;
	            xSemaphoreGiveFromISR(ghLock[IPC_C0_CMD_CAM_CTRL], &xHigherPriorityTaskWoken);
	            if(xHigherPriorityTaskWoken == pdTRUE)
	            {
	                portYIELD_FROM_ISR();
	            }

			}
			break;
		case IPC_C1_CMD_CAM_EVENT:
			if(stCmd.RecvId == PI5008_CORE_0){
				PP_U32 u32CamCh, u32CamPlugin;
				u32CamCh = PPDRV_MBOX_GetData(CORE0_D0_IDX);
				u32CamPlugin = PPDRV_MBOX_GetData(CORE0_D1_IDX);
				//CamStatusEvent(u32CamCh, u32CamPlugin);
				LOG_DEBUG_ISR("Cam ch: 0x%x, plugin: 0x%x\n", u32CamCh, u32CamPlugin);

                /* Update VIN Ch & Port */
                {/*{{{*/
                    int portNum;
                    PP_S8 s8CamCh[4] = {-1, };
                    PP_U8 u8CamChStatus, u8PlugStatus;

                    for(portNum = 0; portNum < 4; portNum++)
                    {
                        u8CamChStatus = u32CamCh>>(portNum*8)&0xFF;
                        u8PlugStatus = u32CamPlugin>>(portNum*8)&0xFF;
                        if( (u8CamChStatus > 3) || (u8PlugStatus == 0x01) ) //Plug-Out
                        {
                            //LOG_DEBUG_ISR("Cam: %d, PlugOut\n", portNum);
                            s8CamCh[portNum] = -1;
                        }
                        else if( (u8CamChStatus <= 3) && (u8PlugStatus == 0x02) ) //Plug-In
                        {
                            //LOG_DEBUG_ISR("Cam: %d-%d, PlugIn\n", portNum, u8CamChStatus);
                            s8CamCh[portNum] = u8CamChStatus;
                        }
                        else
                        {
                            //LOG_DEBUG_ISR("Error!!! Unknown camch status.\n");
                            s8CamCh[portNum] = -1;
                        }
                    }
                    //Send event to task.
                    if( (AppTask_SendCmdFromISR(CMD_EMERGENCY_CAMERA_PLUG, (PP_U16)-1, TASK_EMERGENCY, 0, s8CamCh, sizeof(PP_S8)*4)) != eSUCCESS)
                    {
                        LOG_DEBUG_ISR("Error!!! Fail send command\n");
                    }
                }/*}}}*/
                u32Resp = 1;
			}
			break;

#endif
		default:
			break;


	}

	if(stCmd.Attr & (1<<eATTR_BIT_REQACK)){
		if(u32Resp)
			PPAPI_IPC_SendACK(PI5008_CORE_ID, stCmd.SendId, stCmd.Cmd);
		else
			PPAPI_IPC_SendNACK(PI5008_CORE_ID, stCmd.SendId, stCmd.Cmd);

	}


}


static PP_RESULT_E IRQWaitClear(PP_U32 u32CoreId, PP_U32 u32TimeOut)
{
#ifdef PI5008_FREE_RTOS
	PP_U32 u32Tick = GetTickCount();
	while(PPDRV_MBOX_GetIRQMsg(u32CoreId)){
		OSAL_sleep(1);
		if(u32TimeOut != 0xffffffff && (GetTickCount() - u32Tick > u32TimeOut))
			return eERROR_TIMEOUT;
	}

#else
	PP_U32 u32TimeOutCnt = 0;
	while(PPDRV_MBOX_GetIRQMsg(u32CoreId)){
		if(u32TimeOut != 0xffffffff && (++u32TimeOutCnt > u32TimeOut))
			return eERROR_TIMEOUT;

	}

#endif

	return eSUCCESS;
}


PP_VOID PPAPI_IPC_Initialize(PP_VOID)
{
	int i;

	PPDRV_MBOX_Initialize(PI5008_CORE_ID);
	PPDRV_MBOX_SetISR(IRQHandler);
	memset(gReqACKCmdFlag, 0, sizeof(gReqACKCmdFlag));

#if (PI5008_CORE_ID == PI5008_CORE_0)
	gpszPrintBuf = (PP_CHAR *)(PP_CHAR *)ADDR_NON_CACHEABLE((PP_U32)OSAL_malloc(PRINT_MEM_SIZE));
	gu32PrintBufSize = PRINT_MEM_SIZE;

	gpu8DataMem = (PP_U8 *)ADDR_NON_CACHEABLE((PP_U32)OSAL_malloc(DATA_MEM_SIZE));
	gu32DataMemSize = DATA_MEM_SIZE;

	gpu8CamConMem = (PP_U8 *)ADDR_NON_CACHEABLE((PP_U32)OSAL_malloc(CAM_CON_MEM_SIZE));

    for(i=0;i<IPC_C0_CMD_MAX;i++){
        if( (ghLock[i] = xSemaphoreCreateBinary()) == NULL)
        {
            LOG_DEBUG("ERROR! can't create lock\n");
        }
        gRespReq[i] = 0;

    }

#endif
}

PP_VOID PPAPI_IPC_SetISPReadyDone(PP_VOID)
{

	PPDRV_MBOX_SetData(ISP_READY_IDX, IPC_ISP_READY_PATTERN);

#if 0
	// send irq if destination is ready.
	if(PPAPI_IPC_CheckMainReady()){
		IPC_CMD_U stCmd;

		stCmd.SendId = PI5008_CORE_1;
		stCmd.RecvId = PI5008_CORE_0;
		stCmd.Cmd = IPC_C1_CMD_ISP_READY;
		stCmd.Attr = 0;

		PPDRV_MBOX_SetIRQMsg(stCmd.RecvId, (PP_U32)stCmd.var);
	}
#endif
}

PP_U32 PPAPI_IPC_CheckISPReady(PP_VOID)
{

	return (PPDRV_MBOX_GetData(ISP_READY_IDX) == IPC_ISP_READY_PATTERN ? 1 : 0);
}


PP_VOID PPAPI_IPC_SetMainReadyDone(PP_VOID)
{

	PPDRV_MBOX_SetData(MAIN_READY_IDX, IPC_MAIN_READY_PATTERN);
#if 0
	// send irq if destination is ready.
	if(PPAPI_IPC_CheckISPReady()){
		IPC_CMD_U stCmd;

		stCmd.SendId = PI5008_CORE_0;
		stCmd.RecvId = PI5008_CORE_1;
		stCmd.Cmd = IPC_C0_CMD_MAIN_READY;
		stCmd.Attr = 0;

		PPDRV_MBOX_SetIRQMsg(stCmd.RecvId, (PP_U32)stCmd.var);
	}
#endif
}

PP_U32 PPAPI_IPC_CheckMainReady(PP_VOID)
{

	return (PPDRV_MBOX_GetData(ISP_READY_IDX) == IPC_MAIN_READY_PATTERN ? 1 : 0);
}


PP_VOID PPAPI_IPC_SendACK(PP_U32 IN u32Sender, PP_U32 IN u32Receiver, IN PP_U32 u32ReqCmd)
{
	IPC_CMD_U stCmd;

	stCmd.SendId = u32Sender;
	stCmd.RecvId = u32Receiver;
	stCmd.Cmd = IPC_CMD_ACK;
	stCmd.Attr = 0;

	PPDRV_MBOX_SetData(CORE0_D0_IDX + 3*u32Receiver, u32ReqCmd);
	PPDRV_MBOX_SetIRQMsg(u32Receiver, (PP_U32)stCmd.var);
}

PP_VOID PPAPI_IPC_SendNACK(PP_U32 IN u32Sender, PP_U32 IN u32Receiver, IN PP_U32 u32ReqCmd)
{
	IPC_CMD_U stCmd;

	stCmd.SendId = u32Sender;
	stCmd.RecvId = u32Receiver;
	stCmd.Cmd = IPC_CMD_NACK;
	stCmd.Attr = 0;

	PPDRV_MBOX_SetData(CORE0_D0_IDX + 3*u32Receiver, u32ReqCmd);
	PPDRV_MBOX_SetIRQMsg(u32Receiver, (PP_U32)stCmd.var);
}


#if (PI5008_CORE_ID == PI5008_CORE_0)
PP_RESULT_E PPAPI_IPC_SetPrintMemory(PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 100);
	if(enRet)return enRet;

	PPDRV_MBOX_SetData(CORE1_D0_IDX, (uint32)gpszPrintBuf);
	PPDRV_MBOX_SetData(CORE1_D1_IDX, gu32PrintBufSize);

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_PRINT_MEM_SET;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_PRINT_MEM_SET] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);

//	enRet = WaitAckResp(IPC_C0_CMD_PRINT_MEM_SET, 100);
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_PRINT_MEM_SET], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_PRINT_MEM_SET];
		}
    }



	return enRet;
}

PP_RESULT_E PPAPI_IPC_SetDataMemory(PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 100);
	if(enRet)return enRet;

	PPDRV_MBOX_SetData(CORE1_D0_IDX, (uint32)gpu8DataMem);
	PPDRV_MBOX_SetData(CORE1_D1_IDX, gu32DataMemSize);

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_DATA_MEM_SET;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_DATA_MEM_SET] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);

//	enRet = WaitAckResp(IPC_C0_CMD_DATA_MEM_SET, 100);
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_DATA_MEM_SET], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_DATA_MEM_SET];
		}
    }


	return enRet;
}

PP_RESULT_E PPAPI_IPC_SetBrightCtrl(PP_U8 IN u8On, PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 100);
	if(enRet)return enRet;

	PPDRV_MBOX_SetData(CORE1_D0_IDX, (uint32)u8On);

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_BRIGHT_CTRL;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_BRIGHT_CTRL] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);

//	enRet = WaitAckResp(IPC_C0_CMD_BRIGHT_CTRL, 100);
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_BRIGHT_CTRL], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_BRIGHT_CTRL];
		}
    }


	return enRet;
}

#if defined(CALIB_LIB_USE)

extern PP_CNF_TOTAL_BIN_FORMAT_S Calib_Cnf_Bin;
extern PP_OFFCALIB_OUT_INFO_S Off_Calib_Out;
PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Send_Start(PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 100);
	if(enRet)return enRet;

	
	//PPDRV_MBOX_SetData(CORE1_D0_IDX, 0xCEEDCAFE); 

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_VIEWGEN_SEND_START;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_VIEWGEN_SEND_START] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);


    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_VIEWGEN_SEND_START], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_VIEWGEN_SEND_START];
		}
    }
	return enRet;


	
}

PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Send_SVMcnf(PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 100);
	if(enRet)return enRet;



	
	//PPAPI_Viewgen_Get_External_Value();
	gIpcViewgenParamBuf = (PP_U8 *)&Calib_Cnf_Bin;
	gIpcViewgenParamBufSize = sizeof(PP_CNF_TOTAL_BIN_FORMAT_S);

	
	PPDRV_MBOX_SetData(CORE1_D0_IDX, 0); // //view num
	PPDRV_MBOX_SetData(CORE1_D1_IDX, (uint32)gIpcViewgenParamBuf);
	PPDRV_MBOX_SetData(CORE1_D2_IDX, gIpcViewgenParamBufSize);

	
	PPAPI_SYS_CACHE_Writeback((uint32 *)gIpcViewgenParamBuf, gIpcViewgenParamBufSize);


	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_VIEWGEN_SEND_SVMCNF;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_VIEWGEN_SEND_SVMCNF] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);


    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_VIEWGEN_SEND_SVMCNF], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_VIEWGEN_SEND_SVMCNF];
		}
    }
	return enRet;


	
}

PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Send_OffCalibcnf(PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 100);
	if(enRet)return enRet;

	//PPAPI_Viewgen_Get_External_Value();
	gIpcViewgenOffcalibInfo = (PP_U8 *)&Off_Calib_Out;
	gIpcViewgenOffcalibInfoSize = sizeof(PP_OFFCALIB_OUT_INFO_S);


	PPDRV_MBOX_SetData(CORE1_D1_IDX, (uint32)gIpcViewgenOffcalibInfo);
	PPDRV_MBOX_SetData(CORE1_D2_IDX, gIpcViewgenOffcalibInfoSize);

	
	PPAPI_SYS_CACHE_Writeback((uint32 *)gIpcViewgenOffcalibInfo, gIpcViewgenOffcalibInfoSize);


	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_VIEWGEN_SEND_OFFCNF;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_VIEWGEN_SEND_OFFCNF] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);


    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_VIEWGEN_SEND_OFFCNF], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_VIEWGEN_SEND_OFFCNF];
		}
    }
	return enRet;


	
}

#endif

PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Update_Ack(PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 0xffffffff);
	if(enRet)return enRet;


	
	PPDRV_MBOX_SetData(CORE1_D0_IDX, 1); // Ack



	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_VIEWGEN_UPDATE_ACK;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_VIEWGEN_UPDATE_ACK] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);

//	enRet = WaitAckResp(IPC_C0_CMD_PRINT_MEM_SET, 100);
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_VIEWGEN_UPDATE_ACK], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_VIEWGEN_UPDATE_ACK];
		}
    }
	return enRet;


	
}

// Return: 0: OK, else: Error

PP_RESULT_E PPAPI_IPC_CamControl(PP_CAM_CONTROL_S IN *pstCon, PP_U32 IN u32Count, PP_U32 IN u32Timeout)
{
	PP_RESULT_E enRet = eSUCCESS;
	IPC_CMD_U stCmd;
	PP_CAM_CONTROL_S *pstConTemp;

	if(sizeof(PP_CAM_CONTROL_S) * u32Count > CAM_CON_MEM_SIZE)return eERROR_FAILURE;

	pstConTemp = (PP_CAM_CONTROL_S *)gpu8CamConMem;
	memcpy(pstConTemp, pstCon, sizeof(PP_CAM_CONTROL_S) * u32Count);

	enRet = IRQWaitClear(PI5008_CORE_1, u32Timeout);
	if(enRet)return enRet;

	gRespReq[IPC_C0_CMD_CAM_CTRL] = 1;

	PPDRV_MBOX_SetData(CORE1_D0_IDX, (PP_U32)pstConTemp);
	PPDRV_MBOX_SetData(CORE1_D1_IDX, u32Count);

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_CAM_CTRL;
	stCmd.Attr = 0;//(1 << eATTR_BIT_REQACK);

	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }
    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_CAM_CTRL], u32Timeout) == pdFALSE )
		{
			gRespReq[IPC_C0_CMD_CAM_CTRL] = 0;
			enRet = eERROR_TIMEOUT;
			LOG_CRITICAL("Timeout!\n");
		}
	}

	memcpy(pstCon, (void *)pstConTemp, sizeof(PP_CAM_CONTROL_S)*u32Count );

	return enRet;
}

PP_RESULT_E PPAPI_IPC_ReqCamStatus(PP_U32 OUT *pu32CamCh, PP_U32 OUT *pu32CamPlugin, PP_U32 IN u32Timeout)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, u32Timeout);
	if(enRet)return enRet;

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd = IPC_C0_CMD_CAM_STATUS_REQ;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C0_CMD_CAM_STATUS_REQ] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_1, (PP_U32)stCmd.var);

//	enRet = WaitAckResp(IPC_C0_CMD_BRIGHT_CTRL, 100);
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[IPC_C0_CMD_CAM_STATUS_REQ], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			enRet = eERROR_TIMEOUT;
		}else{
			enRet = gACKResult[IPC_C0_CMD_CAM_STATUS_REQ];
			if(enRet == eSUCCESS){
				if(pu32CamCh)*pu32CamCh = PPDRV_MBOX_GetData(CORE0_D1_IDX);
				if(pu32CamPlugin)*pu32CamPlugin = PPDRV_MBOX_GetData(CORE0_D2_IDX);
			}
		}
    }


	return enRet;
}


#elif (PI5008_CORE_ID == PI5008_CORE_1)

static PP_RESULT_E WaitAckResp(PP_U32 u32ReqCmd, PP_U32 u32TimeOut)
{
	PP_RESULT_E enRet = eSUCCESS;

#ifdef PI5008_FREE_RTOS
	PP_U32 u32Tick = GetTickCount();
	while(gReqACKCmdFlag[u32ReqCmd]){
		OSAL_sleep(1);
		if(u32TimeOut != 0xffffffff && (GetTickCount() - u32Tick > u32TimeOut))
			return eERROR_TIMEOUT;

	}
	if(gACKResult[u32ReqCmd])enRet = eERROR_FAILURE;

#else
	PP_U32 u32TimeOutCnt = 0;
	while(gReqACKCmdFlag[u32ReqCmd]){
		if(u32TimeOut != 0xffffffff && (++u32TimeOutCnt > u32TimeOut))
			return eERROR_TIMEOUT;

	}

#endif

	return enRet;
}


PP_VOID PPAPI_IPC_Core1_Viewgen_Req_Update_Section(PP_VOID)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_0, 0xffffffff);
	if(enRet)return NULL;

	
	//PPDRV_MBOX_SetData(CORE0_D0_IDX, req_size); // cnf file
	//VIEWGEN_LOG("[core1]request alloc size = %d \n",req_size);
	PPDRV_MBOX_SetData(CORE0_D0_IDX, Core1_SectionNum);	
	PPDRV_MBOX_SetData(CORE0_D1_IDX, (PP_U32)pCore1_FBLut);	
	PPDRV_MBOX_SetData(CORE0_D2_IDX, (PP_U32)pCore1_LRLut);	


	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_0;
	stCmd.Cmd = IPC_C1_CMD_VIEWGEN_UPDATE_SECTION;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C1_CMD_VIEWGEN_UPDATE_SECTION] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_0, (PP_U32)stCmd.var);

	enRet = WaitAckResp(IPC_C1_CMD_VIEWGEN_UPDATE_SECTION, 0xffffffff);
	if(enRet!=eSUCCESS)
	{	
		//VIEWGEN_LOG("core1 Viewgeneration request malloc fail!!!!!!!!!!!!\n");
	}
	else
	{
		//VIEWGEN_LOG("get alloc addr = 0x%x \n",(PP_U32)rtn_addr);
	}

	while(!Multicore_Viewgen_Update_Ack_Flag){
	}
	Multicore_Viewgen_Update_Ack_Flag=0;
	
}

PP_VOID PPAPI_IPC_Core1_Viewgen_Done(PP_VOID)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_0, 0xffffffff);
	if(enRet)return NULL;

	PPDRV_MBOX_SetData(CORE0_D0_IDX, 0xFEEDCAFE);	


	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_0;
	stCmd.Cmd = IPC_C1_CMD_VIEWGEN_DONE;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C1_CMD_VIEWGEN_DONE] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_0, (PP_U32)stCmd.var);

	enRet = WaitAckResp(IPC_C1_CMD_VIEWGEN_DONE, 0xffffffff);
	if(enRet!=eSUCCESS)
	{	
		//VIEWGEN_LOG("core1 Viewgeneration request malloc fail!!!!!!!!!!!!\n");
	}
	else
	{
		//VIEWGEN_LOG("get alloc addr = 0x%x \n",(PP_U32)rtn_addr);
	}

	
}

PP_VOID PPAPI_IPC_Core1_Viewgen_Start_Ack(PP_VOID)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_0, 0xffffffff);
	if(enRet)return NULL;

	PPDRV_MBOX_SetData(CORE0_D0_IDX, 0xFEEDCAFE);	


	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_0;
	stCmd.Cmd = IPC_C1_CMD_VIEWGEN_START;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);

	gReqACKCmdFlag[IPC_C1_CMD_VIEWGEN_START] = 1;
	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_0, (PP_U32)stCmd.var);

	enRet = WaitAckResp(IPC_C1_CMD_VIEWGEN_START, 0xffffffff);
	if(enRet!=eSUCCESS)
	{	
		//VIEWGEN_LOG("core1 Viewgeneration request malloc fail!!!!!!!!!!!!\n");
	}
	else
	{
		//VIEWGEN_LOG("get alloc addr = 0x%x \n",(PP_U32)rtn_addr);
	}

	
}


PP_RESULT_E PPAPI_IPC_SendPrintMessage(PP_CHAR IN *fmt, ... )
{
	va_list ap;
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	if(gpszPrintBuf == NULL)return eERROR_FAILURE;
	enRet = IRQWaitClear(PI5008_CORE_0, 0x1000000);
	if(enRet)return enRet;

	va_start(ap, fmt);
	vsprintf(gpszPrintBuf, fmt, ap);
	va_end(ap);

	cache_wb_range((uint32 *)gpszPrintBuf, strlen(gpszPrintBuf));

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_0;
	stCmd.Cmd = IPC_C1_CMD_DBG_PRINT;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);
	gReqACKCmdFlag[IPC_C1_CMD_DBG_PRINT] = 1;

	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_0, (PP_U32)stCmd.var);

	enRet = WaitAckResp(IPC_C1_CMD_DBG_PRINT, 0x1000000);

	return enRet;

}

PP_RESULT_E PPAPI_IPC_SendData(PP_U8 IN *pu8Buf, PP_U32 IN u32Size)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	if(gpu8DataMem == NULL)return eERROR_FAILURE;
	enRet = IRQWaitClear(PI5008_CORE_1, 0x1000000);
	if(enRet)return enRet;

	memcpy(gpu8DataMem, pu8Buf, u32Size);

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_0;
	stCmd.Cmd = IPC_C1_CMD_SEND_DATA;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);
	gReqACKCmdFlag[IPC_C1_CMD_SEND_DATA] = 1;

	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_0, (PP_U32)stCmd.var);

	enRet = WaitAckResp(IPC_C1_CMD_SEND_DATA, 0x1000000);

	return enRet;

}


PP_RESULT_E PPAPI_IPC_CamControlResp(PP_VOID)
{
	IPC_CMD_U stCmd;
	PP_RESULT_E enRet;

	enRet = IRQWaitClear(PI5008_CORE_1, 0x1000000);
	if(enRet)return enRet;

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_0;
	stCmd.Cmd = IPC_C1_CMD_CAM_CTRL_RESP;
	stCmd.Attr = (1 << eATTR_BIT_REQACK);
	gReqACKCmdFlag[IPC_C1_CMD_CAM_CTRL_RESP] = 1;

	PPDRV_MBOX_SetIRQMsg(PI5008_CORE_0, (PP_U32)stCmd.var);

	enRet = WaitAckResp(IPC_C1_CMD_CAM_CTRL_RESP, 0x1000000);

	return enRet;

}

#endif


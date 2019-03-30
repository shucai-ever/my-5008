/*
 * api_ipc.h
 *
 *  Created on: 2018. 1. 15.
 *      Author: ihkong
 */

#ifndef _PI5008_IPC_API_H_
#define _PI5008_IPC_API_H_

#include "type.h"
#include "error.h"

#define PI5008_CORE_0	0
#define PI5008_CORE_1	1

#define PI5008_CORE_ID	PI5008_CORE_0

#if (PI5008_CORE_ID	== PI5008_CORE_0)
#define PI5008_FREE_RTOS
#define PRINT_MEM_SIZE		256
#define DATA_MEM_SIZE		(32*1024)
#define CAM_CON_MEM_SIZE	(8*1024)
#endif

#define MAX_CAM_CTRL_CHANNEL		4
typedef struct ppCAM_CONTROL_S
{
	PP_U32 u32Target;							// [In] 0: DES, 1: SER, 2: Sensor
	PP_U32 u32Channel;							// [In] Cam Channel Num (0~3), 0xFFFFFFFF : All Channel
	PP_U32 u32OPType;							// [In] 0: Read, 1: Write
	PP_U32 u32SlaveId;							// [In] I2C Slave ID (7bit)
	PP_U32 u32RegAddr;							// [In] Register Address
	PP_U32 u32RegVal;							// [In] Register Setting Value (write Operation Only)
	PP_U32 u32RetVal[MAX_CAM_CTRL_CHANNEL];		// [In] Return value
	PP_U32 u32RetError[MAX_CAM_CTRL_CHANNEL];	// [Out] Comm Error Status for each channel (0~3), Format(Little Endian: LSB Aligned): 0xAAAABBBB AAAA: target, BBB: Error Code (0: OK, -1(0xFFF): NAK, -2(0xFFFE): Timeout)

}PP_CAM_CONTROL_S;

PP_VOID PPAPI_IPC_Initialize(PP_VOID);
PP_VOID PPAPI_IPC_SetISPReadyDone(PP_VOID);
PP_U32 PPAPI_IPC_CheckISPReady(PP_VOID);
PP_VOID PPAPI_IPC_SetMainReadyDone(PP_VOID);
PP_U32 PPAPI_IPC_CheckMainReady(PP_VOID);
PP_VOID PPAPI_IPC_SendACK(PP_U32 IN u32Sender, PP_U32 IN u32Receiver, IN PP_U32 u32ReqCmd);
PP_VOID PPAPI_IPC_SendNACK(PP_U32 IN u32Sender, PP_U32 IN u32Receiver, IN PP_U32 u32ReqCmd);

#if (PI5008_CORE_ID == PI5008_CORE_0)
PP_RESULT_E PPAPI_IPC_SetPrintMemory(PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_SetDataMemory(PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_SetBrightCtrl(PP_U8 IN u8On, PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_WriteSensor(PP_U8 IN u32Ch, PP_U32 IN u32Addr, PP_U32 IN u32Data, PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_ReadSensor(PP_U8 IN u32Ch, PP_U32 IN u32Addr, PP_U32 OUT *pu32Data, PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_I2CEnable(PP_U8 IN u8Enable, PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Update_Ack(PP_U32 IN u32Timeout);
#if defined(CALIB_LIB_USE)
PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Send_Start(PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Send_SVMcnf(PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_Core0_Viewgen_Send_OffCalibcnf(PP_U32 IN u32Timeout);
#endif
PP_RESULT_E PPAPI_IPC_CamControl(PP_CAM_CONTROL_S IN *pstCon, PP_U32 IN u32Count, PP_U32 IN u32Timeout);
PP_RESULT_E PPAPI_IPC_ReqCamStatus(PP_U32 OUT *pu32CamCh, PP_U32 OUT *pu32CamPlugin, PP_U32 IN u32Timeout);

#else
PP_RESULT_E PPAPI_IPC_SendPrintMessage(PP_CHAR IN *fmt, ... );
PP_RESULT_E PPAPI_IPC_SendData(PP_U8 IN *pu8Buf, PP_U32 IN u32Size);
PP_RESULT_E PPAPI_IPC_CamControlResp(PP_VOID);
#endif

#endif /* _PI5008_IPC_API_H_ */

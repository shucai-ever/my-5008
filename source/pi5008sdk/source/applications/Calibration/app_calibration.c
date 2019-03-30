#include "type.h"
#include "error.h"

#include "task_manager.h"
#include "task_cache.h"

#include "api_FAT_FTL.h"
#include "api_svm.h"
#include "api_display.h"

#include "api_vpu.h"
#include"api_flash.h"
#if defined(USE_PP_GUI)
#include "application.h"
#endif

#ifdef CALIB_LIB_USE


#include "pcvOffLineCalib.h"
#include "pcvSvmView.h"
#include "pcvTypeDef.h"
#include "api_ipc.h"
#include "api_calibration.h"
#include "app_calibration.h"
#include "task_calibration.h"
#include "sys_api.h"

#define CORE0_VIEWGEN_CNT 28 //section num(0~CORE0_VIEWGEN_CNT-1)

PP_U32 heap_alloc_ext_try_cnt=0;
PP_U32 heap_free_ext_try_cnt=0;

PP_U32 heap_alloc_pcv_try_cnt=0;
PP_U32 heap_free_pcv_try_cnt=0;

PP_OFFCALIB_OUT_INFO_S Off_Calib_Out={0,};
PP_VIEWGEN_PROCESS_INFO_S Viewgen_Process_Info={0,};

PP_U32 multicore_calib_flag=0;


#define USE_RESERVED_MEM 
#define CALIB_USE_DEBUG


PP_U8 gMulticore_Viewgen_Flag=0;
PP_U8 gMulticore_Viewgen_Wait_Cnt=0;
PP_U32 gMulticore_Viewgen_core1_done_Flag=0;


// for feature point to modify manually
#define POINT_NUM_MAX	8
PP_U32 feature_point_num = 8;
PP_POS_S feature_point[POINT_NUM_MAX] = { {340, 100}, {540, 100}, {740, 100}, {940, 100}, {340, 300}, {540, 300}, {740, 300}, {940, 300} };


extern PP_U32 apps_pos_point_num;
extern PP_POS_S *apps_pos_point;








PCV_SVM_VIEW_VER_INFO tVerInfo;

PP_U8 *pFBLut, *pLRLut, *pProcBuf;


#if defined(CALIB_LIB_USE)

PP_U32 Core1_SectionNum;
PP_U8 *pCore1_FBLut;
PP_U8 *pCore1_LRLut;
PP_U32 gCore1_LutBufSize;
#endif



PP_U32 total_start_tick=0, total_end_tick=0;
PP_U32 start_tick=0, end_tick=0;
PP_U32 func_start_tick,func_end_tick=0;

/************************************************************/
PP_U32 start_subview_tick=0,end_subview_tick=0;




//calib global variables
int offcalib_front_width = 1280;
int offcalib_front_height = 720;

int offcalib_left_width = 1280;
int offcalib_left_height = 720;

int offcalib_rear_width = 1280;
int offcalib_rear_height = 720;

int offcalib_right_width = 1280;
int offcalib_right_height = 720;

int viewgen_front_width = 1280;
int viewgen_front_height = 720;




extern PP_CNF_TOTAL_BIN_FORMAT_S Calib_Cnf_Bin;


PP_VOID PPAPP_Offcalib_Main(PP_U16  IN step)
{


	PCV_OFF_LINE_CALIB_VER_INFO tVerInfo;

	PP_U8 *pProcBuf;
	
	PP_U32 i =0;
	
	STATIC PP_S32	found;


	STATIC PP_OFFCALIB_CAMERA_CAPTURE_CH_E camera_ch;
	PP_OFFCALIB_CAP_YUV_INFO_S pYuvBufInfoS;


	PPAPI_Offcalib_Set_Step_Send_Cmd(step);

	

	switch((PP_U32)PPAPI_Offcalib_Get_Step_Send_Cmd())
	{	case eOFFCALIB_START:
			{
				heap_alloc_ext_try_cnt=0;
				heap_free_ext_try_cnt=0;
				heap_alloc_pcv_try_cnt=0;
				heap_free_pcv_try_cnt=0;			
				 
				LOG_DEBUG("****************************************\n");
				LOG_DEBUG("*	PI5008K OFFLINE CALIBRATION START\n");


#if defined(USE_PP_GUI)
				PPAPI_DISPLAY_POPUP_On(ePopUp_Calibration);
#endif

				//PPAPI_SVMMEM_LoadCommonSectionData(0, PP_TRUE); 
				//PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL); // preview

				
				PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_CAPTURE);

				
				#ifdef USE_RESERVED_MEM
					prvMemMangCALIBInit_All();
				#endif


				
				AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

				
				//PPAPI_Lib_Debug_Memory();
			}
				break;
		case eOFFCALIB_CAPTURE:
			{
				camera_ch = PPAPI_Offcalib_Get_Cam_Ch();	
				

				start_tick = GetTickCount();
				total_start_tick = GetTickCount();

				if(camera_ch!=eCALIB_CAMERA_ALL)
				{
					PPAPI_Offcalib_Capture_YUV_Image(&pYuvBufInfoS,camera_ch,CAPTURE_YONLY);
					//PPAPI_Offcalib_Capture_YUV_Image_Save_SD(&pYuvBufInfoS,camera_ch);
					//PPAPI_Offcalib_BMP_Image_Save_SD(&pYuvBufInfoS,camera_ch);
					PPAPI_Offcalib_Capture_YUV_Imgae_Save_Buf(&pYuvBufInfoS,camera_ch);
				}
				else
				{	
					for(i=0;i<eCALIB_CAMERA_ALL;i++)
					{
						PPAPI_Offcalib_Capture_YUV_Image(&pYuvBufInfoS,i,CAPTURE_YONLY);
						//PPAPI_Offcalib_Capture_YUV_Image_Save_SD(&pYuvBufInfoS,i);
						//PPAPI_Offcalib_BMP_Image_Save_SD(&pYuvBufInfoS,i);
						PPAPI_Offcalib_Capture_YUV_Imgae_Save_Buf(&pYuvBufInfoS,i);
					}

				}
				
				
				PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_GET_CNF);
				AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				//PPAPI_Lib_Debug_Memory();


			}
				break;			
		case eOFFCALIB_GET_CNF:
		{	
			//LOG_DEBUG("**	OFFCALIB_GET_CNF[step:%d]\n",eOFFCALIB_GET_CNF);
						
			pcvOffLineCalib_GetVersion(&tVerInfo);
			
			LOG_DEBUG("* OffLineCalibLib Version = %d.%d.%d\n", tVerInfo.major, tVerInfo.middle, tVerInfo.minor);
			LOG_DEBUG("****************************************\n");
			
			

			
			
			PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_GET_WOLRD_POINT);
			/* Pattern Recognition */
			/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

		
			if(camera_ch==eCALIB_CAMERA_ALL)
			{
				for(i=0;i<eCALIB_CAMERA_ALL;i++)
				{
					PPAPI_Offcalib_Get_Cnf(&Off_Calib_Out.stPcv_Param.stPatternParam[i],i);
				}
			}
			else if(camera_ch==eCALIB_CAMERA_FRONT)
			{
				PPAPI_Offcalib_Get_Cnf(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT],eCALIB_CAMERA_FRONT);
			}
			else if(camera_ch==eCALIB_CAMERA_LEFT)
			{
				PPAPI_Offcalib_Get_Cnf(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT],eCALIB_CAMERA_LEFT);
			}
			else if(camera_ch==eCALIB_CAMERA_RIGHT)
			{
				PPAPI_Offcalib_Get_Cnf(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT],eCALIB_CAMERA_RIGHT);
			}
			else if(camera_ch==eCALIB_CAMERA_BACK)
			{
				PPAPI_Offcalib_Get_Cnf(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK],eCALIB_CAMERA_BACK);
			}
			
			
			
			AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
		
			//PPAPI_Lib_Debug_Memory();
		}
			break;

			
		case eOFFCALIB_GET_WOLRD_POINT:
		{
			/**********************************************************************************/
			
			
			if(camera_ch==eCALIB_CAMERA_ALL)
			{
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_FRONT, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT]); 		
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_LEFT, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT]);
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_RIGHT, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT]);
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_REAR, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK]);
			}
			else if(camera_ch==eCALIB_CAMERA_FRONT)
			{
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_FRONT, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT]); 		

			}
			else if(camera_ch==eCALIB_CAMERA_LEFT)
			{
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_LEFT, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT]);
			}
			else if(camera_ch==eCALIB_CAMERA_RIGHT)
			{
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_RIGHT, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT]);
			}		
			else if(camera_ch==eCALIB_CAMERA_BACK)
			{
				pcvOffLineCalib_GetFeatureWorldPoint(PCV_OFF_LINE_CALIB_CAMERA_TYPE_REAR, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK]);
			}			
			PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_FIND_PATTERN);
			AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

			
			//PPAPI_Lib_Debug_Memory();
			
		}
			break;
		case eOFFCALIB_FIND_PATTERN:
		{
			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(Calib_Cnf_Bin.frontCamera.width *  Calib_Cnf_Bin.frontCamera.height * 9); //use same buffer

			found = 0;

			if(camera_ch==eCALIB_CAMERA_ALL)
			{
				found = 1;
				if (PCV_OFF_LINE_CALIB_SUCCESS != pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_FRONT], Calib_Cnf_Bin.frontCamera.width, Calib_Cnf_Bin.frontCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_FRONT))
				{
					found = 0;
					LOG_DEBUG("FRONT CAMERA Pattern is not found.\n");				
				}
				if (PCV_OFF_LINE_CALIB_SUCCESS != pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_LEFT], Calib_Cnf_Bin.leftCamera.width, Calib_Cnf_Bin.leftCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_LEFT))
				{
					found = 0;
					LOG_DEBUG("LEFT CAMERA Pattern is not found.\n");				
				}
				if (PCV_OFF_LINE_CALIB_SUCCESS != pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_RIGHT], Calib_Cnf_Bin.rightCamera.width, Calib_Cnf_Bin.rightCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_RIGHT))
				{
					found = 0;
					LOG_DEBUG("RIGHT CAMERA Pattern is not found.\n");				
				}
				
				if (PCV_OFF_LINE_CALIB_SUCCESS != pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_BACK], Calib_Cnf_Bin.rearCamera.width, Calib_Cnf_Bin.rearCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_REAR))
				{
					found = 0;
					LOG_DEBUG("REAR CAMERA Pattern is not found.\n");				
				}				
				
			}
			else if(camera_ch==eCALIB_CAMERA_FRONT)
			{
				if (PCV_OFF_LINE_CALIB_SUCCESS == pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_FRONT], Calib_Cnf_Bin.frontCamera.width, Calib_Cnf_Bin.frontCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_FRONT))
				{
					found = 1;
				}
				else
				{
					found = 0;
					LOG_DEBUG("FRONT CAMERA Pattern is not found.\n");				
				}
			}
			else if(camera_ch==eCALIB_CAMERA_LEFT)
			{
				if (PCV_OFF_LINE_CALIB_SUCCESS == pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_LEFT], Calib_Cnf_Bin.leftCamera.width, Calib_Cnf_Bin.leftCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_LEFT))
				{
					found = 1;
				}
				else
				{
					found = 0;
					LOG_DEBUG("LEFT CAMERA Pattern is not found.\n");				
				}
			}			
			else if(camera_ch==eCALIB_CAMERA_RIGHT)
			{
				if (PCV_OFF_LINE_CALIB_SUCCESS == pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_RIGHT], Calib_Cnf_Bin.rightCamera.width, Calib_Cnf_Bin.rightCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_RIGHT))
				{
					found = 1;
				}
				else
				{
					found = 0;
					LOG_DEBUG("RIGHT CAMERA Pattern is not found.\n");				
				}
			}			
			else if(camera_ch==eCALIB_CAMERA_BACK)
			{
				if (PCV_OFF_LINE_CALIB_SUCCESS == pcvOffLineCalib_FindCalibrationPattern((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_BACK], Calib_Cnf_Bin.rearCamera.width, Calib_Cnf_Bin.rearCamera.height, &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK], pProcBuf,PCV_OFF_LINE_CALIB_CAMERA_TYPE_REAR))
				{
					found = 1;
				}
				else
				{
					found = 0;
					LOG_DEBUG("REAR CAMERA Pattern is not found.\n");				
				}
			}			


							
			
			PPAPI_Lib_Ext_Free(pProcBuf);

			#if 0
			
			if(found)
			{
				
				if(camera_ch==eCALIB_CAMERA_ALL)
				{
					//LOG_DEBUG("*	[ALL CAMERA]\n");
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT]);
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT]);
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT]);
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK]);
				}				
				else if(camera_ch==eCALIB_CAMERA_FRONT)
				{
					//LOG_DEBUG("*	[FRONT CAMERA]\n");
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT]);
				}				
				else if(camera_ch==eCALIB_CAMERA_LEFT)
				{
					//LOG_DEBUG("*	[LEFT CAMERA]\n");
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT]);
				}				
				else if(camera_ch==eCALIB_CAMERA_RIGHT)
				{
					//LOG_DEBUG("*	[RIGHT CAMERA]\n");
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT]);
				}				
				else if(camera_ch==eCALIB_CAMERA_BACK)
				{
					
					//LOG_DEBUG("*	[REAR CAMERA]\n");
					PPAPI_Offcalib_Debug_Print_FeaturePoint(&Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK]);
				}

				
			}
			#endif
			PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_GET_INTRINSIC_PARAM);
			AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
			
			//PPAPI_Lib_Debug_Memory();
			
		}
			break;
		case eOFFCALIB_GET_INTRINSIC_PARAM:
		{
			/* Camera Calibration */
			
			if (found)
			{
			
				if(camera_ch==eCALIB_CAMERA_ALL)
				{
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT],eCALIB_CAMERA_FRONT);
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT],eCALIB_CAMERA_LEFT);
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT],eCALIB_CAMERA_RIGHT);
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK],eCALIB_CAMERA_BACK);


				}
				else if(camera_ch==eCALIB_CAMERA_FRONT)
				{
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT],eCALIB_CAMERA_FRONT);

				}				
				else if(camera_ch==eCALIB_CAMERA_LEFT)
				{
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT],eCALIB_CAMERA_LEFT);
				}				
				else if(camera_ch==eCALIB_CAMERA_RIGHT)
				{
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT],eCALIB_CAMERA_RIGHT);
				}				
				else if(camera_ch==eCALIB_CAMERA_BACK)
				{
					PPAPI_Offcalib_Get_Cam_Intrinsic_Param(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK],eCALIB_CAMERA_BACK);
				}

				
			
			}
			PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_GET_CAMERA_POSITION);
			AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
			
			//PPAPI_Lib_Debug_Memory();
			

		}
			break;
		case eOFFCALIB_GET_CAMERA_POSITION:
		{
			if(found)
			{
				
				if(camera_ch==eCALIB_CAMERA_ALL)
				{
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT]);
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT]);
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT]);
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK]);

					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK]);


				}				
				else if(camera_ch==eCALIB_CAMERA_FRONT)
				{
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT]);

				}				
				else if(camera_ch==eCALIB_CAMERA_LEFT)
				{
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_LEFT]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT]);
				}				
				else if(camera_ch==eCALIB_CAMERA_RIGHT)
				{
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_RIGHT]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT]);

				}				
				else if(camera_ch==eCALIB_CAMERA_BACK)
				{
					pcvOffLineCalib_GetCameraPosition(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK], &Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_BACK]);
					PPAPI_Offcalib_Debug_Print_Position(&Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK]);

				}
				

			}
			PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_GET_FEATURE_POINT);
			AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

			
			//PPAPI_Lib_Debug_Memory();
			
		}
			break;
		case eOFFCALIB_GET_FEATURE_POINT:
		{

			if(found)
			{
				if(camera_ch!=eCALIB_CAMERA_ALL)
				{
					//only show front camera feature point
					LOG_DEBUG("[%s/%d] fp found! \n", __FUNCTION__,__LINE__);
#if defined(USE_PP_GUI)
					PPAPI_DISPLAY_PROGRESSBAR_On(10, 0); //100% display

					PPAPI_DISPLAY_POPUP_On(ePopUp_Done);
#endif

					for (i = 0; i< Off_Calib_Out.stPcv_Param.stPatternParam[camera_ch].featureCount; i++) //X position
					{
						feature_point[i].u16X = (PP_U16 )Off_Calib_Out.stPcv_Param.stPatternParam[camera_ch].featureIpt[i*2];
						feature_point[i].u16Y = (PP_U16 )Off_Calib_Out.stPcv_Param.stPatternParam[camera_ch].featureIpt[i*2+1];
					}					
					feature_point_num = Off_Calib_Out.stPcv_Param.stPatternParam[camera_ch].featureCount;
#if defined(USE_PP_GUI)

					apps_pos_point = (PP_POS_S *)&feature_point;
					apps_pos_point_num = feature_point_num;
					PPAPI_DISPLAY_PROGRESSBAR_Off();					
					PPAPI_DISPLAY_POPUP_Off();
										
#endif
					if(Off_Calib_Out.u32Try_cnt==0)
					{
#if defined(USE_PP_GUI)
						PPAPI_DISPLAY_CALIB_Point(apps_pos_point, apps_pos_point_num);					
#endif
						PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_WAIT_CMD);
						AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
#if defined(USE_PP_GUI)
						AppTask_SendCmd(CMD_UI_DIALOG, TASK_UI, TASK_UI, eDialog_ManualCalib, NULL_PTR, 0, 1000);
#endif
						Off_Calib_Out.u32Try_cnt++;

					}
					else
					{
#if defined(USE_PP_GUI)
						PPAPI_DISPLAY_DIALOG_Off();
#endif
						PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_END);
						AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
					}
				}
				else
				{			
					PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_END);
					AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

					PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PP_NULL, PP_NULL); // preview
					if(multicore_calib_flag==1)
					{						
						AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, 2, NULL_PTR, 0, 1000); // attr: (0:manual calib) (1:move point) (2:view generation)
					}
					else
					{
						AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, 2, NULL_PTR, 0, 1000); // attr: (0:manual calib) (1:move point) (2:view generation)
					}
					AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				}
			}
			else
			{
				if(camera_ch!=eCALIB_CAMERA_ALL)
				{
				
					LOG_DEBUG("[%s/%d] fp NOT found \n", __FUNCTION__,__LINE__);
					
#if defined(USE_PP_GUI)
					PPAPI_DISPLAY_PROGRESSBAR_On(10, 0); //100% display
					PPAPI_DISPLAY_PROGRESSBAR_Off();
					PPAPI_DISPLAY_POPUP_Off();
					AppTask_SendCmd(CMD_UI_DIALOG, TASK_UI, TASK_UI, eDialog_CalibFailRetry, NULL_PTR, 0, 1000);
#endif
					PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_END);
					AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

					//PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_WAIT_CMD);
					//AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);



				}
				else
				{
					LOG_DEBUG("[%s/%d] fp NOT found \n", __FUNCTION__,__LINE__);
					
#if defined(USE_PP_GUI)
					PPAPI_DISPLAY_PROGRESSBAR_On(10, 0); //100% display
					PPAPI_DISPLAY_PROGRESSBAR_Off();
					PPAPI_DISPLAY_POPUP_Off();
					AppTask_SendCmd(CMD_UI_DIALOG, TASK_UI, TASK_UI, eDialog_CalibFailRetry, NULL_PTR, 0, 1000);
#endif

					PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_END);
					AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
					//PPAPP_Calib_SearchPoint(eCALIB_CAMERA_ALL); 	

					//AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					//AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					

				}
			}		

			
			//PPAPI_Lib_Debug_Memory();
			

		}
			break;
		case eOFFCALIB_END:
		{
			



			Off_Calib_Out.u32Try_cnt = 0;
		
			if(camera_ch==eCALIB_CAMERA_ALL)
			{
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_FRONT]);
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_LEFT]);
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_RIGHT]);
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_BACK]);
			}
			else if(camera_ch==eCALIB_CAMERA_FRONT)
			{
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_FRONT]);
			}
			else if(camera_ch==eCALIB_CAMERA_LEFT)
			{
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_LEFT]);
			}
			else if(camera_ch==eCALIB_CAMERA_RIGHT)
			{
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_RIGHT]);
			}
			else if(camera_ch==eCALIB_CAMERA_BACK)
			{
				PPAPI_Lib_Ext_Free((PP_VOID *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[eCALIB_CAMERA_BACK]);
			}

			

			
			PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_WAIT_CMD);
			AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
			
			
			PPAPI_Lib_Debug_Memory();
			
			prvMemMangCALIBInit_All();
			
			heap_alloc_ext_try_cnt=0;
			heap_free_ext_try_cnt=0;
			heap_alloc_pcv_try_cnt=0;
			heap_free_pcv_try_cnt=0;

			end_tick = GetTickCount();
			LOG_DEBUG("!END_CALIB. total spend time to calib_main() = %d ms\n",end_tick-start_tick);
					 

				 
		}
			break;
	}
	
}




PP_U32 PPAPP_Section_Viewgen_Main(PP_U16  IN step)
{
	STATIC PP_U32 cur_sec_num=0;

	STATIC PP_U32 swing_degree=0;


	
	
	PPAPI_Section_Viewgen_Set_Step_Send_Cmd(step);

	switch((PP_U32)PPAPI_Section_Viewgen_Get_Step_Send_Cmd())
	{
		case eSVIEWGEN_START:
		{
			LOG_DEBUG("**	VIEWGEN_START\n");
			heap_alloc_ext_try_cnt=0;
			heap_free_ext_try_cnt=0;
			heap_alloc_pcv_try_cnt=0;
			heap_free_pcv_try_cnt=0;
						

			start_tick = GetTickCount();



		
			
#if defined(USE_PP_GUI)
			PPAPI_DISPLAY_POPUP_On(ePopUp_ViewGen);
#endif

			
			
			#ifdef USE_RESERVED_MEM
		
				prvMemMangCALIBInit_All();
				
				//PPAPI_Lib_Debug_Memory();
			#endif
			

			PPAPI_Viewgen_Get_External_Value();	

			
			//PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
			//AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

			
			gMulticore_Viewgen_Flag=0;
			PPAPI_IPC_Core0_Viewgen_Send_Start(0xffffffff);
			
			PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_WAIT_CMD);
			AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

			

		}
			break;



			
		case eSVIEWGEN_PREPARE_MAKE:
		{
			func_start_tick = GetTickCount();

			#if 1 // skip cylindrical view
			if(cur_sec_num<=8 && (Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL||Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT)) //cam view
			{	
				cur_sec_num++;
				PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
				AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				break;
			}
			#endif
			PPAPI_Section_Viewgen_Prepare_Make(cur_sec_num,swing_degree);	
			PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_GET_FB_LUT);
			AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
		}
			break;	
		case eSVIEWGEN_GET_FB_LUT:
		{
			if(cur_sec_num<Calib_Cnf_Bin.mdViewHeader.viewCount-1)
			{
				if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_2D) //2d top view
				{			
					PPAPI_Viewgen_Make_Top_2d_FB_LUT();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_3D) //3d view
				{
					PPAPI_Viewgen_Make_RS_3D_FB_LUT();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_SD) //3d view
				{
					PPAPI_Viewgen_Make_RS_SD_FB_LUT();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT) //cam view
				{
					PPAPI_Viewgen_Make_RS_UNDIS_FB_LUT();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL) //cam view
				{
					PPAPI_Viewgen_Make_RS_CYLIND_FB_LUT();
				}

			}
			else
			{
				PPAPI_Viewgen_Make_RS_3D_FB_LUT();
			}
			
			PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_GET_LR_LUT);
			AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
			//PPAPI_Lib_Debug_Memory();

		}
			break;		
	
		case eSVIEWGEN_GET_LR_LUT:
		{
		//	LOG_DEBUG("**	[topview : STEP(2/5) VIEWGEN_GET_TOP_2D_LR_LUT\n");
			if(cur_sec_num<Calib_Cnf_Bin.mdViewHeader.viewCount-1)
			{
				if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_2D) //2d top view
				{			
					PPAPI_Viewgen_Make_Top_2d_LR_LUT();				
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_3D) //3d view
				{
					PPAPI_Viewgen_Make_RS_3D_LR_LUT();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_SD) //3d view
				{
					PPAPI_Viewgen_Make_RS_SD_LR_LUT();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT) //cam view
				{
					PPAPI_Viewgen_Make_RS_UNDIS_LR_LUT();
				}
				
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL) //cam view
				{
					PPAPI_Viewgen_Make_RS_CYLIND_LR_LUT();
				}
			}
			else
			{
				PPAPI_Viewgen_Make_RS_3D_LR_LUT();

			}
			PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_UPDATE);
			AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
			//PPAPI_Lib_Debug_Memory();
			

		}
			break;
		case eSVIEWGEN_UPDATE:
		{
			//PPAPI_Debug_Save_Data_SD(0,pFBLut_2DTOP,528*720*4);
			//PPAPI_Debug_Save_Data_SD(1,pLRLut_2DTOP,528*720*4);
		
			if(cur_sec_num<Calib_Cnf_Bin.mdViewHeader.viewCount-1)
			{
				if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_2D) //2d top view
				{							
					PPAPI_Section_Viewgen_Update(cur_sec_num,(PP_U32)pFBLut,(PP_U32)pLRLut);
					PPAPI_Viewgen_Free_2DTOP_Values();				
				
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_3D) //3d view
				{
					PPAPI_Section_Viewgen_Update(cur_sec_num,(PP_U32)pFBLut,(PP_U32)pLRLut);
					PPAPI_Viewgen_Free_3D_Values();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_SD) //3d view
				{
					PPAPI_Section_Viewgen_Update(cur_sec_num,(PP_U32)pFBLut,(PP_U32)pLRLut);
					PPAPI_Viewgen_Free_SD_Values();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT) //cam view
				{
					PPAPI_Section_Viewgen_Update(cur_sec_num,(PP_U32)pFBLut,(PP_U32)pLRLut);
					PPAPI_Viewgen_Free_UNDIS_Values();
				}
				else if(Calib_Cnf_Bin.mdViewHeader.subView[cur_sec_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL) //cam view
				{
					PPAPI_Section_Viewgen_Update(cur_sec_num,(PP_U32)pFBLut,(PP_U32)pLRLut);
					PPAPI_Viewgen_Free_CYLIND_Values();
				}
			
			}
			else
			{
				PPAPI_Section_Viewgen_Update(cur_sec_num,(PP_U32)pFBLut,(PP_U32)pLRLut);
				PPAPI_Viewgen_Free_3D_Values();

			}
		
			//LOG_DEBUG(" 	[CORE0_VIEWGEN]cur_sec_num/swing_degree = [%d/%d degree]\n",cur_sec_num,swing_degree);

			if(gMulticore_Viewgen_Flag)
			{
			
				//PPAPI_Lib_GetCameraProjectPoint();//test smoh for ADAS
				
				if(cur_sec_num>=Calib_Cnf_Bin.mdViewHeader.viewCount-1) //swing view code
				{
					cur_sec_num+=Calib_Cnf_Bin.swingView.flbInterval/Calib_Cnf_Bin.swingView.degreeInterval;					
					swing_degree=swing_degree+Calib_Cnf_Bin.swingView.flbInterval;
				
					if(cur_sec_num==(Calib_Cnf_Bin.mdViewHeader.viewCount-1)+(CORE0_VIEWGEN_CNT-Calib_Cnf_Bin.mdViewHeader.viewCount+1)*Calib_Cnf_Bin.swingView.flbInterval/Calib_Cnf_Bin.swingView.degreeInterval)
					{
						if(gMulticore_Viewgen_core1_done_Flag)
						{
							PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_END);
							AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
						}
						else
						{
							PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_WAIT_CMD);
							AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
						}
					}
					else
					{
						PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
						AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);

					}
				
				}
				else
				{
					cur_sec_num++;
					
					PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
					AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				
				}
				
			}
			else
			{
				if(cur_sec_num>=Calib_Cnf_Bin.mdViewHeader.viewCount-1) //swing view code
				{
					cur_sec_num+=Calib_Cnf_Bin.swingView.flbInterval/Calib_Cnf_Bin.swingView.degreeInterval;					
					swing_degree=swing_degree+Calib_Cnf_Bin.swingView.flbInterval;

					
					if(swing_degree>=360)
					{
						PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_END);
						AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
						
					}
					else
					{
						PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
						AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
					
					}
				}
				else
				{			
				
					cur_sec_num++;
					
					PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
					AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				}
			}


			if(cur_sec_num==2)
			{
				PPAPI_SVMMEM_LoadCommonSectionData(cur_sec_num, PP_TRUE); 
				PPAPI_SVM_SetView(eVIEWMODE_LOAD_TOP2D_FRONTSD, PP_NULL, PP_NULL); // preview
			}
			
			func_end_tick = GetTickCount();
			LOG_DEBUG(" [%d] section view time 11111to make  = %d ms\n",cur_sec_num,func_end_tick-func_start_tick);
			//PPAPI_Lib_Debug_Memory();

			

		}
			break;			
		case eSVIEWGEN_END:
		{
			LOG_DEBUG("**	VIEWGEN_END\n");
			#if defined(USE_PP_GUI)
			PPAPI_DISPLAY_PROGRESSBAR_On(10, 0);
			PPAPI_DISPLAY_PROGRESSBAR_Off();
			#endif
			end_tick = GetTickCount();

			

		
			PPAPI_Viewgen_Free_Global_Values();
			
			
			PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_WAIT_CMD);
			AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			
			PPAPI_Lib_Debug_Memory();
			LOG_DEBUG("END_VIEWGEN. total spend time to calib_main() = %d ms\n",end_tick-start_tick);
			heap_alloc_ext_try_cnt=0;
			heap_free_ext_try_cnt=0;
			heap_alloc_pcv_try_cnt=0;
			heap_free_pcv_try_cnt=0;
			cur_sec_num=0;
			swing_degree=0;

			
			gMulticore_Viewgen_Flag=3;

			
			total_end_tick = GetTickCount();
			LOG_DEBUG("calibration+viewgen time = [%d] ms / [%d] sec \n",total_end_tick-total_start_tick,(total_end_tick-total_start_tick)/1000);

			//LOG_DEBUG("do rebooting");
			//PPAPI_SYS_Reset();
			prvMemMangCALIBInit_All();

			#if defined(USE_PP_GUI)
			PPAPP_CACHE_Reload(PP_TRUE,PP_TRUE);
			PPAPI_DISPLAY_POPUP_On(ePopUp_Done);			
			OSAL_sleep(1000);
			PPAPI_DISPLAY_POPUP_Off();
			#endif



		}
			break;	
		case eSVIEWGEN_WAIT_CMD:
		{
			if(gMulticore_Viewgen_Flag==1)
			{
				PPAPI_IPC_Core0_Viewgen_Send_SVMcnf(0xffffffff);
				PPAPI_IPC_Core0_Viewgen_Send_OffCalibcnf(0xffffffff);
				
				PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
				AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				gMulticore_Viewgen_Flag=2;

			}


			if(gMulticore_Viewgen_Flag==0)
			{
				if(gMulticore_Viewgen_Wait_Cnt>100)
				{
					gMulticore_Viewgen_Flag=0;
					PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_PREPARE_MAKE);
					AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				}
				else
				{
					gMulticore_Viewgen_Wait_Cnt++;
					
					PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_WAIT_CMD);
					AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				}

	
			}
		}
			break;	


			
	}
	


	return 0;

}


PP_VOID PPAPP_Section_Viewgen_Get_Req_Core1_Update(PP_U32 section_num)
{
	PPAPI_Section_Viewgen_Update(section_num,(PP_U32)pCore1_FBLut,(PP_U32)pCore1_LRLut);
}

PP_VOID PPAPP_Section_Viewgen_Core1_Done(PP_VOID)
{

	if((PP_U32)PPAPI_Section_Viewgen_Get_Step_Send_Cmd()!=eSVIEWGEN_WAIT_CMD)
	{
		LOG_DEBUG("**	Waiting Core0 END\n");
		gMulticore_Viewgen_core1_done_Flag = 1;
	}
	else
	{

	
		PPAPI_Section_Viewgen_Set_Step_Send_Cmd(eSVIEWGEN_END);
		AppTask_SendCmd(CMD_VIEWGEN_START, TASK_CALIBRATION, TASK_CALIBRATION, PPAPI_Section_Viewgen_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
		LOG_DEBUG("**	CORE1& CORE0 ALL VIEWGEN_END\n");

	}
}




#else // //CALIB_LIB_USE


#include "app_calibration.h"




















#endif //CALIB_LIB_USE



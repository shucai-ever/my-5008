#include <stdio.h>
#include <unistd.h>
#include <nds32_intrinsic.h>

#include "ver.h"
#include "type.h"
#include "system.h"
#include "debug.h"
#include "osal.h"
#include "cache.h"
#include "pinmux.h"
#include "clock.h"

#include "adc.h"
#include "task_manager.h"

#include "proc.h"
#include "sub_intr.h"
#include "spi.h"
#include "du_drv.h"
#include "api_vin.h"
#include "api_vpu.h"
#include "uart_drv.h"
#include "api_diag.h"
#include "sys_api.h"
#include "dma.h"
#include "wdt.h"
#include "timer.h"
#include "api_ipc.h"
#include "task_uartcon.h"
#include "api_svm.h"
#include "task_display.h"
#include "task_cache.h"
#include "api_pvitx_func.h"
#include "api_pvirx_func.h"
#include "dram.h"
#include "api_flash.h"
#include "api_FAT_FTL.h"
#include "board_config.h"
#include "def_enum_name.h"
#include "api_display.h"
#include "task_fwdn.h"
#include "api_calibration.h"
#if defined(USE_PP_GUI)
#include "application.h"
#endif
#include "gpio.h"

#define NAND_FTL_TEST	0

/***************************************************************************************************************************************************************
 * main
***************************************************************************************************************************************************************/
int main(void)
{
	PP_U32 u32WaitCnt = 0;
    PP_U32 u32ChipID = 0, u32SvmVer = 0, u32VpuVer = 0, u32DuVer = 0, u32BootSection;

    /* System initialize independent OS */
    {
        cache_enable();

        init_pinmux();
        init_pad_io();

        SYS_PROC_initialize();
        AppUARTCon_Initialize(eBAUDRATE_115200, eDATA_BIT_8, eSTOP_BIT_1, ePARITY_NONE);
        PPDRV_GPIO_Initialize();

        //Identify Ver, Chip ID 
        {/*{{{*/
            LOG_DEBUG("\n\n### PI5008 of PixelPlus Ver: App[%s] ([%s]) ###\n", __PI5008_VER__, __DATE__);

            LOG_DEBUG("VIDEO_IN_TYPE:%s\n", pNameVideoInType[VIDEO_IN_TYPE]);
            LOG_DEBUG("   CAMERA_IN: ");
#if ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_MIPI_BAYER)
            LOG_DEBUG("%s, ", pNameMipiVidBit[(BD_CAMERA_IN_FMT&MIPI_VID_BIT_MASKBIT)>>MIPI_VID_BIT_BITSHIFT]);
#elif ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_MIPI_YUV)
            LOG_DEBUG("%s, ", pNameMipiVidBit[(BD_CAMERA_IN_FMT&MIPI_VID_BIT_MASKBIT)>>MIPI_VID_BIT_BITSHIFT]);
#elif ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_PVI)
            LOG_DEBUG("%s, ", pNameVidStandard[(BD_CAMERA_IN_FMT&VID_STANDARD_MASKBIT)>>VID_STANDARD_BITSHIFT]);
#elif ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_PARALLEL) //reserved
            LOG_DEBUG("%s, ", pNameVidType[(BD_CAMERA_IN_FMT&VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT]);
#else
 #error "Invalid BD_CAMERA_IN_FMT"
#endif 
            LOG_DEBUG("%s, ", pNameVidResol[(BD_CAMERA_IN_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidFrame[(BD_CAMERA_IN_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("   SVM_IN: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_SVM_IN_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidFrame[(BD_SVM_IN_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("   SVM_OUT: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_SVM_OUT_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidFrame[(BD_SVM_OUT_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("   DU_IN: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_DU_IN_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidFrame[(BD_DU_IN_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("   DU_OUT: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_DU_OUT_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s, ", pNameVidFrame[(BD_DU_OUT_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidType[(BD_DU_OUT_FMT&VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT]);
            LOG_DEBUG("   QUAD_OUT: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_QUAD_OUT_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidFrame[(BD_QUAD_OUT_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("   VPU_IN: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_VPU_IN_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s\n", pNameVidFrame[(BD_VPU_IN_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("   REC_OUT: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_RO_OUT_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s, ", pNameVidFrame[(BD_RO_OUT_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("%s, ", pNameVidType[(BD_RO_OUT_FMT&VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT]);
            LOG_DEBUG("MuxChCnt:%d\n", BD_RO_OUT_MUXCH_CNT);
#if ( (BD_PVITX_OUT_FMT & PVITX_SRC_MASKBIT) != PVITX_SRC_NONE )
            LOG_DEBUG("   PVITX_OUT: ");
            LOG_DEBUG("%s, ", pNameVidResol[(BD_PVITX_OUT_FMT&VID_RESOL_MASKBIT)>>VID_RESOL_BITSHIFT]);
            LOG_DEBUG("%s, ", pNameVidFrame[(BD_PVITX_OUT_FMT&VID_FRAME_MASKBIT)>>VID_FRAME_BITSHIFT]);
            LOG_DEBUG("%s, ", pNameVidStandard[(BD_PVITX_OUT_FMT&VID_STANDARD_MASKBIT)>>VID_STANDARD_BITSHIFT]);
            LOG_DEBUG("%s\n", pNamePviTxSrc[(BD_PVITX_OUT_FMT&PVITX_SRC_MASKBIT)>>PVITX_SRC_BITSHIFT]);
#endif

            u32ChipID = PPAPI_SYS_GetChipID();
            LOG_DEBUG("CHIP (RevID[%02X], PackOpt[%s], BootMD[%s]), ", (u32ChipID&0xFF), pNameChipOption[(u32ChipID>>8)&0x3], pNameBootMode[(u32ChipID>>12)&0x7]);

            PPAPI_SYS_SetDeviceID(0x5008);

            u32SvmVer = PPAPI_SVM_GetVersion();
            LOG_DEBUG("SVM_ID[%08X] ", u32SvmVer);    //0x17101800(fixed value)
            if(u32SvmVer != 0x17101800)
            {
                LOG_DEBUG("ERROR!!!. SVM Ver ID fail. Check ID(0x17101800).\n");
            }

            u32VpuVer = PPAPI_VPU_GetVer();
            LOG_DEBUG("VPU_ID[%08X] ", u32VpuVer);    //0x20171202(fixed value)
            if(u32VpuVer != 0x20171202)
            {
                LOG_DEBUG("ERROR!!!. VPU Ver ID fail. Check ID(0x20171202).\n");
            }

            u32DuVer = PPDRV_DU_GetVersion();
		    LOG_DEBUG("DU_ID[%08X] ", u32DuVer);    //0x20171130(fixed value)
		    if(u32DuVer != 0x20171130)
            {
                LOG_DEBUG("ERROR!!!. DU Ver ID fail. Check ID(0x20171130).\n");
            }

		    LOG_DEBUG("\n");

		    u32BootSection = PPAPI_SYS_GetBootSection();
		    if((u32BootSection & 0xff) == eFLASH_SECT_IFW1){
		    	LOG_DEBUG("ISP f/w damaged. Upgrade ISP firmware\n");
		    }
		    if(((u32BootSection >> 8) & 0xff) == eFLASH_SECT_MFW1){
		    	LOG_DEBUG("Main f/w damaged. Upgrade Main firmware\n");
		    }


        }/*}}}*/

        {
            extern char __bss_start;
            extern char _end;
            uint32 size = 0;
            extern char _stack;

#if !defined(FPGA_ASIC_TOP)
        	LOG_DEBUG("cpu: %lf MHz, ddr: %lf MHz, apb: %lf MHz, timer: %lf MHz, uart: %lf MHz\n", (double)u32CPUClk/1000000, (double)u32DDRClk/1000000, (double)u32APBClk/1000000, (double)u32TimerClk/1000000, (double)u32UARTClk/1000000);
#endif //!defined(FPGA_ASIC_TOP)

            LOG_DEBUG("tick rate:%d\n",(uint32)configTICK_RATE_HZ);
            LOG_DEBUG("stack addr:0x%08x(last dram addr)\n", (uint32)&_stack);
            size = &_end - &__bss_start;
            LOG_DEBUG("bss_start:0x%08x, end:0x%08x, size:%d\n", (uint32)((uint32 *)&__bss_start), (uint32)((uint32 *)&_end), (uint32)size);
            LOG_DEBUG("heap total:%d, avail:%d\n", (uint32)configTOTAL_HEAP_SIZE, (uint32)xPortGetFreeHeapSize());
            LOG_DEBUG("stack minimal size:%d\n", (uint32)configMINIMAL_STACK_SIZE);
        }

#if NAND_FTL_TEST
        if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NAND) != eSUCCESS){
        //if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NOR) != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash init fail. Halt\n");
			while(1);
		}
		if(0){
		    PP_FTL_STATS_S stStats;
	    	if(PPAPI_FTL_Initialize(gstFlashNandID.u8FTLDeviceID, &stStats) != eSUCCESS){
				LOG_DEBUG("ERROR!!! Flash FTL init fail. Halt\n");
				while(1);
	    	}
			LOG_DEBUG("FTL total blocks: %d\n", stStats.u32BlocksTotal);
			LOG_DEBUG("FTL Reserved blocks: %d\n", stStats.u32BlocksReserved);
			LOG_DEBUG("FTL Bad blocks: %d\n", stStats.u32BlocksBad);
			LOG_DEBUG("FTL Pages per block: %d\n", stStats.u32PagesPerBlock);
			LOG_DEBUG("FTL Bytes per page: %d\n", stStats.u32BytesPerPages);

		}

#else

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
        if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NOR) != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash init fail. Halt\n");
			while(1);
		}
		if(PPAPI_FLASH_ReadHeader() != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash data invalid. Halt\n");
			while(1);
		}
#else
        if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NAND) != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash init fail. Halt\n");
			while(1);
		}

		{
		    PP_FTL_STATS_S stStats;


	    	if(PPAPI_FTL_Initialize(gstFlashNandID.u8FTLDeviceID, &stStats) != eSUCCESS){
				LOG_DEBUG("ERROR!!! Flash FTL init fail. Halt\n");
				while(1);
	    	}


			LOG_DEBUG("FTL total blocks: %d\n", stStats.u32BlocksTotal);
			LOG_DEBUG("FTL Reserved blocks: %d\n", stStats.u32BlocksReserved);
			LOG_DEBUG("FTL Bad blocks: %d\n", stStats.u32BlocksBad);
			LOG_DEBUG("FTL Pages per block: %d\n", stStats.u32PagesPerBlock);
			LOG_DEBUG("FTL Bytes per page: %d\n", stStats.u32BytesPerPages);
		}
		if(PPAPI_FLASH_ReadHeader() != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash data invalid. Halt\n");
			while(1);
		}

		if(PPAPI_FLASH_ReadFTLHeader() != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash FTL data invalid. Halt\n");
			while(1);
		}

#endif	//#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)

#endif	//#if NAND_FTL_TEST


#ifdef CACHE_VIEW_USE
		PPAPI_SVMMEM_Initialize(eVIEWMODE_LOAD_TOP2D_SWING_START, eVIEWMODE_LOAD_TOP2D_SWING_MAX);
#else
		PPAPI_SVMMEM_Initialize(VIEWMODE_NULL, VIEWMODE_NULL);
#endif

        init_dram_reserved();

        PPAPI_IPC_Initialize();

        SYSAPI_UART_initialize(eUART_CH_1, eBAUDRATE_115200, eDATA_BIT_8, eSTOP_BIT_1, ePARITY_NONE);

        PPDRV_DMA_Initialize();
        PPDRV_WDT_Initialize(eWDT_TIMEOUT_POW2_23);		// 1.2 sec

        PPDRV_ADC_Initialize();
        PPAPI_VIN_Initialize();
        PPDRV_TIMER_AddProc();

#if !NAND_FTL_TEST

		//Enable Sub irq. 
	    SUB_INTR_Initialize();

        if(u32SvmVer)
        {
            PPAPI_SVM_Initialize();
        }
        if(u32DuVer)
        {
			#if defined(USE_PP_GUI)
			PPAPI_DISPLAY_LoadHeader();
			#endif // USE_PP_GUI
			PPAPI_DISPlAY_Initialize();
        }

#if defined(USE_PP_GUI)//CALIB_LIB_USE
// calib_initialize function origianlly for Calibration application
// but some information is used by GUI 
// so if use GUI, it should be called.
        PPAPI_Lib_Calib_Initialize();
#endif

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
        PPAPI_PVIRX_Initialize();
#endif //(VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
#if ( (BD_PVITX_OUT_FMT & PVITX_SRC_MASKBIT) != PVITX_SRC_NONE )
        PPAPI_PVITX_Initialize();
#endif

        PPAPI_DIAG_Initialize();
#endif

        if(u32VpuVer)
        {
            PPAPI_VPU_Initialize();
        }

        if(PPAPI_FATFS_Initialize()){
        	LOG_DEBUG("FATFS_Initialize fail(%d)\n", PPAPI_FATFS_GetLastError());
        }
    }

#ifdef CACHE_VIEW_USE
	PPAPP_CACHE_LoadData(eVIEWMODE_LOAD_TOP2D_SWING_START, eVIEWMODE_LOAD_TOP2D_SWING_MAX, -1);
#endif

#if 0
#include "task_fwdn.h"
    FWDN_FLASH_UpgradeTemp("flash_image_1.bin");
    while(1);
#endif
	////////////////////////////////////////////////////////////////////////////

    //Create Queue,EventGroup,Thread
	AppTask_Init();

#if !NAND_FTL_TEST
    // waiting for Stable input from ISP/PVI ready
    u32WaitCnt = 0;
    do
    {
        if(PPAPI_IPC_CheckISPReady())
            break;
    } while ( u32WaitCnt++ < 0x00100000 );
    if(u32WaitCnt >= 0x00100000)
    {
        LOG_DEBUG("Error! wait core1 stable input.\n");
    }
    else
    {
        LOG_DEBUG("Core1 Ready done.\n");
    }

    PPAPI_VIN_SetGenlockParam();//lowTH highTH

	#if defined(USE_PP_GUI)
	// Logo disable & display view mode
    if(u32DuVer && u32SvmVer)
	{
		PP_SCENE_S *liveView = NULL_PTR;
		liveView = PPAPP_UI_GetScene(eScene_LiveView);
		PPAPP_UI_LiveView(liveView, CMD_UI_INIT_SCENE);
	}
	#else
	PPAPI_SVM_SetOutputReplaceColorOnOff(PP_FALSE);
	#endif // USE_PP_GUI
	
#endif

#if MCU_CTRL_METHOD
	PPDRV_SPI_Initialize(eSPI_CHANNEL_1, 1, 0, 0, 8 );
	PPDRV_SPI_IRQEnable(eSPI_CHANNEL_1, SPI_INTR_RXFIFO_THRD|SPI_INTR_TRANSFER_END|SPI_INTR_RXFIFO_OVER);
#endif

    //start vTaskStartScheduler();
	OSAL_start_os();

	// No Execution
	 
	return 1;
}

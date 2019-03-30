#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

//#include "prj_config_def_enum.h"

/*==================================================*/
/*															*/
/*	Below Definitions that SHOULD NOT BE MODIFIED!	*/
/*															*/
/*==================================================*/

//==========================================
//	A. Video input type
//==========================================
#define VIDEO_IN_TYPE_MIPI_BAYER			0 //Bayer format
#define VIDEO_IN_TYPE_MIPI_YUV			    1 //YUV format
#define VIDEO_IN_TYPE_PVI					2
#define VIDEO_IN_TYPE_PARALLEL				3 //reserved
//==========================================
//	B. Mipi bayer/yuv raw bit format
//==========================================
#define MIPI_VID_BIT_BITSHIFT       (0)
#define MIPI_VID_BIT_MASKBIT        (0xF<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_RAW6			(0<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_RAW7			(1<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_RAW8			(2<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_RAW10			(3<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_RAW12			(4<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_RAW14			(5<<MIPI_VID_BIT_BITSHIFT)
#define MIPI_VID_BIT_YUV8_2XRAW8	(6<<MIPI_VID_BIT_BITSHIFT) //YUV8 or 2x8 Raw
#define MIPI_VID_BIT_2XRAW10		(7<<MIPI_VID_BIT_BITSHIFT)
//==========================================
//	C. Video Signal format: yuv8/16, embedded/external sync, RGB, Bayer8/10
//==========================================
#define VID_TYPE_BITSHIFT 			(4)
#define VID_TYPE_MASKBIT 			(0xF<<VID_TYPE_BITSHIFT)
#define VID_TYPE_YC8_EMB			(0<<VID_TYPE_BITSHIFT)
#define VID_TYPE_YC8_EXT			(1<<VID_TYPE_BITSHIFT)
#define VID_TYPE_YC16_EMB			(2<<VID_TYPE_BITSHIFT)
#define VID_TYPE_YC16_EXT			(3<<VID_TYPE_BITSHIFT)
#define VID_TYPE_RGB24				(4<<VID_TYPE_BITSHIFT)
#define VID_TYPE_YUV24				(5<<VID_TYPE_BITSHIFT)
#define VID_TYPE_BAYER_8BIT			(6<<VID_TYPE_BITSHIFT)
#define VID_TYPE_BAYER_10BIT		(7<<VID_TYPE_BITSHIFT)
//==========================================
//	D. Video Frame: 25/30/50/60
//==========================================
#define VID_FRAME_BITSHIFT 			(8)
#define VID_FRAME_MASKBIT 			(0xF<<VID_FRAME_BITSHIFT)
#define VID_FRAME_NTSC_30 			(0<<VID_FRAME_BITSHIFT)
#define VID_FRAME_PAL_25 			(1<<VID_FRAME_BITSHIFT)
#define VID_FRAME_NTSC_60 			(2<<VID_FRAME_BITSHIFT)
#define VID_FRAME_PAL_50 			(3<<VID_FRAME_BITSHIFT)
//==========================================
//	E. Video Resolution
//==========================================
#define VID_RESOL_BITSHIFT 			(12)
#define VID_RESOL_MASKBIT 			(0xF<<VID_RESOL_BITSHIFT)
#define VID_RESOL_SD720H 			(0<<VID_RESOL_BITSHIFT) //CVBS 720x480i, 720x576i
#define VID_RESOL_SD960H 			(1<<VID_RESOL_BITSHIFT) //CVBS 960x480i, 960x576i
#define VID_RESOL_SDH720 			(2<<VID_RESOL_BITSHIFT) //Reserved. 720x480p, 720x576p
#define VID_RESOL_SDH960 			(3<<VID_RESOL_BITSHIFT) //Reserved. 960x480p, 960x576p
#define VID_RESOL_HD720P 			(4<<VID_RESOL_BITSHIFT) //1280x720p
#define VID_RESOL_HD960P 			(5<<VID_RESOL_BITSHIFT) //1280x960p
#define VID_RESOL_HD1080P 			(6<<VID_RESOL_BITSHIFT) //1920x1080p
#define VID_RESOL_HD800_480P 		(7<<VID_RESOL_BITSHIFT) //800x480p
#define VID_RESOL_HD1024_600P 		(8<<VID_RESOL_BITSHIFT) //1024x600p
//==========================================
//	F. Analog Video Standard
//==========================================
#define VID_STANDARD_BITSHIFT 		(16)
#define VID_STANDARD_MASKBIT 		(0xF<<VID_STANDARD_BITSHIFT)
#define VID_STANDARD_CVBS 		    (0<<VID_STANDARD_BITSHIFT) //CVBS (720x480i, 720x576i, 960x480i, 960x576i)
#define VID_STANDARD_SDH 			(1<<VID_STANDARD_BITSHIFT) //Reserved (720x480p, 720x576p)
#define VID_STANDARD_PVI 			(2<<VID_STANDARD_BITSHIFT) //Analog HD PVI(pixelplus) standard.
#define VID_STANDARD_CVI 			(3<<VID_STANDARD_BITSHIFT) //Analog HD CVI(Dahwa) standard.
#define VID_STANDARD_HDT 			(4<<VID_STANDARD_BITSHIFT) //Analog HD TVI(HikVision) standard.
#define VID_STANDARD_HDA 			(5<<VID_STANDARD_BITSHIFT) //Analog HD AHD(NextChip) standard.
//==========================================
//	G. PARALLEL Type 
//==========================================
#define VID_PARALLEL_TYPE_BITSHIFT	(20)
#define VID_PARALLEL_TYPE_MASKBIT	(0xF<<VID_PARALLEL_TYPE_BITSHIFT)
#define VID_PARALLEL_TYPE_VIN_BAYER	(0x0<<VID_PARALLEL_TYPE_BITSHIFT) //Bayer
#define VID_PARALLEL_TYPE_VIN_YUV	(0x1<<VID_PARALLEL_TYPE_BITSHIFT) //YUV
//==========================================
//	H. PVITX SOURCE
//==========================================
#define PVITX_SRC_BITSHIFT			(24)
#define PVITX_SRC_MASKBIT			(0xF<<PVITX_SRC_BITSHIFT)
#define PVITX_SRC_NONE				(0<<PVITX_SRC_BITSHIFT) // = don't use pvitx
#define PVITX_SRC_DU				(1<<PVITX_SRC_BITSHIFT) //Same with DU display
#define PVITX_SRC_QUAD				(2<<PVITX_SRC_BITSHIFT) //Quad

//==========================================
// I.FLASH TYPE Option
//==========================================
#define FLASH_TYPE_NOR				0
#define FLASH_TYPE_NAND				1

//==========================================
// J.UPGRADE METHOD OPTION
//==========================================
#define UPGRADE_METHOD_SDCARD				0
#define UPGRADE_METHOD_SPI					1

//==========================================
// K.MCU CONTROL METHOD OPTION
//==========================================
#define MCU_CTRL_METHOD_UART 				0
#define MCU_CTRL_METHOD_SPI 				1

/*================================================*/
/*												  */
/*	Below Definitions that SHOULD BE SET BY USER! */
/*												  */
/*================================================*/


//==========================================
// 1. Definitions dependent on the board.
//==========================================
//# Use FPGA board. If not use FPGA, comment. 
//#define FPGA_ASIC_TOP

//==========================================
// 2. Video Input Type DEFINITION
// VIDEO_IN_TYPE = (A.Video input type)
//==========================================
#define VIDEO_IN_TYPE	VIDEO_IN_TYPE_PVI // <--- user select
//                      ^^^^^^^^^^^^^^^^^^^^^^^^

//==========================================
// 3. Video Input Format DEFINITION
// BD_VIN_FMT = (E.Video Resolution)|(D.Video Frame)
//==========================================
#define BD_VIN_FMT	((VID_RESOL_HD720P) | (VID_FRAME_NTSC_30)) // <--- user select
//                    ^^^^^^^^^^^^^^^^     ^^^^^^^^^^^^^^^^

//==========================================
// 4. CAMERA Input Format DEFINITION
// MIPI_BAYER, MIPI_YUV:
//  BD_CAMERA_IN_FMT = (BD_VIN_FMT) | (B.MIPI Bit format)
// PVI:
//  BD_CAMERA_IN_FMT = (BD_VIN_FMT) | (F. Analog Video Standard)
//==========================================
#if ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_MIPI_BAYER)
 #define BD_CAMERA_IN_FMT		((BD_VIN_FMT) | (MIPI_VID_BIT_RAW12)) // <--- user select
//                                               ^^^^^^^^^^^^^^^^^^
#elif ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_MIPI_YUV)
 #define BD_CAMERA_IN_FMT		((BD_VIN_FMT) | (MIPI_VID_BIT_YUV8_2XRAW8))
#elif ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_PVI)
 #define BD_CAMERA_IN_FMT		((BD_VIN_FMT) | (VID_STANDARD_HDA)) // <--- user select
//                                               ^^^^^^^^^^^^^^^^
#elif ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_PARALLEL) //reserved
 #define BD_CAMERA_IN_FMT		((BD_VIN_FMT) | (MIPI_VID_BIT_RAW12))
#else
 #error "Invalid BD_CAMERA_IN_FMT"
#endif 

//==========================================
// 5. SVM VIDEO DEFINITION
// BD_SVM_IN_FMT = (BD_VIN_FMT)
// BD_SVM_OUT_FMT = (E.Video Resolution)|(D.Video Frame)
//==========================================
#define BD_SVM_IN_FMT	(BD_VIN_FMT)
#define BD_SVM_OUT_FMT	((VID_RESOL_HD720P) | (VID_FRAME_NTSC_30)) // <--- user select
//                        ^^^^^^^^^^^^^^^^     ^^^^^^^^^^^^^^^^

//==========================================
// 6. DU VIDEO IN/OUT DEFINITION
// BD_DU_IN_FMT = (BD_SVM_OUT_FMT)
// BD_DU_OUT_FMT = (BD_DU_IN_FMT)|(C.Video Signal format)
//==========================================
#define BD_DU_IN_FMT	((BD_SVM_OUT_FMT))
#define BD_DU_OUT_FMT	((BD_DU_IN_FMT) | (VID_TYPE_RGB24)) // <--- user select
//                                         ^^^^^^^^^^^^^^^^

//==========================================
// 7. QUAD VIDEO OUT DEFINITION
// BD_QUAD_OUT_FMT = (BD_VIN_FMT)
//==========================================
#define BD_QUAD_OUT_FMT  (BD_VIN_FMT) 

//==========================================
// 8. VPU VIDEO IN DEFINITION
// BD_VPU_IN_FMT = (BD_QUAD_OUT_FMT)
//==========================================
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)

//==========================================
// 9. PARALLEL VIDEO IN DEFINITION <reserved>
// BD_VID_PARALLEL_IN_FMT = (BD_VIN_FMT)|(G.Parallel type)
//==========================================
#define BD_VID_PARALLEL_IN_FMT    ((BD_VIN_FMT) | (VID_PARALLEL_TYPE_VIN_BAYER))

//==========================================
// 10. Analog Video OUTPUT VIDEO IN DEFINITION
// BD_PVITX_OUT_FMT = (BD_VIN_FMT)||(F.Video Standard)| (H.PVITX SOURCE Option)
//==========================================
#if ( VIDEO_IN_TYPE	== VIDEO_IN_TYPE_PVI)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_HDA) | (PVITX_SRC_DU)) // <--- user select
//                                        ^^^^^^^^^^^^^^^^     ^^^^^^^^^^^^
#else
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_HDA) | (PVITX_SRC_NONE)) // PVITX_SRC_NONE = don't use pvitx
#endif

//==========================================
// 11. Record Out(RO) OUTPUT VIDEO IN DEFINITION
// BD_RO_OUT_FMT = (BD_VIN_FMT)||(C.Video Signal format)
//==========================================
#define BD_RO_OUT_FMT ((BD_VIN_FMT) | (VID_TYPE_YC16_EMB)) // <--- user select
//                                     ^^^^^^^^^^^^^^^^
#define BD_RO_OUT_MUXCH_CNT (1) //1,2,4. limit[1080p only support 1,2]  // <--- user select
//                           ^

//==========================================
// 12. Video Playback(PB)  VIDEO IN DEFINITION. <reserved> 
// Exclusive with VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL
// BD_VPU_IN_FMT = (BD_VIN_FMT)|(C.Video Signal format)
//==========================================
#if (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL)
#define VIDEO_IN_TYPE_PB_SUPPORT    (0) //support : 1
#define BD_VID_PB_IN_FMT        ((BD_VIN_FMT) | (VID_TYPE_RGB24)) // <--- user select
//                                               ^^^^^^^^^^^^^^^^^^^
#if ( ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT) )
# if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
# define BD_CAMERA_PB_IN_FMT     ((BD_VIN_FMT) | (MIPI_VID_BIT_YUV8_2XRAW8)) // <--- user select
//                                               ^^^^^^^^^^^^^^^^^^^^^^^^
# else
# define BD_CAMERA_PB_IN_FMT     ((BD_VIN_FMT) | (MIPI_VID_BIT_RAW12)) // <--- user select
//                                               ^^^^^^^^^^^^^^^^^^^
# endif
#else
#define BD_CAMERA_PB_IN_FMT     ((BD_VIN_FMT) | (MIPI_VID_BIT_YUV8_2XRAW8)) // <--- user select
//                                               ^^^^^^^^^^^^^^^^^^^^^^^^^
#endif
#endif

//==========================================
// 13. FLASH TYPE DEFINITION
// BD_FLASH_TYPE = (I.FLASH TYPE Option)
//==========================================
#define BD_FLASH_TYPE		FLASH_TYPE_NAND

//==========================================
// 14. SPI NAND FLASH DEFINITION
// SPI_NAND_NON_FTL_BLOCKS = (NAND FLASH NON FTL BLOCK TOTAL NUM)
//==========================================
//#define SPI_NAND_FLASH_BLOCK_TOTAL		(640)	// NonFTL: 640, FTL: 384
#define SPI_NAND_NON_FTL_BLOCKS				(640)	// NonFTL: 640, FTL: 384

//==========================================
// 15. Feature DEFINITION
// VPI_AVAILABLE = (0 or 1)
//==========================================
#define VPU_AVAILABLE	1	

//==========================================
// 16. FLASH CONTROLLER DEFINITION
// FLASH_CTRL_DIV = (0/2/4)
// FLASH_CTRL_DELAY = (0/2/4)
//==========================================
#ifdef FPGA_ASIC_TOP //TODO if chip, must do tunning.
#define FLASH_CTRL_DIV		4
#define FLASH_CTRL_DELAY	2
#else
#define FLASH_CTRL_DIV		0
#define FLASH_CTRL_DELAY	3
#endif

//==========================================
// 17. FLASH SPI CONTROLLER DEFINITION
// SPI_CTRL_DIV = (0 or 1)
//==========================================
#define SPI_CTRL_DIV		0

//==========================================
// 19. FLASH UPGRADE METHOD DEFINITION
// UPGRADE_METHOD = (J.UPGRADE METHOD OPTION)
//==========================================
#define UPGRADE_METHOD		UPGRADE_METHOD_SDCARD
//#define UPGRADE_METHOD		UPGRADE_METHOD_SPI

//==========================================
// 20. MCU CONTROL METHOD DEFINITION
// MCU_CTRL_METHOD = (K.MCU CONTROL METHOD OPTION)
//==========================================
//#define MCU_CTRL_METHOD 	MCU_CTRL_METHOD_SPI
#define MCU_CTRL_METHOD 	MCU_CTRL_METHOD_UART

//==========================================
// 20. WATCHDOG DEFINITION
//==========================================
#define WATCHDFOG_ENABLE	0	// 0: disable, 1: enable

//==========================================
// 20. STANDBY MODE CTRL DEFINITION
//==========================================
#define STANDBY_ENABLE		0
#define STANDBY_GPIO_GROUP	0	// GPIO device group number
#define STANDBY_GPIO_PIN	3	// GPIO pin number

/*================================================*/
/*												  */
/*	Below Definitions that are SET AUTOMOTICALLY! */
/*												  */
/*================================================*/

//==========================================
// 1. Video IN INTERLACE DEFINITION
//==========================================
#if ( ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
	#define BD_VIN_INTERLACE			(1)
#else
	#define BD_VIN_INTERLACE			(0)
#endif
//==========================================
// 2. Video OUT INTERLACE DEFINITION
//==========================================
#if ( ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
	#define BD_VOUT_INTERLACE			(1)
#else
	#define BD_VOUT_INTERLACE			(0)
#endif












































////////////////////////////////////////////////////////////////////////////////
// TEST
#if 0
// test redefine
#define TEST_SD720H_NTSC30 ((VID_RESOL_SD720H) | (VID_FRAME_NTSC_30))
#define TEST_SD720H_PAL25 ((VID_RESOL_SD720H) | (VID_FRAME_PAL_25))
#define TEST_SD960H_NTSC30 ((VID_RESOL_SD960H) | (VID_FRAME_NTSC_30))
#define TEST_SD960H_PAL25 ((VID_RESOL_SD960H) | (VID_FRAME_PAL_25))

#define TEST_SDH720_NTSC60 ((VID_RESOL_SDH720) | (VID_FRAME_NTSC_60))
#define TEST_SDH720_PAL50 ((VID_RESOL_SDH720) | (VID_FRAME_PAL_50))
#define TEST_SDH960_NTSC60 ((VID_RESOL_SDH960) | (VID_FRAME_NTSC_60))
#define TEST_SDH960_PAL50 ((VID_RESOL_SDH960) | (VID_FRAME_PAL_50))

#define TEST_HD720P_NTSC30 ((VID_RESOL_HD720P) | (VID_FRAME_NTSC_30))
#define TEST_HD720P_PAL25 ((VID_RESOL_HD720P) | (VID_FRAME_PAL_25))
#define TEST_HD720P_NTSC60 ((VID_RESOL_HD720P) | (VID_FRAME_NTSC_60))
#define TEST_HD720P_PAL50 ((VID_RESOL_HD720P) | (VID_FRAME_PAL_50))
#define TEST_HD960P_NTSC30 ((VID_RESOL_HD960P) | (VID_FRAME_NTSC_30))
#define TEST_HD960P_PAL25 ((VID_RESOL_HD960P) | (VID_FRAME_PAL_25))
#define TEST_HD1080P_NTSC30 ((VID_RESOL_HD1080P) | (VID_FRAME_NTSC_30))
#define TEST_HD1080P_PAL25 ((VID_RESOL_HD1080P) | (VID_FRAME_PAL_25))
#define TEST_HD800_480P_NTSC60 ((VID_RESOL_HD800_480P) | (VID_FRAME_NTSC_60))
#define TEST_HD800_480P_PAL50 ((VID_RESOL_HD800_480P) | (VID_FRAME_PAL_50))
#define TEST_HD1024_600P_NTSC60 ((VID_RESOL_HD1024_600P) | (VID_FRAME_NTSC_60))
#define TEST_HD1024_600P_PAL50 ((VID_RESOL_HD1024_600P) | (VID_FRAME_PAL_50))

#undef BD_VIN_FMT
#undef BD_CAMERA_IN_FMT
#undef BD_SVM_IN_FMT
#undef BD_SVM_OUT_FMT
#undef BD_DU_IN_FMT	
#undef BD_DU_OUT_FMT	
#undef BD_QUAD_OUT_FMT
#undef BD_VPU_IN_FMT
#undef BD_PVITX_OUT_FMT 
#if 1  //test 8bit 720h->720h
#define BD_VIN_FMT	(TEST_SD720H_NTSC30)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SD720H))
#define BD_SVM_IN_FMT	(TEST_SD720H_NTSC30)
#define BD_SVM_OUT_FMT	(TEST_SD720H_NTSC30)
//#define BD_SVM_OUT_FMT	(TEST_SDH720_PAL50)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SD720H) | (PVITX_SRC_DU))

	//test 720H->720p
 #if 1
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_HD720P_NTSC30)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT  (BD_VIN_FMT)
 //#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_VPU_IN_FMT  (BD_VIN_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #endif
#elif 0  //test 8bit 960h->960h
#define BD_VIN_FMT	(TEST_SD960H_NTSC30)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SD960H))
#define BD_SVM_IN_FMT	(TEST_SD960H_NTSC30)
#define BD_SVM_OUT_FMT	(TEST_SD960H_NTSC30)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SD960H) | (PVITX_SRC_DU))
 #if 0
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_HD1080P_NTSC30)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT (BD_VIN_FMT)
 #define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #endif
 #if 1
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_HD720P_NTSC60)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT  (BD_VIN_FMT)
 //#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_VPU_IN_FMT  (BD_VIN_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #endif
#elif 0 //test 8bit 720x480p60->720i60
#define BD_VIN_FMT	(TEST_SDH720_NTSC60)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SDH))
#define BD_SVM_IN_FMT	(TEST_SDH720_NTSC60)
#define BD_SVM_OUT_FMT	(BD_SVM_IN_FMT)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(TEST_SD720H_NTSC30 | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SDH) | (PVITX_SRC_DU))
#elif 0 //test 8bit 720x480p60->720p30
#define BD_VIN_FMT	(TEST_SDH720_NTSC60)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SDH))
#define BD_SVM_IN_FMT	(TEST_SDH720_NTSC60)
#define BD_SVM_OUT_FMT	(TEST_HD720P_NTSC30)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SDH) | (PVITX_SRC_DU))
#elif 0 //test 8bit 960x480p60->960x480i60
#define BD_VIN_FMT	(TEST_SDH960_NTSC60)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SDH))
#define BD_SVM_IN_FMT	(TEST_SDH960_NTSC60)
#define BD_SVM_OUT_FMT	(TEST_SDH960_NTSC60)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(TEST_SD960H_NTSC30 | (VID_TYPE_YC8_EMB))
//#define BD_DU_OUT_FMT	(BD_SVM_OUT_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SDH) | (PVITX_SRC_DU))
#elif 0 //test 8bit 960x480p60->960x480p60
#define BD_VIN_FMT	(TEST_SDH960_NTSC60)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SDH))
#define BD_SVM_IN_FMT	(TEST_SDH960_NTSC60)
#define BD_SVM_OUT_FMT	(TEST_HD720P_NTSC30)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SDH) | (PVITX_SRC_DU))
#elif 0 //test 8bit 960x576p50->960x576p50
#define BD_VIN_FMT	(TEST_SDH960_PAL50)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_SDH))
#define BD_SVM_IN_FMT	(TEST_SDH960_PAL50)
#define BD_SVM_OUT_FMT	(TEST_HD720P_PAL50)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SDH) | (PVITX_SRC_DU))
#elif 1 //test 8bit 720p30->720p30
#define BD_VIN_FMT	(TEST_HD720P_NTSC30)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_HDA))
#define BD_SVM_IN_FMT	(TEST_HD720P_NTSC30)
#define BD_SVM_OUT_FMT	(TEST_HD720P_NTSC30)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #if 0
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_HD1080P_NTSC30)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT (BD_VIN_FMT)
 #define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #endif
 #if 0
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_SDH720_NTSC60)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(TEST_SD720H_NTSC30 | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT (BD_VIN_FMT)
 #define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_SD720H) | (PVITX_SRC_DU))
 #endif
#elif 0 //test 8bit 720p60->720p60
#define BD_VIN_FMT	(TEST_HD720P_PAL50)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_PVI))
#define BD_SVM_IN_FMT	(TEST_HD720P_PAL50)
#define BD_SVM_OUT_FMT	(TEST_HD720P_PAL50)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
#elif 0 //test 16bit 960p30->960p30
#define BD_VIN_FMT	(TEST_HD960P_NTSC30)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_PVI))
#define BD_SVM_IN_FMT	(TEST_HD960P_NTSC30)
#define BD_SVM_OUT_FMT	(TEST_HD960P_NTSC30)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #if 0
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_HD720P_NTSC30)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT (BD_VIN_FMT)
 #define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #endif
 #if 0
 #undef BD_SVM_OUT_FMT
 #undef BD_DU_IN_FMT
 #undef BD_DU_OUT_FMT
 #undef BD_QUAD_OUT_FMT
 #undef BD_VPU_IN_FMT
 #undef BD_PVITX_OUT_FMT
 #define BD_SVM_OUT_FMT	(TEST_HD1080P_NTSC30)
 #define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
 #define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
 #define BD_QUAD_OUT_FMT (BD_VIN_FMT)
 #define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
 #define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
 #endif
#elif 0 //test 8bit 1080p30->1080p30
#define BD_VIN_FMT	(TEST_HD1080P_NTSC30)
#define BD_CAMERA_IN_FMT	((BD_VIN_FMT) | (VID_STANDARD_PVI))
#define BD_SVM_IN_FMT	(TEST_HD1080P_NTSC30)
#define BD_SVM_OUT_FMT	(TEST_HD1080P_NTSC30)
#define BD_DU_IN_FMT	(BD_SVM_OUT_FMT)
#define BD_DU_OUT_FMT	(BD_DU_IN_FMT | (VID_TYPE_YC8_EMB))
#define BD_QUAD_OUT_FMT (BD_VIN_FMT)
#define BD_VPU_IN_FMT  (BD_QUAD_OUT_FMT)
#define BD_PVITX_OUT_FMT ((BD_VIN_FMT) | (VID_STANDARD_PVI) | (PVITX_SRC_DU))
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////



#endif // __BOARD_CONFIG_H__

#include "type.h"
#include "error.h"

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


#include "pcvOffLineCalib.h"
#include "pcvSvmView.h"
#include "pcvTypeDef.h"
#include "api_calibration.h"
#include "app_calibration.h"
//#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dram_config.h"

#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

//for capture
extern _VPUConfig *gpVPUConfig;

//for debug about alloc & free
extern PP_U32 heap_alloc_ext_try_cnt;
extern PP_U32 heap_free_ext_try_cnt;
extern PP_U32 heap_alloc_pcv_try_cnt;
extern PP_U32 heap_free_pcv_try_cnt;

//information used inside SDK about offline calibration & view generation
extern PP_OFFCALIB_OUT_INFO_S Off_Calib_Out;
extern PP_VIEWGEN_PROCESS_INFO_S Viewgen_Process_Info;

//definition use reserved mem or not
#define USE_RESERVED_MEM 

//viewgen global varialbes about each camera
PCV_SVM_VIEW_CAMERA_PARAM Viewgen_tFrontCamParam;
PCV_SVM_VIEW_CAMERA_PARAM Viewgen_tLeftCamParam;
PCV_SVM_VIEW_CAMERA_PARAM Viewgen_tRearCamParam;
PCV_SVM_VIEW_CAMERA_PARAM Viewgen_tRightCamParam;

//viewgen surface
PCV_SVM_VIEW_SURFACE_2D_PARAM tSurface2dParam;
PCV_SVM_VIEW_SURFACE_3D_PARAM tSurface3dParam;

//viewgen blend
PCV_SVM_VIEW_BLEND_PARAM tBlendParam;

//ViewGrid
PCV_SVM_VIEW_VIEW_GRID_PARAM tViewGridParam;


//viewgen mdviewparam & camview param
PCV_SVM_VIEW_MD_VIEW_PARAM tMdViewParam;
PCV_SVM_VIEW_CAM_VIEW_PARAM tUndistortViewParam;
PCV_SVM_VIEW_CAM_VIEW_PARAM tCylindricalViewParam;

//viewgen lib version structure
PCV_SVM_VIEW_VER_INFO tVerInfo;

//viewgen buffer
extern PP_U8 *pFBLut, *pLRLut, *pProcBuf;

//viewgen global param
PP_S32 scanType, fieldType;
PP_S32 viewSampleMode;
PP_S32 blendSampleMode;

PP_S32 viewGridXSize,viewGridYSize;

//for tick
extern PP_U32 func_start_tick,func_end_tick;

//svm configuration binary format
PP_CNF_TOTAL_BIN_FORMAT_S Calib_Cnf_Bin;
PP_CNF_TOTAL_BIN_FORMAT_S Calib_Cnf_Bin_Sub;


extern PP_U8 gMulticore_Viewgen_Flag;

/*
    FreeRTOS V8.0.0 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * A sample implementation of pvMemMangCALIBMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */

extern _gstDramReserved gstDramReserved;

#define configADDR_MEMMANGCALIB                    ( (uint8_t *) (gstDramReserved.u32AddrCalibrationLib) )
#define configTOTAL_MEMMANGCALIB_SIZE				( ( size_t ) (CALIB_RESERVED_BUFFSIZE)) //1MByte DDR

/* Allocate the memory for the memMangCALIB. */
static uint8_t *pucMemMangCALIB = NULL;

/////////////////////////////////////////////////////////////////////////////////////    

/* Block sizes must not get too small. */
#define memMangCALIB_MINIMUM_BLOCK_SIZE	( ( size_t ) ( memMangCALIB_STRUCT_SIZE * 2 ) )

/* Assumes 8bit bytes! */
#define memMangCALIB_BITS_PER_BYTE		( ( size_t ) 8 )

/* A few bytes might be lost to byte aligning the mem start address. */
#define ADJUSTED_MEMMANGCALIB_SIZE	( configTOTAL_MEMMANGCALIB_SIZE - portBYTE_ALIGNMENT )

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct MEMMANGCALIB_BLOCK_LINK
{
	struct MEMMANGCALIB_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} MEMMANGCALIB_BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvMemMangCALIBInsertBlockIntoFreeList( MEMMANGCALIB_BlockLink_t *pxBlockToInsert );

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const uint16_t memMangCALIB_STRUCT_SIZE	= ( ( sizeof ( MEMMANGCALIB_BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~portBYTE_ALIGNMENT_MASK );

/* Ensure the pxMemMangCALIBEnd pointer will end up on the correct byte alignment. */
static const size_t xTotalMemMangCALIBSize = ( ( size_t ) ADJUSTED_MEMMANGCALIB_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
static MEMMANGCALIB_BlockLink_t xMemMangCALIBStart, *pxMemMangCALIBEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xMemMangCALIBFreeBytesRemaining = ( ( size_t ) ADJUSTED_MEMMANGCALIB_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );
static size_t xMemMangCALIBMinimumEverFreeBytesRemaining = ( ( size_t ) ADJUSTED_MEMMANGCALIB_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an MEMMANGCALIB_BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xMemMangCALIBBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/

void *pvMemMangCALIBMalloc( size_t xWantedSize )
{
MEMMANGCALIB_BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
void *pvReturn = NULL;

	vTaskSuspendAll();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( pxMemMangCALIBEnd == NULL )
		{
			prvMemMangCALIBInit();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the MEMMANGCALIB_BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & xMemMangCALIBBlockAllocatedBit ) == 0 )
		{
			/* The wanted size is increased so it can contain a MEMMANGCALIB_BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( xWantedSize > 0 )
			{
				xWantedSize += memMangCALIB_STRUCT_SIZE;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
				{
					/* Byte alignment required. */
					xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( ( xWantedSize > 0 ) && ( xWantedSize <= xMemMangCALIBFreeBytesRemaining ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &xMemMangCALIBStart;
				pxBlock = xMemMangCALIBStart.pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != pxMemMangCALIBEnd )
				{
					/* Return the memory space pointed to - jumping over the
					MEMMANGCALIB_BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + memMangCALIB_STRUCT_SIZE );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - xWantedSize ) > memMangCALIB_MINIMUM_BLOCK_SIZE )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
						pxBlock->xBlockSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						prvMemMangCALIBInsertBlockIntoFreeList( ( pxNewBlockLink ) );
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					xMemMangCALIBFreeBytesRemaining -= pxBlock->xBlockSize;

					if( xMemMangCALIBFreeBytesRemaining < xMemMangCALIBMinimumEverFreeBytesRemaining )
					{
						xMemMangCALIBMinimumEverFreeBytesRemaining = xMemMangCALIBFreeBytesRemaining;
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xBlockSize |= xMemMangCALIBBlockAllocatedBit;
					pxBlock->pxNextFreeBlock = NULL;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		traceMALLOC( pvReturn, xWantedSize );
	}
	xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	#endif

	return pvReturn;
}
/*-----------------------------------------------------------*/

void vMemMangCALIBFree( void *pv )
{
uint8_t *puc = ( uint8_t * ) pv;
MEMMANGCALIB_BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an MEMMANGCALIB_BlockLink_t structure immediately
		before it. */
		puc -= memMangCALIB_STRUCT_SIZE;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		/* Check the block is actually allocated. */
		configASSERT( ( pxLink->xBlockSize & xMemMangCALIBBlockAllocatedBit ) != 0 );
		configASSERT( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & xMemMangCALIBBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~xMemMangCALIBBlockAllocatedBit;

				vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					xMemMangCALIBFreeBytesRemaining += pxLink->xBlockSize;
					traceFREE( pv, pxLink->xBlockSize );
					prvMemMangCALIBInsertBlockIntoFreeList( ( ( MEMMANGCALIB_BlockLink_t * ) pxLink ) );
				}
				xTaskResumeAll();
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
}
/*-----------------------------------------------------------*/

size_t xMemMangCALIBGetFreeSize( void )
{
	return xMemMangCALIBFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xMemMangCALIBGetMinimumEverFreeSize( void )
{
	return xMemMangCALIBMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vMemMangCALIBInitialiseBlocks( void )
{
	/* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/
void prvMemMangCALIBInit_All( void )
{
	memset(&xMemMangCALIBStart,0,sizeof(MEMMANGCALIB_BlockLink_t));
	
	//memset(pxMemMangCALIBEnd,0,sizeof(MEMMANGCALIB_BlockLink_t));
	pxMemMangCALIBEnd = NULL;
	
	xMemMangCALIBFreeBytesRemaining = ( ( size_t ) ADJUSTED_MEMMANGCALIB_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );
	xMemMangCALIBMinimumEverFreeBytesRemaining = ( ( size_t ) ADJUSTED_MEMMANGCALIB_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );

	xMemMangCALIBBlockAllocatedBit = 0;

	prvMemMangCALIBInit();
}
void prvMemMangCALIBInit( void )
{
MEMMANGCALIB_BlockLink_t *pxFirstFreeBlock;
uint8_t *pucHeapEnd, *pucAlignedHeap;

    pucMemMangCALIB = configADDR_MEMMANGCALIB;

	/* Ensure the heap starts on a correctly aligned boundary. */
	pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE ) (pucMemMangCALIB + portBYTE_ALIGNMENT ) ) & ( ( portPOINTER_SIZE_TYPE ) ~portBYTE_ALIGNMENT_MASK ) );

	/* xMemMangCALIBStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	xMemMangCALIBStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	xMemMangCALIBStart.xBlockSize = ( size_t ) 0;

	/* pxMemMangCALIBEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	pucHeapEnd = pucAlignedHeap + xTotalMemMangCALIBSize;
	pucHeapEnd -= memMangCALIB_STRUCT_SIZE;
	pxMemMangCALIBEnd = ( void * ) pucHeapEnd;
	configASSERT( ( ( ( uint32_t ) pxMemMangCALIBEnd ) & ( ( uint32_t ) portBYTE_ALIGNMENT_MASK ) ) == 0UL );
	pxMemMangCALIBEnd->xBlockSize = 0;
	pxMemMangCALIBEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxMemMangCALIBEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = xTotalMemMangCALIBSize - memMangCALIB_STRUCT_SIZE;
	pxFirstFreeBlock->pxNextFreeBlock = pxMemMangCALIBEnd;

	/* The heap now contains pxMemMangCALIBEnd. */
	xMemMangCALIBFreeBytesRemaining -= memMangCALIB_STRUCT_SIZE;

	/* Work out the position of the top bit in a size_t variable. */
	xMemMangCALIBBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * memMangCALIB_BITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void prvMemMangCALIBInsertBlockIntoFreeList( MEMMANGCALIB_BlockLink_t *pxBlockToInsert )
{
MEMMANGCALIB_BlockLink_t *pxIterator;
uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &xMemMangCALIBStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != pxMemMangCALIBEnd )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxMemMangCALIBEnd;
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}
}






PP_VOID *pcvMalloc(PP_U32 IN size)
{

	
	#ifdef USE_RESERVED_MEM
	{
		heap_alloc_pcv_try_cnt++;
		PP_VOID *ptr = pvMemMangCALIBMalloc(size);	
		return ptr;	
	}
	#else
	{	
		heap_alloc_pcv_try_cnt++;
		PP_VOID *ptr = OSAL_malloc(size); 	
		return ptr;
	}
	#endif
}

PP_VOID pcvFree(PP_VOID IN *ptr)
{
	#ifdef USE_RESERVED_MEM
	{
		heap_free_pcv_try_cnt++;
		if (ptr != 0)
		vMemMangCALIBFree(ptr);
	}
	#else
	{
		heap_free_pcv_try_cnt++;
		if (ptr != 0)
		OSAL_free(ptr); 
	}
	#endif
}


PP_VOID *PPAPI_Lib_Ext_Malloc(PP_U32 IN size)
{
	#ifdef USE_RESERVED_MEM
	{				
		heap_alloc_ext_try_cnt++;
		PP_VOID *ptr = pvMemMangCALIBMalloc(size); 
		return ptr;		
	}
	#else
	{
		heap_alloc_ext_try_cnt++;
		PP_VOID *ptr = OSAL_malloc(size); 
		return ptr;
	}
	#endif //USE_RESERVED_MEM


}

PP_VOID PPAPI_Lib_Ext_Free(PP_VOID IN *ptr)
{	
	#ifdef USE_RESERVED_MEM
	{
		heap_free_ext_try_cnt++;
		if (ptr != 0)
			vMemMangCALIBFree(ptr);		
	}
	#else
	{
		heap_free_ext_try_cnt++;
		if (ptr != 0)
			OSAL_free(ptr);
	}
	#endif //USE_RESERVED_MEM
}





PP_VOID PPAPI_Lib_Debug_Memory(PP_VOID)
{
		#ifdef USE_RESERVED_MEM
			LOG_DEBUG("*	[Calib Rserved Memory Status(used/free/percent) : %lf MByte / %lf MByte / %.2f %% used]\n",(PP_D64)(CALIB_RESERVED_BUFFSIZE-xMemMangCALIBGetFreeSize())/1024/1024,(PP_D64)(xMemMangCALIBGetFreeSize())/1024/1024,(PP_F32)(((PP_D64)CALIB_RESERVED_BUFFSIZE-(PP_D64)xMemMangCALIBGetFreeSize())/(PP_D64)CALIB_RESERVED_BUFFSIZE)*100);
			LOG_DEBUG("*	[check Rserved Memory Status(ever used/ever percent) : %lf MByte / %.2f %% ever used]\n",(PP_D64)(CALIB_RESERVED_BUFFSIZE - xMemMangCALIBGetMinimumEverFreeSize())/1024/1024,(PP_F32)(((PP_D64)CALIB_RESERVED_BUFFSIZE-(PP_D64)xMemMangCALIBGetMinimumEverFreeSize())/(PP_D64)CALIB_RESERVED_BUFFSIZE)*100);
		#endif
			LOG_DEBUG("*	ext alloc/free count [%d/%d] \n",heap_alloc_ext_try_cnt,heap_free_ext_try_cnt);
			LOG_DEBUG("*	pcv alloc/free count [%d/%d] \n",heap_alloc_pcv_try_cnt,heap_free_pcv_try_cnt);
}


PP_RESULT_E PPAPI_Viewgen_Get_View_Blend_Param(PCV_SVM_VIEW_BLEND_PARAM IN *view_blend_param)
{
	if(view_blend_param==NULL)return(eERROR_FAILURE);
	
	
	view_blend_param->shadowAreaFront = Calib_Cnf_Bin.blend.shadowAreaFront;
	view_blend_param->shadowAreaLeft = Calib_Cnf_Bin.blend.shadowAreaLeft;
	view_blend_param->shadowAreaRear = Calib_Cnf_Bin.blend.shadowAreaRear;
	view_blend_param->shadowAreaRight = Calib_Cnf_Bin.blend.shadowAreaRight;
	view_blend_param->alpha0DivAngFront = Calib_Cnf_Bin.blend.alpha0DivAngFront_2d;
	view_blend_param->alpha0DivAngRear = Calib_Cnf_Bin.blend.alpha0DivAngRear_2d;
	view_blend_param->alpha0BlendAreaFront = Calib_Cnf_Bin.blend.alpha0BlendAreaFront_2d;
	view_blend_param->alpha0BlendAreaRear = Calib_Cnf_Bin.blend.alpha0BlendAreaRear_2d;
	view_blend_param->alpha1DivAngFront = Calib_Cnf_Bin.blend.alpha1DivAngFront_2d;
	view_blend_param->alpha1DivAngRear =  Calib_Cnf_Bin.blend.alpha1DivAngRear_2d;
	view_blend_param->alpha1BlendAreaFront = Calib_Cnf_Bin.blend.alpha1BlendAreaFront_2d;
	view_blend_param->alpha1BlendAreaRear = Calib_Cnf_Bin.blend.alpha1BlendAreaRear_2d;
	return eSUCCESS;

}

PP_RESULT_E PPAPI_Viewgen_Get_View_Sur_2D_Param(PCV_SVM_VIEW_SURFACE_2D_PARAM IN *view_sur_2d_param)
{
	if(view_sur_2d_param==NULL)return(eERROR_FAILURE);

	
	view_sur_2d_param->wx = Calib_Cnf_Bin.Surface_2d.wx;
	view_sur_2d_param->wy = Calib_Cnf_Bin.Surface_2d.wy;
	view_sur_2d_param->center.x = Calib_Cnf_Bin.Surface_2d.centerx;
	view_sur_2d_param->center.y = Calib_Cnf_Bin.Surface_2d.centery;
	return eSUCCESS;

}



PP_RESULT_E PPAPI_Viewgen_Get_View_Sur_3D_Param(PCV_SVM_VIEW_SURFACE_3D_PARAM IN *view_sur_3d_param)
{
	if(view_sur_3d_param==NULL)return(eERROR_FAILURE);
	
	view_sur_3d_param->wx = Calib_Cnf_Bin.Surface_3d.wx;
	view_sur_3d_param->wy = Calib_Cnf_Bin.Surface_3d.wy;
	view_sur_3d_param->wz = Calib_Cnf_Bin.Surface_3d.wz;
	view_sur_3d_param->rx = Calib_Cnf_Bin.Surface_3d.rx;
	view_sur_3d_param->ry = Calib_Cnf_Bin.Surface_3d.ry;
	view_sur_3d_param->rz = Calib_Cnf_Bin.Surface_3d.rz;
	view_sur_3d_param->zOffset = Calib_Cnf_Bin.Surface_3d.zOffset;
	view_sur_3d_param->center.x = Calib_Cnf_Bin.Surface_3d.cx;
	view_sur_3d_param->center.y = Calib_Cnf_Bin.Surface_3d.cy;

	return eSUCCESS;

}
PP_RESULT_E PPAPI_Offcalib_Get_Cam_Intrinsic_Param(PCV_OFF_LINE_CALIB_CAMERA_PARAM IN * imp,PP_U8 IN camera_ch)
{
	if(imp==NULL)return(eERROR_FAILURE);
	if(camera_ch>3)return(eERROR_FAILURE);

	switch(camera_ch)
	{
		case 0://front
			{
				/* Initialize Parameter */
				imp->width = Calib_Cnf_Bin.frontCamera.width;
				imp->height = Calib_Cnf_Bin.frontCamera.height;
				imp->optCenterX = Calib_Cnf_Bin.frontCamera.cx;
				imp->optCenterY = Calib_Cnf_Bin.frontCamera.cy;
				imp->focalLenX = Calib_Cnf_Bin.frontCamera.fx;
				imp->focalLenY = Calib_Cnf_Bin.frontCamera.fy;
				imp->hFlip = 0;
				imp->distortTableSize = Calib_Cnf_Bin.frontCamera.distortTableSize;
				imp->pDistortTable = Calib_Cnf_Bin.frontCamera.distortTable;
			}
			break;
		case 1://left
			{
				/* Initialize Parameter */
				imp->width = Calib_Cnf_Bin.leftCamera.width;
				imp->height = Calib_Cnf_Bin.leftCamera.height;
				imp->optCenterX = Calib_Cnf_Bin.leftCamera.cx;
				imp->optCenterY = Calib_Cnf_Bin.leftCamera.cy;
				imp->focalLenX = Calib_Cnf_Bin.leftCamera.fx;
				imp->focalLenY = Calib_Cnf_Bin.leftCamera.fy;
				imp->hFlip = 0;
				imp->distortTableSize = Calib_Cnf_Bin.leftCamera.distortTableSize;
				imp->pDistortTable = Calib_Cnf_Bin.leftCamera.distortTable;
			}		
			break;
		
		case 2: //right
			{				
				/* Initialize Parameter */
				imp->width = Calib_Cnf_Bin.rightCamera.width;
				imp->height = Calib_Cnf_Bin.rightCamera.height;
				imp->optCenterX = Calib_Cnf_Bin.rightCamera.cx;
				imp->optCenterY = Calib_Cnf_Bin.rightCamera.cy;
				imp->focalLenX = Calib_Cnf_Bin.rightCamera.fx;
				imp->focalLenY = Calib_Cnf_Bin.rightCamera.fy;
				imp->hFlip = 0;
				imp->distortTableSize = Calib_Cnf_Bin.rightCamera.distortTableSize;
				imp->pDistortTable = Calib_Cnf_Bin.rightCamera.distortTable;
			}
			break;
		case 3://back
			{
				
				/* Initialize Parameter */
				imp->width = Calib_Cnf_Bin.rearCamera.width;
				imp->height = Calib_Cnf_Bin.rearCamera.height;
				imp->optCenterX = Calib_Cnf_Bin.rearCamera.cx;
				imp->optCenterY = Calib_Cnf_Bin.rearCamera.cy;
				imp->focalLenX = Calib_Cnf_Bin.rearCamera.fx;
				imp->focalLenY = Calib_Cnf_Bin.rearCamera.fy;
				imp->hFlip = 0;
				imp->distortTableSize = Calib_Cnf_Bin.rearCamera.distortTableSize;
				imp->pDistortTable = Calib_Cnf_Bin.rearCamera.distortTable;

			}			
			break;
		

	}
	return eSUCCESS;

	
}
PP_VOID PPAPI_Offcalib_Debug_Print_FeaturePoint(PCV_OFF_LINE_CALIB_PATTERN_PARAM IN * pat_param)
{
	PP_U32 i=0;
	for (i = 0; i < pat_param->featureCount; i++)
	{
		LOG_DEBUG("featureWPoints[%d] : x = %f, y = %f\n", i, pat_param->featureWpt[3 * i], pat_param->featureWpt[3 * i + 1]);
	}
	for (i = 0; i < pat_param->featureCount; i++)
	{
		LOG_DEBUG("featureIPoints[%d] : x = %f, y = %f\n", i, pat_param->featureIpt[2 * i], pat_param->featureIpt[2 * i + 1]);
	}
	LOG_DEBUG("********************\n");

}
PP_VOID PPAPI_Lib_GetCameraProjectPoint(PP_VOID)
{
	PcvPoint3d32f wp[6];
	PcvPoint32f ip[6];
	int pValid[6];
	int wp_count=6;
	
	wp_count = Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureCount;
	
	wp[0].x=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[0];
	wp[0].y=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[1];
	wp[0].z=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[2];
	
	wp[1].x=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[3];
	wp[1].y=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[4];
	wp[1].z=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[5];
	
	wp[2].x=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[6];
	wp[2].y=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[7];
	wp[2].z=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[8];

	wp[3].x=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[9];
	wp[3].y=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[10];
	wp[3].z=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[11];

	wp[4].x=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[12];
	wp[4].y=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[13];
	wp[4].z=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[14];

	wp[5].x=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[15];
	wp[5].y=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[16];
	wp[5].z=Off_Calib_Out.stPcv_Param.stPatternParam[eCALIB_CAMERA_FRONT].featureWpt[17];


	pcvSvmView_GetCameraProjectPoint(wp, ip, pValid, wp_count, &Viewgen_tFrontCamParam);
	pcvSvmView_GetMdViewProjectPoint(wp, ip,pValid, wp_count,&tMdViewParam);
	
}


PP_VOID PPAPI_Offcalib_Debug_Print_Position(PCV_OFF_LINE_CALIB_CAMERA_PARAM IN * imp)
{
		
		LOG_DEBUG("*	Calibration Result\n");
		LOG_DEBUG("*	[Camera Extrinsics]\n");
		LOG_DEBUG("* r1 = %.5f\n", imp->R[0]);
		LOG_DEBUG("* r2 = %.5f\n", imp->R[1]);
		LOG_DEBUG("* r3 = %.5f\n", imp->R[2]);
		LOG_DEBUG("* r4 = %.5f\n", imp->R[3]);
		LOG_DEBUG("* r5 = %.5f\n", imp->R[4]);
		LOG_DEBUG("* r6 = %.5f\n", imp->R[5]);
		LOG_DEBUG("* r7 = %.5f\n", imp->R[6]);
		LOG_DEBUG("* r8 = %.5f\n", imp->R[7]);
		LOG_DEBUG("* r9 = %.5f\n", imp->R[8]);
		LOG_DEBUG("* t1 = %.5f\n", imp->T[0]);
		LOG_DEBUG("* t2 = %.5f\n", imp->T[1]);
		LOG_DEBUG("* t3 = %.5f\n", imp->T[2]);
		LOG_DEBUG("*	[Camera Position]\n");
		LOG_DEBUG("* x = %.5f (mm)\n", imp->posX);
		LOG_DEBUG("* y = %.5f (mm)\n", imp->posY);
		LOG_DEBUG("* z = %.5f (mm)\n", imp->posZ);
		LOG_DEBUG("* tilt = %.5f (deg)\n", imp->angX);
		LOG_DEBUG("* roll = %.5f (deg)\n", imp->angY);
		LOG_DEBUG("* pan = %.5f (deg)\n", imp->angZ);
		LOG_DEBUG("****************************************\n");
}
PP_OFFCALIB_CAMERA_PATTERN_TYPE_SEL_E PPAPI_Offcalib_Get_Pattern_Select(PP_VOID)
{
	PP_OFFCALIB_CAMERA_PATTERN_TYPE_SEL_E pattern;

	pattern = eCALIB_PATTERN_MAIN;
	//smoh
	//main or sub?
	//main ==0
	//sub == 1
	return pattern;
}

PP_RESULT_E PPAPI_Offcalib_Get_Cnf(PCV_OFF_LINE_CALIB_PATTERN_PARAM  IN * pat_param,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN camera_ch)
{
	if(pat_param==NULL)return(eERROR_FAILURE);
	if(camera_ch>3)return(eERROR_FAILURE);

	PP_CNF_TAG_CAMERA_PARAM_S *stPattern;


	if(PPAPI_Offcalib_Get_Pattern_Select()==eCALIB_PATTERN_MAIN)
	{
		switch(camera_ch)
		{
			case eCALIB_CAMERA_FRONT:
				{
					stPattern = &Calib_Cnf_Bin.frontCamera;					
				}
				break;
			case eCALIB_CAMERA_LEFT:
				{
					stPattern = &Calib_Cnf_Bin.leftCamera;
				}		
				break;
			
			case eCALIB_CAMERA_RIGHT:
				{
					stPattern = &Calib_Cnf_Bin.rightCamera;		
				}
				break;			
			case eCALIB_CAMERA_BACK: 
				{				
					stPattern = &Calib_Cnf_Bin.rearCamera;
				}			
				break;	
			default:
				break;
		}
	}
	else //eCALIB_PATTERN_SUB
	{
		switch(camera_ch)
		{
			case eCALIB_CAMERA_FRONT:
				{
					stPattern = &Calib_Cnf_Bin_Sub.frontCamera;					
				}
				break;
			case eCALIB_CAMERA_LEFT:
				{
					stPattern = &Calib_Cnf_Bin_Sub.leftCamera;				
				}		
				break;			
			case eCALIB_CAMERA_RIGHT:
				{
					stPattern = &Calib_Cnf_Bin_Sub.rightCamera;		
				}
				break;			
			case eCALIB_CAMERA_BACK: 
				{				
					stPattern = &Calib_Cnf_Bin_Sub.rearCamera;
				}			
				break;	
			default:
				break;
		}

	}
	
	pat_param->type = stPattern->patternType;
	pat_param->fW = stPattern->patternW;
	pat_param->fL = stPattern->patternL;
	pat_param->fSW = stPattern->patternSW;
	pat_param->fSL = stPattern->patternSL;
	pat_param->squareSize = stPattern->patternSquareSize;
	
	pat_param->roi[0].left = stPattern->patternRoi0Left;
	pat_param->roi[0].right = stPattern->patternRoi0Right;
	pat_param->roi[0].top = stPattern->patternRoi0Top;
	pat_param->roi[0].bottom = stPattern->patternRoi0Bottom;
	pat_param->roi[1].left = stPattern->patternRoi1Left;
	pat_param->roi[1].right = stPattern->patternRoi1Right;
	pat_param->roi[1].top = stPattern->patternRoi1Top;
	pat_param->roi[1].bottom = stPattern->patternRoi1Bottom;

	
	return eSUCCESS;

	
}


PP_VOID PPAPI_Lib_Calib_Initialize(PP_VOID)
{
		// Load Calib Cnf  Header
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		PPAPI_FLASH_Read(&Calib_Cnf_Bin, gstFlashHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32FlashAddr, gstFlashHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32Size);
#else

		PPAPI_FTL_Read(&Calib_Cnf_Bin, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32Size);
		PPAPI_FTL_Read(&Calib_Cnf_Bin_Sub, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_SUB_TYPE].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_SUB_TYPE].u32Size);

#endif

}

PP_VOID PPAPI_Offcalib_Save_Cam_Ch(PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch)
{
	Off_Calib_Out.eCh_sel = ch;
}
PP_OFFCALIB_CAMERA_CAPTURE_CH_E PPAPI_Offcalib_Get_Cam_Ch(PP_VOID)
{
	return Off_Calib_Out.eCh_sel;
}

PP_VOID PPAPI_Offcalib_Set_Step_Send_Cmd(PP_OFFCALIB_PROCESS_STEP_E  IN step)
{
	Off_Calib_Out.eOffcalib_Progress_Step = step;
}
PP_OFFCALIB_PROCESS_STEP_E PPAPI_Offcalib_Get_Step_Send_Cmd(PP_VOID)
{
	return Off_Calib_Out.eOffcalib_Progress_Step;
}

PP_VOID PPAPI_Section_Viewgen_Set_Step_Send_Cmd(PP_SECTION_VIEWGEN_PROCESS_STEP_E  IN step)
{
	Viewgen_Process_Info.eSViewgen_Progress_Step = step;
}
PP_SECTION_VIEWGEN_PROCESS_STEP_E PPAPI_Section_Viewgen_Get_Step_Send_Cmd(PP_VOID)
{
	return Viewgen_Process_Info.eSViewgen_Progress_Step;
}



PP_VOID PPAPI_Offcalib_BMP_Image_Save_SD(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch)
{
	PP_S32 status=0;
	
	PP_CHAR file_name_bmp[50]={0,};
	STATIC PP_U32 num=0;
	PP_CHAR buffer[50]={0,};
	
    int inWidth = 0, inHeight = 0;
	_VID_RESOL eVidResol = 0;

    if( PPAPI_VIN_GetResol(BD_VIN_FMT, &inWidth, &inHeight, &eVidResol) != eSUCCESS )
    {
        LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return;
    }
		
	status=PPAPI_FATFS_DelVolume();
	LOG_DEBUG("PPAPI_FATFS_DelVolume status = 0x%x\n",status);
	if(PPAPI_FATFS_InitVolume()){
	
		LOG_DEBUG("error %d\n",PPAPI_FATFS_GetLastError());
	}
	sprintf(buffer,"%d",num);

	switch(ch)
	{
		case eCALIB_CAMERA_FRONT:
			{
				sprintf(file_name_bmp,"%s","calib_front_");
				
			}
			break;
		case eCALIB_CAMERA_LEFT:
			{
				sprintf(file_name_bmp,"%s","calib_left_");
			}
			break;
		case eCALIB_CAMERA_RIGHT:
			{
				sprintf(file_name_bmp,"%s","calib_right_");
			}
			break;

		case eCALIB_CAMERA_BACK:
			{
				sprintf(file_name_bmp,"%s","calib_back_");
			}
			break;
		default:
			{
				LOG_DEBUG("channel error!\n");
			}	
			break;
	}
	strcat(file_name_bmp,buffer);
	num++;
	strcat(file_name_bmp,".bmp");

	
	LOG_DEBUG("!!!file_name = %s\n",file_name_bmp);	
	
	if(YuvBufInfo->u8Yonly==0)
	{
		if(SaveBMP24FromYUV(file_name_bmp,(PP_U8 *)YuvBufInfo->u32YuvBufAddr,inWidth,inHeight)==eSUCCESS)
		
		{
			LOG_DEBUG("bmp file save success\n");	
		}
		else
		{
			LOG_DEBUG("bmp file save failed!\n");	
		}
	}
	else
	{

		if(SaveBMP24FromY(file_name_bmp,(PP_U8 *)YuvBufInfo->u32YuvBufAddr,inWidth,inHeight)==eSUCCESS)
		
		{
			LOG_DEBUG("bmp file save success\n");	
		}
		else
		{
			LOG_DEBUG("bmp file save failed!\n");	
		}

	}
	
	if(PPAPI_FATFS_DelVolume()){
		LOG_DEBUG("unmount fail(%d)\n", PPAPI_FATFS_GetLastError());
	}else{
		LOG_DEBUG("unmounted\n");
	}
	
	
}

PP_VOID PPAPI_Offcalib_Capture_YUV_Image_Save_SD(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch)
{
	PP_S32 status=0;
	
	PP_VOID *pfHandle_YUVimage = NULL;

	PP_CHAR file_name[50]={0,};
	STATIC PP_U32 num=0;
	PP_CHAR buffer[50]={0,};
	
		
	status=PPAPI_FATFS_DelVolume();
	LOG_DEBUG("PPAPI_FATFS_DelVolume status = 0x%x\n",status);
	if(PPAPI_FATFS_InitVolume()){
	
		LOG_DEBUG("error %d\n",PPAPI_FATFS_GetLastError());
	}
	sprintf(buffer,"%d",num);

	switch(ch)
	{
		case eCALIB_CAMERA_FRONT:
			{
				sprintf(file_name,"%s","calib_front_");
				//file_name = "calib_front.yuv";
				
			}
			break;
		case eCALIB_CAMERA_LEFT:
			{
				sprintf(file_name,"%s","calib_left_");
				//file_name = "calib_left.yuv";
			}
			break;
		case eCALIB_CAMERA_RIGHT:
			{
				sprintf(file_name,"%s","calib_right_");
				//file_name = "calib_right.yuv";
			}
			break;

		case eCALIB_CAMERA_BACK:
			{
				sprintf(file_name,"%s","calib_back_");
				//file_name = "calib_back.yuv";
			}
			break;
		default:
			{
				LOG_DEBUG("channel error!\n");
			}	
			break;
	}
	strcat(file_name,buffer);
	num++;
	strcat(file_name,".yuv");

	
	pfHandle_YUVimage = PPAPI_FATFS_Open(file_name, "w");
	
	LOG_DEBUG("!!!file_name = %s\n",file_name);
	LOG_DEBUG("pfHandle_YUVimage = 0x%x\n",pfHandle_YUVimage);
	
	status = PPAPI_FATFS_Write((const PP_VOID*)YuvBufInfo->u32YuvBufAddr, 1,YuvBufInfo->u32YuvBufSize,pfHandle_YUVimage);
	LOG_DEBUG("PPAPI_FATFS_Write status = 0x%x\n",status);
	
	status=PPAPI_FATFS_Close(pfHandle_YUVimage);
	LOG_DEBUG("PPAPI_FATFS_Close status = 0x%x\n",status);
	
	if(PPAPI_FATFS_DelVolume()){
		LOG_DEBUG("unmount fail(%d)\n", PPAPI_FATFS_GetLastError());
	}else{
		LOG_DEBUG("unmounted\n");
	}
	
}




PP_VOID PPAPI_Offcalib_Capture_YUV_Image(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch, PP_U8 bYonly)
{

	/* option */
	// [0] - yuv422 
	// [1] - y only
	
	
	PP_U32 chSel,pathSel;
	PP_U32 u32BufPAddr;
    PP_U32 u32BufSize;
    //PP_S32 bYonly = 1; //0:YUV, 1:Y only

 	
	u32BufPAddr = (PP_U32)gpVPUConfig->pBufCapture[0];

	chSel = ch; //camera ch
	pathSel = 0; //0:input, 1:TP
	
	if( PPAPI_VIN_SetCaptureMode(BD_VIN_FMT, chSel, pathSel) != eSUCCESS )
    {
        LOG_WARNING("[(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return;
    }
	if( PPAPI_VIN_GetCaptureImage(BD_VIN_FMT, bYonly, u32BufPAddr, &u32BufSize, QUAD_0CH, PP_TRUE) != eSUCCESS )
    {
        LOG_WARNING("[(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return;
    }

	YuvBufInfo->u32YuvBufAddr = u32BufPAddr;	
	YuvBufInfo->u32YuvBufSize = u32BufSize;
	YuvBufInfo->u8Yonly = bYonly;	


}


PP_S32 PPAPI_Viewgen_Get_Cam_Param(PCV_SVM_VIEW_CAMERA_PARAM  IN * camera_param,PP_U8  IN camera_ch)
{
	if(camera_param==NULL)return(-1);
	if(camera_ch>3)return(-1);

	switch(camera_ch)
	{
		case 0://front
			{
				camera_param->width = Calib_Cnf_Bin.frontCamera.width;
				camera_param->height = Calib_Cnf_Bin.frontCamera.height;

				// if not operation offline camera calibration 
				if(Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posX==0 && Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posY==0)
				{				
					camera_param->posX = Calib_Cnf_Bin.frontCamera.posx;
					camera_param->posY = Calib_Cnf_Bin.frontCamera.posy;
					camera_param->posZ = Calib_Cnf_Bin.frontCamera.posz;
					camera_param->angX = Calib_Cnf_Bin.frontCamera.angx;
					camera_param->angY = Calib_Cnf_Bin.frontCamera.angy;
					camera_param->angZ = Calib_Cnf_Bin.frontCamera.angz;					
					camera_param->hFlip = 0;					
					
					camera_param->focalLenX = Calib_Cnf_Bin.frontCamera.fx;
					camera_param->focalLenY = Calib_Cnf_Bin.frontCamera.fy;
					camera_param->optCenterX = Calib_Cnf_Bin.frontCamera.cx;
					camera_param->optCenterY = Calib_Cnf_Bin.frontCamera.cy;
					camera_param->distortTableSize = Calib_Cnf_Bin.frontCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.frontCamera.distortTable;	
				}
				else 
				{
					camera_param->posX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].posX;
					camera_param->posY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].posY;
					camera_param->posZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].posZ;
					camera_param->angX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].angX;
					camera_param->angY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].angY;
					camera_param->angZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].angZ;
					camera_param->hFlip  = 0;
					
					camera_param->focalLenX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].focalLenX;
					camera_param->focalLenY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].focalLenY;
					camera_param->optCenterX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].optCenterX;
					camera_param->optCenterY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_FRONT].optCenterY;
					camera_param->distortTableSize = Calib_Cnf_Bin.frontCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.frontCamera.distortTable;	
				}						
			}
			break;
		case 1://left
			{
				camera_param->width = Calib_Cnf_Bin.leftCamera.width;
				camera_param->height = Calib_Cnf_Bin.leftCamera.height;
				
				if(Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posX==0 && Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posY==0)
				{				
					camera_param->posX = Calib_Cnf_Bin.leftCamera.posx;
					camera_param->posY = Calib_Cnf_Bin.leftCamera.posy;
					camera_param->posZ = Calib_Cnf_Bin.leftCamera.posz;
					camera_param->angX = Calib_Cnf_Bin.leftCamera.angx;
					camera_param->angY = Calib_Cnf_Bin.leftCamera.angy;
					camera_param->angZ = Calib_Cnf_Bin.leftCamera.angz;					
					camera_param->hFlip = 0;					
					
					camera_param->focalLenX = Calib_Cnf_Bin.leftCamera.fx;
					camera_param->focalLenY = Calib_Cnf_Bin.leftCamera.fy;
					camera_param->optCenterX = Calib_Cnf_Bin.leftCamera.cx;
					camera_param->optCenterY = Calib_Cnf_Bin.leftCamera.cy;
					camera_param->distortTableSize = Calib_Cnf_Bin.leftCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.leftCamera.distortTable;	
				}
				else
				{
					camera_param->posX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].posX;
					camera_param->posY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].posY;
					camera_param->posZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].posZ;
					camera_param->angX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].angX;
					camera_param->angY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].angY;
					camera_param->angZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].angZ;
					camera_param->hFlip  = 0;

					
					camera_param->focalLenX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].focalLenX;
					camera_param->focalLenY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].focalLenY;
					camera_param->optCenterX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].optCenterX;
					camera_param->optCenterY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_LEFT].optCenterY;
					camera_param->distortTableSize = Calib_Cnf_Bin.leftCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.leftCamera.distortTable;	
				}						
			}		
			break;
		
		case 2://right
			{
				
				camera_param->width = Calib_Cnf_Bin.rightCamera.width;
				camera_param->height = Calib_Cnf_Bin.rightCamera.height;
				if(Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posX==0 && Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posY==0)
				{				
					camera_param->posX = Calib_Cnf_Bin.rightCamera.posx;
					camera_param->posY = Calib_Cnf_Bin.rightCamera.posy;
					camera_param->posZ = Calib_Cnf_Bin.rightCamera.posz;
					camera_param->angX = Calib_Cnf_Bin.rightCamera.angx;
					camera_param->angY = Calib_Cnf_Bin.rightCamera.angy;
					camera_param->angZ = Calib_Cnf_Bin.rightCamera.angz;
					camera_param->hFlip = 0;					

					
					camera_param->focalLenX = Calib_Cnf_Bin.rightCamera.fx;
					camera_param->focalLenY = Calib_Cnf_Bin.rightCamera.fy;
					camera_param->optCenterX = Calib_Cnf_Bin.rightCamera.cx;
					camera_param->optCenterY = Calib_Cnf_Bin.rightCamera.cy;
					camera_param->distortTableSize = Calib_Cnf_Bin.rightCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.rightCamera.distortTable;	
				}
				else
				{
					camera_param->posX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].posX;
					camera_param->posY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].posY;
					camera_param->posZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].posZ;
					camera_param->angX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].angX;
					camera_param->angY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].angY;
					camera_param->angZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].angZ;
					camera_param->hFlip  = 0;

					
					camera_param->focalLenX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].focalLenX;
					camera_param->focalLenY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].focalLenY;
					camera_param->optCenterX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].optCenterX;
					camera_param->optCenterY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_RIGHT].optCenterY;
					camera_param->distortTableSize = Calib_Cnf_Bin.rightCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.rightCamera.distortTable;	
				}
			}
			break;
		
		case 3: //back
			{				
				camera_param->width = Calib_Cnf_Bin.rearCamera.width;
				camera_param->height = Calib_Cnf_Bin.rearCamera.height;
				if(Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posX==0 && Off_Calib_Out.stPcv_Param.stCamParam[camera_ch].posY==0)
				{				
					camera_param->posX = Calib_Cnf_Bin.rearCamera.posx;
					camera_param->posY = Calib_Cnf_Bin.rearCamera.posy;
					camera_param->posZ = Calib_Cnf_Bin.rearCamera.posz;
					camera_param->angX = Calib_Cnf_Bin.rearCamera.angx;
					camera_param->angY = Calib_Cnf_Bin.rearCamera.angy;
					camera_param->angZ = Calib_Cnf_Bin.rearCamera.angz;
					camera_param->hFlip = 0;				
					
					camera_param->focalLenX = Calib_Cnf_Bin.rearCamera.fx;
					camera_param->focalLenY = Calib_Cnf_Bin.rearCamera.fy;
					camera_param->optCenterX = Calib_Cnf_Bin.rearCamera.cx;
					camera_param->optCenterY = Calib_Cnf_Bin.rearCamera.cy;
					camera_param->distortTableSize = Calib_Cnf_Bin.rearCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.rearCamera.distortTable;
				}
				else
				{
					camera_param->posX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].posX;
					camera_param->posY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].posY;
					camera_param->posZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].posZ;
					camera_param->angX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].angX;
					camera_param->angY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].angY;
					camera_param->angZ = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].angZ;
					camera_param->hFlip  = 0;
					
					camera_param->focalLenX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].focalLenX;
					camera_param->focalLenY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].focalLenY;
					camera_param->optCenterX = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].optCenterX;
					camera_param->optCenterY = Off_Calib_Out.stPcv_Param.stCamParam[eCALIB_CAMERA_BACK].optCenterY;
					camera_param->distortTableSize = Calib_Cnf_Bin.rearCamera.distortTableSize;
					camera_param->pDistortTable = Calib_Cnf_Bin.rearCamera.distortTable;	
				}			
			}			
			break;
	}
	return 0;
}






PP_VOID PPAPI_Offcalib_Capture_YUV_Imgae_Save_Buf(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch)
{
	Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[ch]=(PP_U32)(PP_U8 *)PPAPI_Lib_Ext_Malloc(YuvBufInfo->u32YuvBufSize);
	memcpy((PP_U8 *)Off_Calib_Out.stOffcalib_Capture_Buf.u32Capture_Buf_Addr[ch],(PP_U8 *)YuvBufInfo->u32YuvBufAddr,YuvBufInfo->u32YuvBufSize);
}

STATIC PP_BOOL PPAPI_Viewgen_SectionLUT_Update(PP_U32 IN u32SectionIndex, PP_SVMMEM_SECTION_DATA_E IN enSectionType, PP_U32* IN pu32SectionData, PP_U32 u32SectionDataSize)
{
	PP_U32 *pu32DstAddr;
	
	if ( !pu32SectionData || !u32SectionDataSize ) return PP_FALSE;
	if ( u32SectionIndex >= PPAPI_SVMMEM_GetSectionCount() ) return PP_FALSE;
	if ( u32SectionDataSize != PPAPI_SVMMEM_GetSectionDataSize(u32SectionIndex, enSectionType) ) return PP_FALSE;

	pu32DstAddr = PPAPI_SVMMEM_GetSectionAddress(u32SectionIndex, enSectionType);

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	pu32DstAddr = (PP_U32 *)((PP_U32)pu32DstAddr + gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr);
	PPAPI_FLASH_Write((PP_U8*)pu32SectionData, (PP_U32)pu32DstAddr, u32SectionDataSize);
#else
	pu32DstAddr = (PP_U32 *)((PP_U32)pu32DstAddr + gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr);
	PPAPI_FTL_Write((PP_U8*)pu32SectionData, (PP_U32)pu32DstAddr, u32SectionDataSize);
#endif

	return PP_TRUE;
}

STATIC PP_U32 PPAPI_Viewgen_SectionLUT_GetSamplingDataSize(PP_U16 IN u16SectionWidth, PP_U16 IN u16SectionHeight)
{
	PP_U32 u32Size = 0;
	PP_U8 u8SamplingWidth, u8SamplingHeight;
	

	if ( !u16SectionWidth || u16SectionWidth % 8 != 0 ||
		 !u16SectionHeight || u16SectionHeight % 8 != 0 )
	{
		return 0;
	}

	u8SamplingWidth = (PP_U8)(u16SectionWidth / 8) + 1;
	u8SamplingHeight = (PP_U8)(u16SectionHeight / 8) + 1;

	u32Size = u8SamplingWidth * u8SamplingHeight * 4;
	if ( u32Size % 16 )
		u32Size += (16 - (u32Size % 16));
	
	return u32Size;
}
PP_VOID PPAPI_Section_Viewgen_Update(PP_U32 IN u32Section_num, PP_U32  IN FB_ADDR,PP_U32  IN LR_ADDR)
{
	PP_U32 u32SectionSize = 0;

	PP_U32 width;
	PP_U32 height;

	if(u32Section_num<=Calib_Cnf_Bin.mdViewHeader.viewCount-1)
	{
		width=Calib_Cnf_Bin.mdViewHeader.subView[u32Section_num].width;
		height=Calib_Cnf_Bin.mdViewHeader.subView[u32Section_num].height;
	}
	else
	{
		width=Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].width;
		height=Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].height;
	}

	u32SectionSize = PPAPI_Viewgen_SectionLUT_GetSamplingDataSize(width, height);

	if ( PPAPI_SVMMEM_GetSectionDataSize(u32Section_num, eSVMMEM_SECTION_DATA_FB_ODD) )
	{
		PPAPI_Viewgen_SectionLUT_Update(u32Section_num, eSVMMEM_SECTION_DATA_FB_ODD, (PP_U32*)FB_ADDR, u32SectionSize);

	}
	if ( PPAPI_SVMMEM_GetSectionDataSize(u32Section_num, eSVMMEM_SECTION_DATA_LR_ODD) )
	{
		PPAPI_Viewgen_SectionLUT_Update(u32Section_num, eSVMMEM_SECTION_DATA_LR_ODD, (PP_U32*)LR_ADDR, u32SectionSize);

	}	
	
	

	/**************************************************************************
	* Common section needs to be reload after updating for preview.
	**************************************************************************/
	if ( PPAPI_SVMMEM_IsCommonSection(u32Section_num) )
		PPAPI_SVMMEM_LoadCommonSectionData(u32Section_num, PP_TRUE);
	
}




PP_RESULT_E PPAPI_Viewgen_Get_External_Value(PP_VOID)
{
	PP_S32	gridSize;




	LOG_DEBUG("**********************************\n");

	pcvSvmView_GetVersion(&tVerInfo);

	LOG_DEBUG("* SvmViewLib Version = %d.%d.%d\n", tVerInfo.major, tVerInfo.middle, tVerInfo.minor);
	
	if (Calib_Cnf_Bin.frontCamera.width!= Calib_Cnf_Bin.leftCamera.width || Calib_Cnf_Bin.frontCamera.width != Calib_Cnf_Bin.rightCamera.width || Calib_Cnf_Bin.frontCamera.width != Calib_Cnf_Bin.rearCamera.width
		|| Calib_Cnf_Bin.frontCamera.height != Calib_Cnf_Bin.leftCamera.height || Calib_Cnf_Bin.frontCamera.height != Calib_Cnf_Bin.rightCamera.height || Calib_Cnf_Bin.frontCamera.height != Calib_Cnf_Bin.rearCamera.height)
	{
		return eERROR_FAILURE;
	}

	PPAPI_Viewgen_Get_Cam_Param(&Viewgen_tFrontCamParam,0);
	PPAPI_Viewgen_Get_Cam_Param(&Viewgen_tLeftCamParam,1);
	PPAPI_Viewgen_Get_Cam_Param(&Viewgen_tRightCamParam,2);
	PPAPI_Viewgen_Get_Cam_Param(&Viewgen_tRearCamParam,3);


	PPAPI_Viewgen_Get_View_Blend_Param(&tBlendParam);


	PPAPI_Viewgen_Get_View_Sur_2D_Param(&tSurface2dParam);
	
	gridSize = pcvSvmView_Get2dSurfaceGridBufSize();
	
	tSurface2dParam.ptSurfaceGrid = (PCV_SVM_VIEW_SURFACE_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_SURFACE_GRID_PARAM));

	tSurface2dParam.ptFrontCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));

	tSurface2dParam.ptLeftCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));

	tSurface2dParam.ptRearCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));

	tSurface2dParam.ptRightCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));

	PPAPI_Viewgen_Get_View_Sur_3D_Param(&tSurface3dParam);

	gridSize = pcvSvmView_Get3dSurfaceGridBufSize();
	tSurface3dParam.ptSurfaceGrid = (PCV_SVM_VIEW_SURFACE_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_SURFACE_GRID_PARAM));
	tSurface3dParam.ptFrontCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));
	tSurface3dParam.ptLeftCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));
	tSurface3dParam.ptRearCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));
	tSurface3dParam.ptRightCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));

	scanType = Calib_Cnf_Bin.mdViewHeader.subView[0].scanType;//PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE;

	fieldType = PCV_SVM_VIEW_FIELD_TYPE_FRAME; //?? how to get from binary?
	
	viewSampleMode = PCV_SVM_VIEW_SAMPLE_MODE_8; //?? how to get from binary?
	
	pcvSvmView_Set2dSurface(&tSurface2dParam);
	pcvSvmView_Set3dSurface(&tSurface3dParam);


	pcvSvmView_SetCameraParameter(&Viewgen_tFrontCamParam);
	pcvSvmView_SetCameraParameter(&Viewgen_tLeftCamParam);
	pcvSvmView_SetCameraParameter(&Viewgen_tRearCamParam);
	pcvSvmView_SetCameraParameter(&Viewgen_tRightCamParam);



	
	pcvSvmView_Set2dSurfaceProjectCamera(&tSurface2dParam, &Viewgen_tFrontCamParam, PCV_SVM_VIEW_CAMERA_TYPE_FRONT);
	pcvSvmView_Set2dSurfaceProjectCamera(&tSurface2dParam, &Viewgen_tLeftCamParam, PCV_SVM_VIEW_CAMERA_TYPE_LEFT);
	pcvSvmView_Set2dSurfaceProjectCamera(&tSurface2dParam, &Viewgen_tRearCamParam, PCV_SVM_VIEW_CAMERA_TYPE_REAR);
	pcvSvmView_Set2dSurfaceProjectCamera(&tSurface2dParam, &Viewgen_tRightCamParam, PCV_SVM_VIEW_CAMERA_TYPE_RIGHT);

	pcvSvmView_Set3dSurfaceProjectCamera(&tSurface3dParam, &Viewgen_tFrontCamParam, PCV_SVM_VIEW_CAMERA_TYPE_FRONT);
	pcvSvmView_Set3dSurfaceProjectCamera(&tSurface3dParam, &Viewgen_tLeftCamParam, PCV_SVM_VIEW_CAMERA_TYPE_LEFT);
	pcvSvmView_Set3dSurfaceProjectCamera(&tSurface3dParam, &Viewgen_tRearCamParam, PCV_SVM_VIEW_CAMERA_TYPE_REAR);
	pcvSvmView_Set3dSurfaceProjectCamera(&tSurface3dParam, &Viewgen_tRightCamParam, PCV_SVM_VIEW_CAMERA_TYPE_RIGHT);




	
	return eSUCCESS;

}

PP_VOID PPAPI_Viewgen_Make_Top_2d_FB_LUT(PP_VOID)
{
	pcvSvmView_Get2dFrViewLut(&tSurface2dParam, &Viewgen_tFrontCamParam, &Viewgen_tRearCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
}
PP_VOID PPAPI_Viewgen_Make_Top_2d_LR_LUT(PP_VOID)
{
	pcvSvmView_Get2dLrViewLut(&tSurface2dParam, &Viewgen_tLeftCamParam, &Viewgen_tRightCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
}
PP_RESULT_E PPAPI_Section_Viewgen_Prepare_Make(PP_U32 section_num,PP_U32 swing_degree)
{
	//PP_S32	gridSize;
	
	if(section_num<Calib_Cnf_Bin.mdViewHeader.viewCount-1)
	{
		if(Calib_Cnf_Bin.mdViewHeader.subView[section_num].width==0)return eERROR_FAILURE;
	
		if(Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_2D)//topview
		{
			tMdViewParam.camType = Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;
			tMdViewParam.width = Calib_Cnf_Bin.mdViewHeader.subView[section_num].width;
			tMdViewParam.height = Calib_Cnf_Bin.mdViewHeader.subView[section_num].height;
			tMdViewParam.scaleX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleX;
			tMdViewParam.scaleY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleY;			
			tMdViewParam.tVirtualCamParam.angX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngX;
			tMdViewParam.tVirtualCamParam.angY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngY;
			tMdViewParam.tVirtualCamParam.angZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngZ;
			tMdViewParam.tVirtualCamParam.posX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosX;
			tMdViewParam.tVirtualCamParam.posY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosY;
			tMdViewParam.tVirtualCamParam.posZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosZ;
			tMdViewParam.tVirtualCamParam.focalLenX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.focalLenY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.optCenterX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].cx;
			tMdViewParam.tVirtualCamParam.optCenterY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].cy;
			tMdViewParam.hFlip = Calib_Cnf_Bin.mdViewHeader.subView[section_num].hFlip;

			
			pcvSvmView_GetViewGridParam(&tViewGridParam, tMdViewParam.width, tMdViewParam.height, scanType, viewSampleMode);
			viewGridXSize = tViewGridParam.width;
			viewGridYSize = tViewGridParam.height;
			tMdViewParam.pSurfaceGridIndex = (PP_U32 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(PP_U32));
			
			
			pcvSvmView_SetCameraParameter(&(tMdViewParam.tVirtualCamParam));
			
			pcvSvmView_Set2dSurfaceProjectView(&tSurface2dParam, &tMdViewParam, scanType, fieldType, viewSampleMode);
			

			
			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * 8);
			pFBLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);
			pLRLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);

			
			
		}
		else if(Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_3D)//3d view
		{		
			tMdViewParam.camType = Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;
			tMdViewParam.width = Calib_Cnf_Bin.mdViewHeader.subView[section_num].width;
			tMdViewParam.height = Calib_Cnf_Bin.mdViewHeader.subView[section_num].height;
			tMdViewParam.scaleX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleX;
			tMdViewParam.scaleY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleY;
			

			tMdViewParam.pSurfaceGridIndex = (PP_U32 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(PP_U32));
			tMdViewParam.tVirtualCamParam.angX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngX;
			tMdViewParam.tVirtualCamParam.angY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngY;
			tMdViewParam.tVirtualCamParam.angZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngZ;
			tMdViewParam.tVirtualCamParam.posX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosX;
			tMdViewParam.tVirtualCamParam.posY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosY;
			tMdViewParam.tVirtualCamParam.posZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosZ;
			tMdViewParam.tVirtualCamParam.focalLenX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.focalLenY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.optCenterX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].cx;
			tMdViewParam.tVirtualCamParam.optCenterY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].cy;
			tMdViewParam.hFlip = Calib_Cnf_Bin.mdViewHeader.subView[section_num].hFlip;

			
			pcvSvmView_GetViewGridParam(&tViewGridParam, tMdViewParam.width, tMdViewParam.height, scanType, viewSampleMode);
			viewGridXSize = tViewGridParam.width;
			viewGridYSize = tViewGridParam.height;
			tMdViewParam.pSurfaceGridIndex = (unsigned int *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(unsigned int));




			pcvSvmView_SetCameraParameter(&(tMdViewParam.tVirtualCamParam));



			pcvSvmView_Set3dSurfaceProjectView(&tSurface3dParam, &tMdViewParam, scanType, fieldType, viewSampleMode);


			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * 8);
			pFBLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);
			pLRLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);



		}
		else if(Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_SD)//3d view
		{		
			tMdViewParam.camType = Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;
			tMdViewParam.width = Calib_Cnf_Bin.mdViewHeader.subView[section_num].width;
			tMdViewParam.height = Calib_Cnf_Bin.mdViewHeader.subView[section_num].height;
			tMdViewParam.scaleX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleX;
			tMdViewParam.scaleY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleY;
			

			tMdViewParam.pSurfaceGridIndex = (PP_U32 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(PP_U32));
			tMdViewParam.tVirtualCamParam.angX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngX;
			tMdViewParam.tVirtualCamParam.angY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngY;
			tMdViewParam.tVirtualCamParam.angZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamAngZ;
			tMdViewParam.tVirtualCamParam.posX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosX;
			tMdViewParam.tVirtualCamParam.posY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosY;
			tMdViewParam.tVirtualCamParam.posZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosZ;
			tMdViewParam.tVirtualCamParam.focalLenX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.focalLenY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.optCenterX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].cx;
			tMdViewParam.tVirtualCamParam.optCenterY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].cy;
			tMdViewParam.hFlip = Calib_Cnf_Bin.mdViewHeader.subView[section_num].hFlip;

			
			pcvSvmView_GetViewGridParam(&tViewGridParam, tMdViewParam.width, tMdViewParam.height, scanType, viewSampleMode);
			viewGridXSize = tViewGridParam.width;
			viewGridYSize = tViewGridParam.height;
			tMdViewParam.pSurfaceGridIndex = (unsigned int *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(unsigned int));



			pcvSvmView_SetCameraParameter(&(tMdViewParam.tVirtualCamParam));



			pcvSvmView_Set3dSurfaceProjectView(&tSurface3dParam, &tMdViewParam, scanType, fieldType, viewSampleMode);



			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * 8);
			pFBLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);
			pLRLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);

		}
		#if 0
		else if(Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT)//undistort view camview
		{		
			tUndistortViewParam.viewType = PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT;
			tUndistortViewParam.width = Calib_Cnf_Bin.mdViewHeader.subView[section_num].width;
			tUndistortViewParam.height = Calib_Cnf_Bin.mdViewHeader.subView[section_num].height;
			tUndistortViewParam.scaleX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleX;
			tUndistortViewParam.scaleY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleY;
			tUndistortViewParam.hFlip = Calib_Cnf_Bin.mdViewHeader.subView[section_num].hFlip;//front2d_view_h_flip;
			tUndistortViewParam.camType = Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;
			tUndistortViewParam.bLdc = Calib_Cnf_Bin.mdViewHeader.subView[section_num].ldc;

			gridSize = pcvSvmView_GetCamViewGridBufSize();
			tUndistortViewParam.tSurfaceParam.ptCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));
			tUndistortViewParam.tSurfaceParam.ptSurfaceGrid = (PCV_SVM_VIEW_SURFACE_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_SURFACE_GRID_PARAM));


			pcvSvmView_GetViewGridParam(&tViewGridParam, tUndistortViewParam.width, tUndistortViewParam.height, scanType, viewSampleMode);
			viewGridXSize = tViewGridParam.width;
			viewGridYSize = tViewGridParam.height;
			tUndistortViewParam.pSurfaceGridIndex = (unsigned int *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(unsigned int));


			
			switch (tUndistortViewParam.camType) {
			case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:
				pcvSvmView_SetUndistortSurfaceProjectView(&Viewgen_tFrontCamParam, &tUndistortViewParam, scanType, fieldType, viewSampleMode);
				break;
			case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
				pcvSvmView_SetUndistortSurfaceProjectView(&Viewgen_tLeftCamParam, &tUndistortViewParam, scanType, fieldType, viewSampleMode);
				break;
			case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
				pcvSvmView_SetUndistortSurfaceProjectView(&Viewgen_tRearCamParam, &tUndistortViewParam, scanType, fieldType, viewSampleMode);
				break;
			case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
				pcvSvmView_SetUndistortSurfaceProjectView(&Viewgen_tRightCamParam, &tUndistortViewParam, scanType, fieldType, viewSampleMode);
				break;
			}
			
			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * 8);
			pFBLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);
			pLRLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);


		}
		else if(Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL)//cylindrical view 
		{		
			tCylindricalViewParam.viewType = PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL;
			tCylindricalViewParam.width = Calib_Cnf_Bin.mdViewHeader.subView[section_num].width;
			tCylindricalViewParam.height = Calib_Cnf_Bin.mdViewHeader.subView[section_num].height;
			tCylindricalViewParam.scaleX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleX;
			tCylindricalViewParam.scaleY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].scaleY;


			tCylindricalViewParam.tVirtualCamParam.posX = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosX;
			tCylindricalViewParam.tVirtualCamParam.posY = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosY;
			tCylindricalViewParam.tVirtualCamParam.posZ = Calib_Cnf_Bin.mdViewHeader.subView[section_num].virCamPosZ;
			tCylindricalViewParam.hFlip = Calib_Cnf_Bin.mdViewHeader.subView[section_num].hFlip;
			tCylindricalViewParam.camType = Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;

			gridSize = pcvSvmView_GetCamViewGridBufSize();
			tCylindricalViewParam.tSurfaceParam.ptCamGrid = (PCV_SVM_VIEW_CAMERA_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_CAMERA_GRID_PARAM));
			tCylindricalViewParam.tSurfaceParam.ptSurfaceGrid = (PCV_SVM_VIEW_SURFACE_GRID_PARAM *)PPAPI_Lib_Ext_Malloc(gridSize * sizeof(PCV_SVM_VIEW_SURFACE_GRID_PARAM));
			tCylindricalViewParam.tSurfaceParam.dist = Calib_Cnf_Bin.Surface_Cylindrical.dist;

			pcvSvmView_GetViewGridParam(&tViewGridParam, tCylindricalViewParam.width, tCylindricalViewParam.height, scanType, viewSampleMode);
			viewGridXSize = tViewGridParam.width;
			viewGridYSize = tViewGridParam.height;
			tCylindricalViewParam.pSurfaceGridIndex = (unsigned int *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(unsigned int));

			pcvSvmView_SetCylindricalSurface(&tCylindricalViewParam);
			
			switch (tCylindricalViewParam.camType) {
			case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:
				pcvSvmView_SetCylindricalSurfaceProjectCamera(&Viewgen_tFrontCamParam, &tCylindricalViewParam);
				break;
			case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
				pcvSvmView_SetCylindricalSurfaceProjectCamera(&Viewgen_tLeftCamParam, &tCylindricalViewParam);
				break;
			case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
				pcvSvmView_SetCylindricalSurfaceProjectCamera(&Viewgen_tRearCamParam, &tCylindricalViewParam);
				break;
			case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
				pcvSvmView_SetCylindricalSurfaceProjectCamera(&Viewgen_tRightCamParam, &tCylindricalViewParam);
				break;
			}
			pcvSvmView_SetCylindricalSurfaceProjectView(&tCylindricalViewParam, scanType, fieldType, viewSampleMode);
			
			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * 8);
			pFBLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);
			pLRLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);

		}
		#endif


	}
	else if(section_num>=Calib_Cnf_Bin.mdViewHeader.viewCount-1)
	{
		if(Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].viewType==PCV_SVM_VIEW_SUB_VIEW_TYPE_DEG360)//3d view
		{		
			tMdViewParam.camType = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].camType;
			tMdViewParam.width = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].width;
			tMdViewParam.height = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].height;
			tMdViewParam.scaleX = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].scaleX;
			tMdViewParam.scaleY = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].scaleY;
			tMdViewParam.tVirtualCamParam.angX = Calib_Cnf_Bin.swingView.CamParam[swing_degree].virCamAngX;
			tMdViewParam.tVirtualCamParam.angY = Calib_Cnf_Bin.swingView.CamParam[swing_degree].virCamAngY;
			tMdViewParam.tVirtualCamParam.angZ = Calib_Cnf_Bin.swingView.CamParam[swing_degree].virCamAngZ;
			tMdViewParam.tVirtualCamParam.posX = Calib_Cnf_Bin.swingView.CamParam[swing_degree].virCamPosX;
			tMdViewParam.tVirtualCamParam.posY = Calib_Cnf_Bin.swingView.CamParam[swing_degree].virCamPosY;
			tMdViewParam.tVirtualCamParam.posZ = Calib_Cnf_Bin.swingView.CamParam[swing_degree].virCamPosZ;
			tMdViewParam.tVirtualCamParam.focalLenX = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.focalLenY = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].virCamFocalLen;
			tMdViewParam.tVirtualCamParam.optCenterX = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].cx;
			tMdViewParam.tVirtualCamParam.optCenterY = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].cy;
			tMdViewParam.hFlip = Calib_Cnf_Bin.mdViewHeader.subView[Calib_Cnf_Bin.mdViewHeader.viewCount-1].hFlip;

			
			pcvSvmView_GetViewGridParam(&tViewGridParam, tMdViewParam.width, tMdViewParam.height, scanType, viewSampleMode);
			viewGridXSize = tViewGridParam.width;
			viewGridYSize = tViewGridParam.height;
			tMdViewParam.pSurfaceGridIndex = (unsigned int *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * sizeof(unsigned int));





			pcvSvmView_SetCameraParameter(&(tMdViewParam.tVirtualCamParam));

			pcvSvmView_Set3dSurfaceProjectView(&tSurface3dParam, &tMdViewParam, scanType, fieldType, viewSampleMode);


			pProcBuf = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize * viewGridYSize * 8);
			pFBLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);
			pLRLut = (PP_U8 *)PPAPI_Lib_Ext_Malloc(viewGridXSize*viewGridYSize * 4);

		}

	}
	else
	{
		
		LOG_DEBUG("ERROR!");
	}

	return eSUCCESS;


}





PP_VOID PPAPI_Viewgen_Make_RS_3D_FB_LUT(PP_VOID)
{
	pcvSvmView_Get3dFrViewLut(&tSurface3dParam, &Viewgen_tFrontCamParam, &Viewgen_tRearCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
}

PP_VOID PPAPI_Viewgen_Make_RS_SD_FB_LUT(PP_VOID)
{
	switch (tMdViewParam.camType) {
	case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:	
		pcvSvmView_GetSdFrViewLut(&tSurface3dParam, &Viewgen_tFrontCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
		pcvSvmView_GetSdFrViewLut(&tSurface3dParam, &Viewgen_tLeftCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
		pcvSvmView_GetSdFrViewLut(&tSurface3dParam, &Viewgen_tRearCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
		pcvSvmView_GetSdFrViewLut(&tSurface3dParam, &Viewgen_tRightCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	}

}
PP_VOID PPAPI_Viewgen_Make_RS_UNDIS_FB_LUT(PP_VOID)
{
	switch (tUndistortViewParam.camType) {
	case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:	
		pcvSvmView_GetCamFrViewLut(&Viewgen_tFrontCamParam, &tUndistortViewParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
		pcvSvmView_GetCamFrViewLut(&Viewgen_tLeftCamParam, &tUndistortViewParam, pProcBuf, pProcBuf, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
		pcvSvmView_GetCamFrViewLut(&Viewgen_tRearCamParam, &tUndistortViewParam, pProcBuf, pProcBuf, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
		pcvSvmView_GetCamFrViewLut(&Viewgen_tRightCamParam, &tUndistortViewParam, pProcBuf, pProcBuf, scanType, fieldType, viewSampleMode);
		break;
	}


}

PP_VOID PPAPI_Viewgen_Make_RS_CYLIND_FB_LUT(PP_VOID)
{
	switch (tCylindricalViewParam.camType) {
	case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:	
		pcvSvmView_GetCamFrViewLut(&Viewgen_tFrontCamParam, &tCylindricalViewParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
		pcvSvmView_GetCamFrViewLut(&Viewgen_tLeftCamParam, &tCylindricalViewParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
		pcvSvmView_GetCamFrViewLut(&Viewgen_tRearCamParam, &tCylindricalViewParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
		pcvSvmView_GetCamFrViewLut(&Viewgen_tRightCamParam, &tCylindricalViewParam, pProcBuf, pFBLut, scanType, fieldType, viewSampleMode);
		break;
	}

}


PP_VOID PPAPI_Viewgen_Make_RS_3D_LR_LUT(PP_VOID)
{
	pcvSvmView_Get3dLrViewLut(&tSurface3dParam, &Viewgen_tLeftCamParam, &Viewgen_tRightCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
}
PP_VOID PPAPI_Viewgen_Make_RS_SD_LR_LUT(PP_VOID)
{
	switch (tMdViewParam.camType) {
	case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:	
		pcvSvmView_GetSdLrViewLut(&tSurface3dParam, &Viewgen_tFrontCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
		pcvSvmView_GetSdLrViewLut(&tSurface3dParam, &Viewgen_tLeftCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
		pcvSvmView_GetSdLrViewLut(&tSurface3dParam, &Viewgen_tRearCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
		pcvSvmView_GetSdLrViewLut(&tSurface3dParam, &Viewgen_tRightCamParam, &tMdViewParam, &tBlendParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	}
}
PP_VOID PPAPI_Viewgen_Make_RS_UNDIS_LR_LUT(PP_VOID)
{
	switch (tUndistortViewParam.camType) {
	case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:	
		pcvSvmView_GetCamLrViewLut(&Viewgen_tFrontCamParam, &tUndistortViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
		pcvSvmView_GetCamLrViewLut(&Viewgen_tLeftCamParam, &tUndistortViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
		pcvSvmView_GetCamLrViewLut(&Viewgen_tRearCamParam, &tUndistortViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
		pcvSvmView_GetCamLrViewLut(&Viewgen_tRightCamParam, &tUndistortViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	}

}
PP_VOID PPAPI_Viewgen_Make_RS_CYLIND_LR_LUT(PP_VOID)
{
	switch (tCylindricalViewParam.camType) {
	case PCV_SVM_VIEW_CAMERA_TYPE_FRONT:	
		pcvSvmView_GetCamLrViewLut(&Viewgen_tFrontCamParam, &tCylindricalViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_LEFT:
		pcvSvmView_GetCamLrViewLut(&Viewgen_tLeftCamParam, &tCylindricalViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_REAR:
		pcvSvmView_GetCamLrViewLut(&Viewgen_tRearCamParam, &tCylindricalViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	case PCV_SVM_VIEW_CAMERA_TYPE_RIGHT:
		pcvSvmView_GetCamLrViewLut(&Viewgen_tRightCamParam, &tCylindricalViewParam, pProcBuf, pLRLut, scanType, fieldType, viewSampleMode);
		break;
	}
}


PP_VOID PPAPI_Viewgen_Free_Global_Values(PP_VOID)
{
	
		PPAPI_Lib_Ext_Free(tSurface2dParam.ptSurfaceGrid);
		PPAPI_Lib_Ext_Free(tSurface2dParam.ptFrontCamGrid);
		PPAPI_Lib_Ext_Free(tSurface2dParam.ptLeftCamGrid);
		PPAPI_Lib_Ext_Free(tSurface2dParam.ptRearCamGrid);
		PPAPI_Lib_Ext_Free(tSurface2dParam.ptRightCamGrid);
	
		PPAPI_Lib_Ext_Free(tSurface3dParam.ptSurfaceGrid);
		PPAPI_Lib_Ext_Free(tSurface3dParam.ptFrontCamGrid);
		PPAPI_Lib_Ext_Free(tSurface3dParam.ptLeftCamGrid);
		PPAPI_Lib_Ext_Free(tSurface3dParam.ptRearCamGrid);
		PPAPI_Lib_Ext_Free(tSurface3dParam.ptRightCamGrid);
		
	


}
PP_VOID PPAPI_Viewgen_Free_2DTOP_Values(PP_VOID)
{
	if(pFBLut!=NULL)
	{
		PPAPI_Lib_Ext_Free(pFBLut);
		PPAPI_Lib_Ext_Free(pLRLut);
		PPAPI_Lib_Ext_Free(tMdViewParam.pSurfaceGridIndex); 	
		PPAPI_Lib_Ext_Free(pProcBuf);
	}

}

PP_VOID PPAPI_Viewgen_Free_3D_Values(PP_VOID)
{
	if(pFBLut!=NULL)
	{
		PPAPI_Lib_Ext_Free(pFBLut);
		PPAPI_Lib_Ext_Free(pLRLut);
		PPAPI_Lib_Ext_Free(tMdViewParam.pSurfaceGridIndex);
		PPAPI_Lib_Ext_Free(pProcBuf);
	}

}
PP_VOID PPAPI_Viewgen_Free_SD_Values(PP_VOID)
{
	if(pFBLut!=NULL)
	{
		PPAPI_Lib_Ext_Free(pFBLut);
		PPAPI_Lib_Ext_Free(pLRLut);
		PPAPI_Lib_Ext_Free(tMdViewParam.pSurfaceGridIndex);
		PPAPI_Lib_Ext_Free(pProcBuf);

	}
}

PP_VOID PPAPI_Viewgen_Free_UNDIS_Values(PP_VOID)
{
	if(pFBLut!=NULL)
	{
		PPAPI_Lib_Ext_Free(tUndistortViewParam.pSurfaceGridIndex);
		PPAPI_Lib_Ext_Free(tUndistortViewParam.tSurfaceParam.ptCamGrid);
		PPAPI_Lib_Ext_Free(tUndistortViewParam.tSurfaceParam.ptSurfaceGrid);
		PPAPI_Lib_Ext_Free(pFBLut);
		PPAPI_Lib_Ext_Free(pLRLut);
		PPAPI_Lib_Ext_Free(pProcBuf);

	}
}
PP_VOID PPAPI_Viewgen_Free_CYLIND_Values(PP_VOID)
{
	if(pFBLut!=NULL)
	{

		PPAPI_Lib_Ext_Free(tCylindricalViewParam.pSurfaceGridIndex);
		PPAPI_Lib_Ext_Free(tCylindricalViewParam.tSurfaceParam.ptCamGrid);
		PPAPI_Lib_Ext_Free(tCylindricalViewParam.tSurfaceParam.ptSurfaceGrid);
		PPAPI_Lib_Ext_Free(pFBLut);
		PPAPI_Lib_Ext_Free(pLRLut);
		PPAPI_Lib_Ext_Free(pProcBuf);

	}
}



PP_U32 PPAPI_Section_Viewgen_Get_ViewType(PP_U32 section_num)
{
	return Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType;
}

PP_U32 PPAPI_Section_Viewgen_Get_CamType (PP_U32 section_num)
{
	return Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;
}




extern PP_OFFCALIB_OUT_INFO_S Off_Calib_Out;
extern PP_VIEWGEN_PROCESS_INFO_S Viewgen_Process_Info;

PP_VOID PPAPI_Lib_Progressbar_Processing(PP_VOID)
{
	STATIC PP_U32 cnt_20ms=0;
	STATIC PP_U32 flag_100ms=0;
	STATIC PP_U32 cnt_100ms=0;
	
	PP_U8 progress_bar_percentage=0;  
	STATIC PP_U8 progress_bar_toggle_flag;  
	/* modify below values */
	STATIC PP_U32 u32UnitTimeMCalibSec=5; // x 100ms per each percentage
	STATIC PP_U32 u32UnitTimeACalibSec=17; // x 100ms per each percentage
	STATIC PP_U32 u32UnitTimeViewGenSec=22; // x 100ms per each percentage
	
	cnt_20ms++;
	if(cnt_20ms==5) //20*15=300ms
	{
		flag_100ms=TRUE;
		cnt_20ms=0;
	}

	if(flag_100ms)
	{
		flag_100ms=FALSE;
		cnt_100ms++;

		if(Off_Calib_Out.eOffcalib_Progress_Step==eOFFCALIB_WAIT_CMD&&Viewgen_Process_Info.eSViewgen_Progress_Step==eSVIEWGEN_WAIT_CMD)
		{
			cnt_100ms=0;
		}
		else if(Off_Calib_Out.eOffcalib_Progress_Step==eOFFCALIB_END || Viewgen_Process_Info.eSViewgen_Progress_Step==eSVIEWGEN_END)
		{
			cnt_100ms=0;
		}
		if(Viewgen_Process_Info.eSViewgen_Progress_Step==eSVIEWGEN_START)
		{
			cnt_100ms=0;
		}
		else
		{
			if(Off_Calib_Out.eOffcalib_Progress_Step!=eOFFCALIB_WAIT_CMD&&Off_Calib_Out.eOffcalib_Progress_Step!=eOFFCALIB_END)
			{
				if(Off_Calib_Out.eCh_sel!=eCALIB_CAMERA_ALL)
				{							
					if(cnt_100ms < u32UnitTimeMCalibSec)progress_bar_percentage=1; // 0.5 sec
					else if(cnt_100ms < u32UnitTimeMCalibSec*2)progress_bar_percentage=2; // 1 sec
					else if(cnt_100ms < u32UnitTimeMCalibSec*3)progress_bar_percentage=3; // 1.5 sec
					else if(cnt_100ms < u32UnitTimeMCalibSec*4)progress_bar_percentage=4; // 2 sec
					else if(cnt_100ms < u32UnitTimeMCalibSec*5)progress_bar_percentage=5; // 2.5 sec 
					else if(cnt_100ms < u32UnitTimeMCalibSec*6)progress_bar_percentage=6; // 3 sec
					else if(cnt_100ms < u32UnitTimeMCalibSec*7)progress_bar_percentage=7; // 3.5 sec
					else if(cnt_100ms < u32UnitTimeMCalibSec*8)progress_bar_percentage=8; // 4 sec
					else progress_bar_percentage=9; // 4.5 sec


				}
				else
				{
					if(cnt_100ms < u32UnitTimeACalibSec)progress_bar_percentage=1; // 1.7sec
					else if(cnt_100ms < u32UnitTimeACalibSec*2)progress_bar_percentage=2; // 3.4 sec
					else if(cnt_100ms < u32UnitTimeACalibSec*3)progress_bar_percentage=3; // 5.1 sec
					else if(cnt_100ms < u32UnitTimeACalibSec*4)progress_bar_percentage=4; // 6.8 sec
					else if(cnt_100ms < u32UnitTimeACalibSec*5)progress_bar_percentage=5; // 8.5 sec 
					else if(cnt_100ms < u32UnitTimeACalibSec*6)progress_bar_percentage=6; // 10.2 sec
					else if(cnt_100ms < u32UnitTimeACalibSec*7)progress_bar_percentage=7; // 11.9 sec
					else if(cnt_100ms < u32UnitTimeACalibSec*8)progress_bar_percentage=8; // 13.6 sec
					else progress_bar_percentage=9; // 15.3 sec

				}
				if(progress_bar_toggle_flag==0)progress_bar_toggle_flag=1;	
				else progress_bar_toggle_flag=0;


#if defined(USE_PP_GUI)
				PPAPI_DISPLAY_PROGRESSBAR_On(progress_bar_percentage, progress_bar_toggle_flag);	
#endif
			}

			if(Viewgen_Process_Info.eSViewgen_Progress_Step!=eSVIEWGEN_WAIT_CMD&&Viewgen_Process_Info.eSViewgen_Progress_Step!=eSVIEWGEN_END)
			{
				if(gMulticore_Viewgen_Flag)
				{
					if(cnt_100ms < u32UnitTimeViewGenSec)progress_bar_percentage=1; // 15sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*2)progress_bar_percentage=2; // 30 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*3)progress_bar_percentage=3; // 45 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*4)progress_bar_percentage=4; // 60 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*5)progress_bar_percentage=5; // 75 sec 
					else if(cnt_100ms < u32UnitTimeViewGenSec*6)progress_bar_percentage=6; // 90 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*7)progress_bar_percentage=7; // 105 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*8)progress_bar_percentage=8; // 120 sec
					else progress_bar_percentage=9; // 135 sec					
				}
				else
				{
					if(cnt_100ms < u32UnitTimeViewGenSec)progress_bar_percentage=1; // 5sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*2)progress_bar_percentage=2; // 10 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*3)progress_bar_percentage=3; // 15 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*4)progress_bar_percentage=4; // 20 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*5)progress_bar_percentage=5; // 25 sec 
					else if(cnt_100ms < u32UnitTimeViewGenSec*6)progress_bar_percentage=6; // 30 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*7)progress_bar_percentage=7; // 35 sec
					else if(cnt_100ms < u32UnitTimeViewGenSec*8)progress_bar_percentage=8; // 40 sec
					else progress_bar_percentage=9; // 45 sec
				}
				if(progress_bar_toggle_flag==0)progress_bar_toggle_flag=1;	
				else progress_bar_toggle_flag=0;


#if defined(USE_PP_GUI)
				PPAPI_DISPLAY_PROGRESSBAR_On(progress_bar_percentage, progress_bar_toggle_flag);
#endif

			}

		}


	}

}

#else // //CALIB_LIB_USE

#include "app_calibration.h"
#include "api_calibration.h"


PP_CNF_TOTAL_BIN_FORMAT_S Calib_Cnf_Bin;
PP_CNF_TOTAL_BIN_FORMAT_S Calib_Cnf_Bin_Sub;

PP_VOID PPAPI_Lib_Calib_Initialize(PP_VOID)
{
		// Load Calib Cnf  Header
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		PPAPI_FLASH_Read(&Calib_Cnf_Bin, gstFlashHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32FlashAddr, gstFlashHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32Size);
#else

		PPAPI_FTL_Read(&Calib_Cnf_Bin, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_MAIN_TYPE].u32Size);
		PPAPI_FTL_Read(&Calib_Cnf_Bin_Sub, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_SUB_TYPE].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_CAM_CALIB_SUB_TYPE].u32Size);

#endif

	
}

PP_U32 PPAPI_Section_Viewgen_Get_ViewType(PP_U32 section_num)
{
	return Calib_Cnf_Bin.mdViewHeader.subView[section_num].viewType;
}

PP_U32 PPAPI_Section_Viewgen_Get_CamType (PP_U32 section_num)
{
	return Calib_Cnf_Bin.mdViewHeader.subView[section_num].camType;
}

#endif //CALIB_LIB_USE

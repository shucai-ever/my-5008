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
 * A sample implementation of pvMemMangODMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/////////////////////////////////////////////////////////////////////////////////////    
#include "dram_config.h"
#include "MemMang_OD.h"    

extern _gstDramReserved gstDramReserved;

#define configADDR_MEMMANGOD                    ( (uint8_t *) (gstDramReserved.u32CaptureRsvBuff[0]) )
#define configTOTAL_MEMMANGOD_SIZE				( ( size_t ) 128*1024 ) //1MByte DDR

/* Allocate the memory for the memMangOD. */
static uint8_t *pucMemMangOD = NULL;

/////////////////////////////////////////////////////////////////////////////////////    

/* Block sizes must not get too small. */
#define memMangOD_MINIMUM_BLOCK_SIZE	( ( size_t ) ( memMangOD_STRUCT_SIZE * 2 ) )

/* Assumes 8bit bytes! */
#define memMangOD_BITS_PER_BYTE		( ( size_t ) 8 )

/* A few bytes might be lost to byte aligning the mem start address. */
#define ADJUSTED_MEMMANGOD_SIZE	( configTOTAL_MEMMANGOD_SIZE - portBYTE_ALIGNMENT )

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct MEMMANGOD_BLOCK_LINK
{
	struct MEMMANGOD_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} MEMMANGOD_BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvMemMangODInsertBlockIntoFreeList( MEMMANGOD_BlockLink_t *pxBlockToInsert );

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const uint16_t memMangOD_STRUCT_SIZE	= ( ( sizeof ( MEMMANGOD_BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~portBYTE_ALIGNMENT_MASK );

/* Ensure the pxMemMangODEnd pointer will end up on the correct byte alignment. */
static const size_t xTotalMemMangODSize = ( ( size_t ) ADJUSTED_MEMMANGOD_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
static MEMMANGOD_BlockLink_t xMemMangODStart, *pxMemMangODEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xMemMangODFreeBytesRemaining = ( ( size_t ) ADJUSTED_MEMMANGOD_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );
static size_t xMemMangODMinimumEverFreeBytesRemaining = ( ( size_t ) ADJUSTED_MEMMANGOD_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK );

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an MEMMANGOD_BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xMemMangODBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/

void *pvMemMangODMalloc( size_t xWantedSize )
{
MEMMANGOD_BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
void *pvReturn = NULL;

	vTaskSuspendAll();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( pxMemMangODEnd == NULL )
		{
			prvMemMangODInit();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the MEMMANGOD_BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & xMemMangODBlockAllocatedBit ) == 0 )
		{
			/* The wanted size is increased so it can contain a MEMMANGOD_BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( xWantedSize > 0 )
			{
				xWantedSize += memMangOD_STRUCT_SIZE;

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

			if( ( xWantedSize > 0 ) && ( xWantedSize <= xMemMangODFreeBytesRemaining ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &xMemMangODStart;
				pxBlock = xMemMangODStart.pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != pxMemMangODEnd )
				{
					/* Return the memory space pointed to - jumping over the
					MEMMANGOD_BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + memMangOD_STRUCT_SIZE );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - xWantedSize ) > memMangOD_MINIMUM_BLOCK_SIZE )
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
						prvMemMangODInsertBlockIntoFreeList( ( pxNewBlockLink ) );
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					xMemMangODFreeBytesRemaining -= pxBlock->xBlockSize;

					if( xMemMangODFreeBytesRemaining < xMemMangODMinimumEverFreeBytesRemaining )
					{
						xMemMangODMinimumEverFreeBytesRemaining = xMemMangODFreeBytesRemaining;
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xBlockSize |= xMemMangODBlockAllocatedBit;
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

void vMemMangODFree( void *pv )
{
uint8_t *puc = ( uint8_t * ) pv;
MEMMANGOD_BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an MEMMANGOD_BlockLink_t structure immediately
		before it. */
		puc -= memMangOD_STRUCT_SIZE;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		/* Check the block is actually allocated. */
		configASSERT( ( pxLink->xBlockSize & xMemMangODBlockAllocatedBit ) != 0 );
		configASSERT( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & xMemMangODBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~xMemMangODBlockAllocatedBit;

				vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					xMemMangODFreeBytesRemaining += pxLink->xBlockSize;
					traceFREE( pv, pxLink->xBlockSize );
					prvMemMangODInsertBlockIntoFreeList( ( ( MEMMANGOD_BlockLink_t * ) pxLink ) );
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

size_t xMemMangODGetFreeSize( void )
{
	return xMemMangODFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xMemMangODGetMinimumEverFreeSize( void )
{
	return xMemMangODMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vMemMangODInitialiseBlocks( void )
{
	/* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

void prvMemMangODInit( void )
{
MEMMANGOD_BlockLink_t *pxFirstFreeBlock;
uint8_t *pucHeapEnd, *pucAlignedHeap;

    pucMemMangOD = configADDR_MEMMANGOD;

	/* Ensure the heap starts on a correctly aligned boundary. */
	pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE ) (pucMemMangOD + portBYTE_ALIGNMENT ) ) & ( ( portPOINTER_SIZE_TYPE ) ~portBYTE_ALIGNMENT_MASK ) );

	/* xMemMangODStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	xMemMangODStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	xMemMangODStart.xBlockSize = ( size_t ) 0;

	/* pxMemMangODEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	pucHeapEnd = pucAlignedHeap + xTotalMemMangODSize;
	pucHeapEnd -= memMangOD_STRUCT_SIZE;
	pxMemMangODEnd = ( void * ) pucHeapEnd;
	configASSERT( ( ( ( uint32_t ) pxMemMangODEnd ) & ( ( uint32_t ) portBYTE_ALIGNMENT_MASK ) ) == 0UL );
	pxMemMangODEnd->xBlockSize = 0;
	pxMemMangODEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxMemMangODEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = xTotalMemMangODSize - memMangOD_STRUCT_SIZE;
	pxFirstFreeBlock->pxNextFreeBlock = pxMemMangODEnd;

	/* The heap now contains pxMemMangODEnd. */
	xMemMangODFreeBytesRemaining -= memMangOD_STRUCT_SIZE;

	/* Work out the position of the top bit in a size_t variable. */
	xMemMangODBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * memMangOD_BITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void prvMemMangODInsertBlockIntoFreeList( MEMMANGOD_BlockLink_t *pxBlockToInsert )
{
MEMMANGOD_BlockLink_t *pxIterator;
uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &xMemMangODStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
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
		if( pxIterator->pxNextFreeBlock != pxMemMangODEnd )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxMemMangODEnd;
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


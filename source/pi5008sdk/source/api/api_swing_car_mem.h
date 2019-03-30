#ifndef __API_SWING_CAR_MEM_H__
#define __API_SWING_CAR_MEM_H__

void *pvMemMangSwingCarMalloc( size_t xWantedSize );
void vMemMangSwingCarFree( void *pv );
size_t xMemMangSwingCarGetFreeSize( void );
size_t xMemMangSwingCarGetMinimumEverFreeSize( void );
void prvMemMangSwingCarInit( void );

#endif // __API_SWING_CAR_MEM_H__

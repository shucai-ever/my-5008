#ifndef __API_DISPLAY_MEM_H__
#define __API_DISPLAY_MEM_H__

void *pvMemMangDisplayMalloc( size_t xWantedSize );
void vMemMangDisplayFree( void *pv );
size_t xMemMangDisplayGetFreeSize( void );
size_t xMemMangDisplayGetMinimumEverFreeSize( void );
void prvMemMangDisplayInit( void );

#endif // __API_DISPLAY_MEM_H__

#ifndef __MEM_MANG_OD_H__    
#define __MEM_MANG_OD_H__    

void *pvMemMangODMalloc( size_t xWantedSize );
void vMemMangODFree( void *pv );
size_t xMemMangODGetFreeSize( void );
size_t xMemMangODGetMinimumEverFreeSize( void );
void prvMemMangODInit( void );

#endif // __MEM_MANG_OD_H__    

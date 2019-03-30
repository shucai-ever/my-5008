#ifndef _PI5008_CACHE_H
#define _PI5008_CACHE_H

void cache_enable();
void cache_disable();
void cache_wb_range(uint32 *addr, sint32 size);
void cache_inv_range(uint32 *addr, sint32 size);
void cache_wb_all(void);
void cache_inv_all(void);


#endif

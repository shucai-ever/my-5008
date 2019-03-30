
#include <stdio.h>
#include "spi.h"
#include "spi_memctrl.h"
#include "osal.h"
#include "cache.h"
#include "board_config.h"
#include "common.h"

static SPI_MEM_REG_T *gpSPIMem = SPI_MEM_CTRL_REG;
static uint8 g_qdma_buf_idx = 0;
static uint8 gu8MemcReadMode = SPI_MEMC_MEMORY_MAP_MODE;

static void spi_memctrl_qdma_wait_done(uint32 idx_flag)
{
	while(gpSPIMem->buf_sel & (idx_flag<<8));
}


void spi_memctrl_set(uint32 div, uint32 rx_delay, uint8 read_cmd, SPI_MEM_FR_INFO fr_info)
{
	uint32 *ptr;

	gpSPIMem->divider = div;
	gpSPIMem->rx_delay = rx_delay;
	gpSPIMem->fr_cmd = read_cmd;

	ptr = (uint32 *)&fr_info;
	gpSPIMem->fr_info = *ptr;

	g_qdma_buf_idx = 0;
	//memcpy(&gpSPIMem->fr_info, &fr_info, 4);

	//printf("[SPI Mem Ctrl Registers]\n");
	//print_hexw((uint32 *)gpSPIMem, 0x24);

	spi_memctrl_mode_set(SPI_MEMC_MEMORY_MAP_MODE, 0);
	gu8MemcReadMode = SPI_MEMC_MEMORY_MAP_MODE;
}

void spi_memctrl_crc_en(uint32 en)
{
	gpSPIMem->crc_en = (en & 1);
}

uint16 spi_memctrl_crc_get(void)
{
	return (uint16)(gpSPIMem->crc_result & 0xffff);
}

int spi_memctrl_mode_set(uint8 mode, uint32 bulk_size)
{
	int ret = 0;
	vuint32 cmd;

	bulk_size &= (~0xf);

	if(bulk_size > 256)bulk_size = 256;
	if(bulk_size < 16)bulk_size = 0;

	if(mode == SPI_MEMC_MEMORY_MAP_MODE){
		if(bulk_size){
			gpSPIMem->buf_start |= (1<<1);
		}else{
			gpSPIMem->buf_start &= (~(1<<1));
		}
		gpSPIMem->dst_size[0] = bulk_size;

//		if(bulk_size > 256 || bulk_size < 16)return 1;
//		gpSPIMem->buf_start |= (1<<1);
//		gpSPIMem->dst_size[0] = bulk_size;
	}else{	// QDMA MODE
		gpSPIMem->buf_start &= (~(1<<1));
	}

	gpSPIMem->buf_sel = 0;
	cmd = gpSPIMem->fr_cmd;
	gpSPIMem->fr_cmd = cmd;

	return ret;
}


inline int spi_memctrl_qdma_fill_buff(uint32 flash_addr, uint8 buf_idx, uint32 dst_addr, uint32 size)
{
	int ret = 0;

	if(size > 256 || size < 16)return 1;
	if(buf_idx > 1)return 1;

	gpSPIMem->dst_addr[buf_idx] = dst_addr;
	gpSPIMem->dst_size[buf_idx] = size;

	while(gpSPIMem->buf_start & 1);

	while(gpSPIMem->buf_sel & (1<<(8+buf_idx)));

	gpSPIMem->fr_addr = flash_addr;

	////
//	gpSPIMem->dst_addr[buf_idx] = dst_addr;
//	gpSPIMem->dst_size[buf_idx] = size;
	////

	gpSPIMem->buf_sel = buf_idx;
	gpSPIMem->buf_start |= 1;


//	while(gpSPIMem->buf_start & 1);
//	while(gpSPIMem->buf_sel & (1<<(8+buf_idx)));

	return ret;
}


/*
inline int spi_memctrl_qdma_fill_buff(uint32 flash_addr, uint8 buf_idx, uint32 dst_addr, uint32 size)
{
	int ret = 0;

	if(size > 256 || size < 16)return 1;
	if(buf_idx > 1)return 1;

	gpSPIMem->dst_addr[buf_idx] = dst_addr;
	gpSPIMem->dst_size[buf_idx] = size;


	gpSPIMem->fr_addr = flash_addr;


	gpSPIMem->buf_sel = buf_idx;
	gpSPIMem->buf_start |= 1;


	while(gpSPIMem->buf_start & 1);
	while(gpSPIMem->buf_sel & 0x33300);

	return ret;
}
*/

void spi_memctrl_read(uint32 flash_addr, uint8 *buf, uint32 size)
{
	uint8 *ptr;

	if(gu8MemcReadMode == SPI_MEMC_QDMA_MODE){
		spi_memctrl_mode_set(SPI_MEMC_MEMORY_MAP_MODE, 0);
		gu8MemcReadMode = SPI_MEMC_MEMORY_MAP_MODE;
	}

#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
	vuint32 cmd;
	gpSPIMem->buf_sel = 0;
	cmd = gpSPIMem->fr_cmd;
	gpSPIMem->fr_cmd = 0;
	gpSPIMem->fr_cmd = cmd;
	while(gpSPIMem->fr_cmd != cmd);
#endif
	ptr = (uint8 *)ADDR_NON_CACHEABLE((uint32)(SPI_MEM_BASE_ADDR + flash_addr));
	memcpy(buf, ptr, size);

}

int spi_memctrl_read_qdma(uint32 flash_addr, uint8 *buf, uint32 size)
{
	int ret;
	//uint8 buf_idx = 0;
	uint32 read_size = 0;
	uint32 buf_size;
	vuint32 cmd;

	if(gu8MemcReadMode == SPI_MEMC_MEMORY_MAP_MODE){
		spi_memctrl_mode_set(SPI_MEMC_QDMA_MODE, 0);
		gu8MemcReadMode = SPI_MEMC_QDMA_MODE;
	}

#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
	gpSPIMem->buf_sel = 0;
	cmd = gpSPIMem->fr_cmd;
	gpSPIMem->fr_cmd = 0;
	gpSPIMem->fr_cmd = cmd;
	while(gpSPIMem->fr_cmd != cmd);

#endif
	do{
		if(size > read_size + 256)buf_size = 256;
		else buf_size = (((size - read_size) + 0xf) & (~0xf));

		//printf("flash addr: 0x%x, buf: 0x%x, read_size: 0x%x, size: 0x%x, idx: %d\n", flash_addr, (uint32)&buf[read_size], read_size, buf_size, buf_idx);
		ret = spi_memctrl_qdma_fill_buff(flash_addr, g_qdma_buf_idx, (uint32)&buf[read_size], buf_size);
		if(ret != 0)return ret;

		flash_addr += buf_size;
		read_size += buf_size;

		g_qdma_buf_idx ^= 1;
	}while(size > read_size);

	while(gpSPIMem->buf_sel & (3<<8));
	//while(gpSPIMem->buf_sel & 0x33300);
	//OSAL_sleep(30);
	//OSAL_sleep(1);

	return ret;

}

void spi_memctrl_clear_cache(void)
{
	vuint32 cmd;
	gpSPIMem->buf_sel = 0;
	cmd = gpSPIMem->fr_cmd;
	gpSPIMem->fr_cmd = 0;
	gpSPIMem->fr_cmd = cmd;
	while(gpSPIMem->fr_cmd != cmd);
}
/*
 * buf: output data buffer
 * size: read size
 * offset: offset in page cache
 * plane: plane secect
 */

#define MEMC_GPIO_CS_ENABLE	0
static int gCheckErase = 1;

#if MEMC_GPIO_CS_ENABLE
uint8 spi_memctrl_read_from_cache(uint8 *buf, uint32 size, uint16 offset, uint8 plane )
{
	uint32 ret;
	uint8 use_qdma;
	uint32 read_unit;
	uint8 *ptr;
	vuint32 cmd;
	uint32 i;
	uint32 totsize;
	uint32 read_max_size;

	//LOG_DEBUG("memc offset: %d, size: %d\n", offset, size);
#if VPU_AVAILABLE
	if(((uint32)buf & 0xf) || (size & 0xf) || (offset & 0xff))use_qdma = 0;
	else use_qdma = 1;

#else
	use_qdma = 0;
#endif

	offset |= (plane<<12);
	if(use_qdma){
		read_max_size = 256;
		//LOG_DEBUG("qdma\n");
	}else{
		read_max_size = 16;
		//LOG_DEBUG("memmap. buf: 0x%x, size: 0x%x, offset: 0x%x\n", (uint32)buf, size, offset);
	}

	gpSPIMem->buf_sel = 0;
	cmd = gpSPIMem->fr_cmd;
	gpSPIMem->fr_cmd = cmd;

	totsize = 0;

	cache_inv_range((uint32 *)((uint32)buf), size);

	while(size > totsize){
		read_unit = ((size-totsize) > read_max_size) ? read_max_size : (size-totsize);
		PPDRV_SPI_CSActivate(3);
		while(PPDRV_SPI_CSGetLevel(3) == 1);
		if(use_qdma){
			cache_inv_range((uint32 *)((uint32)buf+totsize), read_unit);
			spi_memctrl_read_qdma(offset, (uint8 *)((uint32)buf+totsize), read_unit);
			//cache_inv_range((uint32 *)((uint32)buf+totsize), read_unit);

		}else{
			//ptr = (uint8 *)ADDR_NON_CACHEABLE((uint32)(SPI_MEM_BASE_ADDR + offset));
			//memcpy((void *)((uint32)buf+totsize), ptr, read_unit);
			spi_memctrl_read(offset, buf, size);
		}
		PPDRV_SPI_CSDeActivate(3);
		while(PPDRV_SPI_CSGetLevel(3) == 0);
		offset += read_unit;
		totsize += read_unit;
	}

	if(gCheckErase){
		if((size&3) == 0){
			ret = 0xffffffffu;
			for(i=0;i<size;i+=4){
				ret &= *(uint32 *)(buf + i);
			}
			ret = ( ((ret>>24)&0xff) & ((ret>>16)&0xff) & ((ret>>8)&0xff) & ((ret>>0)&0xff) );
		}else{
			ret = 0xffu;
			for(i=0;i<size;i++){
				ret &= buf[i];
			}
		}
	}else{
		ret = 0;
	}

	return (uint8)ret;
}
#else
uint8 spi_memctrl_read_from_cache(uint8 *buf, uint32 size, uint16 offset, uint8 plane )
{
	uint32 ret;
	uint8 use_qdma;
	uint8 *ptr;
	uint32 i;

	PP_UNUSED_ARG(ptr);

	//LOG_DEBUG("memc offset: %d, size: %d\n", offset, size);
#if VPU_AVAILABLE
	if(((uint32)buf & 0xf) || (size & 0xf) || (offset & 0xff))use_qdma = 0;
	else use_qdma = 1;

#else
	use_qdma = 0;
#endif

	offset |= (plane<<12);

	PPDRV_SPI_GPIOCSEnable(3, 0);

	if(use_qdma){
		cache_inv_range((uint32 *)(buf), size);
		spi_memctrl_read_qdma(offset, (uint8 *)((uint32)buf), size);
		//cache_inv_range((uint32 *)(buf), size);

	}else{
		//ptr = (uint8 *)ADDR_NON_CACHEABLE((uint32)(SPI_MEM_BASE_ADDR + offset));
		//memcpy((void *)(buf), ptr, size);
		spi_memctrl_read(offset, buf, size);
	}

	if(gCheckErase){
		if((size&3) == 0){
			ret = 0xffffffffu;
			for(i=0;i<size;i+=4){
				ret &= *(uint32 *)(buf + i);
			}
			ret = ( ((ret>>24)&0xff) & ((ret>>16)&0xff) & ((ret>>8)&0xff) & ((ret>>0)&0xff) );
		}else{
			ret = 0xffu;
			for(i=0;i<size;i++){
				ret &= buf[i];
			}
		}
	}else{
		ret = 0;
	}

	PPDRV_SPI_GPIOCSEnable(3, 1);


	return (uint8)ret;
}

#endif

#include <stdio.h>
#include "spi.h"
#include "spi_memctrl.h"

static SPI_MEM_REG_T *gpSPIMem = SPI_MEM_CTRL_REG;
static uint8 g_qdma_buf_idx = 0;

#if 0
static void spi_memctrl_qdma_wait_done(uint32 idx_flag)
{
	while(gpSPIMem->buf_sel & (idx_flag<<8));
}
#endif


void spi_memctrl_set(uint32 div, uint32 rx_delay, uint8 read_cmd, SPI_MEM_FR_INFO fr_info)
{
	uint32 *ptr;

	gpSPIMem->divider = div;
	gpSPIMem->rx_delay = rx_delay;
	gpSPIMem->fr_cmd = read_cmd;
	gpSPIMem->buf_sel = 0;
	ptr = (uint32 *)&fr_info;
	gpSPIMem->fr_info = *ptr;

	g_qdma_buf_idx = 0;
	//memcpy(&gpSPIMem->fr_info, &fr_info, 4);

	//printf("[SPI Mem Ctrl Registers]\n");
	//print_hexw((uint32 *)gpSPIMem, 0x24);
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

	if(mode == SPI_MEMC_MEMORY_MAP_MODE){
		if(bulk_size > 256 || bulk_size < 16)return 1;

		if(bulk_size){
			gpSPIMem->buf_start |= (1<<1);
		}else{
			gpSPIMem->buf_start &= (~(1<<1));
		}
		gpSPIMem->dst_size[0] = bulk_size;
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

int spi_memctrl_read_qdma(uint32 flash_addr, uint8 *buf, uint32 size)
{
	int ret;
	//uint8 buf_idx = 0;
	uint32 read_size = 0;
	uint32 buf_size;

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


#include <stdio.h>
#include <string.h>
#include "sdcard.h"
#include "./fat/fat.h"
#include "./fat/partition.h"
#include "type.h"
#include "bootloader.h"
#include "cache.h"
#include "debug.h"

#define BLOCK_SIZE			512


/* static data buffer for acceleration */
static unsigned char raw_block[512];
/* offset where the data within raw_block lies on the card */
static offset_t raw_block_address = -1;
/* flag to remember if raw_block was written to the card */
//static unsigned char raw_block_written;


typedef uint8_t (*sdh_read_interval_handler_t)(uint8_t* buffer, offset_t offset, void* p);


uint8_t sd_raw_read(offset_t offset, unsigned char * buffer, uintptr_t length)
{
    offset_t block_address;
    uint16_t block_offset;
    uint16_t read_length;

    while(length > 0)
    {
        /* determine byte count to read at once */
        block_offset = offset & 0x01ff;
        block_address = offset - block_offset;
        read_length = BLOCK_SIZE - block_offset; /* read up to block border */
        if(read_length > length)
            read_length = length;

        /* check if the requested data is cached */
        if(block_address != raw_block_address)
        {
			//ReadSectorOffset(block_address, raw_block);
        	//sdh_read(block_address/BLOCK_SIZE, (unsigned int *)raw_block, BLOCK_SIZE);
			spi_sdio_read_single_block(block_address/BLOCK_SIZE, raw_block);
			raw_block_address = block_address;
			memcpy(buffer, raw_block + block_offset, read_length);
			buffer += read_length;


        }
		else
        {
            /* use cached data */
            memcpy(buffer, raw_block + block_offset, read_length);
            buffer += read_length;
        }

        length -= read_length;
        offset += read_length;
    }

    return 1;

}

uint8_t sd_raw_read_interval(offset_t offset, uint8_t* buffer, uintptr_t interval, uintptr_t length, sdh_read_interval_handler_t callback, void* p)
{
    if(!buffer || interval == 0 || length < interval || !callback)
        return 0;

    while(length >= interval)
    {
        /* as reading is now buffered, we directly
         * hand over the request to sd_raw_read()
         */
        if(!sd_raw_read(offset, buffer, interval))
            return 0;
        if(!callback(buffer, offset, p))
            break;
        offset += interval;
        length -= interval;
    }

    return 1;

}

uint32 sdcard_boot(void)
{
	struct partition_struct* partition = 0;
	struct fat_fs_struct* fs = 0;
	struct fat_dir_entry_struct directory;
	struct fat_file_struct *fd = 0;
	uint32 jump_addr = (uint32)-1;
	uint32 fw_size;
	sint32 seek_offset;
	uint32 *pMem = (uint32 *)ISP_FW_LOAD_ADDR;

	spi_sdio_init(PCLK/4, 0);

	dbg("partition open\n");
	partition = partition_open( sd_raw_read, sd_raw_read_interval, 0/*i_o_write*/, 0/*i_o_write_interval*/, 0);

	if(!partition){
		// the storage device is a "superfloppy", i.e. has no MBR.
		partition = partition_open( sd_raw_read, sd_raw_read_interval, 0,	0, -1);
		if(!partition){
			//dbg("partition open fail\n");
			// ERROR!!!
			goto END_FT;
		}
	}
	
	if(!(fs = fat_open(partition))){
		//dbg("fat open fail\n");
		goto END_FT;
	}

	dbg("isp fw loading\n");
	if(fat_get_dir_entry_of_path(fs, "/isp_firm.bin", &directory) != 0){
		if((fd = fat_open_file(fs, &directory))){

			pMem = (uint32 *)ISP_FW_LOAD_ADDR;

			seek_offset = 0;
			fat_seek_file(fd, (int32_t*)&seek_offset, FAT_SEEK_END );
			fw_size = seek_offset;
			seek_offset = 0;
			fat_seek_file(fd, (int32_t *)&seek_offset, FAT_SEEK_SET);
			
			fat_read_file(fd, (uint8 *)pMem, fw_size);
			cache_wb_range(pMem, fw_size);

			fat_close_file(fd);


			// ISP core reset vector 0x20000000
			*(volatile unsigned int *) SUB_CPU_RESET_VECTOR_ADDR = 0x2000;
			
			// ISP core reset disable
			*(volatile unsigned int *) SUB_CPU_RESET_ADDR = 0x00000001;

		}
	}

	jump_addr = MAIN_FW_LOAD_ADDR;
	//dbg("default jump addr: 0x%x\n", jump_addr);
	
	pMem = (uint32 *)jump_addr;	

	//dbg("fat open file\n");

	dbg("main fw loading\n");
	if(fat_get_dir_entry_of_path(fs, "/main_firm.bin", &directory) != 0){
		if(!(fd = fat_open_file(fs, &directory))){
			jump_addr = (uint32)-1;
			//dbg("fat open file fail\n");
			goto END_FT;
		}
	}else{
		jump_addr = (uint32)-1;
		//dbg("fat open file fail\n");
		goto END_FT;
	}
	
	seek_offset = 0;
	fat_seek_file(fd, (int32_t *)&seek_offset, FAT_SEEK_END );
	fw_size = seek_offset;
	seek_offset = 0;
	fat_seek_file(fd, (int32_t *)&seek_offset, FAT_SEEK_SET);

	fat_read_file(fd, (uint8 *)pMem, fw_size);
	cache_wb_range(pMem, fw_size);

	dbg("fat read file end. jump addr: 0x%x\n", jump_addr);
END_FT:
	if(fd){
		fat_close_file(fd);
		fd = 0;
	}
	if(fs)fat_close(fs);
	if(partition)partition_close(partition);

	return jump_addr;
}

void test_ft(void)
{

}

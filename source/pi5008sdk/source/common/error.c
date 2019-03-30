/*
 * =====================================================================================
 *
 *       Filename:  error.c
 *
 *    Description:  
 *
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>

#include "osal.h"
#include "error.h"
#include "api_flash.h"

#undef DUMP_WRITE_TO_FLASH


STATIC PP_S32 PPAPI_SYSTEM_DumpRegs (PP_VOID)
{
	PP_S32 ret = 0;

    PP_CHAR *pcWritePos = NULL, *pcBufferBase = NULL;
	PP_U32 bufSize = 0;
	PP_U32 length = 0;

    PP_S32 i, j;
	PP_U32 addr = 0;
	PP_U32 count = 0;
	PP_U32 data = 0;

    typedef struct ppstDumpReg_S
    {
        uint32 u32StartAddr;
        uint32 u32EndAddr;
    } PP_stDumpReg_S;

    CONST PP_stDumpReg_S stDumpRegs[] = 
    {
        {0xF0800000, 0xF0800020},
        {0xF0F00000, 0xF0F00478}, //VIN
    };

#ifdef DUMP_WRITE_TO_FLASH
    {
        LOG_DEBUG("current flash info:\n");
        LOG_DEBUG("   flash_type:(%d)\n", gstFlashHeader.u32FlashType);
        LOG_DEBUG("   page_size:(%d)\n", gstFlashHeader.u32PageSize);
        LOG_DEBUG("   pages_block:(%d)\n", gstFlashHeader.u32PagesBlock);
        LOG_DEBUG("   audio addr:0x%08x\n", gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr);
        LOG_DEBUG("   audio size:0x%08x\n", gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32Size);
    }
#endif // DUMP_WRITE_TO_FLASH

    for(i = 0; i < (sizeof(stDumpRegs)/sizeof(PP_stDumpReg_S)); i++)
    {
        LOG_DEBUG("i:%d, start addr:0x%08x, end addr:0x%08x\n", i, stDumpRegs[i].u32StartAddr, stDumpRegs[i].u32EndAddr);

        addr = stDumpRegs[i].u32StartAddr;
        if( !(addr & 0xF0000000) )
        {
            LOG_CRITICAL("Don't support address:0x%08X\n", addr);
            return(ret);
        }

        bufSize += ( (((stDumpRegs[i].u32EndAddr-stDumpRegs[i].u32StartAddr)/sizeof(uint32))+1)*15 );
    }
    LOG_DEBUG("total buf size:%d\n", bufSize);

    //////////////////////////////////////////////////////////////
    if( (pcBufferBase = (char *)OSAL_malloc(bufSize)) == NULL)
    {
        LOG_CRITICAL("Error! malloc fail\n");
        return(-1);
    }
    pcWritePos = pcBufferBase;

    //test flash write
#ifdef DUMP_WRITE_TO_FLASH
    {/*{{{*/
    	PPAPI_FLASH_Read((PP_U8 *)pcBufferBase, gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr, 1024);
        LOG_DEBUG("--flash read--\n");
        LOG_DEBUG("0x%08x\n", (uint32)*(PP_U32 *)pcBufferBase);
    }/*}}}*/
#endif

    *pcWritePos = 0x00;
	length = 0;

    for(i = 0; i < (sizeof(stDumpRegs)/sizeof(PP_stDumpReg_S)); i++)
    {
        addr = stDumpRegs[i].u32StartAddr;
        count = (stDumpRegs[i].u32EndAddr - stDumpRegs[i].u32StartAddr)/sizeof(uint32) + 1;
        //LOG_DEBUG("i:%d, addr:0x%08x, size:%d\n", i, addr, count);

        {
            sprintf(pcWritePos, "0x%08X: ", addr); length += strlen( pcWritePos ); pcWritePos = pcBufferBase + length;
            for(j = 0; j < count; j++)
            {
                data = GetRegValue(addr);
                sprintf(pcWritePos, "0x%08X ", data); length += strlen( pcWritePos ); pcWritePos = pcBufferBase + length;
                addr += 0x4;
                if( (addr & 0xF) == 0 )
                {
                    sprintf(pcWritePos, "\r\n0x%08X: ", addr); length += strlen( pcWritePos ); pcWritePos = pcBufferBase + length;
                }

                if(length >= bufSize)
                {
                    LOG_DEBUG("Error. overflow buffer\n"); break;
                }
            }
            if(length >= bufSize)
            {
                LOG_DEBUG("Error. overflow buffer\n"); break;
            }
            sprintf(pcWritePos, "\n"); length += strlen( pcWritePos ); pcWritePos = pcBufferBase + length;
        }
    }
    //identify
    printf("%s\n", pcBufferBase);
    //TODO LOG_DEBUG is unstable.
    //LOG_DEBUG("%s\n", pcBufferBase);

    //////////////////////////////////////////////////////////////

    OSAL_free(pcBufferBase);

    //test flash write
#ifdef DUMP_WRITE_TO_FLASH
    {/*{{{*/
        LOG_DEBUG("--flash erase:0x%08x, size:%d--\n", gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr, 4096);
        PPAPI_FLASH_Erase(gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr, 4096);
        LOG_DEBUG("--flash write:0x%08x, size:%d--\n", gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr, length);
        PPAPI_FLASH_Write((PP_U8 *)pcBufferBase, gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr, length);

        if( (pcBufferBase = (char *)OSAL_malloc(1024)) == NULL)
        {
            LOG_DEBUG("Error! malloc fail\n");
            return(-1);
        }
        memset(pcBufferBase, 0, 1024);
        PPAPI_FLASH_Read((PP_U8 *)pcBufferBase, gstFlashHeader.stSect[eFLASH_SECT_AUDIO].u32FlashAddr, 1024);
        LOG_DEBUG("--flash read--\n");
        LOG_DEBUG("%s\n", pcBufferBase);
        OSAL_free(pcBufferBase);
    }/*}}}*/
#endif


	return(ret);
}

PP_VOID PPAPI_SYSTEM_Fault (PP_VOID)
{
    PPAPI_SYSTEM_DumpRegs();
}


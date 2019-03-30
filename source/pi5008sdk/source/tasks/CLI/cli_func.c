#ifdef SUPPORT_DEBUG_CLI
#include <type.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "CLI/cli.h"
#include "CLI/cli_func.h"
#include "CLI/cli_uart.h"

#include "osal.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "pi5008_config.h"
#include "task.h"
#include "task_manager.h"
#include "interrupt.h"

#include "board_config.h"

#include "proc.h"
#include "error.h"
#include "sub_intr.h"
#include "adc.h"

#include "api_diag.h"
#include "api_vpu.h"

#include "du_drv.h"
#include "api_svm.h"

#include "vin.h"
#include "api_vin.h"
#include "vin_user_config.h"
#include "cannyedge_drv.h"
#include "pinmux.h"
#include "gpio.h"

/********************************
 * PI5008 PVI
 ********************************/
#include "pvirx.h"
#include "api_pvirx_func.h"
#include "pvitx.h"
#include "api_pvitx_func.h"

#include "task_fwdn.h"
#include "timer.h"
#include "wdt.h"

#include "api_FAT_FTL.h"
#include "sys_api.h"
#include "spi_memctrl.h"
#include "spi.h"
#include "spi_nand_flash.h"

#include "ObjectDetector.h"
#include "dram_config.h"
#include "api_display.h"
#include "app_calibration.h"
#include "api_calibration.h"
#include "api_ipc.h"
#include "api_svm.h"
#include "application.h"
#include "mbox.h"

#define I2CH_M0 0
#define I2CH_M1 1

extern _gstDramReserved gstDramReserved;

static unsigned int pixbase = 0;
static PP_U32 u32CannyEdgeInterruptCnt = 0;

static void _pvitx_mode_func(const enum _pvi_tx_table_type_format pvitx_standard, const enum _pvi_tx_table_resol_format pvitx_resol, const int pattern);
static void _vidinsel_set(const uint8_t vidCh, const uint8_t pathCh, const uint8_t inCh);

extern _VPUConfig *gpVPUConfig;

////////////////////////////////////////////////////////////////////////////////////////////////

inline static unsigned long simple_strtoul(const char *cp, unsigned int base)
{
	unsigned long result = 0,value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8; 
			cp++;
			if ((toupper((int)*cp) == 'X') && isxdigit((int)cp[1])) {
				cp++;
				base = 16;
			}    
		}    
	} else if (base == 16) {
		if (cp[0] == '0' && toupper((int)cp[1]) == 'X') 
			cp += 2;
	}    
	while (isxdigit((int)*cp) &&
			(value = isdigit((int)*cp) ? *cp-'0' : toupper((int)*cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}    
	return result;
}

static uint32 check_pattern(uint32 *pMem, uint32 size, uint32 initval)
{
	uint32 i;
	uint32 ret = (uint32)(-1);

	for(i=0;i<size/4;i++){
		if(*(uint32 *)(pMem+i) != (initval + i*4)){
			ret = i;
			break;
		}
	}

	return ret;
}
static void make_pattern(uint32 *pMem, uint32 size, uint32 initval)
{
	uint32 i;

	for(i=0;i<size/4;i++){
		*(uint32 *)(pMem+i) = (initval + i*4);
	}

}


////////////////////////////////////////////////////////////////////////////////////////////////
extern void vTaskDelay( const TickType_t xTicksToDelay );
void _mdelay(int ms) 
{
	vTaskDelay((ms * configTICK_RATE_HZ)/1000);
	return;
}

int PVIRX_TEST_Hdelay(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-hdelay:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0111;
			regMask = 0x1F;
			u8Temp = (startValue>>8)&0x1F;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0113;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0111;
			regMask = 0x1F;
			u8Temp = (startValue>>8)&0x1F;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0113;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Hactive(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-hactive:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0112;
			regMask = 0x1F;
			u8Temp = (startValue>>8)&0x1F;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0114;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0112;
			regMask = 0x1F;
			u8Temp = (startValue>>8)&0x1F;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0114;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Vdelay(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-vdelay:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0111;
			regMask = 0xE0;
			u8Temp = ((startValue>>8)&0x07)<<5;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0115;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0111;
			regMask = 0xE0;
			u8Temp = ((startValue>>8)&0x07)<<5;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0115;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Vactive(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-vactive:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0112;
			regMask = 0xE0;
			u8Temp = ((startValue>>8)&0x07)<<5;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0116;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0112;
			regMask = 0xE0;
			u8Temp = ((startValue>>8)&0x07)<<5;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);
			regAddr = 0x0116;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Contrast(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-contrast:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0120;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0120;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Brightness(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-brightness:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0121;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0121;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Saturation(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-saturation:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0122;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0122;
			u8Temp = (startValue)&0xFF;
			PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_Hue(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	unsigned char startValue = 0;
	unsigned char endValue = 0;

	unsigned int regAddr = 0;

	startValue = (dataStart/incStep)*incStep;
	endValue = (dataEnd/incStep)*incStep;
	LOG_DEBUG("%d-hue:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	do
	{
		LOG_DEBUG("%d ", startValue);
		regAddr = 0x0123;
		u8Temp = (startValue)&0xFF;
		PPDRV_PVIRX_Write(0, chanAddr, regAddr, u8Temp);

		startValue += incStep;
		_mdelay(delayMsec);
	} while(startValue != endValue);
	LOG_DEBUG("\n");

	return(ret);
}

int PVIRX_TEST_Sharpness(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-sharpness:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0139;
			regMask = 0x0F;
			u8Temp = (startValue)&regMask;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x0139;
			regMask = 0x0F;
			u8Temp = (startValue)&regMask;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

int PVIRX_TEST_HPFCorning(uint8_t chanAddr, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	LOG_DEBUG("%d-HPFCorning\n", chanAddr);

	{/*{{{*/
		LOG_DEBUG("on\n");
		regAddr = 0x013A;
		regMask = 0x80;
		u8Temp = 0x80;
		PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);

	}/*}}}*/
	_mdelay(delayMsec);
	{/*{{{*/
		LOG_DEBUG("off\n");
		regAddr = 0x013A;
		regMask = 0x80;
		u8Temp = 0x00;
		PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);

	}/*}}}*/

	return(ret);
}

int PVIRX_TEST_DemodulationLPF(uint8_t chanAddr, int dataStart, int dataEnd, uint8_t incStep, uint32_t delayMsec)
{
	int ret = 0;

	unsigned char u8Temp = 0;
	int startValue = 0;
	int endValue = 0;

	unsigned int regAddr = 0;
	unsigned char regMask = 0;

	startValue = dataStart;
	endValue = dataEnd;
	LOG_DEBUG("%d-demodulationLPF:%d(%d~%d)\n", chanAddr, startValue, dataStart, dataEnd);

	if(dataStart <= dataEnd) //increase
	{
		while(startValue <= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x013E;
			regMask = 0x0F;
			u8Temp = (startValue)&regMask;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);

			startValue += incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}
	else //decrease
	{
		while(startValue >= endValue)
		{/*{{{*/
			LOG_DEBUG("%d ", startValue);
			regAddr = 0x013E;
			regMask = 0x0F;
			u8Temp = (startValue)&regMask;
			PPDRV_PVIRX_WriteMaskBit(0, chanAddr, regAddr, regMask, u8Temp);

			startValue -= incStep;
			_mdelay(delayMsec);
		}/*}}}*/
		LOG_DEBUG("\n");
	}

	return(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int _test(int argc, const char **argv)
{
	int ret = 0;

	int i;
	char *strArgv = NULL;

	LOG_DEBUG("\n");

	LOG_DEBUG("test argc:%d\n", argc);

	for (i = 0; i < argc; ++i)
	{   
		strArgv = (char *)argv[i];
		LOG_DEBUG("argc:%d-%s\n", i, strArgv);
    }

        // test frame rate..
    {
        //PP_U32 pu32FrameCnt[10];
        PP_U32 pu32FrameCnt[IRQ_VSYNC_DU+1];
        const char *strVsyncName[IRQ_VSYNC_DU+1] = { 
            "QUAD",
            "VIN0",
            "VIN1",
            "VIN2",
            "VIN3",
            "VIN4",
            "SVM",
            "DU",
        };

        while(1)
        {
            PPAPI_DIAG_GetFrameRate(1, pu32FrameCnt);
            for(i = IRQ_VSYNC_QUAD; i <= IRQ_VSYNC_DU; i++)
            {
                if(pu32FrameCnt[i] != 30)
                {
                    printf("VSYNC_%s:%d\n", strVsyncName[i], pu32FrameCnt[i]);
                }
            }
            printf(".\n");
        }
    }
    return(ret);
}

int _pireg(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;

	int i;
	unsigned int addr = 0;
	unsigned int offset = 0;
	unsigned int shift = 0;
	unsigned int count = 0;
	unsigned int data = 0;

	if( (argc <= 0) || (argv == NULL) )
	{
		return(-1);
	}

	if(argc < 3)
	{
		LOG_DEBUG("current pixbase:0x%08X\n", pixbase);
		return(ret);
	}

	strArgv = (char *)argv[1];

	if( !strncmp(strArgv, "r", sizeof("r")) )
	{/*{{{*/
		if(argc == 3) 
		{/*{{{*/
			offset = simple_strtoul(argv[2], 16);

			if(offset & 0xFFF00000)
			{
				pixbase = offset & 0xFFF00000;
				offset &= 0x000FFFFF;
				LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
			}

			count = 1;
		}/*}}}*/
		else if(argc == 4) 
		{/*{{{*/
			offset = simple_strtoul(argv[2], 16);

			if(argv[3][0] == '<')
			{
				strArgv = (char *)&argv[3][1];
				shift = simple_strtoul(strArgv, 10);
				offset <<= shift;
				count = 1;
			}
			else
			{
				if(offset & 0xFFF00000)
				{
					pixbase = offset & 0xFFF00000;
					offset &= 0x000FFFFF;
					LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
				}

				strArgv = (char *)argv[3];
				count = simple_strtoul(strArgv, 10);
			}
		}/*}}}*/
		else if(argc > 4) 
		{/*{{{*/
			offset = simple_strtoul(argv[2], 16);

			if(argv[3][0] == '<')
			{
				strArgv = (char *)&argv[3][1];
				shift = simple_strtoul(strArgv, 10);
				offset <<= shift;
			}
			else 
			{
				strArgv = (char *)&argv[3][0];
				shift = simple_strtoul(strArgv, 10);
				offset <<= shift;
			}

			if(offset & 0xFFF00000)
			{
				pixbase = offset & 0xFFF00000;
				offset &= 0x000FFFFF;
				LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
			}

			strArgv = (char *)argv[4];
			count = simple_strtoul(strArgv, 10);
		}/*}}}*/

		offset &= 0xFFFFFFFC; //32bit address.
		addr = pixbase + offset;
		LOG_DEBUG("read address:0x%08X, count:%d\n", addr, count);
		if( !(addr & 0xF0000000) )
		{
			LOG_DEBUG("Don't support address:0x%08X\n", addr);
			return(ret);
		}

		LOG_DEBUG("0x%08X: ", addr);
		for(i = 0; i < count; i++)
		{
			data = GetRegValue(addr);
			LOG_DEBUG("0x%08X ", data);
			addr += 0x4;
			if( (addr & 0xF) == 0 )
			{
				LOG_DEBUG("\r\n0x%08X: ", addr);
			}
		}
		LOG_DEBUG("\n");

	}/*}}}*/
	if( !strncmp(strArgv, "w", sizeof("w")) )
	{/*{{{*/
		if(argc == 3) //read
		{/*{{{*/
			offset = simple_strtoul(argv[2], 16);

			if(offset & 0xFFF00000)
			{
				pixbase = offset & 0xFFF00000;
				offset &= 0x000FFFFF;
				LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
			}

			count = 1;

			offset &= 0xFFFFFFFC; //32bit address.
			addr = pixbase + offset;
			LOG_DEBUG("read address:0x%08X, count:%d\n", addr, count);
			if( !(addr & 0xF0000000) )
			{
				LOG_DEBUG("Don't support address:0x%08X\n", addr);
				return(ret);
			}

			LOG_DEBUG("0x%08X: ", addr);
			for(i = 0; i < count; i++)
			{
				data = GetRegValue(addr);
				LOG_DEBUG("0x%08X ", data);
				addr += 0x4;
				if( (addr & 0xF) == 0 )
				{
					LOG_DEBUG("\r\n0x%08X: ", addr);
				}
			}
			LOG_DEBUG("\n");
		}/*}}}*/
		else if(argc == 4) //read, write
		{/*{{{*/
			offset = simple_strtoul(argv[2], 16);

			if(argv[3][0] == '<')
			{
				strArgv = (char *)&argv[3][1];
				shift = simple_strtoul(strArgv, 10);
				offset <<= shift;
				count = 1;

				if(offset & 0xFFF00000)
				{
					pixbase = offset & 0xFFF00000;
					offset &= 0x000FFFFF;
					LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
				}

				count = 1;

				offset &= 0xFFFFFFFC; //32bit address.
				addr = pixbase + offset;
				LOG_DEBUG("read address:0x%08X, count:%d\n", addr, count);
				if( !(addr & 0xF0000000) )
				{
					LOG_DEBUG("Don't support address:0x%08X\n", addr);
					return(ret);
				}

				LOG_DEBUG("0x%08X: ", addr);
				for(i = 0; i < count; i++)
				{
					data = GetRegValue(addr);
					LOG_DEBUG("0x%08X ", data);
					addr += 0x4;
					if( (addr & 0xF) == 0 )
					{
						LOG_DEBUG("\r\n0x%08X: ", addr);
					}
				}
				LOG_DEBUG("\n");
			}
			else
			{
				if(offset & 0xFFF00000)
				{
					pixbase = offset & 0xFFF00000;
					offset &= 0x000FFFFF;
					LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
				}

				strArgv = (char *)argv[3];
				data = simple_strtoul(strArgv, 16);

				offset &= 0xFFFFFFFC; //32bit address.
				addr = pixbase + offset;
				LOG_DEBUG("write address:0x%08X, data:0x%08X->0x%08X\n", addr, GetRegValue(addr), data);
				if( !(addr & 0xF0000000) )
				{
					LOG_DEBUG("Don't support address:0x%08X\n", addr);
					return(ret);
				}

				SetRegValue(addr, data);
			}
		}/*}}}*/
		else if(argc > 4) //write
		{/*{{{*/
			offset = simple_strtoul(argv[2], 16);

			if(argv[3][0] == '<')
			{
				strArgv = (char *)&argv[3][1];
				shift = simple_strtoul(strArgv, 10);
				offset <<= shift;
			}
			else 
			{
				strArgv = (char *)&argv[3][0];
				shift = simple_strtoul(strArgv, 10);
				offset <<= shift;
			}

			if(offset & 0xFFF00000)
			{
				pixbase = offset & 0xFFF00000;
				offset &= 0x000FFFFF;
				LOG_DEBUG("set pixbase:0x%08X\n", pixbase);
			}

			strArgv = (char *)argv[4];
			data = simple_strtoul(strArgv, 16);

			offset &= 0xFFFFFFFC; //32bit address.
			addr = pixbase + offset;
			LOG_DEBUG("write address:0x%08X, data:0x%08X->0x%08X\n", addr, GetRegValue(addr), data);
			if( !(addr & 0xF0000000) )
			{
				LOG_DEBUG("Don't support address:0x%08X\n", addr);
				return(ret);
			}

			SetRegValue(addr, data);
		}/*}}}*/
	}/*}}}*/

	return(ret);
}


////////////////////////////////////////////////////////////////////////////////////////////

enum _eCameraStandard gCameraStandard = CVI;
enum _eCameraResolution gCameraResolution = camera_1280x720p25;
enum _eVideoResolution gVideoResolution = video_1280x720p25;
int gTestBitDelayMsec = 0;

enum _ePviRxTestFuncNum {
	_hdelay = 0,
	_hactive,
	_vdelay,
	_vactive,
	_contrast,
	_brightness,
	_saturation,
	_hue,
	_sharpness,
	_HPFcorning,
	_demodulationLPF,
	max_pvirxtestfuncnum
};

const char _strPviRxTestFuncName[max_pvirxtestfuncnum][18] = {
	"hdelay",
	"hactive",
	"vdelay",
	"vactive",
	"contrast",
	"brightness",
	"saturation",
	"hue",
	"sharpness",
	"HPFcorning",
	"demodulationLPF",
};

int _pvirx_test_func(const uint8_t chanAddr, const enum _ePviRxTestFuncNum testFuncNum)
{
	int ret = 0;

	uint8_t startChan = chanAddr;
	enum _ePviRxTestFuncNum startTestFuncNum = testFuncNum;
	//uint8_t vidCh, pathCh, inCh;
	uint8_t u8RegData;
	uint32_t u32CPhaseRef;

	if(chanAddr > 4)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}
	else if(chanAddr == 4)
	{
		startChan = 0;
	}

	if(testFuncNum > max_pvirxtestfuncnum)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}
	else if(testFuncNum == max_pvirxtestfuncnum)
	{
		startTestFuncNum = 0;
	}

	do 
	{
		//Get c phase ref value.
		{
			PPDRV_PVIRX_Read(0, startChan, 0x0146, &u8RegData);
			u32CPhaseRef = u8RegData<<16;
			PPDRV_PVIRX_Read(0, startChan, 0x0147, &u8RegData);
			u32CPhaseRef |= u8RegData<<8;
			PPDRV_PVIRX_Read(0, startChan, 0x0148, &u8RegData);
			u32CPhaseRef |= u8RegData;
		}


		do
		{
			INTC_irq_disable(IRQ_0_VECTOR);
			switch(startTestFuncNum)
			{/*{{{*/
				case _hdelay:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 100;

							_mdelay(1000);
							PVIRX_TEST_Hdelay(startChan, 0, 1024, 16, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Hdelay(startChan, 1024, 0, 16, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _hactive:
					{
						int hResol = 2;
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
						switch(gVideoResolution)
						{/*{{{*/
							case video_720x480i60:
							case video_720x576i50:
								{
									hResol = 720;
								}
								break;
							case video_960x480i60:
							case video_960x576i50:
								{
									hResol = 960;
								}
								break;

							case video_1280x720p60:
							case video_1280x720p50:
							case video_1280x720p30:
							case video_1280x720p25:
							case video_1280x960p30:
							case video_1280x960p25:
								{
									hResol = 1280;
								}
								break;
							case video_1920x1080p30:
							case video_1920x1080p25:
								{
									hResol = 1920;
								}
								break;
							default:
								{
									hResol = 1280;
								}
								break;
						}/*}}}*/

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 10;

							_mdelay(1000);
							PVIRX_TEST_Hactive(startChan, 2, hResol, 4, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Hactive(startChan, hResol, 2, 4, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _vdelay:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 100;

							_mdelay(1000);
							PVIRX_TEST_Vdelay(startChan, 0, 128, 4, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Vdelay(startChan, 128, 0, 4, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _vactive:
					{
						int vResol = 2;
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
						switch(gVideoResolution)
						{/*{{{*/
							case video_720x480i60:
							case video_960x480i60:
								{
									vResol = 240;
								}
								break;
							case video_720x576i50:
							case video_960x576i50:
								{
									vResol = 288;
								}
								break;
							case video_1280x720p60:
							case video_1280x720p50:
							case video_1280x720p30:
							case video_1280x720p25:
								{
									vResol = 720;
								}
								break;
							case video_1280x960p30:
							case video_1280x960p25:
								{
									vResol = 960;
								}
								break;
							case video_1920x1080p30:
							case video_1920x1080p25:
								{
									vResol = 1080;
								}
								break;
							default:
								{
									vResol = 720;
								}
								break;
						}/*}}}*/

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 10;

							_mdelay(1000);
							PVIRX_TEST_Vactive(startChan, 2, vResol, 2, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Vactive(startChan, vResol, 2, 2, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _contrast:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 30;

							_mdelay(1000);
							PVIRX_TEST_Contrast(startChan, 0, 0xFF, 4, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Contrast(startChan, 0xFF, 0, 4, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _brightness:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 30;

							_mdelay(1000);
							PVIRX_TEST_Brightness(startChan, 0, 0xFF, 4, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Brightness(startChan, 0xFF, 0, 4, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _saturation:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 30;

							_mdelay(1000);
							PVIRX_TEST_Saturation(startChan, 0, 0xFF, 4, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_Saturation(startChan, 0xFF, 0, 4, gTestBitDelayMsec);

						}/*}}}*/
					}
					break;
				case _hue:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 30;

							_mdelay(1000);
							PVIRX_TEST_Hue(startChan, 0x80, 0x80, 4, gTestBitDelayMsec);
						}/*}}}*/
					}
					break;
				case _sharpness:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 500;

							_mdelay(1000);
							PVIRX_TEST_Sharpness(startChan, 0xF, 0, 1, gTestBitDelayMsec);
						}/*}}}*/
					}
					break;
				case _HPFcorning:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 500;

							_mdelay(1000);
							PVIRX_TEST_HPFCorning(startChan, gTestBitDelayMsec);
						}/*}}}*/
					}
					break;
				case _demodulationLPF:
					{
						LOG_DEBUG("test ch:%d %s\n", startChan, _strPviRxTestFuncName[startTestFuncNum]);

						{/*{{{*/

#if 0
							gVideoResolution = (enum _eVideoResolution)stDefaultCameraVideoRelation[gCameraResolution].eVideoResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.standard = (enum _eCameraStandard)gCameraStandard;
							gpPviRxDrvHost[startChan]->stPrRxMode.cameraResolution = (enum _eCameraResolution)gCameraResolution;
							gpPviRxDrvHost[startChan]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)gVideoResolution;
							PPAPI_PVIRX_SetInit(startChan);
#endif

							gTestBitDelayMsec = 500;

							_mdelay(1000);
							PVIRX_TEST_DemodulationLPF(startChan, 0, 0xF, 1, gTestBitDelayMsec);

							_mdelay(1000);
							PVIRX_TEST_DemodulationLPF(startChan, 0xF, 0, 1, gTestBitDelayMsec);
						}/*}}}*/
					}
					break;
				default:
					{
						LOG_DEBUG("Unknown test item\n");
						ret = -1;
					}
			}/*}}}*/

			PPAPI_PVIRX_SetInit(startChan);
			//restore c phase ref value.
			{
				PPDRV_PVIRX_Write(0, startChan, 0x0146, (u32CPhaseRef>>16)&0xFF);
				PPDRV_PVIRX_Write(0, startChan, 0x0147, (u32CPhaseRef>>8)&0xFF);
				PPDRV_PVIRX_Write(0, startChan, 0x0148, (u32CPhaseRef)&0xFF);
			}

			INTC_irq_clean(IRQ_0_VECTOR);
			INTC_irq_enable(IRQ_0_VECTOR);
			startTestFuncNum++;
		} while( startTestFuncNum < testFuncNum );

		startChan++;

		if(chanAddr == 4)
		{
#if 0
			vidCh = 0;
			pathCh = 3;
			inCh = startChan;
			_vidinsel_set(vidCh, pathCh, inCh);
#endif
			startTestFuncNum = 0;
		}
	} while( startChan < chanAddr );

#if 0
	// restore channel 0
	if(chanAddr == 4)
	{
		vidCh = 0;
		pathCh = 3;
		inCh = 0;
		_vidinsel_set(vidCh, pathCh, inCh);
	}
#endif

	return(ret);
}



int _pvirx_test(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	uint8_t chanAddr = 0;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	chanAddr = simple_strtoul(strArgv, 10);

	strArgv = (char *)argv[2];
	LOG_DEBUG("argv:%s\n", strArgv);

	if( !strcmp(strArgv, "hdelay") || !strcmp(strArgv, "hd") )
	{
		_pvirx_test_func(chanAddr, _hdelay);
	}
	else if( !strcmp(strArgv, "hactive") || !strcmp(strArgv, "ha") )
	{
		_pvirx_test_func(chanAddr, _hactive);
	}
	else if( !strcmp(strArgv, "vdelay") || !strcmp(strArgv, "vd") )
	{
		_pvirx_test_func(chanAddr, _vdelay);
	}
	else if( !strcmp(strArgv, "vactive") || !strcmp(strArgv, "va") )
	{
		_pvirx_test_func(chanAddr, _vactive);
	}
	else if( !strcmp(strArgv, "contrast") || !strcmp(strArgv, "co") )
	{
		_pvirx_test_func(chanAddr, _contrast);
	}
	else if( !strcmp(strArgv, "brightness") || !strcmp(strArgv, "br") )
	{
		_pvirx_test_func(chanAddr, _brightness);
	}
	else if( !strcmp(strArgv, "saturation") || !strcmp(strArgv, "sa") )
	{
		_pvirx_test_func(chanAddr, _saturation);
	}
	else if( !strcmp(strArgv, "hue") || !strcmp(strArgv, "hu") )
	{
		_pvirx_test_func(chanAddr, _hue);
	}
	else if( !strcmp(strArgv, "sharpness") || !strcmp(strArgv, "sh") )
	{
		_pvirx_test_func(chanAddr, _sharpness);
	}
	else if( !strcmp(strArgv, "HPFcorning") || !strcmp(strArgv, "hp") )
	{
		_pvirx_test_func(chanAddr, _HPFcorning);
	}
	else if( !strcmp(strArgv, "demodulationLPF") || !strcmp(strArgv, "de") )
	{
		_pvirx_test_func(chanAddr, _demodulationLPF);
	}
	else if( !strcmp(strArgv, "all") )
	{
		_pvirx_test_func(chanAddr, max_pvirxtestfuncnum);
	}
	else
	{
		LOG_DEBUG("Unknown test item\n");
		ret = -1;
	}

	return(ret);
}

int _pvirx_mode(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;
	enum _eVideoResolution videoResolution;

	int chn = 0;

	LOG_DEBUG("\n");

	if(argc < 5)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

    //cli> pvirx_mode ch[0~3] standard[0~5] camResol[0~9] videoResol[0~11]
	strArgv = (char *)argv[1];
	chn = (int)simple_strtoul(strArgv, 10);

	strArgv = (char *)argv[2];
	cameraStandard = (enum _eCameraStandard)simple_strtoul(strArgv, 10);

	strArgv = (char *)argv[3];
	cameraResolution = (enum _eCameraResolution)simple_strtoul(strArgv, 10);

	strArgv = (char *)argv[4];
	videoResolution = (enum _eVideoResolution)simple_strtoul(strArgv, 10);

	LOG_DEBUG("set rxmode(chn:%d, standard:%d, cresol:%d, vresol:%d)\n", chn, cameraStandard, cameraResolution, videoResolution);

    gpPviRxDrvHost[chn]->stPrRxMode.standard = (enum _eCameraStandard)cameraStandard;
    gpPviRxDrvHost[chn]->stPrRxMode.cameraResolution = (enum _eCameraResolution)cameraResolution;
    gpPviRxDrvHost[chn]->stPrRxMode.vidOutResolution = (enum _eVideoResolution)videoResolution;
    PPAPI_PVIRX_SetInit(chn);

	return(ret);
}

static int _pvirx_utc_send_func(const uint8_t chanAddr, const uint8_t utcCmd)
{
	int ret = 0;

	enum _eCameraStandard cameraStandard;
	enum _eCameraResolution cameraResolution;

	int dataSize = 0;
	const unsigned char *pData;

	/* utc command table data. */
	const unsigned char pvirx_ptz_table_sd_OSD_UP[(4)] =     {0x00,0x08,0x00,0x00};
	const unsigned char pvirx_ptz_table_sd_OSD_RIGHT[(4)] =  {0x00,0x02,0x00,0x00};
	const unsigned char pvirx_ptz_table_sd_OSD_DOWN[(4)] =   {0x00,0x10,0x00,0x00};
	const unsigned char pvirx_ptz_table_sd_OSD_LEFT[(4)] =   {0x00,0x04,0x00,0x00};
	const unsigned char pvirx_ptz_table_sd_OSD_SELECT[(4)] = {0x02,0x00,0x00,0x00};
	//                                                         ^^^^^^^^^^^^^^^^^^^

	const unsigned char pvirx_ptz_table_pvi_OSD_UP[(7)] =     {0x00,0x00,0x08,/**/0xAA,0x00,0x00,0xB2};
	const unsigned char pvirx_ptz_table_pvi_OSD_RIGHT[(7)] =  {0x00,0x00,0x04,/**/0xAA,0x00,0x00,0xAE};
	const unsigned char pvirx_ptz_table_pvi_OSD_DOWN[(7)] =   {0x00,0x00,0x02,/**/0xAA,0x00,0x00,0xAC};
	const unsigned char pvirx_ptz_table_pvi_OSD_LEFT[(7)] =   {0x00,0x00,0x01,/**/0xAA,0x00,0x00,0xAB};
	const unsigned char pvirx_ptz_table_pvi_OSD_SELECT[(7)] = {0x00,0x00,0x00,/**/0xAA,0x00,0x00,0xAA};
	//                                                          ^^^^^^^^^^^^^^^    ^^^^^^^^^^^^^^^^^^^

	const unsigned char pvirx_ptz_table_cvi_OSD_UP[7] =      {0xA5,0x01,0x89,0x04,0x00,0x00,0x33};
	const unsigned char pvirx_ptz_table_cvi_OSD_RIGHT[7] =   {0xA5,0x01,0x89,0x07,0x00,0x00,0x36};
	const unsigned char pvirx_ptz_table_cvi_OSD_DOWN[7] =    {0xA5,0x01,0x89,0x05,0x00,0x00,0x34};
	const unsigned char pvirx_ptz_table_cvi_OSD_LEFT[7] =    {0xA5,0x01,0x89,0x06,0x00,0x00,0x35};
	//                                                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	const unsigned char pvirx_ptz_table_cvi_OSD_SELECT[21] = {0xA5,0x01,0x50,0x00,0x00,0x00,0xF6,0xA5,0x01,0x40,0x00,0x00,0x00,0xE6,0xA5,0x01,0x40,0x00,0x00,0x00,0xE6}; 
	//                                                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	const unsigned char pvirx_ptz_table_hda_OSD_UP_1080p[(4*6)] =     {0x00,0x00,0x00,0x00,/**/0xAA,0x3C,0xFF,0xFF,/**/0x00,0x08,0x00,0x32,/**/0xAA,0x08,0x00,0xFF,/**/0xAA,0x3C,0x00,0xFF,/**/0xAA,0x3B,0x00,0x00};
	const unsigned char pvirx_ptz_table_hda_OSD_RIGHT_1080p[(4*6)] =  {0x00,0x00,0x00,0x00,/**/0xAA,0x3C,0xFF,0xFF,/**/0x00,0x02,0x32,0x00,/**/0xAA,0x02,0x32,0x00,/**/0xAA,0x3C,0x00,0xFF,/**/0xAA,0x3B,0x00,0x00};
	const unsigned char pvirx_ptz_table_hda_OSD_DOWN_1080p[(4*6)] =   {0x00,0x00,0x00,0x00,/**/0xAA,0x3C,0xFF,0xFF,/**/0x00,0x10,0x00,0x32,/**/0xAA,0x3C,0x00,0x32,/**/0xAA,0x3C,0x00,0xFF,/**/0xAA,0x3B,0x00,0x00};
	const unsigned char pvirx_ptz_table_hda_OSD_LEFT_1080p[(4*6)] =   {0x00,0x00,0x00,0x00,/**/0xAA,0x3C,0xFF,0xFF,/**/0x00,0x04,0x32,0x00,/**/0xAA,0x3C,0x32,0xFF,/**/0xAA,0x3C,0x00,0xFF,/**/0xAA,0x3B,0x00,0x00};
	const unsigned char pvirx_ptz_table_hda_OSD_SELECT_1080p[(4*6)] = {0x00,0x00,0x00,0x00,/**/0xAA,0x3C,0xFF,0xFF,/**/0x02,0x00,0x00,0x00,/**/0xAA,0x3C,0xFF,0xFF,/**/0xAA,0x1B,0x00,0x00,/**/0xAA,0x3B,0x00,0x00};
	//                                                                                                                  ^^^^^^^^^^^^^^^^^^^^

	const unsigned char pvirx_ptz_table_hda_OSD_UP_720p[(4*1)] =     {0x00,0x10,0x10,0x4C};
	const unsigned char pvirx_ptz_table_hda_OSD_DOWN_720p[(4*1)]=    {0x00,0x08,0x08,0x4C};
	const unsigned char pvirx_ptz_table_hda_OSD_LEFT_720p[(4*1)] =   {0x00,0x20,0x20,0x00};
	const unsigned char pvirx_ptz_table_hda_OSD_RIGHT_720p[(4*1)] =  {0x00,0x40,0x40,0x00};
	const unsigned char pvirx_ptz_table_hda_OSD_SELECT_720p[(4*1)] = {0x40,0x00,0x00,0x00};
	//                                                                 ^^^^^^^^^^^^^^^^^^^

	const unsigned char pvirx_ptz_table_hdt_OSD_UP[(4*2)] =    {0xB5,0x00,0x06,0x24,/**/0x00,0x00,0x00,0xDF};
	const unsigned char pvirx_ptz_table_hdt_OSD_RIGHT[(4*2)] = {0xB5,0x00,0x08,0x00,/**/0x24,0x00,0x00,0xE1}; 
	const unsigned char pvirx_ptz_table_hdt_OSD_DOWN[(4*2)] =  {0xB5,0x00,0x07,0x24,/**/0x00,0x00,0x00,0xE0};
	const unsigned char pvirx_ptz_table_hdt_OSD_LEFT[(4*2)] =  {0xB5,0x00,0x09,0x00,/**/0x24,0x00,0x00,0xE2}; 
	const unsigned char pvirx_ptz_table_hdt_OSD_SELECT[(4*2)]= {0xB5,0x00,0x17,0x5F,/**/0x00,0x00,0x00,0x2B}; 
	//const uint8_t pvirx_ptz_table_hdt_OSD_SELECT[(4*2)] =    {0xB5,0x00,0x0F,0x00,/**/0x00,0x00,0x00,0xC4}; //iris_plus
	//                                                           ^^^^^^^^^^^^^^^^^^^     ^^^^^^^^^^^^^^^^^^^

	cameraStandard = (enum _eCameraStandard)gpPviRxDrvHost[chanAddr]->stPrRxMode.standard;
	cameraResolution = (enum _eCameraResolution)gpPviRxDrvHost[chanAddr]->stPrRxMode.cameraResolution;

	LOG_DEBUG("chanAddr:%d, utcCmd:%d\n", chanAddr, utcCmd);

	/* set rx fifo configuration */
	switch(cameraStandard)
	{/*{{{*/
		case CVBS:
			{/*{{{*/
				switch(utcCmd)
				{/*{{{*/
					case 0: /* Select */
						{
							dataSize = sizeof(pvirx_ptz_table_sd_OSD_SELECT)/sizeof(pvirx_ptz_table_sd_OSD_SELECT[0]);
							pData = pvirx_ptz_table_sd_OSD_SELECT;
						}
						break;
					case 1: /* Up */
						{
							dataSize = sizeof(pvirx_ptz_table_sd_OSD_UP)/sizeof(pvirx_ptz_table_sd_OSD_UP[0]);
							pData = pvirx_ptz_table_sd_OSD_UP;
						}
						break;
					case 2: /* Right */
						{
							dataSize = sizeof(pvirx_ptz_table_sd_OSD_RIGHT)/sizeof(pvirx_ptz_table_sd_OSD_RIGHT[0]);
							pData = pvirx_ptz_table_sd_OSD_RIGHT;
						}
						break;
					case 3: /* Down */
						{
							dataSize = sizeof(pvirx_ptz_table_sd_OSD_DOWN)/sizeof(pvirx_ptz_table_sd_OSD_DOWN[0]);
							pData = pvirx_ptz_table_sd_OSD_DOWN;
						}
						break;
					case 4: /* Left */
						{
							dataSize = sizeof(pvirx_ptz_table_sd_OSD_LEFT)/sizeof(pvirx_ptz_table_sd_OSD_LEFT[0]);
							pData = pvirx_ptz_table_sd_OSD_LEFT;
						}
						break;
					default:
						{
							LOG_DEBUG("invalid utc cmd\n");
							ret = -1;
						}
						break;

				}/*}}}*/
			}/*}}}*/
			break;
		case PVI:
			{/*{{{*/
				switch(utcCmd)
				{/*{{{*/
					case 0: /* Select */
						{
							dataSize = sizeof(pvirx_ptz_table_pvi_OSD_SELECT)/sizeof(pvirx_ptz_table_pvi_OSD_SELECT[0]);
							pData = pvirx_ptz_table_pvi_OSD_SELECT;
						}
						break;
					case 1: /* Up */
						{
							dataSize = sizeof(pvirx_ptz_table_pvi_OSD_UP)/sizeof(pvirx_ptz_table_pvi_OSD_UP[0]);
							pData = pvirx_ptz_table_pvi_OSD_UP;
						}
						break;
					case 2: /* Right */
						{
							dataSize = sizeof(pvirx_ptz_table_pvi_OSD_RIGHT)/sizeof(pvirx_ptz_table_pvi_OSD_RIGHT[0]);
							pData = pvirx_ptz_table_pvi_OSD_RIGHT;
						}
						break;
					case 3: /* Down */
						{
							dataSize = sizeof(pvirx_ptz_table_pvi_OSD_DOWN)/sizeof(pvirx_ptz_table_pvi_OSD_DOWN[0]);
							pData = pvirx_ptz_table_pvi_OSD_DOWN;
						}
						break;
					case 4: /* Left */
						{
							dataSize = sizeof(pvirx_ptz_table_pvi_OSD_LEFT)/sizeof(pvirx_ptz_table_pvi_OSD_LEFT[0]);
							pData = pvirx_ptz_table_pvi_OSD_LEFT;
						}
						break;
					default:
						{
							LOG_DEBUG("invalid utc cmd\n");
							ret = -1;
						}
						break;

				}/*}}}*/
			}/*}}}*/
			break;
		case HDA:
			{
				if(cameraResolution >= camera_1920x1080p30)
				{/*{{{*/
					switch(utcCmd)
					{/*{{{*/
						case 0: /* Select */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_SELECT_1080p)/sizeof(pvirx_ptz_table_hda_OSD_SELECT_1080p[0]);
								pData = pvirx_ptz_table_hda_OSD_SELECT_1080p;
							}
							break;
						case 1: /* Up */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_UP_1080p)/sizeof(pvirx_ptz_table_hda_OSD_UP_1080p[0]);
								pData = pvirx_ptz_table_hda_OSD_UP_1080p;
							}
							break;
						case 2: /* Right */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_RIGHT_1080p)/sizeof(pvirx_ptz_table_hda_OSD_RIGHT_1080p[0]);
								pData = pvirx_ptz_table_hda_OSD_RIGHT_1080p;
							}
							break;
						case 3: /* Down */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_DOWN_1080p)/sizeof(pvirx_ptz_table_hda_OSD_DOWN_1080p[0]);
								pData = pvirx_ptz_table_hda_OSD_DOWN_1080p;
							}
							break;
						case 4: /* Left */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_LEFT_1080p)/sizeof(pvirx_ptz_table_hda_OSD_LEFT_1080p[0]);
								pData = pvirx_ptz_table_hda_OSD_LEFT_1080p;
							}
							break;
						default:
							{
								LOG_DEBUG("invalid utc cmd\n");
								ret = -1;
							}
							break;

					}/*}}}*/
				}/*}}}*/
				else
				{/*{{{*/
					switch(utcCmd)
					{/*{{{*/
						case 0: /* Select */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_SELECT_720p)/sizeof(pvirx_ptz_table_hda_OSD_SELECT_720p[0]);
								pData = pvirx_ptz_table_hda_OSD_SELECT_720p;
							}
							break;
						case 1: /* Up */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_UP_720p)/sizeof(pvirx_ptz_table_hda_OSD_UP_720p[0]);
								pData = pvirx_ptz_table_hda_OSD_UP_720p;
							}
							break;
						case 2: /* Right */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_RIGHT_720p)/sizeof(pvirx_ptz_table_hda_OSD_RIGHT_720p[0]);
								pData = pvirx_ptz_table_hda_OSD_RIGHT_720p;
							}
							break;
						case 3: /* Down */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_DOWN_720p)/sizeof(pvirx_ptz_table_hda_OSD_DOWN_720p[0]);
								pData = pvirx_ptz_table_hda_OSD_DOWN_720p;
							}
							break;
						case 4: /* Left */
							{
								dataSize = sizeof(pvirx_ptz_table_hda_OSD_LEFT_720p)/sizeof(pvirx_ptz_table_hda_OSD_LEFT_720p[0]);
								pData = pvirx_ptz_table_hda_OSD_LEFT_720p;
							}
							break;
						default:
							{
								LOG_DEBUG("invalid utc cmd\n");
								ret = -1;
							}
							break;

					}/*}}}*/
				}/*}}}*/
			}
			break;
		case CVI:
			{/*{{{*/
				switch(utcCmd)
				{/*{{{*/
					case 0: /* Select */
						{
							dataSize = sizeof(pvirx_ptz_table_cvi_OSD_SELECT)/sizeof(pvirx_ptz_table_cvi_OSD_SELECT[0]);
							pData = pvirx_ptz_table_cvi_OSD_SELECT;
						}
						break;
					case 1: /* Up */
						{
							dataSize = sizeof(pvirx_ptz_table_cvi_OSD_UP)/sizeof(pvirx_ptz_table_cvi_OSD_UP[0]);
							pData = pvirx_ptz_table_cvi_OSD_UP;
						}
						break;
					case 2: /* Right */
						{
							dataSize = sizeof(pvirx_ptz_table_cvi_OSD_RIGHT)/sizeof(pvirx_ptz_table_cvi_OSD_RIGHT[0]);
							pData = pvirx_ptz_table_cvi_OSD_RIGHT;
						}
						break;
					case 3: /* Down */
						{
							dataSize = sizeof(pvirx_ptz_table_cvi_OSD_DOWN)/sizeof(pvirx_ptz_table_cvi_OSD_DOWN[0]);
							pData = pvirx_ptz_table_cvi_OSD_DOWN;
						}
						break;
					case 4: /* Left */
						{
							dataSize = sizeof(pvirx_ptz_table_cvi_OSD_LEFT)/sizeof(pvirx_ptz_table_cvi_OSD_LEFT[0]);
							pData = pvirx_ptz_table_cvi_OSD_LEFT;
						}
						break;
					default:
						{
							LOG_DEBUG("invalid utc cmd\n");
							ret = -1;
						}
						break;

				}/*}}}*/
			}/*}}}*/
			break;
		case HDT_OLD:
		case HDT_NEW:
			{/*{{{*/
				switch(utcCmd)
				{/*{{{*/
					case 0: /* Select */
						{
							dataSize = sizeof(pvirx_ptz_table_hdt_OSD_SELECT)/sizeof(pvirx_ptz_table_hdt_OSD_SELECT[0]);
							pData = pvirx_ptz_table_hdt_OSD_SELECT;
						}
						break;
					case 1: /* Up */
						{
							dataSize = sizeof(pvirx_ptz_table_hdt_OSD_UP)/sizeof(pvirx_ptz_table_hdt_OSD_UP[0]);
							pData = pvirx_ptz_table_hdt_OSD_UP;
						}
						break;
					case 2: /* Right */
						{
							dataSize = sizeof(pvirx_ptz_table_hdt_OSD_RIGHT)/sizeof(pvirx_ptz_table_hdt_OSD_RIGHT[0]);
							pData = pvirx_ptz_table_hdt_OSD_RIGHT;
						}
						break;
					case 3: /* Down */
						{
							dataSize = sizeof(pvirx_ptz_table_hdt_OSD_DOWN)/sizeof(pvirx_ptz_table_hdt_OSD_DOWN[0]);
							pData = pvirx_ptz_table_hdt_OSD_DOWN;
						}
						break;
					case 4: /* Left */
						{
							dataSize = sizeof(pvirx_ptz_table_hdt_OSD_LEFT)/sizeof(pvirx_ptz_table_hdt_OSD_LEFT[0]);
							pData = pvirx_ptz_table_hdt_OSD_LEFT;
						}
						break;
					default:
						{
							LOG_DEBUG("invalid utc cmd\n");
							ret = -1;
						}
						break;

				}/*}}}*/
			}/*}}}*/
			break;
		default:
			{
				LOG_DEBUG("Invalid camera Standard(%d)\n", cameraStandard);
				ret = -1;
				break;
			}
	}/*}}}*/

	if( (ret = PPAPI_PVIRX_UTC_SendData(chanAddr, cameraStandard, cameraResolution, dataSize, pData)) < 0)
	{
		LOG_DEBUG("invalid utc send data\n");
	}

	return(ret);
}


int _pvirx_utcsend(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	uint8_t chanAddr, utcCmd;

	LOG_DEBUG("\n");

	if(argc < 3)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	chanAddr = simple_strtoul(strArgv, 10) & 0x3;

	strArgv = (char *)argv[2];
	if( !strcmp(strArgv, "S") || !strcmp(strArgv, "s") )
	{
		utcCmd = 0; //sel
	}
	else if( !strcmp(strArgv, "U") || !strcmp(strArgv, "u") )
	{
		utcCmd = 1; //up
	}
	else if( !strcmp(strArgv, "R") || !strcmp(strArgv, "r") )
	{
		utcCmd = 2; //right
	}
	else if( !strcmp(strArgv, "D") || !strcmp(strArgv, "d") )
	{
		utcCmd = 3; //down
	}
	else if( !strcmp(strArgv, "L") || !strcmp(strArgv, "l") )
	{
		utcCmd = 4; //left
	}
	else
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	ret = _pvirx_utc_send_func(chanAddr, utcCmd);

	if(argc == 4)
	{
		int count = 0;

		strArgv = (char *)argv[3];
		count = simple_strtoul(strArgv, 10);

		while(count-- >= 0)
		{
			_mdelay(1000);
			ret = _pvirx_utc_send_func(chanAddr, utcCmd);
		}
	}

	return(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////
static void _pvitx_pattern_func(const int pattern)
{
	uint32_t u32Temp;

	u32Temp = GetRegValue(0xF090184C);

	LOG_DEBUG("pattern sel:0x%x\n", pattern&0xF);

	u32Temp &= ~0xF0;
	u32Temp |= (pattern&0xF)<<4;

	SetRegValue(0xF090184C, u32Temp);

	return;
}

static int TestControlHDPVI_TX_HBLK(const int enable)
{/*{{{*/
	unsigned int addr = 0;
	unsigned int data = 0;
	int i;

	addr = 0x11;
	data = GetRegValue(0xF0901800+((addr)<<2));

	/* disable test hblk */
	if(enable == 0)
	{
		data &= ~0x80;
		SetRegValue(0xF0901800+((addr)<<2), data);
		return(0);
	}
	else
	{
		/* enable test hblk */
		data |= 0x80;
		SetRegValue(0xF0901800+((addr)<<2), data);
	}

	addr = 0x0C;
	data = GetRegValue(0xF0901800+((addr)<<2));

	for(i = 1; i <= 10; i++)
	{
		/* + */
		data &= ~0x10;
		LOG_DEBUG("+%d\n", i);
		data &= ~0x0F;
		data |= i;
		LOG_DEBUG("0x%02x-0x%02x\n", addr, data);
		SetRegValue(0xF0901800+((addr)<<2), data);
		_mdelay(1000);

		/* - */
		data |= 0x10;
		LOG_DEBUG("-%d\n", i);
		data &= ~0x0F;
		data |= i;
		LOG_DEBUG("0x%02x-0x%02x\n", addr, data);
		SetRegValue(0xF0901800+((addr)<<2), data);
		_mdelay(1000);

	}

	/* 0 */
	addr = 0x0C;
	data = GetRegValue(0xF0901800+((addr)<<2));
	data &= ~0x10;

	LOG_DEBUG("+0\n");
	data &= ~0x0F;
	data |= 0x00;
	LOG_DEBUG("0x%02x-0x%02x\n", addr, data);
	SetRegValue(0xF0901800+((addr)<<2), data);

	_mdelay(4000);

	return(0);
}/*}}}*/

static int TestControlHDPVI_TX_BurstLV(const int step)
{/*{{{*/
	unsigned int addr = 0;
	unsigned int data = 0;

	uint16_t u16CurBurstLVNum = 0;
	uint16_t u16StartBurstLVNum = 0, u16EndBurstLVNum = 0;
	uint16_t u16OldData;
	uint16_t u16WrData;
	uint16_t u16WrTemp;
	uint32_t i, j;
	uint8_t typeFormat = gpPviTxDrvHost->pviTxType;
	uint8_t resolFormat = gpPviTxDrvHost->pviTxResol;
	const _SD_PVI_TX_TABLE_T *pSdTbl;
	const _HD_PVI_TX_TABLE_T *pHdTbl;

	addr = 0x65;
	{/*{{{*/
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
		{
			LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
			for(i = 0; i < 2; i++)
			{
				pSdTbl = pvi_tx_table_sd;
				for(j = 0; j <= 0xFF; j++)
				{
					if(pSdTbl[j].addr == (addr+i))
					{
						data = pSdTbl[j].data[resolFormat];
						SetRegValue(0xF0901800+((addr+i)<<2), data);
						break;
					}
				}
			}
		}
		else //HD
		{
			resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

			switch(typeFormat)
			{/*{{{*/
				case pvi_tx_table_format_PVI:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
						pHdTbl = pvi_tx_table_pvi;
					}
					break;
				case pvi_tx_table_format_HDA:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
						pHdTbl = pvi_tx_table_hda;
					}
					break;
				case pvi_tx_table_format_CVI:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
						pHdTbl = pvi_tx_table_cvi;
					}
					break;
				case pvi_tx_table_format_HDT:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
						pHdTbl = pvi_tx_table_hdt;
					}
					break;
				default:
					{
						LOG_DEBUG("Set reg:unknown->pvi\n");
						pHdTbl = pvi_tx_table_pvi;
					}
					break;
			}/*}}}*/
			for(i = 0; i < 2; i++)
			{/*{{{*/
				for(j = 0; j <= 0xFF; j++)
				{
					if(pHdTbl[j].addr == (addr+i))
					{
						data = pHdTbl[j].data[resolFormat];
						SetRegValue(0xF0901800+((addr+i)<<2), data);
						break;
					}
				}
			}/*}}}*/

		}
	}/*}}}*/

	addr = 0x65;
	for(i = 0; i < 2; i++)
	{
		data = GetRegValue(0xF0901800+((addr+i)<<2));
		u16CurBurstLVNum |= data<<(i*8);
	}
	u16OldData = u16CurBurstLVNum & 0xFC00;
	u16CurBurstLVNum &= 0x03FF;

	/* Sequence gain max->min */
	{/*{{{*/
		/* +3DB ~ -24DB */
		u16StartBurstLVNum = (u16CurBurstLVNum + (u16CurBurstLVNum/2) ); //+3DB
		u16EndBurstLVNum = (u16CurBurstLVNum / (16)) ; //-24DB
		LOG_DEBUG("+3DB~-24DB Cur:0x%04x (0x%04x~0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum, u16EndBurstLVNum);

		addr = 0x65;
		for(u16WrData = u16StartBurstLVNum; u16WrData >= (u16EndBurstLVNum); u16WrData-=step)
		{
			u16WrTemp = (u16WrData & 0x03FF) | u16OldData;
			//LOG_DEBUG("--0x%x-0x%04x\n", addr, u16WrTemp);
			LOG_DEBUG(".");
			for(i = 0; i < 2; i++)
			{
				data = (u16WrTemp >> (i*8)) & 0xFF;
				SetRegValue(0xF0901800+((addr+i)<<2), data);
			}
			_mdelay(30);
		}
		LOG_DEBUG("\n");
		_mdelay(1000);
	}/*}}}*/
	/* Sequence gain min->max */
	{/*{{{*/
		/* -24DB ~ +3DB */
		u16StartBurstLVNum = (u16CurBurstLVNum / (16)) ; //-24DB
		u16EndBurstLVNum = (u16CurBurstLVNum + (u16CurBurstLVNum/2) ); //+3DB
		LOG_DEBUG("-24DB~+3DB Cur:0x%04x (0x%04x~0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum, u16EndBurstLVNum);

		addr = 0x65;
		for(u16WrData = u16StartBurstLVNum; u16WrData <= (u16EndBurstLVNum); u16WrData+=step)
		{
			u16WrTemp = (u16WrData & 0x03FF) | u16OldData;
			//LOG_DEBUG("++0x%x-0x%04x\n", addr, u16WrTemp);
			LOG_DEBUG(".");
			for(i = 0; i < 2; i++)
			{
				data = (u16WrTemp >> (i*8)) & 0xFF;
				SetRegValue(0xF0901800+((addr+i)<<2), data);
			}
			_mdelay(30);
		}
		LOG_DEBUG("\n");
		_mdelay(1000);

	}/*}}}*/

	/* Swing gain */
	//change +3, -24, +3, 0
	{/*{{{*/
		/* +3DB */
		u16StartBurstLVNum = (u16CurBurstLVNum + (u16CurBurstLVNum/2) ); //+3DB
		LOG_DEBUG("+3DB Cur:0x%04x (0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum);

		addr = 0x65;
		u16WrTemp = (u16StartBurstLVNum & 0x03FF) | u16OldData;
		LOG_DEBUG("0x%x-0x%04x\n", addr, u16WrTemp);
		for(i = 0; i < 2; i++)
		{
			data = (u16WrTemp >> (i*8)) & 0xFF;
			SetRegValue(0xF0901800+((addr+i)<<2), data);
		}
		_mdelay(1000);

		/* -24DB */
		u16StartBurstLVNum = (u16CurBurstLVNum / (16)) ; //-24DB
		LOG_DEBUG("-24DB Cur:0x%04x (0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum);

		addr = 0x65;
		u16WrTemp = (u16StartBurstLVNum & 0x03FF) | u16OldData;
		LOG_DEBUG("0x%x-0x%04x\n", addr, u16WrTemp);
		for(i = 0; i < 2; i++)
		{
			data = (u16WrTemp >> (i*8)) & 0xFF;
			SetRegValue(0xF0901800+((addr+i)<<2), data);
		}
		_mdelay(1000);

		/* +3DB */
		u16StartBurstLVNum = (u16CurBurstLVNum + (u16CurBurstLVNum/2) ); //+3DB
		LOG_DEBUG("+3DB Cur:0x%04x (0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum);

		addr = 0x65;
		u16WrTemp = (u16StartBurstLVNum & 0x03FF) | u16OldData;
		LOG_DEBUG("0x%x-0x%04x\n", addr, u16WrTemp);
		for(i = 0; i < 2; i++)
		{
			data = (u16WrTemp >> (i*8)) & 0xFF;
			SetRegValue(0xF0901800+((addr+i)<<2), data);
		}
		_mdelay(1000);

		/* 0DB */
		u16StartBurstLVNum = (u16CurBurstLVNum * 1); //0DB
		LOG_DEBUG("0DB Cur:0x%04x (0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum);

		addr = 0x65;
		u16WrTemp = (u16StartBurstLVNum & 0x03FF) | u16OldData;
		LOG_DEBUG("0x%x-0x%04x\n", addr, u16WrTemp);
		for(i = 0; i < 2; i++)
		{
			data = (u16WrTemp >> (i*8)) & 0xFF;
			SetRegValue(0xF0901800+((addr+i)<<2), data);
		}
		_mdelay(1000);
	}/*}}}*/

	/* -30DB */
	u16StartBurstLVNum = (u16CurBurstLVNum / (32)); //-30DB
	LOG_DEBUG("-30DB Cur:0x%04x (0x%04x)\n", u16CurBurstLVNum, u16StartBurstLVNum);

	addr = 0x65;
	u16WrData = u16StartBurstLVNum;
	u16WrTemp = (u16WrData & 0x03FF) | u16OldData;
	LOG_DEBUG("0x%x-0x%04x\n", addr, u16WrTemp);
	for(i = 0; i < 2; i++)
	{
		data = (u16WrTemp >> (i*8)) & 0xFF;
		SetRegValue(0xF0901800+((addr+i)<<2), data);
	}
	_mdelay(1000);

	addr = 0x65;
	{/*{{{*/
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
		{
			LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
			for(i = 0; i < 2; i++)
			{
				pSdTbl = pvi_tx_table_sd;
				for(j = 0; j <= 0xFF; j++)
				{
					if(pSdTbl[j].addr == (addr+i))
					{
						data = pSdTbl[j].data[resolFormat];
						SetRegValue(0xF0901800+((addr+i)<<2), data);
						break;
					}
				}
			}
		}
		else //HD
		{
			resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

			switch(typeFormat)
			{/*{{{*/
				case pvi_tx_table_format_PVI:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
						pHdTbl = pvi_tx_table_pvi;
					}
					break;
				case pvi_tx_table_format_HDA:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
						pHdTbl = pvi_tx_table_hda;
					}
					break;
				case pvi_tx_table_format_CVI:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
						pHdTbl = pvi_tx_table_cvi;
					}
					break;
				case pvi_tx_table_format_HDT:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
						pHdTbl = pvi_tx_table_hdt;
					}
					break;
				default:
					{
						LOG_DEBUG("Set reg:unknown->pvi\n");
						pHdTbl = pvi_tx_table_pvi;
					}
					break;
			}/*}}}*/
			for(i = 0; i < 2; i++)
			{/*{{{*/
				for(j = 0; j <= 0xFF; j++)
				{
					if(pHdTbl[j].addr == (addr+i))
					{
						data = pHdTbl[j].data[resolFormat];
						SetRegValue(0xF0901800+((addr+i)<<2), data);
						break;
					}
				}
			}/*}}}*/

		}
	}/*}}}*/

	_mdelay(1000);

	//SyncGain test C on/off: check monitor image C value.
	//Default -> (TX 0x66 C_OFF 1->0)

	addr = 0x66;
	data = GetRegValue(0xF0901800+((addr)<<2));

	data |= 0x40;
	LOG_DEBUG("C OFF (0x%02x)\n", data);
	SetRegValue(0xF0901800+((addr)<<2), data);
	_mdelay(1000);

	data &= ~(0x40);
	LOG_DEBUG("C ON (0x%02x)\n", data);
	SetRegValue(0xF0901800+((addr)<<2), data);
	_mdelay(1000);

	return(0);
}/*}}}*/

static int TestControlHDPVI_TX_YGAIN(const int delayMsec, const int step)
{/*{{{*/

	unsigned int addr = 0;
	unsigned int data = 0;

	uint16_t u16CurYGain = 0;
	uint16_t u16StartYGain = 0, u16EndYGain = 0;
	uint16_t u16WrData;
	uint32_t i, j;
	uint8_t typeFormat = gpPviTxDrvHost->pviTxType;
	uint8_t resolFormat = gpPviTxDrvHost->pviTxResol;
	int cnt;
	const _SD_PVI_TX_TABLE_T *pSdTbl;
	const _HD_PVI_TX_TABLE_T *pHdTbl;

	{/*{{{*/
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		addr = 0x1C;
		{/*{{{*/
			if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
			{
				LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
				for(i = 0; i < 4; i++)
				{
					pSdTbl = pvi_tx_table_sd;
					for(j = 0; j <= 0xFF; j++)
					{
						if(pSdTbl[j].addr == (addr+i))
						{
							data = pSdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}
			}
			else //HD
			{
				resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

				switch(typeFormat)
				{/*{{{*/
					case pvi_tx_table_format_PVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
					case pvi_tx_table_format_HDA:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
							pHdTbl = pvi_tx_table_hda;
						}
						break;
					case pvi_tx_table_format_CVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
							pHdTbl = pvi_tx_table_cvi;
						}
						break;
					case pvi_tx_table_format_HDT:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
							pHdTbl = pvi_tx_table_hdt;
						}
						break;
					default:
						{
							LOG_DEBUG("Set reg:unknown->pvi\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
				}/*}}}*/
				for(i = 0; i < 4; i++)
				{/*{{{*/
					for(j = 0; j <= 0xFF; j++)
					{
						if(pHdTbl[j].addr == (addr+i))
						{
							data = pHdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}/*}}}*/

			}
		}/*}}}*/

		addr = 0x1C;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurYGain = data<<2;
		addr = 0x1F;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurYGain |= (data&0x3);

		u16CurYGain &= 0x03FF;

		/* decrease */
		/* +6DB ~ -18DB */
		u16StartYGain = ((u16CurYGain*2) > 0x3FF) ? (0x3FF):(u16CurYGain*2); //+6DB
		u16EndYGain = (u16CurYGain / (8)) ; //-18DB
		LOG_DEBUG("+6DB~-18DB Y Cur:0x%04x (0x%04x~0x%04x)\n", u16CurYGain, u16StartYGain, u16EndYGain);

		for(u16WrData = u16StartYGain; u16WrData >= (u16EndYGain); u16WrData-=step)
		{
			//LOG_DEBUG("\t0x%x-0x%04x\n", addr, (u16WrData & 0x03FF));
			LOG_DEBUG(".");

			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x03;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (u16WrData)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}

			_mdelay(delayMsec);
		}
		LOG_DEBUG("\n");

		/* increase */
		/* -18DB ~ +6DB */
		u16StartYGain = (u16CurYGain / (8)) ; //-18DB
		u16EndYGain = ((u16CurYGain*2) > 0x3FF) ? (0x3FF):(u16CurYGain*2); //+6DB
		LOG_DEBUG("-18DB~+6DB Y Cur:0x%04x (0x%04x~0x%04x)\n", u16CurYGain, u16StartYGain, u16EndYGain);

		for(u16WrData = u16StartYGain; u16WrData <= (u16EndYGain); u16WrData+=step)
		{
			//LOG_DEBUG("\t0x%x-0x%04x\n", addr, (u16WrData & 0x03FF));
			LOG_DEBUG(".");

			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x03;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (u16WrData)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}


			_mdelay(delayMsec);
		}
		LOG_DEBUG("\n");
		_mdelay(1000);
	}/*}}}*/

	/* +6DB->-18DB->+6DB->0DB _mdelay(1000) */
	for(cnt = 0; cnt < 3; cnt++)
	{/*{{{*/
		LOG_DEBUG("Set default\n");
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		addr = 0x1C;
		{/*{{{*/
			if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
			{
				LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
				for(i = 0; i < 4; i++)
				{
					pSdTbl = pvi_tx_table_sd;
					for(j = 0; j <= 0xFF; j++)
					{
						if(pSdTbl[j].addr == (addr+i))
						{
							data = pSdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}
			}
			else //HD
			{
				resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

				switch(typeFormat)
				{/*{{{*/
					case pvi_tx_table_format_PVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
					case pvi_tx_table_format_HDA:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
							pHdTbl = pvi_tx_table_hda;
						}
						break;
					case pvi_tx_table_format_CVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
							pHdTbl = pvi_tx_table_cvi;
						}
						break;
					case pvi_tx_table_format_HDT:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
							pHdTbl = pvi_tx_table_hdt;
						}
						break;
					default:
						{
							LOG_DEBUG("Set reg:unknown->pvi\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
				}/*}}}*/
				for(i = 0; i < 4; i++)
				{/*{{{*/
					for(j = 0; j <= 0xFF; j++)
					{
						if(pHdTbl[j].addr == (addr+i))
						{
							data = pHdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}/*}}}*/

			}
		}/*}}}*/
		_mdelay(1000);

		addr = 0x1C;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurYGain = data<<2;
		addr = 0x1F;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurYGain |= (data&0x3);

		u16CurYGain &= 0x03FF;

		/* +6DB */
		u16WrData = ((u16CurYGain*2) > 0x3FF) ? (0x3FF):(u16CurYGain*2); //+6DB
		LOG_DEBUG("+6DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x03;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (u16WrData)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* -18DB */
		u16WrData = (u16CurYGain / (8)) ; //-18DB
		LOG_DEBUG("-18DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x03;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (u16WrData)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* +6DB */
		u16WrData = ((u16CurYGain*2) > 0x3FF) ? (0x3FF):(u16CurYGain*2); //+6DB
		LOG_DEBUG("+6DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x03;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (u16WrData)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* 0DB */
		u16WrData = (u16CurYGain * 1); //0DB
		LOG_DEBUG("0DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x03;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (u16WrData)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);
	}/*}}}*/

	return(0);
}/*}}}*/

static int TestControlHDPVI_TX_CGAIN(const int delayMsec, const int step)
{/*{{{*/
	unsigned int addr = 0;
	unsigned int data = 0;

	uint16_t u16CurCGain[2] = {0, }; //CB,CR Gain
	uint16_t u16StartCGain[2] = {0, }; 
	uint16_t u16EndCGain[2] = {0, };
	uint16_t u16WrData[2];
	uint16_t u16Data;
	uint32_t i, j;
	uint8_t typeFormat = gpPviTxDrvHost->pviTxType;
	uint8_t resolFormat = gpPviTxDrvHost->pviTxResol;
	int cnt;
	const _SD_PVI_TX_TABLE_T *pSdTbl;
	const _HD_PVI_TX_TABLE_T *pHdTbl;

	{/*{{{*/
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		addr = 0x1C;
		{/*{{{*/
			if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
			{
				LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
				for(i = 0; i < 4; i++)
				{
					pSdTbl = pvi_tx_table_sd;
					for(j = 0; j <= 0xFF; j++)
					{
						if(pSdTbl[j].addr == (addr+i))
						{
							data = pSdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}
			}
			else //HD
			{
				resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

				switch(typeFormat)
				{/*{{{*/
					case pvi_tx_table_format_PVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
					case pvi_tx_table_format_HDA:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
							pHdTbl = pvi_tx_table_hda;
						}
						break;
					case pvi_tx_table_format_CVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
							pHdTbl = pvi_tx_table_cvi;
						}
						break;
					case pvi_tx_table_format_HDT:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
							pHdTbl = pvi_tx_table_hdt;
						}
						break;
					default:
						{
							LOG_DEBUG("Set reg:unknown->pvi\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
				}/*}}}*/
				for(i = 0; i < 4; i++)
				{/*{{{*/
					for(j = 0; j <= 0xFF; j++)
					{
						if(pHdTbl[j].addr == (addr+i))
						{
							data = pHdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}/*}}}*/

			}
		}/*}}}*/

		addr = 0x1D; //CB[9:2]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[0] = data<<2;
		addr = 0x1E; //CR[9:2]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[1] = data<<2;
		addr = 0x1F;//CB,CR[1:0]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[0] |= (data>>2)&0x3; //CB
		u16CurCGain[1] |= (data>>4)&0x3; //CR

		u16CurCGain[0] &= 0x03FF;
		u16CurCGain[1] &= 0x03FF;

		/* decrease */
		/* +6DB ~ -18DB */
		u16StartCGain[0] = ((u16CurCGain[0]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[0]*2); //+6DB
		u16StartCGain[1] = ((u16CurCGain[1]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[1]*2); //+6DB
		u16EndCGain[0] = (u16CurCGain[0] / (8)) ; //-18DB
		u16EndCGain[1] = (u16CurCGain[1] / (8)) ; //-18DB
		LOG_DEBUG("+6DB~-18DB CB Cur:0x%04x (0x%04x~0x%04x)\n", u16CurCGain[0], u16StartCGain[0], u16EndCGain[0]);
		LOG_DEBUG("+6DB~-18DB CR Cur:0x%04x (0x%04x~0x%04x)\n", u16CurCGain[1], u16StartCGain[1], u16EndCGain[1]);

		for(u16WrData[0] = u16StartCGain[0], u16WrData[1] = u16StartCGain[1]; u16WrData[0] >= (u16EndCGain[0]); u16WrData[0]-=step, u16WrData[1]-=step)
		{
			LOG_DEBUG(".");
			addr = 0x1D;
			data = (u16WrData[0]>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);
			addr = 0x1E;
			data = (u16WrData[1]>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x3C;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (((u16WrData[1]&0x3)<<4)|((u16WrData[0]&0x3)<<2))&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}

			_mdelay(delayMsec);
		}
		LOG_DEBUG("\n");
		_mdelay(1000);

		/* increase */
		/* -18DB ~ -12DB */
		u16StartCGain[0] = (u16CurCGain[0] / (8)) ; //-18DB
		u16StartCGain[1] = (u16CurCGain[1] / (8)) ; //-18DB
		u16EndCGain[0] = ((u16CurCGain[0]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[0]*2); //+6DB
		u16EndCGain[1] = ((u16CurCGain[1]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[1]*2); //+6DB
		LOG_DEBUG("-18DB~+6DB CB Cur:0x%04x (0x%04x~0x%04x)\n", u16CurCGain[0], u16StartCGain[0], u16EndCGain[0]);
		LOG_DEBUG("-18DB~+6DB CR Cur:0x%04x (0x%04x~0x%04x)\n", u16CurCGain[1], u16StartCGain[1], u16EndCGain[1]);

		for(u16WrData[0] = u16StartCGain[0], u16WrData[1] = u16StartCGain[1]; u16WrData[0] <= (u16EndCGain[0]); u16WrData[0]+=step, u16WrData[1]+=step)
		{
			LOG_DEBUG(".");
			addr = 0x1D;
			data = (u16WrData[0]>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);
			addr = 0x1E;
			data = (u16WrData[1]>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x3C;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= (((u16WrData[1]&0x3)<<4)|((u16WrData[0]&0x3)<<2))&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}

			_mdelay(delayMsec);
		}
		LOG_DEBUG("\n");
		_mdelay(1000);

	}/*}}}*/

	/* +6DB->-18DB->+6DB->0DB _mdelay(1000) */
	for(cnt = 0; cnt < 3; cnt++)
	{/*{{{*/
		LOG_DEBUG("Set default\n");
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		addr = 0x1C;
		{/*{{{*/
			if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
			{
				LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
				for(i = 0; i < 4; i++)
				{
					pSdTbl = pvi_tx_table_sd;
					for(j = 0; j <= 0xFF; j++)
					{
						if(pSdTbl[j].addr == (addr+i))
						{
							data = pSdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}
			}
			else //HD
			{
				resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

				switch(typeFormat)
				{/*{{{*/
					case pvi_tx_table_format_PVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
					case pvi_tx_table_format_HDA:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
							pHdTbl = pvi_tx_table_hda;
						}
						break;
					case pvi_tx_table_format_CVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
							pHdTbl = pvi_tx_table_cvi;
						}
						break;
					case pvi_tx_table_format_HDT:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
							pHdTbl = pvi_tx_table_hdt;
						}
						break;
					default:
						{
							LOG_DEBUG("Set reg:unknown->pvi\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
				}/*}}}*/
				for(i = 0; i < 4; i++)
				{/*{{{*/
					for(j = 0; j <= 0xFF; j++)
					{
						if(pHdTbl[j].addr == (addr+i))
						{
							data = pHdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}/*}}}*/

			}
		}/*}}}*/
		_mdelay(1000);

		addr = 0x1D; //CB[9:2]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[0] = data<<2;
		addr = 0x1E; //CR[9:2]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[1] = data<<2;
		addr = 0x1F;//CB,CR[1:0]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[0] |= (data>>2)&0x3; //CB
		u16CurCGain[1] |= (data>>4)&0x3; //CR

		u16CurCGain[0] &= 0x03FF;
		u16CurCGain[1] &= 0x03FF;

		/* +6DB */
		u16Data = ((u16CurCGain[0]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[0]*2);
		LOG_DEBUG("+6DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16Data);
		{
			addr = 0x1D;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x0C;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<2)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		u16Data = ((u16CurCGain[1]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[1]*2);
		LOG_DEBUG("+6DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16Data);
		{
			addr = 0x1E;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* -18DB */
		u16Data = (u16CurCGain[0] / (8)) ;
		LOG_DEBUG("-18DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16Data);
		{
			addr = 0x1D;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x0C;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<2)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		u16Data = (u16CurCGain[1] / (8)) ;
		LOG_DEBUG("-18DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16Data);
		{
			addr = 0x1E;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* +6DB */
		u16Data = ((u16CurCGain[0]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[0]*2);
		LOG_DEBUG("+6DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16Data);
		{
			addr = 0x1D;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x0C;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<2)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		u16Data = ((u16CurCGain[1]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[1]*2);
		LOG_DEBUG("+6DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16Data);
		{
			addr = 0x1E;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* 0DB */
		u16Data = (u16CurCGain[0] * 1);
		LOG_DEBUG("0DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16Data);
		{
			addr = 0x1D;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x0C;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<2)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		u16Data = (u16CurCGain[1] * 1);
		LOG_DEBUG("0DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16Data);
		{
			addr = 0x1E;
			data = (u16Data>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16Data&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);
	}/*}}}*/

	return(0);
}/*}}}*/

static int TestControlHDPVI_TX_YCGAIN(const int delayMsec, const int step)
{/*{{{*/
	unsigned int addr = 0;
	unsigned int data = 0;

	uint16_t u16CurYGain = 0;
	uint16_t u16CurCGain[2] = {0, }; //CB,CR Gain
	uint8_t u8OldData;
	uint16_t u16WrData;
	uint32_t i, j;
	uint8_t typeFormat = gpPviTxDrvHost->pviTxType;
	uint8_t resolFormat = gpPviTxDrvHost->pviTxResol;
	int cnt;
	const _SD_PVI_TX_TABLE_T *pSdTbl;
	const _HD_PVI_TX_TABLE_T *pHdTbl;

	/* +6DB->-18DB->+6DB->0DB _mdelay(1000) */
	for(cnt = 0; cnt < 3; cnt++)
	{/*{{{*/
		LOG_DEBUG("Set default\n");
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		addr = 0x1C;
		{/*{{{*/
			if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
			{
				LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
				for(i = 0; i < 4; i++)
				{
					pSdTbl = pvi_tx_table_sd;
					for(j = 0; j <= 0xFF; j++)
					{
						if(pSdTbl[j].addr == (addr+i))
						{
							data = pSdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}
			}
			else //HD
			{
				resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

				switch(typeFormat)
				{/*{{{*/
					case pvi_tx_table_format_PVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
					case pvi_tx_table_format_HDA:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
							pHdTbl = pvi_tx_table_hda;
						}
						break;
					case pvi_tx_table_format_CVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
							pHdTbl = pvi_tx_table_cvi;
						}
						break;
					case pvi_tx_table_format_HDT:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
							pHdTbl = pvi_tx_table_hdt;
						}
						break;
					default:
						{
							LOG_DEBUG("Set reg:unknown->pvi\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
				}/*}}}*/
				for(i = 0; i < 4; i++)
				{/*{{{*/
					for(j = 0; j <= 0xFF; j++)
					{
						if(pHdTbl[j].addr == (addr+i))
						{
							data = pHdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}/*}}}*/

			}
		}/*}}}*/
		_mdelay(1000);

		addr = 0x1C;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurYGain = data<<2;
		addr = 0x1F;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurYGain |= (data&0x3);

		u16CurYGain &= 0x03FF;

		addr = 0x1D; //CB[9:2]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[0] = data<<2;
		addr = 0x1E; //CR[9:2]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[1] = data<<2;
		addr = 0x1F;//CB,CR[1:0]
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurCGain[0] |= (data>>2)&0x3; //CB
		u16CurCGain[1] |= (data>>4)&0x3; //CR

		u16CurCGain[0] &= 0x03FF;
		u16CurCGain[1] &= 0x03FF;

		/* +6DB */
		u16WrData = ((u16CurYGain*2) > 0x3FF) ? (0x3FF):(u16CurYGain*2);
		LOG_DEBUG("+6DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData = u16WrData&0x3;
		}

		u16WrData = ((u16CurCGain[0]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[0]*2);
		LOG_DEBUG("+6DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16WrData);
		{
			addr = 0x1D;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData |= ((u16WrData&0x3)<<2);

		}
		u16WrData = ((u16CurCGain[1]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[1]*2);
		LOG_DEBUG("+6DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16WrData);
		{
			addr = 0x1E;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16WrData&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* -18DB */
		u16WrData = (u16CurYGain / (8)) ; //-18DB
		LOG_DEBUG("-18DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData = u16WrData&0x3;
		}

		u16WrData = (u16CurCGain[0] / (8)) ;
		LOG_DEBUG("-18DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16WrData);
		{
			addr = 0x1D;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData |= ((u16WrData&0x3)<<2);

		}
		u16WrData = (u16CurCGain[1] / (8)) ;
		LOG_DEBUG("-18DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16WrData);
		{
			addr = 0x1E;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16WrData&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* +6DB */
		u16WrData = ((u16CurYGain*2) > 0x3FF) ? (0x3FF):(u16CurYGain*2);
		LOG_DEBUG("+6DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData = u16WrData&0x3;
		}

		u16WrData = ((u16CurCGain[0]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[0]*2);
		LOG_DEBUG("+6DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16WrData);
		{
			addr = 0x1D;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData |= ((u16WrData&0x3)<<2);

		}
		u16WrData = ((u16CurCGain[1]*2) > 0x3FF) ? (0x3FF):(u16CurCGain[1]*2);
		LOG_DEBUG("+6DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16WrData);
		{
			addr = 0x1E;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16WrData&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

		/* 0DB */
		u16WrData = (u16CurYGain * 1);
		LOG_DEBUG("0DB Y Cur:0x%04x (0x%04x)\n", u16CurYGain, u16WrData);
		{
			addr = 0x1C;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData = u16WrData&0x3;
		}

		u16WrData = (u16CurCGain[0] * 1);
		LOG_DEBUG("0DB CB Cur:0x%04x (0x%04x)\n", u16CurCGain[0], u16WrData);
		{
			addr = 0x1D;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			u8OldData |= ((u16WrData&0x3)<<2);

		}
		u16WrData = (u16CurCGain[0] * 1);
		LOG_DEBUG("0DB CR Cur:0x%04x (0x%04x)\n", u16CurCGain[1], u16WrData);
		{
			addr = 0x1E;
			data = (u16WrData>>2) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x1F;
			{
				unsigned char mask = 0x30;
				unsigned char data;
				data = GetRegValue(0xF0901800+((addr)<<2));
				data &= ~mask;
				data |= ((u16WrData&0x3)<<4)&mask;
				SetRegValue(0xF0901800+((addr)<<2), data);
			}
		}
		_mdelay(1000);

	}/*}}}*/

	return(0);
}/*}}}*/

static int TestControlHDPVI_TX_YCGAIN_ONOFF(const int delayMsec, const int step)
{/*{{{*/
	unsigned int addr = 0;
	unsigned int data = 0;

	uint32_t i, j;
	uint8_t typeFormat = gpPviTxDrvHost->pviTxType;
	uint8_t resolFormat = gpPviTxDrvHost->pviTxResol;
	int cnt;
	int testTotalMsec = 0;
	const int testMsecArray[6] = {2000, 1000, 500, 250, 125, 62};
	int testMsec;
	const _SD_PVI_TX_TABLE_T *pSdTbl;
	const _HD_PVI_TX_TABLE_T *pHdTbl;

	for(cnt = 0; cnt < 6; cnt++)
	{
		testMsec = testMsecArray[cnt];
		LOG_DEBUG("\tTEST On<->Off %d msec period.(%d/5)\n", testMsec, cnt);
		for(testTotalMsec = 0; testTotalMsec < 8000; testTotalMsec += testMsec*2)
		{/*{{{*/
			LOG_DEBUG("Set default. (on)\n");
			typeFormat = gpPviTxDrvHost->pviTxType;
			resolFormat = gpPviTxDrvHost->pviTxResol;
			addr = 0x1C;
			{/*{{{*/
				if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
				{
					LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
					for(i = 0; i < 4; i++)
					{
						pSdTbl = pvi_tx_table_sd;
						for(j = 0; j <= 0xFF; j++)
						{
							if(pSdTbl[j].addr == (addr+i))
							{
								data = pSdTbl[j].data[resolFormat];
								SetRegValue(0xF0901800+((addr+i)<<2), data);
								break;
							}
						}
					}
				}
				else //HD
				{
					resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

					switch(typeFormat)
					{/*{{{*/
						case pvi_tx_table_format_PVI:
							{
								LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
								pHdTbl = pvi_tx_table_pvi;
							}
							break;
						case pvi_tx_table_format_HDA:
							{
								LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
								pHdTbl = pvi_tx_table_hda;
							}
							break;
						case pvi_tx_table_format_CVI:
							{
								LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
								pHdTbl = pvi_tx_table_cvi;
							}
							break;
						case pvi_tx_table_format_HDT:
							{
								LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
								pHdTbl = pvi_tx_table_hdt;
							}
							break;
						default:
							{
								LOG_DEBUG("Set reg:unknown->pvi\n");
								pHdTbl = pvi_tx_table_pvi;
							}
							break;
					}/*}}}*/
					for(i = 0; i < 4; i++)
					{/*{{{*/
						for(j = 0; j <= 0xFF; j++)
						{
							if(pHdTbl[j].addr == (addr+i))
							{
								data = pHdTbl[j].data[resolFormat];
								SetRegValue(0xF0901800+((addr+i)<<2), data);
								break;
							}
						}
					}/*}}}*/

				}
			}/*}}}*/

			_mdelay(testMsec);

			/* off */
			LOG_DEBUG("Off\n");
			addr = 0x1C;
			data = 0;
			for(i = 0; i < 4; i++)
			{
				SetRegValue(0xF0901800+((addr+i)<<2), data);
			}

			_mdelay(testMsec);
		}/*}}}*/
	}

	LOG_DEBUG("End Set default.\n");
	typeFormat = gpPviTxDrvHost->pviTxType;
	resolFormat = gpPviTxDrvHost->pviTxResol;
	addr = 0x1C;
	{/*{{{*/
		if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
		{
			LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
			for(i = 0; i < 4; i++)
			{
				pSdTbl = pvi_tx_table_sd;
				for(j = 0; j <= 0xFF; j++)
				{
					if(pSdTbl[j].addr == (addr+i))
					{
						data = pSdTbl[j].data[resolFormat];
						SetRegValue(0xF0901800+((addr+i)<<2), data);
						break;
					}
				}
			}
		}
		else //HD
		{
			resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

			switch(typeFormat)
			{/*{{{*/
				case pvi_tx_table_format_PVI:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
						pHdTbl = pvi_tx_table_pvi;
					}
					break;
				case pvi_tx_table_format_HDA:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
						pHdTbl = pvi_tx_table_hda;
					}
					break;
				case pvi_tx_table_format_CVI:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
						pHdTbl = pvi_tx_table_cvi;
					}
					break;
				case pvi_tx_table_format_HDT:
					{
						LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
						pHdTbl = pvi_tx_table_hdt;
					}
					break;
				default:
					{
						LOG_DEBUG("Set reg:unknown->pvi\n");
						pHdTbl = pvi_tx_table_pvi;
					}
					break;
			}/*}}}*/
			for(i = 0; i < 4; i++)
			{/*{{{*/
				for(j = 0; j <= 0xFF; j++)
				{
					if(pHdTbl[j].addr == (addr+i))
					{
						data = pHdTbl[j].data[resolFormat];
						SetRegValue(0xF0901800+((addr+i)<<2), data);
						break;
					}
				}
			}/*}}}*/

		}
	}/*}}}*/


	return(0);
}/*}}}*/

static int TestControlHDPVI_TX_GAIN_SYNC(const int delayMsec, const int step)
{/*{{{*/
	unsigned int addr = 0;
	unsigned int data = 0;

	uint16_t u16CurGainSync = 0;
	uint8_t u8OldData;
	uint16_t u16WrData;
	uint32_t i, j;
	uint8_t typeFormat = gpPviTxDrvHost->pviTxType;
	uint8_t resolFormat = gpPviTxDrvHost->pviTxResol;
	int cnt;
	const _SD_PVI_TX_TABLE_T *pSdTbl;
	const _HD_PVI_TX_TABLE_T *pHdTbl;

	//0x60,0x61 Sync_LV +25%,-25%,0%. _mdelay(1000)
	for(cnt = 0; cnt < 3; cnt++)
	{/*{{{*/
		LOG_DEBUG("Set default\n");
		typeFormat = gpPviTxDrvHost->pviTxType;
		resolFormat = gpPviTxDrvHost->pviTxResol;
		addr = 0x60;
		{/*{{{*/
			if(resolFormat <= pvi_tx_table_format_960x576i50) //CVBS
			{
				LOG_DEBUG("Set reg:hdpvi_tx_SD\n");
				for(i = 0; i < 2; i++)
				{
					pSdTbl = pvi_tx_table_sd;
					for(j = 0; j <= 0xFF; j++)
					{
						if(pSdTbl[j].addr == (addr+i))
						{
							data = pSdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}
			}
			else //HD
			{
				resolFormat = resolFormat - pvi_tx_table_format_1280x720p60; //HD

				switch(typeFormat)
				{/*{{{*/
					case pvi_tx_table_format_PVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_PVI\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
					case pvi_tx_table_format_HDA:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDA\n");
							pHdTbl = pvi_tx_table_hda;
						}
						break;
					case pvi_tx_table_format_CVI:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_CVI\n");
							pHdTbl = pvi_tx_table_cvi;
						}
						break;
					case pvi_tx_table_format_HDT:
						{
							LOG_DEBUG("Set reg:pvi_tx_table_format_HDT\n");
							pHdTbl = pvi_tx_table_hdt;
						}
						break;
					default:
						{
							LOG_DEBUG("Set reg:unknown->pvi\n");
							pHdTbl = pvi_tx_table_pvi;
						}
						break;
				}/*}}}*/
				for(i = 0; i < 2; i++)
				{/*{{{*/
					for(j = 0; j <= 0xFF; j++)
					{
						if(pHdTbl[j].addr == (addr+i))
						{
							data = pHdTbl[j].data[resolFormat];
							SetRegValue(0xF0901800+((addr+i)<<2), data);
							break;
						}
					}
				}/*}}}*/

			}
		}/*}}}*/
		_mdelay(1000);

		addr = 0x60;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurGainSync = data;
		addr = 0x61;
		data = GetRegValue(0xF0901800+((addr)<<2));
		u16CurGainSync |= (data&0x3)<<8;

		u8OldData = data&0xFC;
		u16CurGainSync &= 0x03FF;

		/* +25% */
		u16WrData = (u16CurGainSync*125)/100;
		LOG_DEBUG("+25%% Cur:0x%04x (0x%04x)\n", u16CurGainSync, u16WrData);
		{
			addr = 0x60;
			data = (u16WrData) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x61;
			data = ((u16WrData>>8)&0x3)|u8OldData;
			SetRegValue(0xF0901800+((addr)<<2), data);
		}
		_mdelay(1000);

		/* -25% */
		u16WrData = (u16CurGainSync*75)/100;
		LOG_DEBUG("-25%% Cur:0x%04x (0x%04x)\n", u16CurGainSync, u16WrData);
		{
			addr = 0x60;
			data = (u16WrData) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x61;
			data = ((u16WrData>>8)&0x3)|u8OldData;
			SetRegValue(0xF0901800+((addr)<<2), data);
		}
		_mdelay(1000);

		/* 0% */
		u16WrData = (u16CurGainSync*100)/100;
		LOG_DEBUG("0%% Cur:0x%04x (0x%04x)\n", u16CurGainSync, u16WrData);
		{
			addr = 0x60;
			data = (u16WrData) & 0xFF;
			SetRegValue(0xF0901800+((addr)<<2), data);

			addr = 0x61;
			data = ((u16WrData>>8)&0x3)|u8OldData;
			SetRegValue(0xF0901800+((addr)<<2), data);
		}
		_mdelay(1000);

	}/*}}}*/

	return(0);
}/*}}}*/

static int TestHDAVsyncChnage(void)
{/*{{{*/
	int ret = 0;

	//uint8_t bkTxType = gpPviTxDrvHost->pviTxType;
	//uint8_t bkTxResol = gpPviTxDrvHost->pviTxResol;
	uint8_t baseTxType = gpPviTxDrvHost->pviTxType;
	uint8_t baseTxResol = gpPviTxDrvHost->pviTxResol;
	uint8_t testTxType = gpPviTxDrvHost->pviTxType;
	uint8_t testTxResol = gpPviTxDrvHost->pviTxResol;
	uint8_t patternType = 14; //default pattern
	int waitTimeOutMsec = 6000;

	/* start resol */
	testTxType = pvi_tx_table_format_HDA;								
	testTxResol = pvi_tx_table_format_1280x720p60;

	for(testTxResol = pvi_tx_table_format_1280x720p60 ; testTxResol<pvi_tx_table_format_1920x1080p25; testTxResol++)
	{/*{{{*/
		if(testTxType>=pvi_tx_table_format_1280x960p30 && testTxType<=pvi_tx_table_format_1280x960p25)
		{
			continue;
		}

		PPAPI_PVITX_Set(testTxType, testTxResol);
		_pvitx_pattern_func(patternType);

		if(testTxType<pvi_tx_table_format_1920x1080p30)
		{
			/* Change V Sync */
			SetRegValue(0xF0901800+((0x19)<<2), 0xF4);
			SetRegValue(0xF0901800+((0x1A)<<2), 0x00);
			SetRegValue(0xF0901800+((0x1B)<<2), 0x00);
			SetRegValue(0xF0901800+((0x1C)<<2), 0xA3);
			SetRegValue(0xF0901800+((0x1D)<<2), 0x8B);
			SetRegValue(0xF0901800+((0x1E)<<2), 0xC4);
			SetRegValue(0xF0901800+((0x1F)<<2), 0x3E);
			SetRegValue(0xF0901800+((0x20)<<2), 0x33);
			SetRegValue(0xF0901800+((0x21)<<2), 0x33);
			SetRegValue(0xF0901800+((0x22)<<2), 0x44);
			SetRegValue(0xF0901800+((0x23)<<2), 0x55);
			SetRegValue(0xF0901800+((0x24)<<2), 0x11);
			SetRegValue(0xF0901800+((0x25)<<2), 0x11);
			SetRegValue(0xF0901800+((0x26)<<2), 0x33);
			SetRegValue(0xF0901800+((0x27)<<2), 0x33);
			SetRegValue(0xF0901800+((0x28)<<2), 0x33);
			SetRegValue(0xF0901800+((0x29)<<2), 0x00);
		}
		else //FULLHD
		{
			/* Change V Sync */
			SetRegValue(0xF0901800+((0x25)<<2), 0x11);
			SetRegValue(0xF0901800+((0x27)<<2), 0x55);

		}

		LOG_DEBUG("\tStart Test V Sync Changed\n");

		LOG_DEBUG("\tTest: %d/%d [type:%s resol:%s]\n\n", testTxResol, pvi_tx_table_format_1920x1080p25,
				_STR_PVI_TX_TYPE_FORMAT[testTxType], _STR_PVI_TX_RESOL_FORMAT[testTxResol]);

		_mdelay(waitTimeOutMsec);
		/* reset */

		PPAPI_PVITX_Set(baseTxType, baseTxResol);

		_pvitx_pattern_func(patternType);

		LOG_DEBUG("\tReset Test V Sync Original\n");

		LOG_DEBUG("\tTest: %d/%d [type:%s resol:%s]\n\n", baseTxResol, pvi_tx_table_format_1920x1080p25,
				_STR_PVI_TX_TYPE_FORMAT[baseTxType], _STR_PVI_TX_RESOL_FORMAT[baseTxResol]);

		_mdelay(waitTimeOutMsec);
	}/*}}}*/

	return(ret);
}/*}}}*/

enum _ePviTxTestFuncNum {
	_agcgain_minmax = 0,
	_agcgain_onoff,
	_agcgain_sync,
	_chromagain_tracking,
	_hpll_hollingrange,
	_hda_vsync_change,
	max_pvitxtestfuncnum
};

const char _strPviTxTestFuncName[max_pvitxtestfuncnum][20] = {
	"agcgain_minmax",
	"agcgain_onoff",
	"agcgain_sync",
	"chromagain_tracking",
	"hpll_hollingrange",
	"hda_vsync_change",
};

int _pvitx_test_func(const enum _ePviTxTestFuncNum testFuncNum)
{
	int ret = 0;

	enum _ePviTxTestFuncNum startTestFuncNum = testFuncNum;

	if(testFuncNum > max_pvitxtestfuncnum)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}
	else if(testFuncNum == max_pvitxtestfuncnum)
	{
		startTestFuncNum = 0;
	}

	do
	{
		switch(startTestFuncNum)
		{/*{{{*/
			case _agcgain_minmax:
				{
					LOG_DEBUG("%s\n", _strPviTxTestFuncName[startTestFuncNum]);

					/* testygain 50 4 */
					if( TestControlHDPVI_TX_YGAIN(50, 4) < 0)
					{
						break;
					}
					/* testcgain 50 4 */
					if( TestControlHDPVI_TX_CGAIN(50, 4) < 0)
					{
						break;
					}
					/* testycgain 50 4 */
					if( TestControlHDPVI_TX_YCGAIN(50, 4) < 0)
					{
						break;
					}
				}
				break;
			case _agcgain_onoff:
				{
					LOG_DEBUG("%s\n", _strPviTxTestFuncName[startTestFuncNum]);

					if( TestControlHDPVI_TX_YCGAIN_ONOFF(50, 4) < 0)
					{
						break;
					}
				}
				break;
			case _agcgain_sync:
				{
					LOG_DEBUG("%s\n", _strPviTxTestFuncName[startTestFuncNum]);

					/* testgainsync 50 4 */
					if( TestControlHDPVI_TX_GAIN_SYNC(50, 4) < 0)
					{
						break;
					}
				}
				break;
			case _chromagain_tracking:
				{
					LOG_DEBUG("%s\n", _strPviTxTestFuncName[startTestFuncNum]);

					/* testblv 10 */
					if( TestControlHDPVI_TX_BurstLV(2) < 0)
					{
						break;
					}
				}
				break;
			case _hpll_hollingrange:
				{
					LOG_DEBUG("%s\n", _strPviTxTestFuncName[startTestFuncNum]);

					/* testhblk on f0 */
					/* pattern on */
					_pvitx_pattern_func(15);
					/* test hblk margin */
					if( TestControlHDPVI_TX_HBLK(1)  < 0)
					{
						break;
					}
				}
				break;
			case _hda_vsync_change:
				{
					LOG_DEBUG("%s\n", _strPviTxTestFuncName[startTestFuncNum]);

					if( TestHDAVsyncChnage()  < 0)
					{
						break;
					}
				}
				break;
			default:
				{
					LOG_DEBUG("Unknown test item\n");
					ret = -1;
				}
		}/*}}}*/
		PPAPI_PVITX_Set(gpPviTxDrvHost->pviTxType, gpPviTxDrvHost->pviTxResol);

		startTestFuncNum++;
	} while( startTestFuncNum < testFuncNum );


	return(ret);
}

int _pvitx_test(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	LOG_DEBUG("argv:%s\n", strArgv);

    if( !strcmp(strArgv, "agcgain_minmax") || !strcmp(strArgv, "am") )
	{
		_pvitx_test_func(_agcgain_minmax);
	}
	else if( !strcmp(strArgv, "agcgain_onoff") || !strcmp(strArgv, "ao") )
	{
		_pvitx_test_func(_agcgain_onoff);
	}
	else if( !strcmp(strArgv, "agcgain_sync") || !strcmp(strArgv, "as") )
	{
		_pvitx_test_func(_agcgain_sync);
	}
	else if( !strcmp(strArgv, "chromagain_tracking") || !strcmp(strArgv, "ct") )
	{
		_pvitx_test_func(_chromagain_tracking);
	}
	else if( !strcmp(strArgv, "hpll_hollingrange") || !strcmp(strArgv, "hh") )
	{
		_pvitx_test_func(_hpll_hollingrange);
	}
	else if( !strcmp(strArgv, "hda_vsync_change") || !strcmp(strArgv, "hv") )
	{
		_pvitx_test_func(_hda_vsync_change);
	}
    else if( !strcmp(strArgv, "all") )
    {
		_pvitx_test_func(max_pvitxtestfuncnum);
    }
	else
	{
		LOG_DEBUG("Unknown test item\n");
		ret = -1;
	}

	return(ret);
}

static void _pvitx_mode_func(const enum _pvi_tx_table_type_format pvitx_standard, const enum _pvi_tx_table_resol_format pvitx_resol, const int pattern)
{
	uint32_t u32Temp;

	PPAPI_PVITX_Set(pvitx_standard, pvitx_resol);

	if(pattern >= 0)
	{
		u32Temp = GetRegValue(0xF090184C);

		LOG_DEBUG("pattern sel:0x%x\n", pattern&0xF);

		u32Temp &= ~0xF0;
		u32Temp |= (pattern&0xF)<<4;

		SetRegValue(0xF090184C, u32Temp);
	}

	return;
}

int _pvitx_mode(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	enum _pvi_tx_table_type_format pvitx_standard;
	enum _pvi_tx_table_resol_format pvitx_resol;
	int pattern = -1;

	LOG_DEBUG("\n");

	if(argc < 3)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	pvitx_standard = (enum _pvi_tx_table_type_format)simple_strtoul(strArgv, 10);

	strArgv = (char *)argv[2];
	pvitx_resol = (enum _pvi_tx_table_resol_format)simple_strtoul(strArgv, 10);

	if(argc == 4)
	{
		strArgv = (char *)argv[3];
		pattern = simple_strtoul(strArgv, 10);
	}

	LOG_DEBUG("set txmode(standard:%d, resol:%d, pattern:0x%x)\n", pvitx_standard, pvitx_resol, pattern);

	_pvitx_mode_func(pvitx_standard, pvitx_resol, pattern);

	return(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////
static void _vidinsel_set(const uint8_t vidCh, const uint8_t pathCh, const uint8_t inCh)
{
	uint32_t u32Temp;

	u32Temp = GetRegValue(0xF0F0000C);

	LOG_DEBUG("vidInSel, vid:%d, path:%d, in:%d\n", vidCh, pathCh, inCh);

	u32Temp &= ~(0xF<<(vidCh*4));
	u32Temp |= ((pathCh<<2)|(inCh))<<(vidCh*4);

	SetRegValue(0xF0F0000C, u32Temp);
}

int _vidinsel(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	uint8_t vidCh, pathCh, inCh;

	LOG_DEBUG("\n");

	if(argc < 4)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	vidCh = simple_strtoul(strArgv, 10) & 0x3;

	strArgv = (char *)argv[2];
	pathCh = simple_strtoul(strArgv, 10) & 0x3;

	strArgv = (char *)argv[3];
	inCh = simple_strtoul(strArgv, 10) & 0x3;

	_vidinsel_set(vidCh, pathCh, inCh);

	return(ret);
}

static void _svminsel_set(const uint8_t svmCh, const uint8_t pathCh, const uint8_t inCh)
{
	uint32_t u32Temp;

	u32Temp = GetRegValue(0xF0F00014);

	LOG_DEBUG("svmInSel, svm:%d, path:%d, in:%d\n", svmCh, pathCh, inCh);

	u32Temp &= ~(0xF<<(svmCh*4));
	u32Temp |= ((pathCh<<2)|(inCh))<<(svmCh*4);

	SetRegValue(0xF0F00014, u32Temp);
}

int _svminsel(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	uint8_t svmCh, pathCh, inCh;

	LOG_DEBUG("\n");

	if(argc < 4)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	svmCh = simple_strtoul(strArgv, 10) & 0x3;

	strArgv = (char *)argv[2];
	pathCh = simple_strtoul(strArgv, 10) & 0x3;

	strArgv = (char *)argv[3];
	inCh = simple_strtoul(strArgv, 10) & 0x3;

	_svminsel_set(svmCh, pathCh, inCh);

	return(ret);
}

static void _recinsel_set(const uint8_t recCh, const uint8_t pathCh, const uint8_t inCh)
{
	uint32_t u32Temp;

	u32Temp = GetRegValue(0xF0F0001C);

	LOG_DEBUG("recInSel, rec:%d, path:%d, in:%d\n", recCh, pathCh, inCh);

	u32Temp &= ~(0xF<<(recCh*4));
	u32Temp |= ((pathCh<<2)|(inCh))<<(recCh*4);

	SetRegValue(0xF0F0001C, u32Temp);
}

int _recinsel(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	uint8_t recCh, pathCh, inCh;

	LOG_DEBUG("\n");

	if(argc < 4)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	recCh = simple_strtoul(strArgv, 10) & 0x3;

	strArgv = (char *)argv[2];
	pathCh = simple_strtoul(strArgv, 10) & 0x1;

	strArgv = (char *)argv[3];
	inCh = simple_strtoul(strArgv, 10) & 0x3;

	_recinsel_set(recCh, pathCh, inCh);

	return(ret);
}

int _recpath(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	int b8bit = 0;
	int muxCh = 1;

	uint32_t data;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	b8bit = simple_strtoul(strArgv, 10);
	if(b8bit == 8) b8bit = 1;
	else b8bit = 0;

	strArgv = (char *)argv[2];
	muxCh = simple_strtoul(strArgv, 10);
	if( (muxCh != 1) && (muxCh != 2) && (muxCh != 4) )
	{
		LOG_DEBUG("Invalid muxCh(%d)\n", muxCh);
		return(-1);
	}


	LOG_DEBUG("set recpath %dbit, muxCh:%d\n", (b8bit == 1)?8:16, muxCh);

	{
		VIN_REC_FMT_CTRL_CONFIG_U PI_BD_CONFIG_REC_FMT_CTRL_val;
		VIN_REC0_CH_SEL_CONFIG_U PI_BD_CONFIG_REC0_CH_SEL_val;
		VIN_REC1_CH_SEL_CONFIG_U PI_BD_CONFIG_REC1_CH_SEL_val;

		PI_BD_CONFIG_REC_FMT_CTRL_val.var = GetRegValue(0xF0F00180);
		PI_BD_CONFIG_REC0_CH_SEL_val.var = GetRegValue(0xF0F00184);
		PI_BD_CONFIG_REC1_CH_SEL_val.var = GetRegValue(0xF0F00188);

		LOG_DEBUG("Current: Mux%dch, bt%d, %dbit, rate:%d\n",
				(PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md == 0)? 1:
				((PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md == 1)? 2:4),
				(PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_bt656 == 0)? 1120:656,
				(PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_16bit == 0)? 8:16,
				PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate);


		if(b8bit == 1)
		{
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_16bit = 0;
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_bt656 = 1;
		}
		else
		{
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_16bit = 1;
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_bt656 = 0;
		}

		if(muxCh == 1)
		{
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md = 0;

    		LOG_DEBUG("===>Sel input ch0 -> ch0\n");
		    data = GetRegValue(0xF0F0001C);
		    data &= 0xFFFFFFF0;
		    SetRegValue(0xF0F0001C, data);

#if ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P) 
            LOG_DEBUG("===>Sel datarate0\n");
            PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 0;

            if(b8bit == 1)
            {
                LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
            }
            else
            {
                LOG_DEBUG("===>Sel clk_phase(148M(x) / 74M(o)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x30;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x30;
            }
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P) 
            LOG_DEBUG("===>Sel datarate1\n");
            PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 1;

            if(b8bit == 1)
            {
                LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
            }
            else
            {
                LOG_DEBUG("===>Sel clk_phase(148M(x) / 74M(o)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x30;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x30;
            }
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) 
            LOG_DEBUG("===>Sel datarate3\n");
            PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 3;

            if(b8bit == 1)
            {
                LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
            }
            else
            {
                LOG_DEBUG("===>Sel clk_phase(148M(x) / 74M(o)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x30;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x30;
            }
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) 
            LOG_DEBUG("===>Sel datarate2\n");
            PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 2;

            if(b8bit == 1)
            {
                LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
            }
            else
            {
                LOG_DEBUG("===>Sel clk_phase(148M(x) / 74M(o)_37M(x))\n");
                PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x30;
                PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x30;
            }
#else
            LOG_DEBUG("don't support\n");
#endif
		}
		else if(muxCh == 2)
		{
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md = 1;
    		    
    		LOG_DEBUG("===>Sel input ch1,ch0 -> ch0\n");
		    data = GetRegValue(0xF0F0001C);
		    data &= 0xFFFFFF00;
		    SetRegValue(0xF0F0001C, data);

#if ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P) 
    		LOG_DEBUG("===>Sel datarate1\n");
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 1;

    		LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
		    PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
    		PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) 
    		LOG_DEBUG("===>Sel datarate3\n");
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 3;

    		LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
		    PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
    		PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) 
    		LOG_DEBUG("===>Sel datarate2\n");
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 2;

    		LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
		    PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
    		PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
#else
            LOG_DEBUG("don't support\n");
#endif
		}
		else
		{
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md = 2;
    		    
    		LOG_DEBUG("===>Sel input ch3,ch2,ch1,ch0 -> ch0\n");
		    data = GetRegValue(0xF0F0001C);
		    data &= 0xFFFF0000;
		    SetRegValue(0xF0F0001C, data);

#if ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P) 
    		LOG_DEBUG("===>Sel datarate1\n");
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 1;

    		LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
		    PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
    		PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) 
    		LOG_DEBUG("===>Sel datarate3\n");
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 3;

    		LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
		    PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
    		PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
#elif ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) 
    		LOG_DEBUG("===>Sel datarate2\n");
			PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate = 2;

    		LOG_DEBUG("===>Sel clk_phase(148M(o) / 74M(x)_37M(x))\n");
		    PI_BD_CONFIG_REC0_CH_SEL_val.param.rec_clk_phase = 0x20;
    		PI_BD_CONFIG_REC1_CH_SEL_val.param.rec_clk_phase = 0x20;
#else
            LOG_DEBUG("don't support\n");
#endif
		}

		SetRegValue(0xF0F00180, PI_BD_CONFIG_REC_FMT_CTRL_val.var);
		SetRegValue(0xF0F00184, PI_BD_CONFIG_REC0_CH_SEL_val.var);
		SetRegValue(0xF0F00188, PI_BD_CONFIG_REC1_CH_SEL_val.var);

		LOG_DEBUG("===>Set: Mux%dch, bt%d, %dbit, rate:%d\n",
				(PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md == 0)? 1:
				((PI_BD_CONFIG_REC_FMT_CTRL_val.param.ch_mux_md == 1)? 2:4),
				(PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_bt656 == 0)? 1120:656,
				(PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_16bit == 0)? 8:16,
				PI_BD_CONFIG_REC_FMT_CTRL_val.param.outfmt_rate);

		SetRegValue(0xF0100054, 0x02020202);
		LOG_DEBUG("===>Set: Out mode rec FPGA pin\n");

	}

	return(ret);
}

int _setQuadView(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	int ch = 0;
	int chSel = 0;
	int pathSel = 0;

	printf("\n");

	if(argc < 2)
	{
		printf("Invalid usage\n");
		return(-1);
	}

    strArgv = (char *)argv[1];
    ch = simple_strtoul(strArgv, 10);

    // test 1ch full mode setting.
    printf("test.. 1ch full mode setting.\n");
    if(ch <= 3)
    {
        chSel = ch;
        pathSel = 0; //0:input, 1:TP
        if( PPAPI_VIN_SetQuadViewMode(BD_VIN_FMT, FALSE, chSel, pathSel) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        printf("SetQuad : input ch%d full\n", ch);
    }
    else if(ch == 4) //PB
    {
        chSel = 0;  //PB
        pathSel = 1; //0:input, 1:TP or PB
        if( PPAPI_VIN_SetQuadViewMode(BD_VIN_FMT, FALSE, chSel, pathSel) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        printf("SetQuad : PB full\n");
    }
    else //Quad
    {
        chSel = 0;  //TP
        pathSel = 0; //0:input, 1:TP or PB
        if( PPAPI_VIN_SetQuadViewMode(BD_VIN_FMT, TRUE, chSel, pathSel) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        printf("SetQuad : Quad\n");
    }
    return(ret);
}

int _caminsel(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	int camCh[4] = {0, };
	int i;

	LOG_DEBUG("\n");

	if(argc < 4)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	camCh[0] = simple_strtoul(strArgv, 10);
	strArgv = (char *)argv[2];
	camCh[1] = simple_strtoul(strArgv, 10);
	strArgv = (char *)argv[3];
	camCh[2] = simple_strtoul(strArgv, 10);
	strArgv = (char *)argv[4];
	camCh[3] = simple_strtoul(strArgv, 10);

    for(i = 0; i < 4; i++)
    {
        if( camCh[i] > 0 )
        {
            camCh[i] -= 1;
        }
        else
        {
            camCh[i] = -1;
        }
    }

    LOG_DEBUG("Cam:%d, %d, %d, %d\n", camCh[0], camCh[1], camCh[2], camCh[3]);
    if( (camCh[0] > 3) || (camCh[1] > 3) || (camCh[2] > 3) || (camCh[3] > 3) )
    {
        LOG_DEBUG("Error! Invalid ch\n");
        return(-1);
    }

    PPAPI_VIN_SetVIDPort(camCh[0], camCh[1],  camCh[2], camCh[3], -1);
    for(i = 0; i < 4; i++)
    {
        if( camCh[i] < 0 )
        {
            PPAPI_VIN_SetSVMChannel(i, 1, 2); //Test pattern 
        }
        else
        {
            PPAPI_VIN_SetSVMChannel(i, 0, i); //Normal camera 
        }
    }


	return(ret);
}

int _capture(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	int ch = 0;
	int bYOnly = 0;
	int chSel = 0;
	int pathSel = 0;

	uint32 u32BufPAddr;
	uint32 u32RdVAddr;
	uint32 u32BufSize;

	char sdFileName[80];

	LOG_DEBUG("\n");

	if(argc < 4)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}
	strArgv = (char *)argv[1];
	ch = simple_strtoul(strArgv, 10);
	strArgv = (char *)argv[2];
	bYOnly = simple_strtoul(strArgv, 10);
    if(argc == 4)
    {
        strArgv = (char *)argv[3];
        strcpy(sdFileName,  strArgv);
    }

	{/*{{{*/
		u32BufPAddr = (uint32)gpVPUConfig->pBufCapture[0];
		u32RdVAddr = (uint32)gpVPUConfig->pVBufCapture[0];
		LOG_DEBUG("Buf P:0x%08x, V:0x%08x\n", u32BufPAddr, u32RdVAddr);

		LOG_DEBUG("Capture format:UYVY\n");
		if(ch <= 3)
		{
			chSel = ch;
			pathSel = 0; //0:input, 1:TP
            if( PPAPI_VIN_SetCaptureMode(BD_VIN_FMT, chSel, pathSel) != eSUCCESS )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            LOG_DEBUG("SetQuad : input ch%d full\n", ch);

        }
        else if(ch == 4) //PB
        {
            chSel = 0; //PB
            pathSel = 1; //0:input, 1:TP or PB
            if( PPAPI_VIN_SetCaptureMode(BD_VIN_FMT, chSel, pathSel) != eSUCCESS )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            printf("SetQuad : PB full\n");

        }
        else //TP
        {
            chSel = 1; //TP
            pathSel = 1; //0:input, 1:TP or PB
            if( PPAPI_VIN_SetCaptureMode(BD_VIN_FMT, chSel, pathSel) != eSUCCESS )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            printf("SetQuad : TP full\n");

		}
		if( PPAPI_VIN_GetCaptureImage(BD_VIN_FMT, bYOnly, u32BufPAddr, &u32BufSize, QUAD_0CH, PP_TRUE) != eSUCCESS )
        {
            LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
        LOG_DEBUG("pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32BufPAddr, u32BufPAddr+u32BufSize, u32BufSize);
    }/*}}}*/

	//SD card write.
    if(argc == 4)
    {
		void *pfHandle = NULL;
		void *pBuf = (void *)(uint32)gpVPUConfig->pVBufCapture[0];
		uint32 bufSize = u32BufSize;

		LOG_DEBUG("sd write: %s, size:%d\n", sdFileName, bufSize);

		PPAPI_FATFS_DelVolume();
		if(!PPAPI_FATFS_InitVolume()){
			pfHandle = PPAPI_FATFS_Open(sdFileName,"w");
			if( (PPAPI_FATFS_Write(pBuf, 1, bufSize, pfHandle) != bufSize) )
	        {
	            LOG_DEBUG("Error can't write(%s)\n", sdFileName);
	        };
			PPAPI_FATFS_Close(pfHandle);
		}else{
			// error
			LOG_DEBUG("Error can't open(%s)\n", sdFileName);
		}
        LOG_DEBUG("Done write(%s)\n", sdFileName);
	}

	return(ret);
}

int _capscl(int argc, const char **argv)
{
	int ret = 0;

	int ch = 0;

    unsigned int sclWidth, sclHeight;
    unsigned int bufOffset = 960*480*2;
    unsigned char capChBit;

	uint32 u32pBufPAddr[4];
	uint32 u32pRdVAddr[4];
	uint32 u32pBufSize[4];

	char sdFileName[80];

    //sclWidth = 640;
    //sclHeight = 480;
    sclWidth = 320;
    sclHeight = 240;
    capChBit = 0xE; //B:R:L:F

	LOG_DEBUG("\n");

	if(argc < 1)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

    {
            for(ch = 0; ch < 4; ch++)
            {
                u32pBufPAddr[ch] = (uint32)gpVPUConfig->pBufCapture[0] + (bufOffset * ch );
                u32pRdVAddr[ch] = (uint32)gpVPUConfig->pVBufCapture[0] + (bufOffset * ch );
                LOG_DEBUG("Buf%d P:0x%08x, V:0x%08x\n", ch, u32pBufPAddr[ch], u32pRdVAddr[ch]);
            }

            LOG_DEBUG("Capture SDK format:UYVY\n");

            if( PPAPI_VIN_SetCaptureUserMode(BD_VIN_FMT, sclWidth, sclHeight, capChBit) != eSUCCESS )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            LOG_DEBUG("SetUserMode ch:%x(%dx%d)\n", capChBit, sclWidth, sclHeight);

            if( PPAPI_VIN_GetCaptureUserImage(BD_VIN_FMT, sclWidth, sclHeight, capChBit, u32pBufPAddr, u32pBufSize) != eSUCCESS )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            LOG_DEBUG("Buf0 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[0], u32pBufPAddr[0]+u32pBufSize[0], u32pBufSize[0]);
            LOG_DEBUG("Buf1 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[1], u32pBufPAddr[1]+u32pBufSize[1], u32pBufSize[1]);
            LOG_DEBUG("Buf2 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[2], u32pBufPAddr[2]+u32pBufSize[2], u32pBufSize[2]);
            LOG_DEBUG("Buf3 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[3], u32pBufPAddr[3]+u32pBufSize[3], u32pBufSize[3]);
    }
#if 0
    //caplow
    {
            for(ch = 0; ch < 4; ch++)
            {
                u32pBufPAddr[ch] = (uint32)gpVPUConfig->pBufCapture[0] + (bufOffset * ch );
                u32pRdVAddr[ch] = (uint32)gpVPUConfig->pVBufCapture[0] + (bufOffset * ch );
                LOG_DEBUG("Buf%d P:0x%08x, V:0x%08x\n", ch, u32pBufPAddr[ch], u32pRdVAddr[ch]);
            }

            LOG_DEBUG("Capture LOW format:UYVY\n");

            if( PI5008_PPAPI_VIN_SetCaptureUserMode(sclWidth, sclHeight, capChBit) != 0 )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            LOG_DEBUG("SetUserMode ch:%x(%dx%d)\n", capChBit, sclWidth, sclHeight);

            if( PI5008_PPAPI_VIN_GetCaptureUserImage(sclWidth, sclHeight, capChBit, u32pBufPAddr, u32pBufSize) != 0 )
            {
                LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                return eERROR_INVALID_ARGUMENT;
            }
            LOG_DEBUG("Buf0 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[0], u32pBufPAddr[0]+u32pBufSize[0], u32pBufSize[0]);
            LOG_DEBUG("Buf1 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[1], u32pBufPAddr[1]+u32pBufSize[1], u32pBufSize[1]);
            LOG_DEBUG("Buf2 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[2], u32pBufPAddr[2]+u32pBufSize[2], u32pBufSize[2]);
            LOG_DEBUG("Buf3 pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[3], u32pBufPAddr[3]+u32pBufSize[3], u32pBufSize[3]);
    }
#endif

#if 1
    //SD card write.
    {
        void *pfHandle = NULL;
        void *pBuf = NULL;
        uint32 bufSize = 0;

        for(ch = 0; ch < 4; ch++)
        {
            if( capChBit & (1<<ch) )
            {
                pBuf = (void *)(uint32)u32pRdVAddr[ch];
                bufSize = u32pBufSize[ch];
                sprintf(sdFileName, "test_ch%d.yuv", ch);
                LOG_DEBUG("sd write: %s, size:%d\n", sdFileName, bufSize);

                PPAPI_FATFS_DelVolume();
                if(!PPAPI_FATFS_InitVolume()){
                    pfHandle = PPAPI_FATFS_Open(sdFileName,"w");
                    if( (PPAPI_FATFS_Write(pBuf, 1, bufSize, pfHandle) != bufSize) )
                    {
                        LOG_DEBUG("Error can't write(%s)\n", sdFileName);
                    };
                    PPAPI_FATFS_Close(pfHandle);
                }else{
                    // error
                    LOG_DEBUG("Error can't open(%s)\n", sdFileName);
                }
                LOG_DEBUG("Done write(%s)\n", sdFileName);
            }
        }
    }
#endif

	return(ret);
}

int _monitor_pvi(int argc, const char **argv)
{
	int ret = 0;

	//char *strArgv = NULL;
	int ch = 0;

	_stPVIRX_VidStatusReg stVidStatusReg;

	LOG_DEBUG("\n");

	if(argc < 1)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	//	strArgv = (char *)argv[1];
	//	ch = simple_strtoul(strArgv, 10);

	while (!drv_uart_is_kbd_hit()) 
	{
		for(ch = 0; ch < MAX_PVIRX_CHANCNT; ch++)
		{
			PPAPI_PVIRX_MonitorCurVidStatusReg(ch, &stVidStatusReg);
		}
		LOG_DEBUG("\r\nany key exit..\n\n");

		_mdelay(1000);
	}

	return(ret);
}

int _adc(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	int ch = 0;

	int data0, data1;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];

	if( !strncmp(strArgv, "read", sizeof("read")) )
	{
		while (!drv_uart_is_kbd_hit()) 
		{
			data0 = GetRegValue(0xF0A00010);
			data1 = GetRegValue(0xF0A00014);
			LOG_DEBUG("ADC: %02x,%02x(%02x)\n", data0, data1, data1-data0);
			_mdelay(100);
		}
	}
	else if( !strncmp(strArgv, "en", sizeof("en")) )
	{
		data0 = GetRegValue(0xF0A00018);
		data0 |= 0x100;
		SetRegValue(0xF0A00018, data0);
		data1 = GetRegValue(0xF0A0001C);
		data1 |= 0x100;
		SetRegValue(0xF0A0001C, data1);
	}
	else if( !strncmp(strArgv, "dis", sizeof("dis")) )
	{
		data0 = GetRegValue(0xF0A00018);
		data0 &= ~0x100;
		SetRegValue(0xF0A00018, data0);
		data1 = GetRegValue(0xF0A0001C);
		data1 &= ~0x100;
		SetRegValue(0xF0A0001C, data1);
	}
	else
	{

		while (!drv_uart_is_kbd_hit()) 
		{
			for(ch = 0; ch < 2; ch++)
			{
				PPDRV_ADC_Start((1<<ch), 1);
				LOG_DEBUG("ADC Ch%d: %02x\n", ch, PPDRV_ADC_GetData(ch));
			}

			LOG_DEBUG("\r\nany key exit..\n\n");

			_mdelay(1000);
		}
	}

	return(ret);
}

int _genlock(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;

	int data0, refCh = 0;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];

	if( !strncmp(strArgv, "en", sizeof("en")) )
	{
		data0 = GetRegValue(0xF0F00220);
		data0 |= 0xF0000000;
		SetRegValue(0xF0F00220, data0);
	}
	else if( !strncmp(strArgv, "dis", sizeof("dis")) )
	{
		data0 = GetRegValue(0xF0F00220);
		data0 &= ~0xF0000000;
		SetRegValue(0xF0F00220, data0);
	}
	else
	{
		refCh = simple_strtoul(strArgv, 10);

		data0 = GetRegValue(0xF0F00220);
		data0 &= ~0x07000000;
		data0 |= (refCh&0x7)<<24;
		SetRegValue(0xF0F00220, data0);

		LOG_DEBUG("refch:%d\n", refCh);

	}

	return(ret);
}

int _vpuctl(int argc, const char **argv)
{/*{{{*/
	int ret = 0;

	char *strArgv = NULL;

	uint32 *pu32BufFast;
	uint32 *pu32BufBrief;
	uint16 *pu16BufFC;
	uint32 u32FCEndPos;
	uint16 u16FCEndValue;
//	uint16 u16FCLineMax;
//	uint16 u16FCPrev = 0, u16FCCur;
//	uint32 u32FastData, u32FastPos;
	int i;
	int runInx = 0, runMax = 0;
//	int fcLine;
	int zoneNum = 0;
	int lastFrmInx = 0;
	uint32 u32TimeOut;
	_VPUStatus stVPUStatus;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];
	if( !strncmp(strArgv, "start", sizeof("start")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_FAST start(%d)\n", runMax);
		// set fast config
		{
			USER_VPU_FB_CONFIG_U stVPUFBConfig;

			PPAPI_VPU_FAST_GetConfig(&stVPUFBConfig);
			//stVPUFBConfig.fld.fast_n = FAST_PX_NUM;
			//stVPUFBConfig.fld.ch_sel = eVpuChannel;
			//stVPUFBConfig.fld.use_5x3_nms = FALSE;
			//stVPUFBConfig.fld.brief_enable = TRUE;
			//stVPUFBConfig.fld.roi_enable = FALSE;
			//stVPUFBConfig.fld.scl_enable = bScaleEnable;
			stVPUFBConfig.fld.dma2otf_enable = FALSE;
			stVPUFBConfig.fld.otf2dma_enable = FALSE;
			PPAPI_VPU_FAST_SetConfig(&stVPUFBConfig);
		}
		u32TimeOut = 1000;
		PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.

        for (zoneNum = 0; zoneNum < eVPU_ZONE_MAX; zoneNum++)
        {
            u32FCEndPos = gpVPUConfig->pVpuZoneParam[zoneNum].fc_height;
            lastFrmInx = gpVPUConfig->stFrmInx.inxFrmLast;

            pu32BufFast = (uint32 *)gpVPUConfig->ppVBufFast[lastFrmInx][zoneNum];
            pu32BufBrief = (uint32 *)gpVPUConfig->ppVBufBrief[lastFrmInx][zoneNum];
            pu16BufFC = (uint16 *)gpVPUConfig->ppVBufFC[lastFrmInx][zoneNum];
            u16FCEndValue = pu16BufFC[u32FCEndPos-1];
            LOG_DEBUG("zone:%d, endpos:%d, endFcValue:%d fcLimit:%d\n", zoneNum, u32FCEndPos, u16FCEndValue, VPU_MAX_FAST_LIMIT_COUNT);
#if 0 //print result data
            if(u16FCEndValue != 0)
            {/*{{{*/
                for(fcLine = 0, u16FCPrev = 0, u32FastPos = 0; fcLine < u32FCEndPos; fcLine++)
                {
                    u16FCCur = pu16BufFC[fcLine];
                    u16FCLineMax = u16FCCur - u16FCPrev;
                    LOG_DEBUG("line:%d, FcLineMax:%d, FCValue:%d\n", fcLine, u16FCLineMax, u16FCCur);

                    for(fcInx = 0; fcInx < u16FCLineMax; fcInx++)
                    {
                        u32FastData = pu32BufFast[u32FastPos++];
                        LOG_DEBUG("%08x(%d,%d), ", u32FastData, u32FastData>>16, u32FastData&0xFFFF);
                        if(u32FastPos >= VPU_MAX_FAST_LIMIT_COUNT) break;
                    }
                    LOG_DEBUG(":%d\n", u32FastPos);

                    u16FCPrev = u16FCCur;
                }
            }/*}}}*/
#endif
        }
	}/*}}}*/
	else if( !strncmp(strArgv, "dma2d2d", sizeof("dma2d2d")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_DMA start(%d)\n", runMax);

		if(runMax == -1)
		{
			/* test m2m */
			while (!drv_uart_is_kbd_hit()) 
			{
				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_COPY;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32WrStride, u32Width, u32Height;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

				for(i = 0; i < (gpVPUConfig->u32BufCaptureSize>>2)/2; i+=(8*3))
				{
					*(((uint32*)u32RdAddr)+(i+0)) = 0x10305070;
					*(((uint32*)u32RdAddr)+(i+1)) = 0x11315171;
					*(((uint32*)u32RdAddr)+(i+2)) = 0x12325272;
					*(((uint32*)u32RdAddr)+(i+3)) = 0x13335373;
					*(((uint32*)u32RdAddr)+(i+4)) = 0x14345474;
					*(((uint32*)u32RdAddr)+(i+5)) = 0x15355575;
					*(((uint32*)u32RdAddr)+(i+6)) = 0x16365676;
					*(((uint32*)u32RdAddr)+(i+7)) = 0x17375777;

					*(((uint32*)u32RdAddr)+(i+8)) =  0x20305070;
					*(((uint32*)u32RdAddr)+(i+9)) =  0x21315171;
					*(((uint32*)u32RdAddr)+(i+10)) = 0x22325272;
					*(((uint32*)u32RdAddr)+(i+11)) = 0x23335373;
					*(((uint32*)u32RdAddr)+(i+12)) = 0x24345474;
					*(((uint32*)u32RdAddr)+(i+13)) = 0x25355575;
					*(((uint32*)u32RdAddr)+(i+14)) = 0x26365676;
					*(((uint32*)u32RdAddr)+(i+15)) = 0x27375777;

					*(((uint32*)u32RdAddr)+(i+16)) = 0x30305070;
					*(((uint32*)u32RdAddr)+(i+17)) = 0x31315171;
					*(((uint32*)u32RdAddr)+(i+18)) = 0x32325272;
					*(((uint32*)u32RdAddr)+(i+19)) = 0x33335373;
					*(((uint32*)u32RdAddr)+(i+20)) = 0x34345474;
					*(((uint32*)u32RdAddr)+(i+21)) = 0x35355575;
					*(((uint32*)u32RdAddr)+(i+22)) = 0x36365676;
					*(((uint32*)u32RdAddr)+(i+23)) = 0x37375777;
				}

				/* 2D:16x1 */
				{/*{{{*/
					u32Width = 16;
					u32Height = 1;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:16x1\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
				/* 2D:16x2 */
				{/*{{{*/
					u32Width = 16;
					u32Height = 2;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:16x2\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
				/* 2D:16x4 */
				{/*{{{*/
					u32Width = 16;
					u32Height = 4;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:16x4\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
				/* 2D:1280x360 */
				{/*{{{*/
					u32Width = 1280;
					u32Height = 360;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:1280x360\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/

				LOG_DEBUG("\r\nany key exit..\n\n");
			}
		}
		else
		{
			for(runInx = 0; runInx < runMax; runInx++)
			{
				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_COPY;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32WrStride,  u32Width, u32Height;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

				for(i = 0; i < (gpVPUConfig->u32BufCaptureSize>>2)/2; i+=(8*3))
				{
					*(((uint32*)u32RdAddr)+(i+0)) = 0x10305070;
					*(((uint32*)u32RdAddr)+(i+1)) = 0x11315171;
					*(((uint32*)u32RdAddr)+(i+2)) = 0x12325272;
					*(((uint32*)u32RdAddr)+(i+3)) = 0x13335373;
					*(((uint32*)u32RdAddr)+(i+4)) = 0x14345474;
					*(((uint32*)u32RdAddr)+(i+5)) = 0x15355575;
					*(((uint32*)u32RdAddr)+(i+6)) = 0x16365676;
					*(((uint32*)u32RdAddr)+(i+7)) = 0x17375777;

					*(((uint32*)u32RdAddr)+(i+8)) =  0x20305070;
					*(((uint32*)u32RdAddr)+(i+9)) =  0x21315171;
					*(((uint32*)u32RdAddr)+(i+10)) = 0x22325272;
					*(((uint32*)u32RdAddr)+(i+11)) = 0x23335373;
					*(((uint32*)u32RdAddr)+(i+12)) = 0x24345474;
					*(((uint32*)u32RdAddr)+(i+13)) = 0x25355575;
					*(((uint32*)u32RdAddr)+(i+14)) = 0x26365676;
					*(((uint32*)u32RdAddr)+(i+15)) = 0x27375777;

					*(((uint32*)u32RdAddr)+(i+16)) = 0x30305070;
					*(((uint32*)u32RdAddr)+(i+17)) = 0x31315171;
					*(((uint32*)u32RdAddr)+(i+18)) = 0x32325272;
					*(((uint32*)u32RdAddr)+(i+19)) = 0x33335373;
					*(((uint32*)u32RdAddr)+(i+20)) = 0x34345474;
					*(((uint32*)u32RdAddr)+(i+21)) = 0x35355575;
					*(((uint32*)u32RdAddr)+(i+22)) = 0x36365676;
					*(((uint32*)u32RdAddr)+(i+23)) = 0x37375777;
				}

				/* 2D:16x1 */
				{/*{{{*/
					u32Width = 16;
					u32Height = 1;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:16x1\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
				/* 2D:16x2 */
				{/*{{{*/
					u32Width = 16;
					u32Height = 2;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:16x2\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
				/* 2D:16x4 */
				{/*{{{*/
					u32Width = 16;
					u32Height = 4;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:16x4\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
				/* 2D:1280x360 */
				{/*{{{*/
					u32Width = 1280;
					u32Height = 360;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D:1280x360\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i, j;
						for(i = 0; i < (u32Width*u32Height)>>2; i++)
						{
							if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
							{
								LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
								for(j = i - 4; j < (i+4); j++)
								{
									LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
								}
								break;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
			}
		}
	}/*}}}*/
	else if( !strncmp(strArgv, "dma1d2d", sizeof("dma1d2d")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_DMA start(%d)\n", runMax);

		if(runMax == -1)
		{
			/* test m2m */
			while (!drv_uart_is_kbd_hit()) 
			{
				uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32WrStride,  u32Width, u32Height, u32RdSize;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

			    /* all clear */
				for(i = 0; i < gpVPUConfig->u32BufCaptureSize>>2; i++)
				{
					*(((uint32*)u32RdAddr)+i) = 0;
				}

			    /* set rd address data */
				for(i = 0; i < (gpVPUConfig->u32BufCaptureSize>>2)/2; i+=(8*3))
				{
					*(((uint32*)u32RdAddr)+(i+0)) = 0x10305070;
					*(((uint32*)u32RdAddr)+(i+1)) = 0x11315171;
					*(((uint32*)u32RdAddr)+(i+2)) = 0x12325272;
					*(((uint32*)u32RdAddr)+(i+3)) = 0x13335373;
					*(((uint32*)u32RdAddr)+(i+4)) = 0x14345474;
					*(((uint32*)u32RdAddr)+(i+5)) = 0x15355575;
					*(((uint32*)u32RdAddr)+(i+6)) = 0x16365676;
					*(((uint32*)u32RdAddr)+(i+7)) = 0x17375777;

					*(((uint32*)u32RdAddr)+(i+8)) =  0x20305070;
					*(((uint32*)u32RdAddr)+(i+9)) =  0x21315171;
					*(((uint32*)u32RdAddr)+(i+10)) = 0x22325272;
					*(((uint32*)u32RdAddr)+(i+11)) = 0x23335373;
					*(((uint32*)u32RdAddr)+(i+12)) = 0x24345474;
					*(((uint32*)u32RdAddr)+(i+13)) = 0x25355575;
					*(((uint32*)u32RdAddr)+(i+14)) = 0x26365676;
					*(((uint32*)u32RdAddr)+(i+15)) = 0x27375777;

					*(((uint32*)u32RdAddr)+(i+16)) = 0x30305070;
					*(((uint32*)u32RdAddr)+(i+17)) = 0x31315171;
					*(((uint32*)u32RdAddr)+(i+18)) = 0x32325272;
					*(((uint32*)u32RdAddr)+(i+19)) = 0x33335373;
					*(((uint32*)u32RdAddr)+(i+20)) = 0x34345474;
					*(((uint32*)u32RdAddr)+(i+21)) = 0x35355575;
					*(((uint32*)u32RdAddr)+(i+22)) = 0x36365676;
					*(((uint32*)u32RdAddr)+(i+23)) = 0x37375777;
				}

				/* 1D2D */
				{/*{{{*/
					u32Width = 16;
					u32Height = 3;
					u32RdSize = u32Width*u32Height;
					u32WrStride = u32Width*2; // temp

					LOG_DEBUG("#### 1D2D:%dx%d\n", (PP_U16)u32Width, (PP_U16)u32Height);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
                    PPAPI_VPU_DMA_1Dto2D_Start(u32RdAddr, (PP_U16)u32RdSize, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i;
						for(i = 0; i < (u32Width*u32Height); i++)
						{
							if( (i % 8) == 0 ) printf("\n");
							LOG_DEBUG("0x%08X, ", ((uint32)*((uint32*)u32WrAddr+i)) );
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/

				LOG_DEBUG("\r\nany key exit..\n\n");
			}
		}
		else
		{
			for(runInx = 0; runInx < runMax; runInx++)
			{
				uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32WrStride,  u32Width, u32Height, u32RdSize;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

			    /* all clear */
				for(i = 0; i < gpVPUConfig->u32BufCaptureSize>>2; i++)
				{
					*(((uint32*)u32RdAddr)+i) = 0;
				}

			    /* set rd address data */
				for(i = 0; i < (gpVPUConfig->u32BufCaptureSize>>2)/2; i+=(8*3))
				{
					*(((uint32*)u32RdAddr)+(i+0)) = 0x10305070;
					*(((uint32*)u32RdAddr)+(i+1)) = 0x11315171;
					*(((uint32*)u32RdAddr)+(i+2)) = 0x12325272;
					*(((uint32*)u32RdAddr)+(i+3)) = 0x13335373;
					*(((uint32*)u32RdAddr)+(i+4)) = 0x14345474;
					*(((uint32*)u32RdAddr)+(i+5)) = 0x15355575;
					*(((uint32*)u32RdAddr)+(i+6)) = 0x16365676;
					*(((uint32*)u32RdAddr)+(i+7)) = 0x17375777;

					*(((uint32*)u32RdAddr)+(i+8)) =  0x20305070;
					*(((uint32*)u32RdAddr)+(i+9)) =  0x21315171;
					*(((uint32*)u32RdAddr)+(i+10)) = 0x22325272;
					*(((uint32*)u32RdAddr)+(i+11)) = 0x23335373;
					*(((uint32*)u32RdAddr)+(i+12)) = 0x24345474;
					*(((uint32*)u32RdAddr)+(i+13)) = 0x25355575;
					*(((uint32*)u32RdAddr)+(i+14)) = 0x26365676;
					*(((uint32*)u32RdAddr)+(i+15)) = 0x27375777;

					*(((uint32*)u32RdAddr)+(i+16)) = 0x30305070;
					*(((uint32*)u32RdAddr)+(i+17)) = 0x31315171;
					*(((uint32*)u32RdAddr)+(i+18)) = 0x32325272;
					*(((uint32*)u32RdAddr)+(i+19)) = 0x33335373;
					*(((uint32*)u32RdAddr)+(i+20)) = 0x34345474;
					*(((uint32*)u32RdAddr)+(i+21)) = 0x35355575;
					*(((uint32*)u32RdAddr)+(i+22)) = 0x36365676;
					*(((uint32*)u32RdAddr)+(i+23)) = 0x37375777;
				}

				/* 1D2D */
				{/*{{{*/
					u32Width = 16;
					u32Height = 3;
					u32RdSize = u32Width*u32Height;
					u32WrStride = u32Width*2; // temp

					LOG_DEBUG("#### 1D2D:%dx%d\n", (PP_U16)u32Width, (PP_U16)u32Height);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
                    PPAPI_VPU_DMA_1Dto2D_Start(u32RdAddr, (PP_U16)u32RdSize, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i;
						for(i = 0; i < (u32Width*u32Height); i++)
						{
							if( (i % 8) == 0 ) printf("\n");
							LOG_DEBUG("0x%08X, ", ((uint32)*((uint32*)u32WrAddr+i)) );
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
			}
		}
	}/*}}}*/
	else if( !strncmp(strArgv, "dma2d1d", sizeof("dma2d1d")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_DMA start(%d)\n", runMax);

		if(runMax == -1)
		{
			/* test m2m */
			while (!drv_uart_is_kbd_hit()) 
			{
				uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32Width, u32Height, u32WrSize;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

			    /* all clear */
				for(i = 0; i < gpVPUConfig->u32BufCaptureSize>>2; i++)
				{
					*(((uint32*)u32RdAddr)+i) = 0;
				}

			    /* set rd address data */
				for(i = 0; i < (gpVPUConfig->u32BufCaptureSize>>2)/2; i+=(8*3))
				{
					*(((uint32*)u32RdAddr)+(i+0)) = 0x10305070;
					*(((uint32*)u32RdAddr)+(i+1)) = 0x11315171;
					*(((uint32*)u32RdAddr)+(i+2)) = 0x12325272;
					*(((uint32*)u32RdAddr)+(i+3)) = 0x13335373;
					*(((uint32*)u32RdAddr)+(i+4)) = 0x14345474;
					*(((uint32*)u32RdAddr)+(i+5)) = 0x15355575;
					*(((uint32*)u32RdAddr)+(i+6)) = 0x16365676;
					*(((uint32*)u32RdAddr)+(i+7)) = 0x17375777;

					*(((uint32*)u32RdAddr)+(i+8)) =  0x20305070;
					*(((uint32*)u32RdAddr)+(i+9)) =  0x21315171;
					*(((uint32*)u32RdAddr)+(i+10)) = 0x22325272;
					*(((uint32*)u32RdAddr)+(i+11)) = 0x23335373;
					*(((uint32*)u32RdAddr)+(i+12)) = 0x24345474;
					*(((uint32*)u32RdAddr)+(i+13)) = 0x25355575;
					*(((uint32*)u32RdAddr)+(i+14)) = 0x26365676;
					*(((uint32*)u32RdAddr)+(i+15)) = 0x27375777;

					*(((uint32*)u32RdAddr)+(i+16)) = 0x30305070;
					*(((uint32*)u32RdAddr)+(i+17)) = 0x31315171;
					*(((uint32*)u32RdAddr)+(i+18)) = 0x32325272;
					*(((uint32*)u32RdAddr)+(i+19)) = 0x33335373;
					*(((uint32*)u32RdAddr)+(i+20)) = 0x34345474;
					*(((uint32*)u32RdAddr)+(i+21)) = 0x35355575;
					*(((uint32*)u32RdAddr)+(i+22)) = 0x36365676;
					*(((uint32*)u32RdAddr)+(i+23)) = 0x37375777;
				}

				/* 2D1D */
				{/*{{{*/
					u32Width = 16;
					u32Height = 3;
					u32WrSize = u32Width*u32Height;
					u32RdStride = u32Width*2; // temp

					LOG_DEBUG("#### 2D1D:%dx%d\n", (PP_U16)u32Width, (PP_U16)u32Height);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
                    PPAPI_VPU_DMA_2Dto1D_Start((PP_U16)u32Width, (PP_U16)u32Height, u32RdAddr, u32RdStride, u32WrAddr, (PP_U16)u32WrSize, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i;
						for(i = 0; i < (u32Width*u32Height); i++)
						{
							if( (i % 8) == 0 ) printf("\n");
							LOG_DEBUG("0x%08X, ", ((uint32)*((uint32*)u32WrAddr+i)) );
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/

				LOG_DEBUG("\r\nany key exit..\n\n");
			}
		}
		else
		{
			for(runInx = 0; runInx < runMax; runInx++)
			{
				uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32Width, u32Height, u32WrSize;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

			    /* all clear */
				for(i = 0; i < gpVPUConfig->u32BufCaptureSize>>2; i++)
				{
					*(((uint32*)u32RdAddr)+i) = 0;
				}

			    /* set rd address data */
				for(i = 0; i < (gpVPUConfig->u32BufCaptureSize>>2)/2; i+=(8*3))
				{
					*(((uint32*)u32RdAddr)+(i+0)) = 0x10305070;
					*(((uint32*)u32RdAddr)+(i+1)) = 0x11315171;
					*(((uint32*)u32RdAddr)+(i+2)) = 0x12325272;
					*(((uint32*)u32RdAddr)+(i+3)) = 0x13335373;
					*(((uint32*)u32RdAddr)+(i+4)) = 0x14345474;
					*(((uint32*)u32RdAddr)+(i+5)) = 0x15355575;
					*(((uint32*)u32RdAddr)+(i+6)) = 0x16365676;
					*(((uint32*)u32RdAddr)+(i+7)) = 0x17375777;

					*(((uint32*)u32RdAddr)+(i+8)) =  0x20305070;
					*(((uint32*)u32RdAddr)+(i+9)) =  0x21315171;
					*(((uint32*)u32RdAddr)+(i+10)) = 0x22325272;
					*(((uint32*)u32RdAddr)+(i+11)) = 0x23335373;
					*(((uint32*)u32RdAddr)+(i+12)) = 0x24345474;
					*(((uint32*)u32RdAddr)+(i+13)) = 0x25355575;
					*(((uint32*)u32RdAddr)+(i+14)) = 0x26365676;
					*(((uint32*)u32RdAddr)+(i+15)) = 0x27375777;

					*(((uint32*)u32RdAddr)+(i+16)) = 0x30305070;
					*(((uint32*)u32RdAddr)+(i+17)) = 0x31315171;
					*(((uint32*)u32RdAddr)+(i+18)) = 0x32325272;
					*(((uint32*)u32RdAddr)+(i+19)) = 0x33335373;
					*(((uint32*)u32RdAddr)+(i+20)) = 0x34345474;
					*(((uint32*)u32RdAddr)+(i+21)) = 0x35355575;
					*(((uint32*)u32RdAddr)+(i+22)) = 0x36365676;
					*(((uint32*)u32RdAddr)+(i+23)) = 0x37375777;
				}

				/* 2D1D */
				{/*{{{*/
					u32Width = 16;
					u32Height = 3;
					u32WrSize = u32Width*u32Height;
					u32RdStride = u32Width*2; // temp

					LOG_DEBUG("#### 2D1D:%dx%d\n", (PP_U16)u32Width, (PP_U16)u32Height);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
                    PPAPI_VPU_DMA_2Dto1D_Start((PP_U16)u32Width, (PP_U16)u32Height, u32RdAddr, u32RdStride, u32WrAddr, (PP_U16)u32WrSize, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
					//verify for debug
					{
						uint32 i;
						for(i = 0; i < (u32Width*u32Height); i++)
						{
							if( (i % 8) == 0 ) printf("\n");
							LOG_DEBUG("0x%08X, ", ((uint32)*((uint32*)u32WrAddr+i)) );
						}

						//clear for next
						memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize);
						PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

						if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}
				}/*}}}*/
			}
		}
	}/*}}}*/
	    #if 0
	else if( !strncmp(strArgv, "rle", sizeof("rle")) )
	{/*{{{*/
		LOG_DEBUG("VPU_DMA RLE start(tiger_640x480_INDEX)\n");

		{
			uint32 srcInxAddr;
			uint16 srcInxWidth;
			uint16 srcInxHeight;
#if 0	// by ihkong. because of build error
			{
				extern void dutest_get_rsc (const int inx, uint32 *addr, uint16 *width, uint16 *height, int *format);
				//int inx = 87; //rect_16x16_INDEX;
				int inx = 94; //tiger_640x480_INDEX;
				int format;
				dutest_get_rsc (inx, &srcInxAddr, &srcInxWidth, &srcInxHeight, &format);
				if(format != 1) //index file
				{
					LOG_DEBUG("Error. Isn't index format file\n");
				}
			}
#endif
			/* test rle */
			//while (!drv_uart_is_kbd_hit()) 
			{
				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_COPY;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdAddr = (uint32)srcInxAddr;
				uint32 u32RdStride, u32WrStride, u32Width, u32Height;
				uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0];

				#if 0
				{/*{{{*/
					u32Width = srcInxWidth;
					u32Height = srcInxHeight;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 2D COPY\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32 *)u32RdAddr+0),
							(uint32)*((uint32 *)u32RdAddr+1),
							(uint32)*((uint32 *)u32RdAddr+2),
							(uint32)*((uint32 *)u32RdAddr+3),
							(uint32)*((uint32 *)u32RdAddr+4),
							(uint32)*((uint32 *)u32RdAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));

					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
					u32TimeOut = 1000;
					VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, u32WrStride, u32Width, u32Height, u32WrAddr, u32TimeOut);
					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize);

					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
				}/*}}}*/
				#endif
				//RLE encoding
				{/*{{{*/
					eDMAMode = eDMA_RLE;
					eDMAMinorMode = eDMA_MINOR_COPY;
					eRDMAType = eRDMA_2D;
					eWDMAType = eWDMA_1D;
					eOTF2DMAType = eOTF2DMA_DISABLE;
					//u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
					u32RdAddr = (uint32)srcInxAddr;
					uint32 u32WrVAddr = (uint32)gpVPUConfig->pVBufCapture[1]; //change address
					uint32 u32WrPAddr = (uint32)gpVPUConfig->pBufCapture[1]; //change address
					int i;


					u32Width = srcInxWidth;
					u32Height = srcInxHeight;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("#### 1D RLE\n");
					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
//					PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32RdAddr, gpVPUConfig->u32BufCaptureSize);
					for(i = 0; i < 100; i++)
					{
					LOG_DEBUG ("Image(%4d) : 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", i,
							(uint32)*((uint32 *)u32RdAddr+(0+(i*4))),
							(uint32)*((uint32 *)u32RdAddr+(1+(i*4))),
							(uint32)*((uint32 *)u32RdAddr+(2+(i*4))),
							(uint32)*((uint32 *)u32RdAddr+(3+(i*4))));
					}
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32 *)u32WrVAddr+0),
							(uint32)*((uint32 *)u32WrVAddr+1),
							(uint32)*((uint32 *)u32WrVAddr+2),
							(uint32)*((uint32 *)u32WrVAddr+3),
							(uint32)*((uint32 *)u32WrVAddr+4),
							(uint32)*((uint32 *)u32WrVAddr+5));


					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x-0x%08x\n", u32RdAddr, u32WrPAddr, u32WrPAddr + (u32Width*u32Height));
					u32TimeOut = 1000;
					VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, u32WrStride, u32Width, u32Height, u32WrPAddr, u32TimeOut);

					LOG_DEBUG ("RLE->WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32 *)u32WrVAddr+0),
							(uint32)*((uint32 *)u32WrVAddr+1),
							(uint32)*((uint32 *)u32WrVAddr+2),
							(uint32)*((uint32 *)u32WrVAddr+3),
							(uint32)*((uint32 *)u32WrVAddr+4),
							(uint32)*((uint32 *)u32WrVAddr+5));
				}/*}}}*/
			}
		}
	}/*}}}*/
	    #endif
	else if( !strncmp(strArgv, "evendma", sizeof("evendma")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_DMA Evencopy start(%d)\n", runMax);

		if(runMax == -1)
		{
			/* test m2m evenbyte */
			while (!drv_uart_is_kbd_hit()) 
			{/*{{{*/
				int i;

				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_EVENBYTE;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdVAddr = (uint32)gpVPUConfig->pVBufCapture[0];
				uint32 u32RdPAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32WrStride, u32Width, u32Height;
				uint32 u32WrVAddr = (uint32)gpVPUConfig->pVBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32WrPAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32TimeOut = 1000;
				uint8 *pu8RdBuf, *pu8WrBuf;

				pu8RdBuf = (uint8*)u32RdVAddr;
				pu8WrBuf = (uint8*)u32WrVAddr;

				for(i = 0; i < gpVPUConfig->u32BufCaptureSize; i++)
				{
					*(pu8RdBuf+i) = i;
				}
				LOG_DEBUG ("Rd0 : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
						(uint32)*(gpVPUConfig->pVBufCapture[0]+0),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+1),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+2),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+3),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+4),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+5));

#if 1
				{/*{{{*/
					u32Width = (1920>>5)<<5;
					u32Height = 1080/4;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdVAddr+0),
							(uint32)*((uint32*)u32RdVAddr+1),
							(uint32)*((uint32*)u32RdVAddr+2),
							(uint32)*((uint32*)u32RdVAddr+3),
							(uint32)*((uint32*)u32RdVAddr+4),
							(uint32)*((uint32*)u32RdVAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));


					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdPAddr, u32WrPAddr);
					u32TimeOut = 10000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrPAddr, u32WrStride, u32TimeOut);
					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));
					//verify for debug
					{/*{{{*/
						int rdInx = 0;
						int wrInx = 0;

						if( eDMAMinorMode == eDMA_MINOR_EVENBYTE)
						{
							rdInx = 0;
							wrInx = 0;
						}
						else if( eDMAMinorMode == eDMA_MINOR_ODDBYTE)
						{
							rdInx = 1;
							wrInx = 0;
						}
						else
						{
							rdInx = 0;
							wrInx = 0;
						}

						while(rdInx < (u32Width*u32Height)>>2)
						{
							if( (*(pu8RdBuf+rdInx)) != (*(pu8WrBuf+wrInx)) )
							{
								LOG_DEBUG("ERRROR %d, Rd:0x%02x, Wr:0x%02x\n", rdInx,
										((uint8)*(pu8RdBuf+rdInx)), ((uint8)*(pu8WrBuf+wrInx)) );
								break;
							}

							if( (eDMAMinorMode == eDMA_MINOR_EVENBYTE) || (eDMAMinorMode == eDMA_MINOR_ODDBYTE) )
							{
								rdInx += 2;
								wrInx++;
							}
							else
							{
								rdInx++;
								wrInx++;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrVAddr, 0, gpVPUConfig->u32BufCaptureSize);
						if( rdInx >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}/*}}}*/
				}/*}}}*/
#endif

				LOG_DEBUG("\r\nany key exit..\n\n");
			}/*}}}*/
		}
		else
		{
			/* test m2m evenbyte */
			for(runInx = 0; runInx < runMax; runInx++)
			{/*{{{*/
				int i;

				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_EVENBYTE;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdVAddr = (uint32)gpVPUConfig->pVBufCapture[0];
				uint32 u32RdPAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32WrStride, u32Width, u32Height;
				uint32 u32WrVAddr = (uint32)gpVPUConfig->pVBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32WrPAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32TimeOut = 1000;
				uint8 *pu8RdBuf, *pu8WrBuf;

				pu8RdBuf = (uint8*)u32RdVAddr;
				pu8WrBuf = (uint8*)u32WrVAddr;

				for(i = 0; i < gpVPUConfig->u32BufCaptureSize; i++)
				{
					*(pu8RdBuf+i) = i;
				}
				LOG_DEBUG ("Rd0 : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
						(uint32)*(gpVPUConfig->pVBufCapture[0]+0),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+1),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+2),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+3),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+4),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+5));

#if 1
				{/*{{{*/
					u32Width = (1920>>5)<<5;
					u32Height = 1080/4;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdVAddr+0),
							(uint32)*((uint32*)u32RdVAddr+1),
							(uint32)*((uint32*)u32RdVAddr+2),
							(uint32)*((uint32*)u32RdVAddr+3),
							(uint32)*((uint32*)u32RdVAddr+4),
							(uint32)*((uint32*)u32RdVAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));


					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdPAddr, u32WrPAddr);
					u32TimeOut = 10000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrPAddr, u32WrStride, u32TimeOut);
					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));
					//verify for debug
					{/*{{{*/
						int rdInx = 0;
						int wrInx = 0;

						if( eDMAMinorMode == eDMA_MINOR_EVENBYTE)
						{
							rdInx = 0;
							wrInx = 0;
						}
						else if( eDMAMinorMode == eDMA_MINOR_ODDBYTE)
						{
							rdInx = 1;
							wrInx = 0;
						}
						else
						{
							rdInx = 0;
							wrInx = 0;
						}

						while(rdInx < (u32Width*u32Height)>>2)
						{
							if( (*(pu8RdBuf+rdInx)) != (*(pu8WrBuf+wrInx)) )
							{
								LOG_DEBUG("ERRROR %d, Rd:0x%02x, Wr:0x%02x\n", rdInx,
										((uint8)*(pu8RdBuf+rdInx)), ((uint8)*(pu8WrBuf+wrInx)) );
								break;
							}

							if( (eDMAMinorMode == eDMA_MINOR_EVENBYTE) || (eDMAMinorMode == eDMA_MINOR_ODDBYTE) )
							{
								rdInx += 2;
								wrInx++;
							}
							else
							{
								rdInx++;
								wrInx++;
							}
						}
						//clear for next
						memset ((uint32 *)u32WrVAddr, 0, gpVPUConfig->u32BufCaptureSize);

						if( rdInx >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");

					}/*}}}*/
				}/*}}}*/
#endif
			}/*}}}*/
		}
	}/*}}}*/
	else if( !strncmp(strArgv, "odddma", sizeof("odddma")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_DMA Oddcopy start(%d)\n", runMax);

		if(runMax == -1)
		{
			/* test m2m oddbyte */
			while (!drv_uart_is_kbd_hit()) 
			{/*{{{*/
				int i;

				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_ODDBYTE;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdVAddr = (uint32)gpVPUConfig->pVBufCapture[0];
				uint32 u32RdPAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32WrStride, u32Width, u32Height;
				uint32 u32WrVAddr = (uint32)gpVPUConfig->pVBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32WrPAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32TimeOut = 1000;
				uint8 *pu8RdBuf, *pu8WrBuf;

				pu8RdBuf = (uint8*)u32RdVAddr;
				pu8WrBuf = (uint8*)u32WrVAddr;

				for(i = 0; i < gpVPUConfig->u32BufCaptureSize; i++)
				{
					*(pu8RdBuf+i) = i;
				}
				LOG_DEBUG ("Rd0 : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
						(uint32)*(gpVPUConfig->pVBufCapture[0]+0),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+1),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+2),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+3),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+4),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+5));

#if 1
				{/*{{{*/
					u32Width = (1920>>5)<<5;
					u32Height = 1080/4;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdVAddr+0),
							(uint32)*((uint32*)u32RdVAddr+1),
							(uint32)*((uint32*)u32RdVAddr+2),
							(uint32)*((uint32*)u32RdVAddr+3),
							(uint32)*((uint32*)u32RdVAddr+4),
							(uint32)*((uint32*)u32RdVAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));


					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdPAddr, u32WrPAddr);
					u32TimeOut = 10000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrPAddr, u32WrStride, u32TimeOut);
					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));
					//verify for debug
					{/*{{{*/
						int rdInx = 0;
						int wrInx = 0;

						if( eDMAMinorMode == eDMA_MINOR_EVENBYTE)
						{
							rdInx = 0;
							wrInx = 0;
						}
						else if( eDMAMinorMode == eDMA_MINOR_ODDBYTE)
						{
							rdInx = 1;
							wrInx = 0;
						}
						else
						{
							rdInx = 0;
							wrInx = 0;
						}

						while(rdInx < (u32Width*u32Height)>>2)
						{
							if( (*(pu8RdBuf+rdInx)) != (*(pu8WrBuf+wrInx)) )
							{
								LOG_DEBUG("ERRROR %d, Rd:0x%02x, Wr:0x%02x\n", rdInx,
										((uint8)*(pu8RdBuf+rdInx)), ((uint8)*(pu8WrBuf+wrInx)) );
								break;
							}

							if( (eDMAMinorMode == eDMA_MINOR_EVENBYTE) || (eDMAMinorMode == eDMA_MINOR_ODDBYTE) )
							{
								rdInx += 2;
								wrInx++;
							}
							else
							{
								rdInx++;
								wrInx++;
							}
						}

						//clear for next
						memset ((uint32 *)u32WrVAddr, 0, gpVPUConfig->u32BufCaptureSize);
						if( rdInx >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
						else break;

					}/*}}}*/
				}/*}}}*/
#endif

				LOG_DEBUG("\r\nany key exit..\n\n");
			}/*}}}*/
		}
		else
		{
			/* test m2m oddbyte */
			for(runInx = 0; runInx < runMax; runInx++)
			{/*{{{*/
				int i;

				_eDMAMODE eDMAMode = eDMA_2D;
				_eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_ODDBYTE;
				_eRDMATYPE eRDMAType = eRDMA_2D;
				_eWDMATYPE eWDMAType = eWDMA_2D;
				_eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
				uint32 u32RdVAddr = (uint32)gpVPUConfig->pVBufCapture[0];
				uint32 u32RdPAddr = (uint32)gpVPUConfig->pBufCapture[0];
				uint32 u32RdStride, u32WrStride, u32Width, u32Height;
				uint32 u32WrVAddr = (uint32)gpVPUConfig->pVBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32WrPAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);
				uint32 u32TimeOut = 1000;
				uint8 *pu8RdBuf, *pu8WrBuf;

				pu8RdBuf = (uint8*)u32RdVAddr;
				pu8WrBuf = (uint8*)u32WrVAddr;

				for(i = 0; i < gpVPUConfig->u32BufCaptureSize; i++)
				{
					*(pu8RdBuf+i) = i;
				}
				LOG_DEBUG ("Rd0 : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
						(uint32)*(gpVPUConfig->pVBufCapture[0]+0),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+1),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+2),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+3),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+4),
						(uint32)*(gpVPUConfig->pVBufCapture[0]+5));

#if 1
				{/*{{{*/
					u32Width = (1920>>5)<<5;
					u32Height = 1080/4;
					u32RdStride = u32Width;
					u32WrStride = u32Width;

					LOG_DEBUG("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
					LOG_DEBUG ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdVAddr+0),
							(uint32)*((uint32*)u32RdVAddr+1),
							(uint32)*((uint32*)u32RdVAddr+2),
							(uint32)*((uint32*)u32RdVAddr+3),
							(uint32)*((uint32*)u32RdVAddr+4),
							(uint32)*((uint32*)u32RdVAddr+5));
					LOG_DEBUG ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));


					LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n", u32RdPAddr, u32WrPAddr);
					u32TimeOut = 10000;
					PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrPAddr, u32WrStride, u32TimeOut);
					LOG_DEBUG ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrVAddr+0),
							(uint32)*((uint32*)u32WrVAddr+1),
							(uint32)*((uint32*)u32WrVAddr+2),
							(uint32)*((uint32*)u32WrVAddr+3),
							(uint32)*((uint32*)u32WrVAddr+4),
							(uint32)*((uint32*)u32WrVAddr+5));
					//verify for debug
					{/*{{{*/
						int rdInx = 0;
						int wrInx = 0;

						if( eDMAMinorMode == eDMA_MINOR_EVENBYTE)
						{
							rdInx = 0;
							wrInx = 0;
						}
						else if( eDMAMinorMode == eDMA_MINOR_ODDBYTE)
						{
							rdInx = 1;
							wrInx = 0;
						}
						else
						{
							rdInx = 0;
							wrInx = 0;
						}

						while(rdInx < (u32Width*u32Height)>>2)
						{
							if( (*(pu8RdBuf+rdInx)) != (*(pu8WrBuf+wrInx)) )
							{
								LOG_DEBUG("ERRROR %d, Rd:0x%02x, Wr:0x%02x\n", rdInx,
										((uint8)*(pu8RdBuf+rdInx)), ((uint8)*(pu8WrBuf+wrInx)) );
								break;
							}

							if( (eDMAMinorMode == eDMA_MINOR_EVENBYTE) || (eDMAMinorMode == eDMA_MINOR_ODDBYTE) )
							{
								rdInx += 2;
								wrInx++;
							}
							else
							{
								rdInx++;
								wrInx++;
							}
						}
						//clear for next
						memset ((uint32 *)u32WrVAddr, 0, gpVPUConfig->u32BufCaptureSize);

						if( rdInx >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");

					}/*}}}*/
				}/*}}}*/
#endif
			}/*}}}*/
		}
	}/*}}}*/
	else if( !strncmp(strArgv, "hamd", sizeof("hamd")) )
	{/*{{{*/
		//runMax = -1 => endless.
		if(argc > 2)
		{
			strArgv = (char *)argv[2];
			runMax = simple_strtoul(strArgv, 10);
			if(runMax == 0) 
			{
				runMax = -1;
			}
		}
		else
		{
			runMax = 1;
		}
		LOG_DEBUG("VPU_HammingD start(%d)\n", runMax);
		// set fast config
		{
			USER_VPU_FB_CONFIG_U stVPUFBConfig;

			PPAPI_VPU_FAST_GetConfig(&stVPUFBConfig);
			//stVPUFBConfig.fld.fast_n = FAST_PX_NUM;
			//stVPUFBConfig.fld.ch_sel = eVpuChannel;
			//stVPUFBConfig.fld.use_5x3_nms = FALSE;
			//stVPUFBConfig.fld.brief_enable = TRUE;
			//stVPUFBConfig.fld.roi_enable = FALSE;
			//stVPUFBConfig.fld.scl_enable = bScaleEnable;
			stVPUFBConfig.fld.dma2otf_enable = FALSE;
			stVPUFBConfig.fld.otf2dma_enable = FALSE;
			PPAPI_VPU_FAST_SetConfig(&stVPUFBConfig);
		}
		u32TimeOut = 1000;
		PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.

		if(runMax == -1)
		{
			/* test m2m */
			while (!drv_uart_is_kbd_hit()) 
			{

				if( (PPAPI_VPU_WaitFrmUpdate(1000)) == eSUCCESS)
				{

					PPAPI_VPU_GetStatus(&stVPUStatus);
#if 1
					//print HW info
					{/*{{{*/
						uint32_t i;
						//uint32_t j;
						uint32_t k;
						uint32_t inxFrm;

						inxFrm = stVPUStatus.stFrmInx.inxFrmPrev;
						//for (j = 0; j < eVPU_FRAME_MAX; j++)
						{
							//for (i = 0; i < eVPU_ZONE_MAX; i++)
							for (i = 0; i < 1; i++)
							{
								LOG_DEBUG("Print FAST result of VPU.(Zone:%d)\n", i);
								pu32BufFast = (uint32 *)stVPUStatus.ppVBufFast[inxFrm][i];
								pu32BufBrief = (uint32 *)stVPUStatus.ppVBufBrief[inxFrm][i];
								pu16BufFC = (uint16 *)stVPUStatus.ppVBufFC[inxFrm][i];

								u32FCEndPos = stVPUStatus.pVpuZoneParam[i].fc_height;
								LOG_DEBUG("   endpos:%d, endFcValue:%d\n", u32FCEndPos, pu16BufFC[u32FCEndPos-1]);

								for (k = 0; k < pu16BufFC[u32FCEndPos-1]; k++)
								{
									uint32_t x, y;
									uint32_t *d = (uint32_t *)&pu32BufBrief[k*4];
									x = ((pu32BufFast[k] & 0xFFFF0000) >> 16);
									y = ((pu32BufFast[k] & 0x0000FFFF));
									LOG_DEBUG ("HW[%d]Zone[%d]  %d ( %d,  %d) = %08x%08x%08x%08x\n", inxFrm, i, k, x, y, d[3], d[2], d[1], d[0]);
								}
							}
						}

					}/*}}}*/
#endif

					PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000);

#if 1
					LOG_DEBUG("Print HamminD result of VPU.\n");
					{/*{{{*/
						pVPU_MATCHING_RESULT_POS_T hw_HD;
						for (zoneNum = 0; zoneNum < eVPU_ZONE_MAX; zoneNum++)
						{
							hw_HD = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[zoneNum];
							for (i = 0; i < stVPUStatus.u32HDMatchResultCount[zoneNum]; i++)
							{
								LOG_DEBUG ("HW (%u:%u) : (%u, %u) => (%u, %u)\n", zoneNum, i, hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
							}

						}

					}/*}}}*/
#endif
				}

				//LOG_DEBUG("\r\nany key exit..\n\n");
			}
			PPAPI_VPU_FAST_Stop();
		}
		else
		{
			for(runInx = 0; runInx < runMax; runInx++)
			{

				PPAPI_VPU_GetStatus(&stVPUStatus);
				PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000);
			}
			PPAPI_VPU_FAST_Stop();
		}
	}/*}}}*/
#if 0
	else if( !strncmp(strArgv, "stop", sizeof("stop")) )
	{
		LOG_DEBUG("VPU stop\n");
		VPU_stop ();
	}
	else if( !strncmp(strArgv, "draw", sizeof("draw")) )
	{
		LOG_DEBUG("VPU draw feature point\n");
		VPU_draw_start ();
	}
	else if( !strncmp(strArgv, "mon", sizeof("mon")) )
	{
		while (!drv_uart_is_kbd_hit()) 
		{
			print_vpu_counts ();
			LOG_DEBUG("\r\nany key exit..\n\n");

			_mdelay(1000);
		}
	}
#else
	else if( !strncmp(strArgv, "dump", sizeof("dump")) )
	{
		LOG_DEBUG("VPU dump register\n");
		PPAPI_VPU_DumpReg();
	}
#endif
	else
	{
	}

	return(ret);
}/*}}}*/

int _alivetask(int argc, const char **argv)
{
	int ret = 0;

//	int i;
	int taskNum = 0;

	LOG_DEBUG("\n");

    LOG_DEBUG("Task total:%d(exclude CLI)\n", MAX_TASK-1);
    for(taskNum = 0; taskNum < MAX_TASK; taskNum++)
    {
	    if( taskNum == TASK_CLI) continue; 
        LOG_DEBUG("%d: task name:%s\n",  taskNum, psTaskName[taskNum]);
    }

    for(taskNum = 0; taskNum < MAX_TASK; taskNum++)
    {
	    if( taskNum == TASK_CLI) continue; 

        LOG_DEBUG("send alive ping to task[%s]\n",  psTaskName[taskNum]);
        AppTask_SendCmd(0, TASK_CLI, taskNum, 1<<QUEUE_CMDATTR_BIT_REQACK, NULL, 0, 1000);
        //Wait ack
        {/*{{{*/
            int myHandleNum = TASK_CLI;
            int timeOut = 1000; //msec
            EventBits_t eventWaitBits;
            EventBits_t eventResultBits;
            int bExitWait = 0;

            int i;

            printf("Wait Ack\n");
            while(!bExitWait)
            {
                //LOG_DEBUG("WaitBits(%s)\n", __FUNCTION__);
                if(gHandle[myHandleNum].phEventGroup)
                {
                    eventWaitBits = 0x00FFFFFF; //0x00FFFFFF all bit
                    eventResultBits = OSAL_EVENTGROUP_CMD_WaitBits(gHandle[myHandleNum].phEventGroup, eventWaitBits, pdTRUE, pdFALSE, timeOut);

                    // process event bit
                    if(eventResultBits & (1<<EVENT_CLI_INIT))
                    {
                        printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_AUDIO_INIT, psEventNameCLI[EVENT_AUDIO_INIT]);
                    }
                    else if(eventResultBits & (1<<EVENT_CLI_MSG))
                    {
                        //LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_AUDIO_MSG, psEventNameCLI[EVENT_AUDIO_MSG]);
                        {/*{{{*/
                            if(gHandle[myHandleNum].phQueue)
                            {
                                int queueCnt = 0;
                                stQueueItem queueItem;
                                if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                                {
                                    //LOG_DEBUG("get Queue cnt:%d\n", queueCnt);
                                    for(i = 0; i < queueCnt; i++)
                                    {
                                        if( OSAL_QUEUE_CMD_Receive(gHandle[myHandleNum].phQueue, &queueItem, 0) == eSUCCESS )
                                        {
                                            /*
                                               LOG_DEBUG("%s:Rcv cmd:%08x, sender:%d(%s), attr:%x, length:%d\n", psTaskName[myHandleNum],
                                               queueItem.u32Cmd, queueItem.u16Sender, psTaskName[queueItem.u16Sender], queueItem.u16Attr, queueItem.u32Length);
                                             */
                                            if(queueItem.u32Cmd == CMD_ACK)
                                            {						
                                                printf("alive cnt:0x%08x\n", (uint32)(*(uint32 *)queueItem.pData));
                                                bExitWait = 1;
                                            }

                                            if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
                                            {
                                                OSAL_free(queueItem.pData);
                                            }

                                        }
                                    }
                                }
                                else
                                {
                                    //ignore
                                    //LOG_DEBUG("Don't remain queue.\n");
                                }
                            }
                            else
                            {
                                LOG_CRITICAL("Invalid handle.\n");
                            }

                        }/*}}}*/
                    }
                    else if(eventResultBits == 0)
                    {
                        //LOG_DEBUG("timeout\n");
                    }
                    else
                    {
                        LOG_DEBUG("unknown Event(0x%X)\n", eventResultBits);
                    }
                }
            }
        }/*}}}*/

        OSAL_sleep(1000);
    }

    return(ret);
}


////////////////////////////////////////////////////////////////////////////////////////////////
void BGTestTask_base(void *pdata)
{
	int cnt = 0;

	LOG_DEBUG("run\n");
	while(1)
	{
		LOG_DEBUG("%s count:%d\n", __FUNCTION__, cnt++);
		OSAL_sleep(1000);
	}

	LOG_DEBUG("exit\n");
}       
void BGTestTask_vpudma(void *pdata)
{
	int cnt = 0;

	int i;
	uint32 u32TimeOut;
    uint32 u32VpuVer = PPAPI_VPU_GetVer();

    LOG_DEBUG("run\n");
	while(1)
	{
        /* test m2m */
        if(u32VpuVer != 0)
        {
            _eDMAMODE eDMAMode = eDMA_2D;
            _eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_COPY;
            _eRDMATYPE eRDMAType = eRDMA_2D;
            _eWDMATYPE eWDMAType = eWDMA_2D;
            _eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
            uint32 u32RdAddr = (uint32)gpVPUConfig->pBufCapture[0];
            uint32 u32RdStride, u32WrStride, u32Width, u32Height;
            uint32 u32WrAddr = (uint32)gpVPUConfig->pBufCapture[0]+(gpVPUConfig->u32BufCaptureSize/2);

            for(i = 0; i < gpVPUConfig->u32BufCaptureSize/4; i++)
            {
                *(((uint32*)u32RdAddr)+i) = 0x12345678;
            }

            /* 2D:16x1 */
            {/*{{{*/
                u32Width = 16;
                u32Height = 1;
                u32RdStride = u32Width;
                u32WrStride = u32Width;

                LOG_DEBUG("#### VPUDMA 2D:16x1\n");
                //printf("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
                /*
                printf ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
                printf ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */

                //printf("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                u32TimeOut = 1000;
                PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize/2);

                /*
                printf ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */
                //verify for debug
                {
                    uint32 i, j;
                    for(i = 0; i < (u32Width*u32Height)>>2; i++)
                    {
						if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
                        {
                            LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
                            for(j = i - 4; j < (i+4); j++)
                            {
                                LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
                            }
                            break;
                        }
                    }

                    //clear for next
                    memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize/2);

                    if( i >= ((u32Width*u32Height)>>2) ) printf("ok\n");
                    else break;

                }
            }/*}}}*/
            /* 2D:16x2 */
            {/*{{{*/
                u32Width = 16;
                u32Height = 2;
                u32RdStride = u32Width;
                u32WrStride = u32Width;

                LOG_DEBUG("#### VPUDMA 2D:16x2\n");
                //printf("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
                /*
                printf ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
                printf ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */

                //printf("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                u32TimeOut = 1000;
                PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize/2);

                /*
                printf ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                 */
                //verify for debug
                {
                    uint32 i, j;
                    for(i = 0; i < (u32Width*u32Height)>>2; i++)
                    {
						if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
                        {
                            LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
                            for(j = i - 4; j < (i+4); j++)
                            {
                                LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
                            }
                            break;
                        }
                    }

                    //clear for next
                    memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize/2);

                    if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
                    else break;

                }
            }/*}}}*/
            /* 2D:16x4 */
            {/*{{{*/
                u32Width = 16;
                u32Height = 4;
                u32RdStride = u32Width;
                u32WrStride = u32Width;

                LOG_DEBUG("#### VPUDMA 2D:16x4\n");
                //printf("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
                /*
                printf ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
                printf ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */

                //printf("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                u32TimeOut = 1000;
                PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize/2);

                /*
                printf ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */
                //verify for debug
                {
                    uint32 i, j;
                    for(i = 0; i < (u32Width*u32Height)>>2; i++)
                    {
						if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
                        {
                            LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
                            for(j = i - 4; j < (i+4); j++)
                            {
                                LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
                            }
                            break;
                        }
                    }

                    //clear for next
                    memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize/2);

                    if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
                    else break;

                }
            }/*}}}*/
            /* 2D:1280x360 */
            {/*{{{*/
                u32Width = 1280;
                u32Height = 360;
                u32RdStride = u32Width;
                u32WrStride = u32Width;

                LOG_DEBUG("#### VPUDMA 2D:1280x360\n");
                //printf("dma mode:%d-%d, dmasize:0x%08x, buf size:0x%08x\n", eDMAMode, eDMAMinorMode, u32Width*u32Height, gpVPUConfig->u32BufCaptureSize);
                /*
                printf ("Rd : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32RdAddr+0),
							(uint32)*((uint32*)u32RdAddr+1),
							(uint32)*((uint32*)u32RdAddr+2),
							(uint32)*((uint32*)u32RdAddr+3),
							(uint32)*((uint32*)u32RdAddr+4),
							(uint32)*((uint32*)u32RdAddr+5));
                printf ("WrP : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */

                //printf("Rd:0x%08x, Wr:0x%08x\n", u32RdAddr, u32WrAddr);
                u32TimeOut = 1000;
                PPAPI_VPU_DMA_Start (eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdAddr, u32RdStride, (PP_U16)u32Width, (PP_U16)u32Height, u32WrAddr, u32WrStride, u32TimeOut);
                PPAPI_SYS_CACHE_Invalidate ((uint32 *)u32WrAddr, gpVPUConfig->u32BufCaptureSize/2);

                /*
                printf ("WrA : 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
							(uint32)*((uint32*)u32WrAddr+0),
							(uint32)*((uint32*)u32WrAddr+1),
							(uint32)*((uint32*)u32WrAddr+2),
							(uint32)*((uint32*)u32WrAddr+3),
							(uint32)*((uint32*)u32WrAddr+4),
							(uint32)*((uint32*)u32WrAddr+5));
                */
                //verify for debug
                {
                    uint32 i, j;
                    for(i = 0; i < (u32Width*u32Height)>>2; i++)
                    {
						if( ((uint32)*((uint32*)u32RdAddr+i)) != ((uint32)*((uint32*)u32WrAddr+i)) )
                        {
                            LOG_DEBUG("Error! i:%d/%d, Rd:0x%08x, Wr:0x%08x\n", i, (u32Width*u32Height)>>2,
                                    ((uint32)*((uint32*)u32RdAddr+i)), ((uint32)*((uint32*)u32WrAddr+i)) );
                            for(j = i - 4; j < (i+4); j++)
                            {
                                LOG_DEBUG("Rd:0x%08x, Wr:0x%08x\n",
                                        ((uint32)*((uint32*)u32RdAddr+j)), ((uint32)*((uint32*)u32WrAddr+j)) );
                            }
                            break;
                        }
                    }

                    //clear for next
                    memset ((uint32 *)u32WrAddr, 0, gpVPUConfig->u32BufCaptureSize/2);

                    if( i >= ((u32Width*u32Height)>>2) ) LOG_DEBUG("ok\n");
                    else break;

                }
            }/*}}}*/

            LOG_DEBUG("%s count:%d\n", __FUNCTION__, cnt++);
        }
        OSAL_sleep(100);
    }

	LOG_DEBUG("exit\n");
}       
void BGTestTask_uikey(void *pdata)
{
	int cnt = 0;

    uint32 u32Cmd;
	uint8 u8Key = 0;

#define MAXKEYCNT  (8)
    const char keys[MAXKEYCNT] = {'w', 'w', 'x', 'x', 'a', 'a', 'd', 'd'};
    int keyInx = 0;

	LOG_DEBUG("run\n");
	while(1)
	{
		LOG_DEBUG("%s count:%d\n", __FUNCTION__, cnt++);
        OSAL_sleep(500);

        {/*{{{*/
            u8Key = keys[keyInx++];
            keyInx %= MAXKEYCNT;
            {
                LOG_DEBUG("recvkey:%c(0x%02x)\n", u8Key, u8Key);
                u32Cmd = MAX_CMD_UI;
                switch(u8Key)
                {
                    case 's':	u32Cmd = CMD_UI_KEY_CENTER;	break;
                    case 'w':	u32Cmd = CMD_UI_KEY_UP;		break;
                    case 'x':	u32Cmd = CMD_UI_KEY_DOWN;	break;
                    case 'a':	u32Cmd = CMD_UI_KEY_LEFT;	break;
                    case 'd':	u32Cmd = CMD_UI_KEY_RIGHT;	break;
                    default: LOG_DEBUG("Unknown key('%c')\n", u8Key); break;
                }
                if( u32Cmd != MAX_CMD_UI)
                {
                    AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
                }
            }
        }/*}}}*/
    }

    LOG_DEBUG("exit\n");
}

extern const char *psTaskName[];
extern TaskParam_t gstTaskParam[];
void BGTestTask_uartcon(void *pdata)
{
	uint32 u32Cmd;
	uint32 cnt = 0;
	uint32 stick, etick, diff;
	sint32 i;

	LOG_DEBUG("uart console test task start\n");

	u32Cmd = CMD_DBG_PRINT_MSG_ON;
    AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_MONITOR, 0, 0, 0, 1000);

    stick = GetTickCount();
    while(1){
        if(!cnt%0x100) LOG_DEBUG("uart console test task.count: 0x%x\n", cnt++);
        OSAL_sleep(1);

    	etick = GetTickCount();
    	if((diff = (uint32)(etick - stick)) > 1000){
    		for(i=0;i<MAX_TASK;i++){
    			LOG_DEBUG("[%s]loop cnt: %d / s\n", psTaskName[i], gstTaskParam[i].u32TaskLoopCnt*1000/diff);
    			//printf("[%s]loop cnt: %d / s\n", psTaskName[i], gstTaskParam[i].u32TaskLoopCnt*1000/diff);
    			gstTaskParam[i].u32TaskLoopCnt = 0;
    		}

    		stick = GetTickCount();

    	}
    }

    LOG_DEBUG("exit\n");
}

void edge_doneCB(void)
{
	u32CannyEdgeInterruptCnt++;
}

void BGTestTask_cannyedge(void *pdata)
{
	PPDRV_CANNYEDGE_InitializeByConfig(3);
	PPDRV_CANNYEDGE_SetDoneHandler(edge_doneCB);
	PPDRV_CANNYEDGE_SetEnable(PP_TRUE);

	while(1){
		OSAL_sleep(1000);
		LOG_DEBUG("%s : %d\n", "Cannyedge intr count", u32CannyEdgeInterruptCnt);
	}
}

void BGTestTask__delete_cannyedge(void)
{
	PPDRV_CANNYEDGE_SetEnable(PP_FALSE);
}

void BGTestTask_fast(void *pdata)
{
	uint32 cnt = 0;
	sint32 i;

	LOG_DEBUG("fast test task start\n");


    {/*{{{*/
        _VPUStatus stVPUStatus;
//        int runInx = 0;
        int runMax = 0;
        USER_VPU_FB_CONFIG_U stVPUFBConfig;
        uint32 u32TimeOut;
        int zoneNum = 0;
//        PP_U32 u32FCEndPos;
//        int lastFrmInx = 0;
//        PP_U32 *pu32BufFast;
//        PP_U32 *pu32BufBrief;
//        PP_U16 *pu16BufFC;
//        uint16 u16FCEndValue;
       // int i;
//        PP_U32 slooptick, elooptick;

        // set fast config
        {
            PPAPI_VPU_FAST_GetConfig(&stVPUFBConfig);
            //stVPUFBConfig.fld.fast_n = FAST_PX_NUM;
            //stVPUFBConfig.fld.ch_sel = eVpuChannel;
            //stVPUFBConfig.fld.use_5x3_nms = FALSE;
            //stVPUFBConfig.fld.brief_enable = TRUE;
            //stVPUFBConfig.fld.roi_enable = FALSE;
            //stVPUFBConfig.fld.scl_enable = bScaleEnable;
            stVPUFBConfig.fld.dma2otf_enable = FALSE;
            stVPUFBConfig.fld.otf2dma_enable = FALSE;
            PPAPI_VPU_FAST_SetConfig(&stVPUFBConfig);
        }

        LOG_DEBUG("VPU_FAST start\n");

		runMax = -1;
		u32TimeOut = 1000;
		PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.

        while(1)
        {
            //slooptick = GetTickCount();

            LOG_DEBUG("F");
            if( (PPAPI_VPU_WaitFrmUpdate(1000)) == eSUCCESS)
            {

                PPAPI_VPU_GetStatus(&stVPUStatus);
#if 0
                //print HW info
                {/*{{{*/
                    uint32_t i, j, k, inxFrm;

                    inxFrm = stVPUStatus.stFrmInx.inxFrmPrev;
                    //for (j = 0; j < eVPU_FRAME_MAX; j++)
                    {
                        //for (i = 0; i < TOTAL_ZONE_NUM; i++)
                        for (i = 0; i < 1; i++)
                        {
                            LOG_DEBUG("Print FAST result of VPU.(Zone:%d)\n", i);
                            pu32BufFast = (uint32 *)stVPUStatus.ppVBufFast[inxFrm][i];
                            pu32BufBrief = (uint32 *)stVPUStatus.ppVBufBrief[inxFrm][i];
                            pu16BufFC = (uint16 *)stVPUStatus.ppVBufFC[inxFrm][i];

                            u32FCEndPos = stVPUStatus.pVpuZoneParam[i].fc_height;
                            LOG_DEBUG("   endpos:%d, endFcValue:%d\n", u32FCEndPos, pu16BufFC[u32FCEndPos-1]);

                            for (k = 0; k < pu16BufFC[u32FCEndPos-1]; k++)
                            {
                                uint32_t x, y;
                                uint32_t *d = (uint32_t *)&pu32BufBrief[k*4];
                                x = ((pu32BufFast[k] & 0xFFFF0000) >> 16);
                                y = ((pu32BufFast[k] & 0x0000FFFF));
                                LOG_DEBUG ("HW[%d]Zone[%d]  %d ( %d,  %d) = %08x%08x%08x%08x\n", inxFrm, i, k, x, y, d[3], d[2], d[1], d[0]);
                            }
                        }
                    }

                }/*}}}*/
#endif

                LOG_DEBUG("H");
                PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000);

#if 0
                //LOG_DEBUG("Print&Draw HamminD result of VPU.\n");
                {/*{{{*/
                    uint32_t u32TotalMatchCnt = 0;
                    pVPU_MATCHING_RESULT_POS_T hw_HD;
                    for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
                    {
                        u32TotalMatchCnt += stVPUStatus.u32HDMatchResultCount[zoneNum];

                        //LOG_DEBUG("zone:%d-%d\n", zoneNum, stVPUStatus.u32HDMatchResultCount[zoneNum]);
                        hw_HD = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[zoneNum];
                        for (i = 0; i < stVPUStatus.u32HDMatchResultCount[zoneNum]; i++)
                        {
                            /*
                               LOG_DEBUG ("HW (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                               hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
                               LOG_DEBUG ("Draw (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                               hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);

                             */
                        }

                    }

//                    LOG_DEBUG("TotalMatchCnt:%d\n", u32TotalMatchCnt);
                }/*}}}*/
#endif
            }

            //elooptick = GetTickCount();
            //LOG_DEBUG("- %d\n", elooptick - slooptick);
        }
        LOG_DEBUG("stop fast\n");
        PPAPI_VPU_FAST_Stop();
    }/*}}}*/

    LOG_DEBUG("exit\n");
}

void BGTestTask__delete_fast(void)
{
    LOG_DEBUG("stop fast\n");
    PPAPI_VPU_FAST_Stop();
}

enum {
    bgTestTask_base = 0,
	bgTestTask_vpudma,
	bgTestTask_uikey,
	bgTestTask_uartcon,
	bgTestTask_cannyedge,
	bgTestTask_fast,
	MAX_bgTempTask
}ebgTest;
typedef struct {
	char strExeName[40];
	char strTaskName[40];
	void (*fh)(void*);
	TaskHandle_t hTask;
	int status; //0:stop, 1:start
}_bgTestTask;

static _bgTestTask stBgTestTask[MAX_bgTempTask] = 
{
	{ .strExeName = "base", .strTaskName =  "BGTestTask_base", .fh = BGTestTask_base, .hTask = NULL },
	{ .strExeName = "vpudma", .strTaskName =  "BGTestTask_vpudma", .fh = BGTestTask_vpudma, .hTask = NULL },
	{ .strExeName = "uikey", .strTaskName =  "BGTestTask_uikey", .fh = BGTestTask_uikey, .hTask = NULL },
	{ .strExeName = "uartcon", .strTaskName =  "BGTestTask_uartcon", .fh = BGTestTask_uartcon, .hTask = NULL },
	{ .strExeName = "cannyedge", .strTaskName =  "BGTest_cannyedge", .fh = BGTestTask_cannyedge, .hTask = NULL },
	{ .strExeName = "fast", .strTaskName =  "BGTestTask_fast", .fh = BGTestTask_fast, .hTask = NULL },
};

int _bgtest(int argc, const char **argv)
{
	int ret = 0;

	int i;
	int taskNum = 0;

	LOG_DEBUG("\n");

	if(argc == 1)
	{
		for(i = 0; i < MAX_bgTempTask; i++)
		{
			if( stBgTestTask[i].fh == NULL) continue;
			LOG_DEBUG("%d: exeName:%s bgTask:%s, status:%d(1:Start,0:Stop)\n",  i, stBgTestTask[i].strExeName, stBgTestTask[i].strTaskName, stBgTestTask[i].status);
		}
		return(0);
	}

	if(argc >= 2)
	{ 
		if( !strcmp(argv[1], "1") )
		{
			UBaseType_t uxPriority;
			uxPriority = uxTaskPriorityGet(NULL);

			for(i = 0; i < MAX_bgTempTask; i++)
			{
				if( stBgTestTask[i].fh == NULL) continue;
		        if( (argc == 2) || (!strcmp(argv[2], "all")) ) //do all bgtask
        		{
					taskNum = i;
					if( (xTaskCreate(stBgTestTask[taskNum].fh, stBgTestTask[taskNum].strTaskName, CONFIG_MINIMAL_STACK_SIZE, NULL, uxPriority, &stBgTestTask[taskNum].hTask)) != pdPASS)
					{
						LOG_DEBUG("Fail create task!.\n");
						return(-1);
					}
					stBgTestTask[taskNum].status = 1;
					LOG_DEBUG("Start bg test[%s] status:%d\n", stBgTestTask[taskNum].strTaskName, stBgTestTask[taskNum].status);
                }
                else if( argc == 3 )
                {
                    taskNum = simple_strtoul(argv[2], 10);
                    if( taskNum == i )
                    {
                        if( (xTaskCreate(stBgTestTask[taskNum].fh, stBgTestTask[taskNum].strTaskName, CONFIG_MINIMAL_STACK_SIZE, NULL, uxPriority, &stBgTestTask[taskNum].hTask)) != pdPASS)
                        {
                        	LOG_DEBUG("Fail create task!.\n");
                            return(-1);
                        }
                        stBgTestTask[taskNum].status = 1;
                        LOG_DEBUG("Start bg test[%s] status:%d\n", stBgTestTask[taskNum].strTaskName, stBgTestTask[taskNum].status);
                        break;
                    }
                }
			}
		}
		else if( !strcmp(argv[1], "0") )
		{
            for(i = 0; i < MAX_bgTempTask; i++)
            {
                taskNum = i;
                if(stBgTestTask[taskNum].hTask)
                {
                    vTaskDelete(stBgTestTask[taskNum].hTask);
                    printf("deleted task\n");
                    stBgTestTask[taskNum].status = 0;
                    printf("Delete bg test[%s] status:%d\n", stBgTestTask[taskNum].strTaskName, stBgTestTask[taskNum].status);
	                if(taskNum == bgTestTask_uartcon)
                    {
                        AppTask_SendCmd(CMD_DBG_PRINT_MSG_OFF, TASK_CLI, TASK_MONITOR, 0, 0, 0, 1000);
                    }

					if(taskNum == bgTestTask_cannyedge )
					{
						BGTestTask__delete_cannyedge();
					}

					if(taskNum == bgTestTask_fast )
					{
						BGTestTask__delete_fast();
					}
                }
			}
		}
		else
		{
			LOG_DEBUG("Invalid!.\n");
			return(-1);
		}
	}


	return(ret);
}

int _proc(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;
	int procArgc = 0;
	const char **procArgv = NULL;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	strArgv = (char *)argv[1];

	if( (argc >= 3) && ((char *)argv[2] != NULL))
	{
		procArgv[procArgc] = argv[2];
		procArgc++;
		LOG_DEBUG("proc argc:%d, %s\n", procArgc, procArgv[procArgc-1]);
	}

	if( !strncmp(strArgv, "irq", sizeof("irq")) )
	{
		while (!drv_uart_is_kbd_hit()) 
		{
			LOG_DEBUG("current tick:%d\n", GetTickCount());
			SYS_PROC_PrintIrq(procArgc, procArgv);

			LOG_DEBUG("\r\nany key exit..\n\n");

			_mdelay(1000);
		}
	}
	else if( !strncmp(strArgv, "device", sizeof("device")) )
	{
		while (!drv_uart_is_kbd_hit()) 
		{
			SYS_PROC_PrintDevice(procArgc, procArgv);

			LOG_DEBUG("\r\nany key exit..\n\n");

			_mdelay(1000);
		}
	}
	else
	{
	}

	return(ret);
}

int _event(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;

	LOG_DEBUG("\n");

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if( (strArgv = (char *)argv[1]) == NULL)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

#if 0
	{
#include "event.h"
#include "event_queue.h"

		UI_EVENT event = EVENT_NONE;
		event = strArgv[0];

		LOG_DEBUG("%s send event:%c\n", __FUNCTION__, event);
		SendEvent(event, 0);
	}
#endif
#if 0
	{/*{{{*/
		uint32 u32Cmd = 0xf0123456;
		while (!drv_uart_is_kbd_hit()) 
		{

			if(ghQueueCmd.phQueue[QUEUE_CMDTEST])
			{
				queueCmdParam queueParam;
				memset(&queueParam, 0, sizeof(queueCmdParam));
				queueParam.u32Cmd = u32Cmd++;
				queueParam.u8Sender = QUEUE_CMDSYSTEM;
				queueParam.u16Attr = (1<<QUEUE_CMDATTR_BIT_REQACK);
				queueParam.u8Length = 3;
				queueParam.u8Params[0] = 10;
				queueParam.u8Params[1] = 21;
				queueParam.u8Params[2] = 32;
				if( OSAL_QUEUE_CMD_Send(ghQueueCmd.phQueue[QUEUE_CMDTEST], &queueParam, 0) != eSUCCESS )
				{
					LOG_DEBUG("Fail Send queue-CMD\n");

				}

			}
			LOG_DEBUG("\r\nany key exit..\n\n");

			_mdelay(100);
		}
	}/*}}}*/
#endif

	return(ret);
}

#include "application.h"
int _fwdn(int argc, const char **argv)
{
	int ret = 0;

#if(UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)
	FWDN_FLASH_UPDATE_S stFWDNParam[FLASH_UPDATE_SECTION_MAX];
	PP_U32 u32ElementCnt;

	if(PPAPP_UPGRADE_GetParam(stFWDNParam, FLASH_UPDATE_SECTION_MAX, &u32ElementCnt) == eSUCCESS){
		AppTask_SendCmd(CMD_UI_FWDN, TASK_CLI, TASK_UI, 0, stFWDNParam, sizeof(FWDN_FLASH_UPDATE_S)*u32ElementCnt, 3000);
	}
#elif(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
	AppTask_SendCmd(CMD_UI_FWDN, TASK_CLI, TASK_UI, 0, NULL_PTR, 0, 3000);
#endif

	return(ret);
}


static int ls(const char *path)
{


	PP_FNFIND_S stFind;
	uint8 spaces;
	sint8 filename[256];
	sint32 ret = 1;

	if(path[strlen(path)-1] == '/')
		sprintf(filename, "%s*.*", path);
	else
		sprintf(filename, "%s/*.*", path);

	if(!PPAPI_FATFS_FindFirst(filename, &stFind)){
		do{
			// shows max 64 characters
			if(strlen(stFind.szFileName) > 64){
				stFind.szFileName[63] = '~';
				stFind.szFileName[64] = 0;
			}
			spaces = 32 - strlen(stFind.szFileName);
			LOG_DEBUG("%s%c", stFind.szFileName, (stFind.pu8Attr & FATFS_ATTR_DIR) ? '/' : ' ');
			while(spaces--)LOG_DEBUG(" ");
			LOG_DEBUG("%d\n", stFind.u32FileSize);
		}while(!PPAPI_FATFS_FindNext(&stFind));
		//LOG_DEBUG("findnext end(%d)\n", PPAPI_FATFS_GetLastError());
	}else{
		LOG_DEBUG("Findfirst fail(%d)\n", PPAPI_FATFS_GetLastError());
		ret = -1;
	}

	return ret;
}

static void fileread(const char *filename, uint32 addr, uint32 size)
{
	PP_VOID *fh = NULL;
	uint32 stick, etick;

	if((fh = PPAPI_FATFS_Open(filename,"r")) == NULL){
		LOG_DEBUG("File open fail(%d)\n", PPAPI_FATFS_GetLastError());
		return;
	}

	stick = GetTickCount();
	if(PPAPI_FATFS_Read((PP_VOID *)addr, 1, size, fh) != size){
		LOG_DEBUG("File read fail(%d)\n", PPAPI_FATFS_GetLastError());
	}else{
		etick = GetTickCount();
		LOG_DEBUG("File read done. addr: 0x%x, size: %d, %d ms(%d KBytes/s)\n", addr, size, (etick-stick), (etick-stick) ? size / (etick-stick) * 1000 / 1024 : 0);
	}
	print_hex((uint8 *)addr, (size > 256) ? 256 : size);

	if(fh){
		PPAPI_FATFS_Close(fh);
	}
}

static void filewrite(const char *filename, uint32 addr, uint32 size)
{
	PP_VOID *fh = NULL;
	uint32 stick, etick;

	if((fh = PPAPI_FATFS_Open(filename,"w")) == NULL){
		LOG_DEBUG("File open fail(%d)\n", PPAPI_FATFS_GetLastError());
		return;
	}

	stick = GetTickCount();
	if(PPAPI_FATFS_Write((PP_VOID *)addr, 1, size, fh) != size){
		LOG_DEBUG("File write fail(%d)\n", PPAPI_FATFS_GetLastError());
	}else{
		etick = GetTickCount();
		LOG_DEBUG("File write done. addr: 0x%x, size: %d, %d ms(%d KBytes/s)\n", addr, size, (etick-stick), size / (etick-stick) * 1000 / 1024);
	}
	if(fh){
		PPAPI_FATFS_Close(fh);
	}
}


int _sdfs(int argc, const char **argv)
{
	PP_RESULT_E ret = 0;
	int i;

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1], "mount") == 0){
		if(PPAPI_FATFS_InitVolume()){
			LOG_DEBUG("mount fail(%d)\n", PPAPI_FATFS_GetLastError());
		}else{
			LOG_DEBUG("mounted\n");
		}
	}else if(strcmp(argv[1], "umount") == 0){
		if(PPAPI_FATFS_DelVolume()){
			LOG_DEBUG("unmount fail(%d)\n", PPAPI_FATFS_GetLastError());
		}else{
			LOG_DEBUG("unmounted\n");
		}
	}else if(strcmp(argv[1], "ls") == 0){
		if(argc < 3){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		if(ls(argv[2]) < 0){
			LOG_DEBUG("ls fail\n");
		}
	}else if(strcmp(argv[1], "read") == 0){
		uint32 addr, size;

		if(argc < 5){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);
		if(addr < 0x20000000 || addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", addr);
			return(-1);
		}

		fileread(argv[2], addr, size);
	}else if(strcmp(argv[1], "write") == 0){
		uint32 addr, size;

		if(argc < 5){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);
		if(addr < 0x20000000 || addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", addr);
			return(-1);
		}
		filewrite(argv[2], addr, size);
	}else if(strcmp(argv[1], "check") == 0){
		if(!PPAPI_FATFS_CheckVolume()){
			LOG_DEBUG("Volume is working\n");
		}else{
			LOG_DEBUG("Volume is not usable(%d)\n", PPAPI_FATFS_GetLastError());
		}
	}else if(strcmp(argv[1], "fattest") == 0){
		ret = PPAPI_FATFS_EnterFS();
		if(ret)LOG_DEBUG("EnterFS Fail(%d)\n", ret);

		PPAPI_FATFS_DoTest();

        PPAPI_FATFS_ReleaseFS();

	}else if(strcmp(argv[1],"cfg") == 0){
		uint8 buf[16];
		uint8 c;
		void *handle;
		handle = PPAPI_FATFS_Open("flash.cfg","w");
		if(!handle){
			LOG_DEBUG("can not open file\n");
			return -1;

		}

		i=0;
		int val;
		while(PPAPI_FATFS_Read(&c, 1, 1, handle) > 0){
			if(c == '['){
				i = 0;
				//mode = 1;
			}else if(c == ']'){
				//mode = 2;
				buf[i] = 0;
				LOG_DEBUG("%s\n", buf);

			}else if(c == '='){
				i = 0;
				//mode = 1;
			}else if(c == '\n'){
				//mode = 3;
				buf[i] = 0;
				sscanf((const char *)buf, "%d", &val);
				LOG_DEBUG("%s, %d\n", buf, val);

			}else if(c == '\r'){

			}else{
				buf[i++] = c;
			}

		}
		PPAPI_FATFS_Close(handle);
	}

	return(1);
}


int _remoconkey(int argc, const char **argv)
{
	int ret = 0;
    uint32 u32Cmd;
	uint8 u8Key = 0;

	LOG_DEBUG("\n");

	if(argc < 1)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

    {/*{{{*/
        LOG_DEBUG("\r\n'Q' key exit..\n\n");
        while( (u8Key = drv_uart_get_char()) != 'Q')
        {
            LOG_DEBUG("recvkey:%c(0x%02x)\n", u8Key, u8Key);
            u32Cmd = MAX_CMD_UI;
			if(u8Key == 'p')
			{
				u32Cmd = CMD_DISPLAY_PGL_ANGLE_TEST;
				if( u32Cmd != MAX_CMD_UI)
	            {
	                AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_DISPLAY, 0, 0, 0, 100);
	            }
			}
			else
			{
			    switch(u8Key)
			    {
                    case 's':	u32Cmd = CMD_UI_KEY_CENTER;	break;
                    case 'w':	u32Cmd = CMD_UI_KEY_UP;		break;
                    case 'x':	u32Cmd = CMD_UI_KEY_DOWN;	break;
                    case 'a':	u32Cmd = CMD_UI_KEY_LEFT;	break;
                    case 'd':	u32Cmd = CMD_UI_KEY_RIGHT;	break;
				    case 'm':	u32Cmd = CMD_UI_KEY_MENU;	break;
				    case 'y':	u32Cmd = CMD_UI_TRIGGER_REVERSE;	break;
				    case 'u':	u32Cmd = CMD_UI_TRIGGER_TURN;		break;
				    case 'i':	u32Cmd = CMD_UI_TRIGGER_EMERGENCY;	break;
				    case 'o':	u32Cmd = CMD_UI_TRIGGER_DRIVE;		break;
					case 'g':	u32Cmd = CMD_UI_CAR_DOOR;			break;
				    
                    default: LOG_DEBUG("Unknown key('%c')\n", u8Key); break;
                }
                if( u32Cmd != MAX_CMD_UI)
                {
                    AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
                }
			}

            LOG_DEBUG("\r\n'Q' key exit..\n\n");
        }
    }/*}}}*/

	return(ret);
}

int _viewmode(int argc, const char **argv)
{
	PP_S32 view_mode;

	if(argc < 1)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	view_mode = simple_strtoul(argv[1], 0);
	PPAPI_SVM_SetView((PP_VIEWMODE_E)view_mode,PP_NULL , PP_NULL);
#ifdef USE_PP_GUI
	PPAPP_View_ChangeImage((PP_VIEWMODE_E)view_mode);
#endif

	return 0;
}
int _calib(int argc, const char **argv)
{
	int ret = 0;
//	uint32 u32Cmd;
//	uint8 u8Key = 0;

	LOG_DEBUG("\n");
	LOG_DEBUG("\r\nCalibration Test !!!\n\n");

#if defined(CALIB_LIB_USE)

#else
	printf("\r\nNo define CALIB_LIB_USE !!!\n\n");
#endif

	return(ret);
}

int _flashop(int argc, const char **argv)
{
	int ret = 0;
	uint32 flash_addr = 0;
	uint32 dram_addr = 0;
	uint32 block_addr = 0;
	uint32 page_addr = 0;
	uint32 size = 0;
	uint8 *pBuf = NULL;
	uint32 stick = 0, etick = 0;

	if(argc < 2){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}


	if(strcmp(argv[1], "init") == 0){
		if(argc < 3){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		if(strcmp(argv[2], "nor") == 0){
			if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NOR) != eSUCCESS){
				LOG_DEBUG("ERROR!!! Flash init fail. Halt\n");
				return(-1);
			}
		}else if(strcmp(argv[2], "nand") == 0){
			if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NAND) != eSUCCESS){
				LOG_DEBUG("ERROR!!! Flash init fail. Halt\n");
				return(-1);
			}
		}else{
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

	}else if(strcmp(argv[1], "erase") == 0){
		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		flash_addr = simple_strtoul(argv[2], 0);
		size = simple_strtoul(argv[3], 0);

		LOG_DEBUG("flash erasing at 0x%x, size: 0x%x\n", flash_addr, size);
		stick = GetTickCount();
		PPAPI_FLASH_Erase(flash_addr, size);

		etick = GetTickCount();
		LOG_DEBUG("flash erase done size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);
	}else if(strcmp(argv[1],"nand_erase_all_force") == 0){
		int i;

		for(i=0;i<gstFlashNandID.u32TotalSize/gstFlashNandID.u32EraseBlockSize;i++)
			PPDRV_SNAND_FLASH_EraseBlock(i);

	}else if(strcmp(argv[1], "readdma") == 0){
		if(argc < 5){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		flash_addr = simple_strtoul(argv[2], 0);
		dram_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		pBuf = (uint8 *)dram_addr;
		LOG_DEBUG("flash reading at 0x%x, size: 0x%x\n", flash_addr, size);
		stick = GetTickCount();
		if(PPAPI_FLASH_ReadQDMA(pBuf, flash_addr, size) == eSUCCESS){
			etick = GetTickCount();
			print_hex(pBuf, 64);
			LOG_DEBUG("crc16: 0x%x, 0x%x\n", PPAPI_FLASH_GetCRC16(), CalcCRC16IBM(pBuf, size));
			LOG_DEBUG("flash readdma done size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		}else{
			LOG_DEBUG("flash read fail\n");
		}
	}else if(strcmp(argv[1], "read") == 0){
		if(argc < 5){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		flash_addr = simple_strtoul(argv[2], 0);
		dram_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);

		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("flash reading at 0x%x, size: 0x%x\n", flash_addr, size);
		stick = GetTickCount();
		PPAPI_FLASH_Read(pBuf, flash_addr, size);
		etick = GetTickCount();
		print_hex(pBuf, 64);
		LOG_DEBUG("crc16: 0x%x, 0x%x\n", PPAPI_FLASH_GetCRC16(), CalcCRC16IBM(pBuf, size));
		LOG_DEBUG("flash read done size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

	}else if(strcmp(argv[1], "write") == 0){
		if(argc < 5){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		flash_addr = simple_strtoul(argv[2], 0);
		dram_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);

		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("flash writing at 0x%x, size: 0x%x\n", flash_addr, size);
		stick = GetTickCount();
		PPAPI_FLASH_Write(pBuf, flash_addr, size);
		//PPAPI_FLASH_WriteErasedBlock(pBuf, flash_addr, size);
		etick = GetTickCount();
		print_hex(pBuf, 64);
		LOG_DEBUG("flash writing done\n");
		LOG_DEBUG("flash write done size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);
	}else if(strcmp(argv[1], "readspare") == 0){	// Todo: should be removed. Temp for nand flash test
		if(argc < 6){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		dram_addr = simple_strtoul(argv[4], 0);
		size = simple_strtoul(argv[5], 0);
		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("flash reading spare at block: 0x%x, page: 0x%x, size: 0x%x\n", block_addr, page_addr, size);
		PPDRV_SNAND_FLASH_ReadSpare(pBuf, block_addr, page_addr, 0, size);
		print_hex(pBuf, size);
		LOG_DEBUG("crc16: 0x%x, 0x%x\n", PPAPI_FLASH_GetCRC16(), CalcCRC16IBM(pBuf, size));
	}else if(strcmp(argv[1], "writespare") == 0){	// Todo: should be removed. Temp for nand flash test
		if(argc < 6){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		dram_addr = simple_strtoul(argv[4], 0);
		size = simple_strtoul(argv[5], 0);
		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("flash writing spare at block: 0x%x, page: 0x%x, size: 0x%x\n", block_addr, page_addr, size);
		PPDRV_SNAND_FLASH_WriteSpare(pBuf, block_addr, page_addr, 0, size);
		print_hex(pBuf, size);
		LOG_DEBUG("crc16: 0x%x, 0x%x\n", PPAPI_FLASH_GetCRC16(), CalcCRC16IBM(pBuf, size));
	}else if(strcmp(argv[1], "readcache") == 0){
		if(argc < 5){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		flash_addr = simple_strtoul(argv[2], 0);
		dram_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);
		pBuf = (uint8 *)dram_addr;

		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		spi_memctrl_read(flash_addr, pBuf, size);
		print_hex(pBuf, (size > 64) ? 64 : size);
	}else if(strcmp(argv[1], "getfeature") == 0){
		if(argc < 3){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		flash_addr = simple_strtoul(argv[2], 0);
		LOG_DEBUG("feature(0x%x): 0x%x\n", flash_addr, PPDRV_SNAND_FLASH_GetFeature(flash_addr));
	}else if(strcmp(argv[1], "setfeature") == 0){
		uint32 value;
		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		flash_addr = simple_strtoul(argv[2], 0);
		value = simple_strtoul(argv[3], 0);
		PPDRV_SNAND_FLASH_SetFeature(flash_addr, value);
		LOG_DEBUG("feature(0x%x): 0x%x\n", flash_addr, PPDRV_SNAND_FLASH_GetFeature(flash_addr));
	}else if(strcmp(argv[1], "checkbad") == 0){
		uint32 start_block;
		uint32 end_block;
		uint32 bad_cnt = 0;
		uint32 i;

		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		start_block = simple_strtoul(argv[2], 0);
		end_block = simple_strtoul(argv[3], 0);

		for(i=start_block;i<end_block;i++){
			if(PPDRV_SNAND_FLASH_CheckBad(i)){
				bad_cnt++;
				LOG_DEBUG("%d th block bad\n", i);
			}
		}
		LOG_DEBUG("%d bad blocks found\n", bad_cnt);

	}else if(strcmp(argv[1], "markbad") == 0){	// Todo: should be removed. Temp for nand flash test
		uint8 tmpbuf[4] = {0x00, 0xff, 0xff, 0xff};

		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);

		LOG_DEBUG("write bad mark block: 0x%x, page: 0x%x\n", block_addr, page_addr);
		PPAPI_FLASH_Erase(block_addr*gstFlashNandID.u32EraseBlockSize, gstFlashNandID.u32EraseBlockSize);
		PPDRV_SNAND_FLASH_WriteSpare(tmpbuf, block_addr, page_addr, 0, 4);
	}else if(strcmp(argv[1],"pattern") == 0){
		uint32 *buf = (uint32 *)0x28000000;
		int i;

		pBuf = (uint8 *)dram_addr;
		for(i=0;i<0x100000/4;i++){
			*(uint32 *)(buf + i) = 0x2000000 + i*4;
		}
		print_hex((uint8 *)buf, 256);
	}else if(strcmp(argv[1],"test") == 0){
		uint8 *pDst;
		uint32 offset[4] = {0x810, 0x820, 0x20080, 0x220a0};
		uint32 sub_size[4] = {0x10, 0x40, 0x2000, 0x40030};
		uint32 i;

		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		flash_addr = simple_strtoul(argv[2], 0);
		size = simple_strtoul(argv[3], 0);

		pBuf = (uint8 *)OSAL_malloc(size);
		pDst = (uint8 *)OSAL_malloc(size);
		if(pBuf == NULL){
			LOG_DEBUG("malloc fail\n");
			return(-1);
		}
		for(i=0;i<size/4;i++){
			//*(uint32 *)(pBuf + i*4) = i*4;
			*(uint32 *)(pBuf + i*4) = (i*0x55aa)/3 + i;
		}
		print_hex(pBuf, 256);

		// test 1 - rw full size
		LOG_DEBUG("\n=============================================================================\n");
		LOG_DEBUG("full size r/w test. flash addr: 0x%x, size: 0x%x\n", flash_addr, size);
		PPAPI_FLASH_Write(pBuf, flash_addr, size);
		PPAPI_FLASH_Read(pDst, flash_addr, size);
		LOG_DEBUG("Compare result\n");
		for(i=0;i<size;i++){
			if(*(uint8 *)(pBuf+i) != *(uint8 *)(pDst+i)){
				 LOG_DEBUG("Fail!!! Different data at %d\n", i);
				 print_hex((uint8 *)(pBuf+i), (size-i) > 32 ? 32 : (size-i));
				 print_hex((uint8 *)(pDst+i), (size-i) > 32 ? 32 : (size-i));
				 break;
			}
		}
		if(i == size){
			LOG_DEBUG("OK\n");
		}

#if 1
		// test 2 - rw partial
		LOG_DEBUG("\n=============================================================================\n");
		LOG_DEBUG("partial r/w test\n");
		PPAPI_FLASH_Erase(flash_addr, size);
		memset(pBuf, 0, size);
		memset(pDst, 0, size);
		PPAPI_FLASH_Write(pBuf, flash_addr, size);

		for(i=0;i<size/4;i++){
			*(uint32 *)(pBuf + i*4) = i*4;
		}

		for(i=0;i<4;i++){
			LOG_DEBUG("Flash write flash_addr: 0x%x, size: 0x%x\n",flash_addr + offset[i], sub_size[i]);
			PPAPI_FLASH_Write((uint8 *)(pBuf + offset[i]), flash_addr + offset[i], sub_size[i]);
		}

		for(i=0;i<offset[0];i++)pBuf[i] = 0;
		for(i=(offset[0]+sub_size[0]);i<offset[1];i++)pBuf[i] = 0;
		for(i=(offset[1]+sub_size[1]);i<offset[2];i++)pBuf[i] = 0;
		for(i=(offset[2]+sub_size[2]);i<offset[3];i++)pBuf[i] = 0;
		for(i=(offset[3]+sub_size[3]);i<size;i++)pBuf[i] = 0;


		for(i=0;i<4;i++){
			LOG_DEBUG("Flash read flash_addr: 0x%x, size: 0x%x\n",flash_addr + offset[i], sub_size[i]);
			PPAPI_FLASH_Read((uint8 *)(pDst + offset[i]), flash_addr + offset[i], sub_size[i]);
			print_hex((uint8 *)(pDst + offset[i]), (sub_size[i] > 32) ? 32 : sub_size[i]);
		}

		for(i=0;i<size;i++){
			if(*(uint8 *)(pBuf+i) != *(uint8 *)(pDst+i)){
				 LOG_DEBUG("Fail!!! Different data at %d\n", i);
				 print_hex((uint8 *)(pBuf+i), (size-i) > 32 ? 32 : (size-i));
				 print_hex((uint8 *)(pDst+i), (size-i) > 32 ? 32 : (size-i));
				 break;
			}
		}
		if(i == size){
			LOG_DEBUG("OK\n");
		}

		// test 3 - read partial using qdma
		LOG_DEBUG("\n=============================================================================\n");
		LOG_DEBUG("partial r/w test - dma\n");
		memset(pDst, 0, size);
		for(i=0;i<4;i++){
			LOG_DEBUG("Flash readdma flash_addr: 0x%x, size: 0x%x\n",flash_addr + offset[i], sub_size[i]);
			PPAPI_FLASH_ReadQDMA((uint8 *)(pDst + offset[i]), flash_addr + offset[i], sub_size[i]);
			print_hex((uint8 *)(pDst + offset[i]), (sub_size[i] > 32) ? 32 : sub_size[i]);
		}

		for(i=0;i<size;i++){
			if(*(uint8 *)(pBuf+i) != *(uint8 *)(pDst+i)){
				 LOG_DEBUG("Fail!!! Different data at %d\n", i);
				 print_hex((uint8 *)(pBuf+i), (size-i) > 32 ? 32 : (size-i));
				 print_hex((uint8 *)(pDst+i), (size-i) > 32 ? 32 : (size-i));
				 break;
			}
		}
		if(i == size){
			LOG_DEBUG("OK\n");
		}

#endif
		if(pBuf)OSAL_free(pBuf);
		if(pDst)OSAL_free(pDst);
	}else if(strcmp(argv[1],"test2") == 0){
		uint32 i;
		uint32 errpos;
		uint32 addr_init = 0x0;
		uint32 addr_end = 0x04000000;
		uint32 stick, etick;

		size = 0x100000;
		pBuf = (uint8 *)OSAL_malloc(size);

		// Erase

		for(i=addr_init;i<addr_end;i+=0x100000){
			LOG_DEBUG("Erase @0x%x/0x%x\n", i, addr_end);
			PPAPI_FLASH_Erase(i, 0x100000);
		}

		// Write
		for(i=addr_init;i<addr_end;i+=0x100000){
			LOG_DEBUG("Writing @0x%x/0x%x\n", i, addr_end);
			make_pattern((uint32 *)pBuf, 0x100000, i);
			PPAPI_FLASH_Write(pBuf, i, 0x100000);
		}

		// Read & Verify
		for(i=addr_init;i<addr_end;i+=0x100000){
			LOG_DEBUG("Read & Verifying @0x%x/0x%x\n", i, addr_end);
			PPAPI_FLASH_Read(pBuf, i, 0x100000);
			errpos = check_pattern((uint32 *)pBuf, 0x100000, i);
			if(errpos != (uint32)-1){
				LOG_DEBUG("Error!!! Wrong data @0x%x\n", (i+errpos));
			}
		}

		// Read DMA & Verify
		for(i=addr_init;i<addr_end;i+=0x100000){
			LOG_DEBUG("Read QDMA & Verifying @0x%x/0x%x\n", i, addr_end);
			stick = GetTickCount();
			PPAPI_FLASH_ReadQDMA(pBuf, i, 0x100000);
			etick = GetTickCount();
			errpos = check_pattern((uint32 *)pBuf, 0x100000, i);
			if(errpos != (uint32)-1){
				LOG_DEBUG("Error!!! Wrong data @0x%x\n", (i+errpos));
			}else{
				LOG_DEBUG("OK. Elapsed time: %d ms, %d KBPS\n", (etick - stick), 0x100000/(etick-stick));
			}
		}


		OSAL_free(pBuf);
	}else{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	return(ret);
}

#include "api_ipc.h"

static int ipc_pf_memcpy()
{
	uint8 *pMemSrc, *pMemDst;
	uint8 *pMemMid;
	uint32 cnt = 0;
	uint32 stick, etick;

	pMemSrc = (uint8 *)OSAL_malloc(32*1024);
	pMemDst = (uint8 *)OSAL_malloc(32*1024);
	pMemMid = (uint8 *)ADDR_NON_CACHEABLE((uint32)OSAL_malloc(32*1024));

	//while (!drv_uart_is_kbd_hit()){
	while(1){
		if(cnt == 0){
			stick = GetTickCount();
		}
		memcpy(pMemMid, pMemSrc, 32*1024);
		memcpy(pMemDst, pMemMid, 32*1024);

		if(++cnt == 100){
			etick = GetTickCount();
			LOG_DEBUG("memcpy time(%d): %d ms\n", cnt, etick - stick);
			cnt = 0;
		}
	}

    return(0);
}

int _ipctest(int argc, const char **argv)
{
//	int ret = 0;
//	int i;

	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

    //if(!PPAPI_IPC_CheckISPReady())
	if(0)
    {
        LOG_DEBUG("Error! Not yet ready core1.\n");
		return(-1);
    }

	if(strcmp(argv[1], "setprn") == 0){
		PPAPI_IPC_SetPrintMemory(1000);
	}
	if(strcmp(argv[1], "setdata") == 0){
		PPAPI_IPC_SetDataMemory(1000);
	}
	if(strcmp(argv[1], "temp") == 0){
		ipc_pf_memcpy();
	}
#if defined(CALIB_LIB_USE)
	if(strcmp(argv[1], "viewgen") == 0){
		PPAPI_IPC_Core0_Viewgen_Send_SVMcnf(0xffffffff);
		PPAPI_IPC_Core0_Viewgen_Send_OffCalibcnf(0xffffffff);
	}
#endif
	if(strcmp(argv[1], "bc") == 0){
		if(argc < 3)
		{
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		if(strcmp(argv[2], "on") == 0){
			LOG_DEBUG("result: %d\n", PPAPI_IPC_SetBrightCtrl(1, 1000));
		}else if(strcmp(argv[2], "off") == 0){
			LOG_DEBUG("result: %d\n", PPAPI_IPC_SetBrightCtrl(0, 1000));
		}
	}

	//[camcon] [cnt]
	if(strcmp(argv[1], "camcon") == 0){
		int cnt;
		int i;

		PP_CAM_CONTROL_S stCon[4] = {
				{.u32Target = 0, .u32Channel=0, .u32OPType = 0, .u32SlaveId = 0x30, .u32RegAddr = 0x4D, .u32RegVal = 0},
				{.u32Target = 0, .u32Channel=1, .u32OPType = 0, .u32SlaveId = 0x30, .u32RegAddr = 0x4D, .u32RegVal = 0},
				{.u32Target = 1, .u32Channel=0xffffffff, .u32OPType = 0, .u32SlaveId = 0x58, .u32RegAddr = 0x0c, .u32RegVal = 0},
				{.u32Target = 2, .u32Channel=0, .u32OPType = 1, .u32SlaveId = 0x10, .u32RegAddr = 0x300E, .u32RegVal = 0xEE},
		};

		if(argc < 3)
		{
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}


#if 1
		cnt = simple_strtoul(argv[2], 0);
		if(cnt == 0 || cnt > 4){
			LOG_DEBUG("Invalid usage\n");
			return (-1);
		}


		for(i=0;i<4;i++)
			LOG_DEBUG("stCon[%d]: 0x%x\n", i, (PP_U32)&stCon[i]);

		if(PPAPI_IPC_CamControl(stCon, cnt, SYS_OS_SUSPEND) == eSUCCESS){
			for(i=0;i<cnt;i++){
				LOG_DEBUG("target: 0x%x, ch: 0x%x, op: 0x%x, slave id: 0x%x, reg addr: 0x%x, regval: 0x%x\n",
						stCon[i].u32Target, stCon[i].u32Channel, stCon[i].u32OPType, stCon[i].u32SlaveId, stCon[i].u32RegAddr, stCon[i].u32RegVal);
				LOG_DEBUG("ret: 0x%x, 0x%x, 0x%x, 0x%x, err: 0x%x, 0x%x, 0x%x, 0x%x\n",
						stCon[i].u32RetVal[0], stCon[i].u32RetVal[1], stCon[i].u32RetVal[2], stCon[i].u32RetVal[3],
						stCon[i].u32RetError[0], stCon[i].u32RetError[1], stCon[i].u32RetError[2], stCon[i].u32RetError[3]);
			}


		}else{
			LOG_DEBUG("Error\n");
		}
#endif
	}
    if(strcmp(argv[1], "camst") == 0)
    {
        PP_U32 u32CamCh, u32PlugIn;
        if(PPAPI_IPC_ReqCamStatus(&u32CamCh, &u32PlugIn, 3000)){
            LOG_DEBUG("Error!!! ReqCamStatus Fail\n");
        }else{
            LOG_DEBUG("ReqCamStatus. CamCh: 0x%x, PlugIn: 0x%x\n", u32CamCh, u32PlugIn);
        }
    }

    return 1;
}

int _gpio(int argc, const char **argv)
{
	int ret = 0;

//    uint32 u32Temp = 0;
//    sint32 dev = GPIO_DEV_0;
//    sint32 pin = 34;
    sint32 value = 0;

	LOG_DEBUG("\n");
#if 1
	if(argc < 2)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	value = simple_strtoul(argv[1], 0);
	PPDRV_GPIO_SetValue(0, 3, value);
	LOG_DEBUG("GPIO0_3: %d\n", PPDRV_GPIO_GetOutValue(0,3));
#else

     {/*{{{*/
         /* serializer reset(PDB). gpio pin gpio1[6] (EVM2.0) */
         PP_U32 u32GPIODev = 1;
         PP_U32 u32GPIOCh = 6;

         // pinmux sel
         set_pinmux(gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val);
         LOG_DEBUG("Serializer Tx PdB pinmux sel. group: %d, bitpos: %d, pinsel: %d\n", gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val );
         PPDRV_GPIO_SetDir(u32GPIODev, u32GPIOCh, eDIR_OUT, 0);


         LOG_DEBUG("Reset serializer\n");
         PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 0);
        _mdelay(500);

         LOG_DEBUG("Release Reset serializer\n");
         PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 1);

     }/*}}}*/
         
#endif	     
	return(ret);
}

int _top(int argc, const char **argv)
{
	int ret = 0;
	char *strArgv = NULL;

    char *pcWriteBuffer = NULL;
    int i;

	LOG_DEBUG("\n");

	if(argc < 1)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

    if( (strArgv = (char *)argv[1]) == NULL)
    {
		LOG_DEBUG("Invalid usage\n");
		return(-1);
    }

    if( (pcWriteBuffer = (char *)OSAL_malloc(1024)) == NULL)
    {
        LOG_DEBUG("Error! malloc fail\n");
        return(-1);
    }
    while (!drv_uart_is_kbd_hit()) 
    {
        {
            extern char __bss_start;
            extern char _end;
            uint32 size = 0;

            printf("cpu clock:%d, tick rate:%d\n", (uint32)CPU_CLK, (uint32)configTICK_RATE_HZ);
            size = &_end - &__bss_start;
            printf("bss_start:0x%08x, end:0x%08x, size:%d\n", (uint32)((uint32 *)&__bss_start), (uint32)((uint32 *)&_end), (uint32)size);
            printf("heap total:%d, curavail:%d, minimumeverfreeheap:%d\n", (uint32)configTOTAL_HEAP_SIZE, (uint32)xPortGetFreeHeapSize(), (uint32)xPortGetMinimumEverFreeHeapSize());
            printf("stack minimal size:%d\n", (uint32)configMINIMAL_STACK_SIZE);
            printf("\n\n");
        }

        { 
            printf("Print task status. Used for debug purposes only.===>\n");
            printf("%16s %8s %8s %8s %8s\n", "taskname", "state", "priority", "availstk", "tasknum");
            //set both configUSE_TRACE_FACILITY & configUSE_STATS_FORMATTING_FUNCTIONS in FreeRTOSConfig.h
            vTaskList(pcWriteBuffer);
            printf("%s\n", pcWriteBuffer);

            printf("Print collected run-time statistics. Used for debug purposes only.===>\n");
            printf("%16s %16s  %8s\n", "taskname", "used tick", "use%");
            //set both configGENERATE_RUN_TIME_STATS & configUSE_STATS_FORMATTING_FUNCTIONS in FreeRTOSConfig.h
            vTaskGetRunTimeStats(pcWriteBuffer);
            printf("%s\n", pcWriteBuffer);
        }

        { 
            printf("Print task statck status. Used for debug purposes only.===>\n");
            printf("%16s %16s\n", "taskname", "highwatermarkstack");
            for(i = 0; i < MAX_TASK; i++)
            {
                printf("%16s %16d\n", psTaskName[i], (uint32)uxTaskGetStackHighWaterMark(gHandle[i].phTask) );
            }
        }

        printf("\r\nany key exit..\n\n");

        _mdelay(1000);
    }
    OSAL_free(pcWriteBuffer);

	return(ret);
}

int _dumpreg(int argc, const char **argv)
{
	int ret = 0;

	LOG_DEBUG("\n");

	if(argc < 1)
	{
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

    PPAPI_SYSTEM_Fault();

	return(ret);
}

int _viewgen(int argc, const char **argv)
{
#if 0
	AppTask_SendCmd(CMD_UI_KEY_MENU, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	AppTask_SendCmd(CMD_UI_KEY_RIGHT, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	AppTask_SendCmd(CMD_UI_KEY_RIGHT, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	AppTask_SendCmd(CMD_UI_KEY_RIGHT, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	AppTask_SendCmd(CMD_UI_KEY_RIGHT, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	
	AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	
	AppTask_SendCmd(CMD_UI_KEY_DOWN, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	
	AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec
	
	
	AppTask_SendCmd(CMD_UI_KEY_LEFT, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
	OSAL_sleep(1000); //wait 100msec

	while(1)
	{

		do
		{
			AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			OSAL_sleep(1000); //wait 160sec
		}while(PPAPI_Section_Viewgen_Get_Step_Send_Cmd()!=eSVIEWGEN_WAIT_CMD);
		OSAL_sleep(10000); //wait 100msec

		
		AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
		OSAL_sleep(1000); //wait 100msec
		
		AppTask_SendCmd(CMD_UI_KEY_CENTER, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
		OSAL_sleep(1000); //wait 100msec
		
		AppTask_SendCmd(CMD_UI_KEY_LEFT, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
		OSAL_sleep(1000); //wait 100msec


	}
#endif	

	return 0;
}

int _reset(int argc, const char **argv)
{
	PPAPI_SYS_Reset();

	return 0;
}

int _nandtest(int argc, const char **argv)
{

	uint32 ret;
	PP_FTL_LL_PHY_S stLLPhy;
	uint8 *pBuf = NULL;
//	uint32 *ptr;
	int i;
//	int spare_size = 16;
//	uint32 flash_addr;
	uint32 block_addr, page_addr, dram_addr;
	uint32 stick, etick;
	uint32 size;
	SPI_MEM_FR_INFO fr_info;


	if(argc < 2){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1], "init") == 0){
		uint32 nand_model = 0;

		memset(&fr_info, 0, sizeof(fr_info));

		fr_info.flash_type = 1;		// 0: nor, 1: nand
		fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
		fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_0;
		fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_NONE;
		fr_info.mode_value = 0;
		fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;
		fr_info.addr_4byte = 0;
		spi_memctrl_set(0, 4, 0xEB, fr_info);
		//spi_memctrl_set(4, 2, 0xEB, fr_info);

		nand_model = simple_strtoul(argv[2], 0);
		ret = PPAPI_FTL_LL_Initialize(nand_model);
		if(ret)LOG_DEBUG("Error!!! FTL init fail(%d)\n", ret);

		ret = PPAPI_FTL_LL_GetPhy(&stLLPhy);
		if(ret)LOG_DEBUG("Error!!! FTL getphy fail(%d)\n", ret);
		else{
			LOG_DEBUG("nBlocks: %d\n", stLLPhy.u32Blocks);
			LOG_DEBUG("Pages per block: %d\n", stLLPhy.u32PagePerBlock);
			LOG_DEBUG("Page size: %d\n", stLLPhy.u32PageDataSize);
			LOG_DEBUG("Free Blocks: %d\n", stLLPhy.u32FreeBlocks);
			LOG_DEBUG("Reserved Blocks: %d\n", stLLPhy.u32ReservedBlocks);
		}

	}else if(strcmp(argv[1], "close") == 0){
		PPDRV_SPI_GPIOCSEnable(eSPI_CHANNEL_FLASH, 0);

	}else if(strcmp(argv[1], "erase") == 0){
		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		block_addr = simple_strtoul(argv[2], 0);
		size = simple_strtoul(argv[3], 0);

		LOG_DEBUG("Flash block erasing at %d, #block: %d\n", block_addr, size);
		for(i=0;i<size;i++){
			ret = PPAPI_FTL_LL_IsBadBlock(block_addr+i);
			if(ret == 0){
				ret = PPAPI_FTL_LL_Erase(block_addr+i);
				if(ret){
					printf("Error!!! FTL Erase Block %d fail(%d)\n", block_addr+i, ret);
					return(-1);
				}
			}else{
				LOG_DEBUG("Block %d is bad.(%d) Skip\n", block_addr + i, ret);
			}
		}
		printf("Erased\n");
	}else if(strcmp(argv[1], "checkbad") == 0){
		int badcnt = 0;
		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		block_addr = simple_strtoul(argv[2], 0);
		size = simple_strtoul(argv[3], 0);

		LOG_DEBUG("Factory marked flash bad block checking at %d, #block: %d\n", block_addr, size);
		for(i=0;i<size;i++){
			ret = PPAPI_FTL_LL_IsBadBlock(block_addr+i);
			if(ret == 0){
			}else{
				LOG_DEBUG("Block %d is bad.(%d) Skip\n", block_addr + i, ret);
				badcnt++;
			}
		}
		printf("%d bad blocks\n", badcnt);

	}else if(strcmp(argv[1], "write_pattern") == 0){
		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		if((pBuf = (uint8 *)OSAL_malloc(gstFlashNandID.u32PageSize + 16)) == NULL){
			LOG_DEBUG("malloc fail\n");
			return(-1);
		}

		for(i=0;i<gstFlashNandID.u32PageSize;i++){
			*(uint32 *)(pBuf + i*4) = i*4;
		}
		for(i=2048;i<gstFlashNandID.u32PageSize+16;i++){
			pBuf[i] = i;
		}

		LOG_DEBUG("Flash writing pattern at block: %d, page: %d\n", block_addr, page_addr);
		ret = PPAPI_FTL_LL_Write(block_addr, page_addr, pBuf, (uint8 *)(pBuf+2048));
		if(ret)printf("Error!!! FTL Write fail(%d)\n", ret);
		else printf("Writing done\n");

		OSAL_free(pBuf);
	}else if(strcmp(argv[1], "write_pattern1") == 0){
		if(argc < 4){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}

		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		if((pBuf = (uint8 *)OSAL_malloc(gstFlashNandID.u32PageSize + 16)) == NULL){
			LOG_DEBUG("malloc fail\n");
			return(-1);
		}

		for(i=0;i<gstFlashNandID.u32PageSize;i++){
			*(uint32 *)(pBuf + i*4) = i*4;
		}
		for(i=2048;i<gstFlashNandID.u32PageSize+16;i++){
			pBuf[i] = i*4;
		}

		LOG_DEBUG("Flash writing pattern at block: %d, page: %d\n", block_addr, page_addr);
		ret = PPAPI_FTL_LL_Write(block_addr, page_addr, pBuf, (uint8 *)(pBuf+2048));
		if(ret)printf("Error!!! FTL Write fail(%d)\n", ret);
		else printf("Writing done\n");

		OSAL_free(pBuf);
	}else if(strcmp(argv[1], "read") == 0){
		if(argc < 6){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);
		dram_addr = simple_strtoul(argv[5], 0);
		pBuf = (uint8 *)dram_addr;

		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("Flash reading at block: %d, page: %d, #pages: %d\n", block_addr, page_addr, size);
		stick = GetTickCount();
		for(i=0;i<size;i++){
			ret = PPAPI_FTL_LL_Read(block_addr, page_addr+i, (uint8 *)(pBuf+i*2048));
			if(ret && ret != 1){
				LOG_DEBUG("Error!!! FTL Read Fail(%d) at block: %d, page: %d\n", ret, block_addr, page_addr + i);
				return(-1);
			}
		}
		etick = GetTickCount();
		print_hex(pBuf, 64);
		LOG_DEBUG("flash read size: %d, %d ms(%d KBytes/s)\n", size*2048, (etick-stick), (etick - stick) ? size*2048 / (etick-stick) * 1000 / 1024 : 0);
	}else if(strcmp(argv[1], "readpart") == 0){
		char *opt_str[4] = {
				"1st half",
				"2nd_half",
				"data",
				"spare"
		};

		uint8 opt_flag[4] = {
				1,
				0,
				0xfe,
				0xff
		};
		if(argc < 6){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);	//option
		dram_addr = simple_strtoul(argv[5], 0);
		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("Flash part reading at block: %d, page: %d, %s\n", block_addr, page_addr, opt_str[size]);
		ret = PPAPI_FTL_LL_ReadPart(block_addr, page_addr, (uint8 *)(pBuf), opt_flag[size]);
		if(ret && ret != 1){
			LOG_DEBUG("Error!!! FTL Read Fail(%d) at block: %d, page: %d\n", ret, block_addr, page_addr);
			return(-1);
		}
		print_hex(pBuf, 64);

	}else if(strcmp(argv[1], "write") == 0){
		if(argc < 6){
			LOG_DEBUG("Invalid usage\n");
			return(-1);
		}
		block_addr = simple_strtoul(argv[2], 0);
		page_addr = simple_strtoul(argv[3], 0);
		size = simple_strtoul(argv[4], 0);
		dram_addr = simple_strtoul(argv[5], 0);
		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("Flash writing at block: %d, page: %d, #pages: %d\n", block_addr, page_addr, size);
		stick = GetTickCount();
		for(i=0;i<size;i++){
			ret = PPAPI_FTL_LL_Write(block_addr, page_addr+i, (uint8 *)(pBuf+i*2048), (uint8 *)(pBuf+i*2048+2048));
			if(ret)printf("Error!!! FTL Write fail(%d) at block: %d, page: %d\n", ret, block_addr, page_addr + i);
		}
		etick = GetTickCount();
		printf("Writing done\n");
		LOG_DEBUG("flash write size: %d, %d ms(%d KBytes/s)\n", size*2048, (etick-stick), (etick - stick) ?  size*2048 / (etick-stick) * 1000 / 1024 : 0);

	}else if(strcmp(argv[1], "lltest") == 0){
		memset(&fr_info, 0, sizeof(fr_info));

		fr_info.flash_type = 1;		// 0: nor, 1: nand
		fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
		fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_0;
		fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_NONE;
		fr_info.mode_value = 0;
		fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;
		fr_info.addr_4byte = 0;
		spi_memctrl_set(0, 4, 0xEB, fr_info);

		LOG_DEBUG("FTL DrvTest Start\n");
		ret = PPAPI_FTL_DrvTest();
		LOG_DEBUG("FTL DrvTest Finished(%d)\n", ret);


	}

	return 1;
}

static void Draw_1BIT_VPU_XY(PP_U8 *base, PP_U32 x, PP_U32 y, PP_U8 bValue)
{
#define VPU_DIS_WIDTH   (1280)
#define VPU_DIS_HEIGHT   (720)
    PP_U32 posBit = x + (y*VPU_DIS_WIDTH);
    PP_U32 posByte = (posBit/8);
    PP_U32 posByteBit = (posBit%8);
    PP_U8 *pBase = (PP_U8 *)base;

    if(bValue)
    {
        *(pBase + posByte) |= (1<<posByteBit);
    }
    else
    {
        *(pBase + posByte) &= ~(1<<posByteBit);
    }
    PPAPI_SYS_CACHE_Writeback((PP_U32 *)(pBase+posByte), 1);

}

static void Draw_1BIT_VPU_Rect(PP_U8 *base, PP_U32 sx, PP_U32 sy, PP_U32 ex, PP_U32 ey, PP_U8 bValue)
{
    PP_U32 px, py;

    for(px = sx; px < ex; px++)
    {
        Draw_1BIT_VPU_XY(base, px, sy, bValue);
        Draw_1BIT_VPU_XY(base, px, ey, bValue);
    }

    for(py = sy; py < ey; py++)
    {
        Draw_1BIT_VPU_XY(base, sx, py, bValue);
        Draw_1BIT_VPU_XY(base, ex, py, bValue);
    }

}

static void Draw_1BIT_VPU_Line(PP_U8 *base, PP_U32 sx, PP_U32 sy, PP_U32 ex, PP_U32 ey, PP_U8 bValue)
{
    int      dx, dy;
    int      p_value;
    int      inc_2dy;
    int      inc_2dydx;
    int      inc_value;
    int      ndx;

    dx       = abs( ex -sx);
    dy       = abs( ey -sy);


    if ( dy <= dx)
    {
        inc_2dy     = 2 * dy;
        inc_2dydx   = 2 * ( dy - dx);

        if ( ex < sx)
        {
            ndx   = sx;
            sx    = ex;
            ex    = ndx;

            ndx   = sy;
            sy    = ey;
            ey    = ndx;
        }
        if ( sy < ey)  inc_value   = 1;
        else           inc_value   = -1;

        Draw_1BIT_VPU_XY(base, sx, sy, 1);
        p_value  = 2 * dy - dx;    
        for (ndx = sx; ndx < ex; ndx++)
        {
            if ( 0 > p_value)    p_value  += inc_2dy;
            else
            {
                p_value  += inc_2dydx;
                sy       += inc_value;
            }
            Draw_1BIT_VPU_XY(base, ndx, sy, 1);
        }
    }
    else
    {
        inc_2dy     = 2 * dx;
        inc_2dydx   = 2 * ( dx - dy);

        if ( ey < sy)
        {
            ndx   = sy;
            sy    = ey;
            ey    = ndx;

            ndx   = sx;
            sx    = ex;
            ex    = ndx;
        }
        if ( sx < ex)  inc_value   = 1;
        else           inc_value   = -1;

        Draw_1BIT_VPU_XY(base, sx, sy, 1);
        p_value  = 2 * dx - dy;    
        for (ndx = sy; ndx < ey; ndx++)
        {
            if ( 0 > p_value)    p_value  += inc_2dy;
            else
            {
                p_value  += inc_2dydx;
                sx       += inc_value;
            }
            Draw_1BIT_VPU_XY(base, sx, ndx, 1);
        }

    }
}

#ifdef DB_LIB_USE
int _testOD(int argc, const char **argv)
{
	int ret = 0;

	char *strArgv = NULL;

	LOG_DEBUG("\n");

    PP_U32 bufSize = VPU_DIS_WIDTH*VPU_DIS_HEIGHT/8;
    PP_RECT_S dispRect = {0, 0, VPU_DIS_WIDTH, VPU_DIS_HEIGHT};
    PP_U32 *pu32DispBuf = (PP_U32 *)gstDramReserved.u32CaptureRsvBuff[0];

    if(argc < 1)
    {
        LOG_DEBUG("Invalid usage\n");
		return(-1);
    }

    strArgv = (char *)argv[1];
    if( !strncmp(strArgv, "init", sizeof("init")) )
    {
        // Move display menu 
        {/*{{{*/
            uint32 u32Cmd;

            LOG_DEBUG("Move Quad display from menu\n");

            u32Cmd = CMD_UI_KEY_MENU;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_LEFT;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_LEFT;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_LEFT;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_LEFT;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_CENTER;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_UP;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_UP;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
          //  u32Cmd = CMD_UI_KEY_UP;
          //  AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            u32Cmd = CMD_UI_KEY_CENTER;
            AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);

        }/*}}}*/
    }
    else if( !strncmp(strArgv, "osd", sizeof("osd")) )
    {
        {
            LOG_DEBUG("Init vpu display\n");
            memset(pu32DispBuf, 0, bufSize); PPAPI_SYS_CACHE_Writeback(pu32DispBuf, bufSize);
            PPAPI_DISPLAY_VPU_Init(dispRect, pu32DispBuf);
            PPAPI_DISPLAY_VPU_Screen(PP_TRUE);
#if 0
            {/*{{{*/
                PP_U32 x, y;
                for(y = 0; y < VPU_DIS_HEIGHT; y++)
                {
                    for(x = 0; x < VPU_DIS_WIDTH; x++)
                    {
                        Draw_1BIT_VPU_XY((PP_U8 *)pu32DispBuf, x, y, 1);
                    }
                }
            }/*}}}*/
#endif
        }

        {
            _VPUStatus stVPUStatus;
            int zoneNum = 0;

            PPAPI_VPU_GetStatus(&stVPUStatus);

            //Draw zone area.
            for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
            {/*{{{*/
                if(stVPUStatus.pVpuZoneParam[zoneNum].isEnable == 0) continue;
                LOG_DEBUG("Draw Zone(%d)\n", zoneNum);

#if 0
                LOG_DEBUG("  Ch En FastTh FastFilter BriefLUT BriefFilter area                fc_height HdTh matchDist\n");
                LOG_DEBUG("  %2d %2d %6x %10d %8d %11d %04dx%04d_%04dx%04d %9d %4x (%04d_%04d|%04d_%04d)\n", zoneNum,
                        stVPUStatus.pVpuZoneParam[zoneNum].isEnable,
                        stVPUStatus.pVpuZoneParam[zoneNum].fastThreshold,
                        stVPUStatus.pVpuZoneParam[zoneNum].fastFilterIndex,
                        stVPUStatus.pVpuZoneParam[zoneNum].briefLUTIndex,
                        stVPUStatus.pVpuZoneParam[zoneNum].briefFilterIndex,
                        stVPUStatus.pVpuZoneParam[zoneNum].sx,
                        stVPUStatus.pVpuZoneParam[zoneNum].sy,
                        stVPUStatus.pVpuZoneParam[zoneNum].ex,
                        stVPUStatus.pVpuZoneParam[zoneNum].ey,
                        stVPUStatus.pVpuZoneParam[zoneNum].fc_height,
                        stVPUStatus.pVpuZoneParam[zoneNum].hd_threshold,
                        stVPUStatus.pVpuZoneParam[zoneNum].match_dist.x_min,
                        stVPUStatus.pVpuZoneParam[zoneNum].match_dist.x_max,
                        stVPUStatus.pVpuZoneParam[zoneNum].match_dist.y_min,
                        stVPUStatus.pVpuZoneParam[zoneNum].match_dist.y_max);
#endif

                Draw_1BIT_VPU_Rect((PP_U8 *)pu32DispBuf, 
                        stVPUStatus.pVpuZoneParam[zoneNum].sx,
                        stVPUStatus.pVpuZoneParam[zoneNum].sy,
                        stVPUStatus.pVpuZoneParam[zoneNum].ex,
                        stVPUStatus.pVpuZoneParam[zoneNum].ey,
                        1);
            }/*}}}*/
        }
    }
    else if( !strncmp(strArgv, "fast", sizeof("fast")) )
    {/*{{{*/
        _VPUStatus stVPUStatus;
//        int runInx = 0;
        int runMax = 0;
        USER_VPU_FB_CONFIG_U stVPUFBConfig;
        uint32 u32TimeOut;
        int zoneNum = 0;
//        PP_U32 u32FCEndPos;
//        int lastFrmInx = 0;
//        PP_U32 *pu32BufFast;
//        PP_U32 *pu32BufBrief;
//        PP_U16 *pu16BufFC;
//        uint16 u16FCEndValue;
        int i;
//        PP_U32 slooptick, elooptick;

        // set fast config
        {
            PPAPI_VPU_FAST_GetConfig(&stVPUFBConfig);
            //stVPUFBConfig.fld.fast_n = FAST_PX_NUM;
            //stVPUFBConfig.fld.ch_sel = eVpuChannel;
            //stVPUFBConfig.fld.use_5x3_nms = FALSE;
            //stVPUFBConfig.fld.brief_enable = TRUE;
            //stVPUFBConfig.fld.roi_enable = FALSE;
            //stVPUFBConfig.fld.scl_enable = bScaleEnable;
            stVPUFBConfig.fld.dma2otf_enable = FALSE;
            stVPUFBConfig.fld.otf2dma_enable = FALSE;
            PPAPI_VPU_FAST_SetConfig(&stVPUFBConfig);
        }

        LOG_DEBUG("VPU_FAST start\n");

		runMax = -1;
		u32TimeOut = 1000;
		PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.

        while (!drv_uart_is_kbd_hit()) 
        {
            //slooptick = GetTickCount();

            LOG_DEBUG("F");
            if( (PPAPI_VPU_WaitFrmUpdate(1000)) == eSUCCESS)
            {

                PPAPI_VPU_GetStatus(&stVPUStatus);
#if 0
                //print HW info
                {/*{{{*/
                    uint32_t i, j, k, inxFrm;

                    inxFrm = stVPUStatus.stFrmInx.inxFrmPrev;
                    //for (j = 0; j < eVPU_FRAME_MAX; j++)
                    {
                        //for (i = 0; i < TOTAL_ZONE_NUM; i++)
                        for (i = 0; i < 1; i++)
                        {
                            LOG_DEBUG("Print FAST result of VPU.(Zone:%d)\n", i);
                            pu32BufFast = (uint32 *)stVPUStatus.ppVBufFast[inxFrm][i];
                            pu32BufBrief = (uint32 *)stVPUStatus.ppVBufBrief[inxFrm][i];
                            pu16BufFC = (uint16 *)stVPUStatus.ppVBufFC[inxFrm][i];

                            u32FCEndPos = stVPUStatus.pVpuZoneParam[i].fc_height;
                            LOG_DEBUG("   endpos:%d, endFcValue:%d\n", u32FCEndPos, pu16BufFC[u32FCEndPos-1]);

                            for (k = 0; k < pu16BufFC[u32FCEndPos-1]; k++)
                            {
                                uint32_t x, y;
                                uint32_t *d = (uint32_t *)&pu32BufBrief[k*4];
                                x = ((pu32BufFast[k] & 0xFFFF0000) >> 16);
                                y = ((pu32BufFast[k] & 0x0000FFFF));
                                LOG_DEBUG ("HW[%d]Zone[%d]  %d ( %d,  %d) = %08x%08x%08x%08x\n", inxFrm, i, k, x, y, d[3], d[2], d[1], d[0]);
                            }
                        }
                    }

                }/*}}}*/
#endif

                LOG_DEBUG("H");
                PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000);

#if 1
                //LOG_DEBUG("Print&Draw HamminD result of VPU.\n");
                {/*{{{*/
                    uint32_t u32TotalMatchCnt = 0;
                    pVPU_MATCHING_RESULT_POS_T hw_HD;
                    for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
                    {
                        u32TotalMatchCnt += stVPUStatus.u32HDMatchResultCount[zoneNum];

                        //LOG_DEBUG("zone:%d-%d\n", zoneNum, stVPUStatus.u32HDMatchResultCount[zoneNum]);
                        hw_HD = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[zoneNum];
                        for (i = 0; i < stVPUStatus.u32HDMatchResultCount[zoneNum]; i++)
                        {
                            /*
                               LOG_DEBUG ("HW (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                               hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
                               LOG_DEBUG ("Draw (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                               hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);

                             */
                            Draw_1BIT_VPU_Line((PP_U8 *)pu32DispBuf, 
                                    hw_HD[i].x1, 
                                    hw_HD[i].y1, 
                                    hw_HD[i].x2, 
                                    hw_HD[i].y2, 1);
                        }

                    }

//                    LOG_DEBUG("TotalMatchCnt:%d\n", u32TotalMatchCnt);
                }/*}}}*/
#endif
            }

            //elooptick = GetTickCount();
            //LOG_DEBUG("- %d\n", elooptick - slooptick);

            //LOG_DEBUG("\r\nany key exit..\n\n");
        }
        PPAPI_VPU_FAST_Stop();
#if 0 //test
        while (!drv_uart_is_kbd_hit()) 
        {/*{{{*/

            //Run Fast 1'st
            {/*{{{*/
                runMax = 1; //runMax = runcount, -1 = endless continue.
                u32TimeOut = 1000;
                PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.

                for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
                {
                    u32FCEndPos = gpVPUConfig->pVpuZoneParam[zoneNum].fc_height;
                    lastFrmInx = gpVPUConfig->stFrmInx.inxFrmLast;

                    pu32BufFast = (uint32 *)gpVPUConfig->ppVBufFast[lastFrmInx][zoneNum];
                    pu32BufBrief = (uint32 *)gpVPUConfig->ppVBufBrief[lastFrmInx][zoneNum];
                    pu16BufFC = (uint16 *)gpVPUConfig->ppVBufFC[lastFrmInx][zoneNum];
                    u16FCEndValue = pu16BufFC[u32FCEndPos-1];
                    //LOG_DEBUG("zone:%d, endpos:%d, endFcValue:%d fcLimit:%d\n", zoneNum, u32FCEndPos, u16FCEndValue, VPU_MAX_FAST_LIMIT_COUNT);
#if 0 //print result data
                    if(u16FCEndValue != 0)
                    {/*{{{*/
                        int fcLine, fcInx;
                        uint16 u16FCPrev = 0, u16FCCur;
                        uint32 u32FastData, u32FastPos;
                        uint16 u16FCLineMax;
                        for(fcLine = 0, u16FCPrev = 0, u32FastPos = 0; fcLine < u32FCEndPos; fcLine++)
                        {
                            u16FCCur = pu16BufFC[fcLine];
                            u16FCLineMax = u16FCCur - u16FCPrev;
                            LOG_DEBUG("line:%d, FcLineMax:%d, FCValue:%d\n", fcLine, u16FCLineMax, u16FCCur);

                            for(fcInx = 0; fcInx < u16FCLineMax; fcInx++)
                            {
                                u32FastData = pu32BufFast[u32FastPos++];
                                LOG_DEBUG("%08x(%d,%d), ", u32FastData, u32FastData>>16, u32FastData&0xFFFF);
                                if(u32FastPos >= VPU_MAX_FAST_LIMIT_COUNT) break;
                            }
                            LOG_DEBUG(":%d\n", u32FastPos);

                            u16FCPrev = u16FCCur;
                        }
                    }/*}}}*/
#endif
                }

#if 1
                //Get Hamming Distance value
                {/*{{{*/
                    PPAPI_VPU_GetStatus(&stVPUStatus);
                    PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000);
                }/*}}}*/
#endif
            }/*}}}*/

            OSAL_sleep(100); //wait 100msec

            //Run Fast 2'nd
            {/*{{{*/
                runMax = 1; //runMax = runcount, -1 = endless continue.
                u32TimeOut = 1000;
                PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.

                for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
                {
                    u32FCEndPos = gpVPUConfig->pVpuZoneParam[zoneNum].fc_height;
                    lastFrmInx = gpVPUConfig->stFrmInx.inxFrmLast;

                    pu32BufFast = (uint32 *)gpVPUConfig->ppVBufFast[lastFrmInx][zoneNum];
                    pu32BufBrief = (uint32 *)gpVPUConfig->ppVBufBrief[lastFrmInx][zoneNum];
                    pu16BufFC = (uint16 *)gpVPUConfig->ppVBufFC[lastFrmInx][zoneNum];
                    u16FCEndValue = pu16BufFC[u32FCEndPos-1];
                    //LOG_DEBUG("zone:%d, endpos:%d, endFcValue:%d fcLimit:%d\n", zoneNum, u32FCEndPos, u16FCEndValue, VPU_MAX_FAST_LIMIT_COUNT);
#if 0 //print result data
                    if(u16FCEndValue != 0)
                    {/*{{{*/
                        int fcLine, fcInx;
                        uint16 u16FCPrev = 0, u16FCCur;
                        uint32 u32FastData, u32FastPos;
                        uint16 u16FCLineMax;
                        for(fcLine = 0, u16FCPrev = 0, u32FastPos = 0; fcLine < u32FCEndPos; fcLine++)
                        {
                            u16FCCur = pu16BufFC[fcLine];
                            u16FCLineMax = u16FCCur - u16FCPrev;
                            LOG_DEBUG("line:%d, FcLineMax:%d, FCValue:%d\n", fcLine, u16FCLineMax, u16FCCur);

                            for(fcInx = 0; fcInx < u16FCLineMax; fcInx++)
                            {
                                u32FastData = pu32BufFast[u32FastPos++];
                                LOG_DEBUG("%08x(%d,%d), ", u32FastData, u32FastData>>16, u32FastData&0xFFFF);
                                if(u32FastPos >= VPU_MAX_FAST_LIMIT_COUNT) break;
                            }
                            LOG_DEBUG(":%d\n", u32FastPos);

                            u16FCPrev = u16FCCur;
                        }
                    }/*}}}*/
#endif
                }

#if 1
                //Get Hamming Distance value
                {/*{{{*/
                    PPAPI_VPU_GetStatus(&stVPUStatus);
                    PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000);
                }/*}}}*/
#endif
            }/*}}}*/

#if 1
            LOG_DEBUG("Get HamminD result of VPU.\n");
            {/*{{{*/
                pVPU_MATCHING_RESULT_POS_T hw_HD;
                for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
                {
                    hw_HD = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[zoneNum];
                    for (i = 0; i < stVPUStatus.u32HDMatchResultCount[zoneNum]; i++)
                    {
                        /*
                        LOG_DEBUG ("HW (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                                hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);

                        LOG_DEBUG ("Draw (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                                hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
                        */
                        Draw_1BIT_VPU_Line((PP_U8 *)pu32DispBuf, 
                                hw_HD[i].x1, 
                                hw_HD[i].y1, 
                                hw_HD[i].x2, 
                                hw_HD[i].y2, 1);
                    }

                }

            }/*}}}*/
#endif

            //LOG_DEBUG("\r\nany key exit..\n\n");

            OSAL_sleep(1000); //wait 1000msec
        }/*}}}*/
        PPAPI_VPU_FAST_Stop();
#endif
    }/*}}}*/
    else if( !strncmp(strArgv, "test", sizeof("test")) )
    {/*{{{*/
        //-------------------------------------------------------------------
        // variables for dynamic blending
        //-------------------------------------------------------------------
        ObjectDetectParam_t		odParam[TOTAL_ZONE_NUM];

        OptFlowPairPos_t * 		currMotionPair[TOTAL_ZONE_NUM];
        uint16_t				currNumMotion[TOTAL_ZONE_NUM] = { 0 };
        uint8_t					blendCorner[4] = { 255, 255, 255, 255 };




        uint16_t				outTotalNumMotion[TOTAL_ZONE_NUM] = { 0 };
        OptFlowPairPos_t * 		outMotionPair[TOTAL_ZONE_NUM];
        int i, iter;

        //-------------------------------------------------------------------
        _VPUStatus stVPUStatus;
//        int runInx = 0;
        int runMax = 0;
        USER_VPU_FB_CONFIG_U stVPUFBConfig;
        uint32 u32TimeOut;
        int zoneNum = 0;
//        PP_U32 u32FCEndPos;
//        int lastFrmInx = 0;
//        PP_U32 *pu32BufFast;
//        PP_U32 *pu32BufBrief;
//        PP_U16 *pu16BufFC;
//        uint16 u16FCEndValue;

//        PP_U32 slooptick, elooptick;

        LOG_DEBUG("Enable DB\n");
        PPAPI_SVM_SetDynamicBlendingOnOff(PP_TRUE);

        // set fast config
        {
            PPAPI_VPU_FAST_GetConfig(&stVPUFBConfig);
            //stVPUFBConfig.fld.fast_n = FAST_PX_NUM;
            //stVPUFBConfig.fld.ch_sel = eVpuChannel;
            //stVPUFBConfig.fld.use_5x3_nms = FALSE;
            //stVPUFBConfig.fld.brief_enable = TRUE;
            //stVPUFBConfig.fld.roi_enable = FALSE;
            //stVPUFBConfig.fld.scl_enable = bScaleEnable;
            stVPUFBConfig.fld.dma2otf_enable = FALSE;
            stVPUFBConfig.fld.otf2dma_enable = FALSE;
            PPAPI_VPU_FAST_SetConfig(&stVPUFBConfig);
        }
        //-------------------------------------------------------------------

        {
            uint32_t allocSize = 0;
            for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
            {
                currMotionPair[zoneNum] = (OptFlowPairPos_t *)OSAL_malloc(sizeof(OptFlowPairPos_t)*VPU_MAX_HDMATCH);
                outMotionPair[zoneNum] = (OptFlowPairPos_t *)OSAL_malloc(sizeof(OptFlowPairPos_t)*VPU_MAX_HDMATCH);

                allocSize += sizeof(OptFlowPairPos_t)*VPU_MAX_HDMATCH;
                allocSize += sizeof(OptFlowPairPos_t)*VPU_MAX_HDMATCH;
            }
            //get malloc size. libod_0.0.7:26KByte
            LOG_DEBUG(" ######### allocSize :%d\n", allocSize);
        }

        {
            uint32_t odVer = 0;
            ObjectDetector_Version(&odVer);
            LOG_DEBUG(" ObjectDetector Ver:%d.%d.%d\n", odVer>>24&0xFF, odVer>>16&0xFF, odVer&0xFFFF);
        }

        LOG_DEBUG(" ObjectDetector_InitParam \n");

        // Pre-processing (one time)

        ObjectDetector_InitParam(odParam);
        //                         OUT
        // ObjectDetector.h ??define ??parameter ?�쏉???��???�귨????�쎌?�占�???????ObjectDetector ???�쎌???????�쎌???

        //----------------------------------------------------------------------------------------------
        LOG_DEBUG("VPU_FAST start\n");
#if 0
        runMax = -1;
        u32TimeOut = 1000;
        LOG_DEBUG("VPU_FAST start(%d)\n", runMax);
        PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.
        //----------------------------------------------------------------------------------------------
#endif

        while (!drv_uart_is_kbd_hit()) 
        {
            //slooptick = GetTickCount();

            runMax = 1;
            u32TimeOut = 1000;
            //LOG_DEBUG("VPU_FAST start(%d)\n", runMax);
            LOG_DEBUG("F");
            PPAPI_VPU_FAST_Start(runMax, u32TimeOut); //runMax = runcount, -1 = endless continue.
                                                      // When manual, 1frame + alpha.

            //if( (PPAPI_VPU_WaitFrmUpdate(1000)) == eSUCCESS)
            {

                PPAPI_VPU_GetStatus(&stVPUStatus);

            LOG_DEBUG("H");
                if( (PPAPI_VPU_HAMMINGD_Start(&stVPUStatus, 1000)) == eSUCCESS)  //2~3msec 
                {

                    //LOG_DEBUG("Get HamminD result of VPU.\n");
                    {/*{{{*/
                        pVPU_MATCHING_RESULT_POS_T hw_HD;
                        for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
                        {
                            outTotalNumMotion[zoneNum] = stVPUStatus.u32HDMatchResultCount[zoneNum];

                            //LOG_DEBUG("zone:%d-%d\n", zoneNum, outTotalNumMotion[zoneNum]);
                            hw_HD = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[zoneNum];
                            for (i = 0; i < stVPUStatus.u32HDMatchResultCount[zoneNum]; i++)
                            {
                                /*
                                   LOG_DEBUG ("HW (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                                   hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);
                                   LOG_DEBUG ("Draw (zone(%u):inx(%u)) : (%u, %u) => (%u, %u)\n", zoneNum, i, 
                                   hw_HD[i].x1, hw_HD[i].y1, hw_HD[i].x2, hw_HD[i].y2);

                                   */
                                   Draw_1BIT_VPU_Line((PP_U8 *)pu32DispBuf, 
                                   hw_HD[i].x1, 
                                   hw_HD[i].y1, 
                                   hw_HD[i].x2, 
                                   hw_HD[i].y2, 1);

                                outMotionPair[zoneNum][i].first.x = hw_HD[i].x1;
                                outMotionPair[zoneNum][i].first.y = hw_HD[i].y1;
                                outMotionPair[zoneNum][i].second.x = hw_HD[i].x2;
                                outMotionPair[zoneNum][i].second.y = hw_HD[i].y2;
                            }
                        }
                    }/*}}}*/

            LOG_DEBUG("O");
                    // process libOD function.
                    {/*{{{*/
                        PP_U32 stick, etick;

                        stick = GetTickCount();

                        // After VPU operation, pass matching pair position and matching pair number at each zone.
                        // first - position for previous frame
                        // second - position for current frame

                        for (i = 0; i < TOTAL_ZONE_NUM; i++) {
                            currNumMotion[i] = outTotalNumMotion[i];

                            for (iter = 0; iter < currNumMotion[i]; iter++)
                            {
                                currMotionPair[i][iter].first.x = outMotionPair[i][iter].first.x;
                                currMotionPair[i][iter].first.y = outMotionPair[i][iter].first.y;
                                currMotionPair[i][iter].second.x = outMotionPair[i][iter].second.x;
                                currMotionPair[i][iter].second.y = outMotionPair[i][iter].second.y;		
                            }
                        }


#if 1
                        odDynamicBlend (1, currMotionPair, currNumMotion, odParam, blendCorner);
                        //             IN      IN              IN           IN        OUT
                        // VPU?�쏙???�쏙?�占?�옙?��?odParam?�쏙???�쌨?�옙 Top view ?�쏙??corner?�쏙??alpha mode ?�쏙?�占�?(0, 64, 128, 192, 255(default)) --> blendCorner (SVM?�쏙??alpha mode?�쏙???�쏙?�占?�옙?�싹?�옙 ?�쏙???�쏙?�占�?

                        LOG_DEBUG("blendCorner[LF(%3d),RF(%3d),LR(%3d),RR(%3d)]\n", blendCorner[0], blendCorner[1], blendCorner[2], blendCorner[3]);

                        LOG_DEBUG("B");
                        PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_FRONTLEFT, blendCorner[0]);
                        PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_FRONTRIGHT, blendCorner[1]);
                        PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_REARLEFT, blendCorner[2]);
                        PPAPI_SVM_SetDynamicBlending(eSVMAPI_DB_REARRIGHT, blendCorner[3]);
#endif

                        etick = GetTickCount();
                        LOG_DEBUG("*%d\n", etick - stick);
                    }/*}}}*/
                }
            }
            //OSAL_sleep(80); //check fastbrief time <= 2frame

            //elooptick = GetTickCount();
            //LOG_DEBUG("-%d\n", elooptick - slooptick);
            //FPGA: average about 200msec

            //LOG_DEBUG("\r\nany key exit..\n\n");
        }

        PPAPI_VPU_FAST_Stop();

        for (zoneNum = 0; zoneNum < TOTAL_ZONE_NUM; zoneNum++)
        {
            OSAL_free(currMotionPair[zoneNum]);
            OSAL_free(outMotionPair[zoneNum]);
        }

    }/*}}}*/
    else if( !strncmp(strArgv, "exit", sizeof("exit")) )
    {
        LOG_DEBUG("DeInit vpu display\n");
        PPAPI_DISPLAY_VPU_Deinit();
        LOG_DEBUG("Disable DB\n");
        PPAPI_SVM_SetDynamicBlendingOnOff(PP_FALSE);
    }
    else if( !strncmp(strArgv, "task", sizeof("task")) )
    {
        strArgv = (char *)argv[2];
        if(!strncmp(strArgv, "on", sizeof("on")) )
        {
            uint32_t bDBEnable = 1;
            LOG_DEBUG("DB Task enable\n");
            AppTask_SendCmd(CMD_DYNBLEND_OPER, TASK_CLI, TASK_DYNBLEND, 0, &bDBEnable, sizeof(bDBEnable), 1000);
        }
        else if(!strncmp(strArgv, "off", sizeof("off")) )
        {
            uint32_t bDBEnable = 0;
            LOG_DEBUG("DB Task disable\n");
            AppTask_SendCmd(CMD_DYNBLEND_OPER, TASK_CLI, TASK_DYNBLEND, 0, &bDBEnable, sizeof(bDBEnable), 1000);
        }
        else
        {
            LOG_DEBUG("Invalid argu\n");
        }
    }
    else
    {
#if 0 //test MemMangOD
        {/*{{{*/
#include "MemMang_OD.h"

            int cnt = 0;
            void *pRsvMem = NULL;

            uint8_t *pucMemMangOD = (uint8_t *)(gstDramReserved.u32CaptureRsvBuff[0]);
            LOG_DEBUG("%x_%x\n", (uint32_t)pucMemMangOD, *pucMemMangOD);

            while(1)
            {
                cnt++;
                LOG_DEBUG("cnt:%d\n", cnt);

                LOG_DEBUG("malloc\n");
                if( (pRsvMem = pvMemMangODMalloc(1024)) == NULL)
                {
                    LOG_DEBUG("Error RsvMalloc\n");
                }
                LOG_DEBUG("memset(0x%02x)\n", cnt&0xFF);
                memset(pRsvMem, cnt&0xFF, 1024);
                LOG_DEBUG("free\n");
                vMemMangODFree(pRsvMem);
                LOG_DEBUG("Freesize:%d\n", xMemMangODGetFreeSize());
                LOG_DEBUG("MinEverFreesize:%d\n", xMemMangODGetMinimumEverFreeSize());
                OSAL_sleep(1000);
            }
        }/*}}}*/
#endif
    }

	return(ret);
}
#endif // DB_LIB_USE



#if 1	// FTL new
int _ftl(int argc, const char **argv)
{
	uint32 ret;
	uint8 *pBuf = NULL;
	uint32 dram_addr;
	uint32 dram_addr1;
	uint32 addr;
	uint32 size;
	uint32 stick, etick;
	uint32 i;
	SPI_MEM_FR_INFO fr_info;
//	uint32 u32MemSize;
	PP_FTL_STATS_S stStats;

	if(argc < 2){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1], "init") == 0){
		uint32 nand_model = 0;

		memset(&fr_info, 0, sizeof(fr_info));

		fr_info.flash_type = 1;		// 0: nor, 1: nand
		if(gstFlashNandID.u8ManufacturerID == NAND_MANID_GIGADEVICE){
			fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_8;
		}else{	// micron, winbond
			fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
		}
		fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_0;
		fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_NONE;
		fr_info.mode_value = 0;
		fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;
		fr_info.addr_4byte = 0;
		spi_memctrl_set(FLASH_CTRL_DIV, FLASH_CTRL_DELAY, 0xEB, fr_info);

		PPAPI_FTL_Release();
		ret = PPAPI_FTL_Initialize(gstFlashNandID.u8FTLDeviceID, &stStats);
		if(ret){
			LOG_DEBUG("FTL Init Fail(%d)\n", ret);
			return(-1);
		}
		LOG_DEBUG("Flash model: %s\n", gstFlashNandID.u8ManufacturerID == NAND_MANID_GIGADEVICE ? "GD5F1G2G" : (gstFlashNandID.u8ManufacturerID == NAND_MANID_MICRON ? "MTFXG01" : (gstFlashNandID.u8ManufacturerID == NAND_MANID_WINBOND ? "W25N01" : "Unknown") ));
		LOG_DEBUG("Total blocks: %d\n", stStats.u32BlocksTotal);
		LOG_DEBUG("Reserved blocks: %d\n", stStats.u32BlocksReserved);
		LOG_DEBUG("Bad blocks: %d\n", stStats.u32BlocksBad);
		LOG_DEBUG("Pages per block: %d\n", stStats.u32PagesPerBlock);
		LOG_DEBUG("Bytes per page: %d\n", stStats.u32BytesPerPages);


	}else if(strcmp(argv[1], "write") == 0){
		addr = simple_strtoul(argv[2], 0);
		size = simple_strtoul(argv[3], 0);
		dram_addr = simple_strtoul(argv[4], 0);
		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("ftl writing addr: 0x%x, size: 0x%x\n", addr, size);
		stick = GetTickCount();
		//ret = PPAPI_FTL_WriteMultipleSector(pBuf, sector_num, sector_cnt);
		ret = PPAPI_FTL_Write(pBuf, addr, size);
		etick = GetTickCount();
		LOG_DEBUG("ftl write size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);
	}else if(strcmp(argv[1], "read") == 0){
		addr = simple_strtoul(argv[2], 0);
		size = simple_strtoul(argv[3], 0);
		dram_addr = simple_strtoul(argv[4], 0);
		pBuf = (uint8 *)dram_addr;
		if(dram_addr < 0x20000000 || dram_addr >= 0x30000000 ){
			LOG_DEBUG("invalid dram addr: 0x%x\n", dram_addr);
			return(-1);
		}

		LOG_DEBUG("ftl reading\n");
		stick = GetTickCount();
		//ret = PPAPI_FTL_ReadMultipleSector(pBuf, sector_num, sector_cnt);
		ret = PPAPI_FTL_Read(pBuf, addr, size);
		etick = GetTickCount();
		LOG_DEBUG("ftl read size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		print_hex(pBuf, size > 64 ? 64 : size);
	}else if(strcmp(argv[1], "release") == 0){
		PPAPI_FTL_Release();
	}else if(strcmp(argv[1], "fattest") == 0){
		ret = PPAPI_FTL_FATTest();
		if(ret)LOG_DEBUG("fat test fail(%d)\n", ret);
	}else if(strcmp(argv[1], "test") == 0){
		addr = 0x0000000;
		//size = 0x4000000;
		size = 0x500000;
		dram_addr = 0x28000000;
		dram_addr1 = 0x2c000000;

		LOG_DEBUG("ftl writing. flash addr: 0x%x, size: 0x%x, dram: 0x%x\n", addr, size, dram_addr);
		pBuf = (uint8 *)dram_addr;
		stick = GetTickCount();
		//ret = PPAPI_FTL_WriteMultipleSector(pBuf, sector_num, sector_cnt);
		ret = PPAPI_FTL_Write(pBuf, addr, size);
		etick = GetTickCount();
		LOG_DEBUG("ftl write size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		pBuf = (uint8 *)dram_addr1;
		LOG_DEBUG("ftl reading. addr: 0x%x, size: 0x%x, dram: 0x%x\n", addr, size, dram_addr1);
		stick = GetTickCount();
		//ret = PPAPI_FTL_ReadMultipleSector(pBuf, sector_num, sector_cnt);
		ret = PPAPI_FTL_Read(pBuf, addr, size);
		etick = GetTickCount();
		LOG_DEBUG("ftl read size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		LOG_DEBUG("Compare result\n");
		for(i=0;i<size;i++){
			if(*(uint8 *)(dram_addr+i) != *(uint8 *)(dram_addr1+i)){
				 LOG_DEBUG("Fail!!! Different data at %d\n", i);
				 print_hex((uint8 *)(dram_addr+i), (size-i) > 32 ? 32 : (size-i));
				 print_hex((uint8 *)(dram_addr1+i), (size-i) > 32 ? 32 : (size-i));
				 break;
			}
		}
		if(i == size){
			LOG_DEBUG("Success\n");
		}


	}else if(strcmp(argv[1],"test2") == 0){
		uint32 errpos;

		dram_addr = 0x28000000;
		dram_addr1 = 0x2c000000;
		addr = 0x3000000 - 0x580000;
		size = 0x100010;

		make_pattern((uint32 *)dram_addr, size, addr);

		// Write
		LOG_DEBUG("ftl writing\n");
		stick = GetTickCount();
		if( (ret = PPAPI_FTL_Write((void *)dram_addr, addr, size)) ){
			LOG_DEBUG("FTL write fail.\n");
		}
		etick = GetTickCount();
		LOG_DEBUG("ftl write size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		// Read & Verify
		LOG_DEBUG("ftl reading\n");
		stick = GetTickCount();
		if((ret = PPAPI_FTL_Read((void *)dram_addr1, addr, size))){
			LOG_DEBUG("FTL read fail.\n");
		}
		etick = GetTickCount();
		LOG_DEBUG("ftl read size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		errpos = check_pattern((uint32 *)dram_addr1, size, addr);
		if(errpos != (uint32)-1){
			LOG_DEBUG("Error!!! Wrong data @0x%x\n", (dram_addr1+errpos*4));
		}else{
			LOG_DEBUG("OK.\n");
		}

	}else if(strcmp(argv[1], "test3") == 0){	// winbond nand flash - spare area ecc protection test
		uint8 buf[64];
		uint8 buf1[8] = {0x00, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
		uint8 buf2[8] = {0x00, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
		uint32 block_addr = 100;
		uint32 page_addr = 2;
		uint8 feature;
		uint32 i;
		uint32 j;

		for(j=0;j<4;j++){
			for(i=0;i<6;i++){
				LOG_DEBUG("========================================\n");
				LOG_DEBUG("spare-%d, %dth byte\n", j, 2+i);
				PPDRV_SNAND_FLASH_EraseBlock(block_addr);

				memcpy(buf1, buf2, sizeof(buf2));
				PPDRV_SNAND_FLASH_ReadSpare(buf, block_addr, page_addr, 0, 64);
				print_hex(buf, 64);

				PPDRV_SNAND_FLASH_WriteSpare(buf1, block_addr, page_addr, j*16, 8);
				PPDRV_SNAND_FLASH_ReadSpare(buf, block_addr, page_addr, j*16, 8);
				print_hex(buf, 8);


				// ecc off
				feature = PPDRV_SNAND_FLASH_GetFeature(0xb0);
				feature &= (~(1<<4));
				PPDRV_SNAND_FLASH_SetFeature(0xb0, feature);

				buf1[2+i] &= (~(1<<3));

				PPDRV_SNAND_FLASH_WriteSpare(buf1, block_addr, page_addr, j*16, 8);
				PPDRV_SNAND_FLASH_ReadSpare(buf, block_addr, page_addr, j*16, 8);
				print_hex(buf, 8);

				// ecc on
				feature = PPDRV_SNAND_FLASH_GetFeature(0xb0);
				feature |= (1<<4);
				PPDRV_SNAND_FLASH_SetFeature(0xb0, feature);

				PPDRV_SNAND_FLASH_ReadSpare(buf, block_addr, page_addr, j*16, 8);
				print_hex(buf, 8);

				if(buf[2+i] == buf2[2+i]){
					LOG_DEBUG("Spare-%d, %dth byte. ECC Pretected\n");
				}else{
					LOG_DEBUG("Spare-%d, %dth byte. ECC not portected\n");
				}
			}
		}
	}else if(strcmp(argv[1], "test4") == 0){
		uint8 buf[16];
		uint8 buf1[64];
		uint32 i;
		uint32 block_addr;
		uint32 start_block = 640;
		uint32 end_block = 700;

		if(argc > 2){
			start_block = simple_strtoul(argv[2], 0);
			end_block = simple_strtoul(argv[3], 0);
		}
		for(i=0;i<16;i++){
			buf[i] = i;
		}
		memset(buf1, 0, sizeof(buf1));

		LOG_DEBUG("feature(0xb0): 0x%x\n", PPDRV_SNAND_FLASH_GetFeature(0xb0));

		PPDRV_SNAND_FLASH_ReadSpare(buf1, block_addr, 0, 0, 64);

		for(block_addr=start_block;block_addr<end_block;block_addr++){

			PPDRV_SNAND_FLASH_WriteSpare(buf, block_addr, 0, 4, 4);
			PPDRV_SNAND_FLASH_WriteSpare(&buf[4], block_addr, 0, 20, 4);
			PPDRV_SNAND_FLASH_WriteSpare(&buf[8], block_addr, 0, 36, 4);
			PPDRV_SNAND_FLASH_WriteSpare(&buf[12], block_addr, 0, 52, 4);
		}

		// check spare
		for(block_addr=start_block;block_addr<end_block;block_addr++){
			PPDRV_SNAND_FLASH_ReadSpare(buf1, block_addr, 0, 0, 64);
			if(buf1[0] != 0xff || buf1[1] != 0xff){
				LOG_DEBUG("Block %d is bad(0x%x, 0x%x)\n", block_addr, buf1[0], buf1[1]);
			}

			if(memcmp(&buf[0], &buf1[4], 4) != 0){
				LOG_DEBUG("Block %d spare data mismatch.\n", block_addr);
			}
			if(memcmp(&buf[4], &buf1[20], 4) != 0){
				LOG_DEBUG("Block %d spare data mismatch.\n", block_addr);
			}
			if(memcmp(&buf[8], &buf1[36], 4) != 0){
				LOG_DEBUG("Block %d spare data mismatch.\n", block_addr);
			}
			if(memcmp(&buf[12], &buf1[52], 4) != 0){
				LOG_DEBUG("Block %d spare data mismatch.\n", block_addr);
			}

		}
	}else if(strcmp(argv[1], "test5") == 0){
		int cnt = 0;
		uint32 initval;
		uint8 *src_ptr;
		uint32 dram_addr2;

		dram_addr = 0x28000000;
		dram_addr1 = 0x29000000;
		dram_addr2 = 0x2a000000;

		size = 10485760;
		addr = 0;

		if(PPAPI_FLASH_Initialize(eFLASH_TYPE_NAND) != eSUCCESS){
			LOG_DEBUG("ERROR!!! Flash init fail. Halt\n");
			return(-1);
		}

		while (!drv_uart_is_kbd_hit()){
			if(PPAPI_FTL_Initialize(gstFlashNandID.u8FTLDeviceID, &stStats) != eSUCCESS){
				LOG_DEBUG("ERROR!!! Flash FTL init fail. Halt\n");
				break;
			}

			if(cnt & 1){
				initval = GetTickCount();
				src_ptr = (uint8 *)dram_addr1;
				make_pattern((uint32 *)src_ptr, size, initval);
			}else{
				src_ptr = (uint8 *)dram_addr;
			}

			// write
			LOG_DEBUG("Writing...\n");
			if(PPAPI_FTL_Write((uint8 *)(src_ptr), addr, size)){
				LOG_DEBUG("FTL write fail\n");
				break;
			}


			// Verify
			LOG_DEBUG("Reading...\n");
			if(PPAPI_FTL_Read((uint8 *)dram_addr2, addr, size)){
				LOG_DEBUG("FTL read fail\n");
			}

			LOG_DEBUG("Verifying...\n");
			for(i=0;i<size;i++){
				if(*(uint8 *)(src_ptr+i) != *(uint8 *)(dram_addr2+i)){
					LOG_DEBUG("Verify fail at 0x%x. 0x%x/0x%x\n", (uint32)(src_ptr + i), *(uint8 *)(src_ptr+i), *(uint8 *)(dram_addr2+i));
					break;
				}
			}

			PPAPI_FTL_Release();

			if(i == size)
				LOG_DEBUG("FTL Test OK. Loop Cnt: %d\n", cnt);
			else{
				LOG_DEBUG("FTL Test Fail\n");
				break;
			}

			cnt++;
		}

	}



	return 1;
}
#else	// old
static uint8 *gpu8Mem = NULL;
static PP_FTLPHY_S gstPhy;
int _ftl(int argc, const char **argv)
{
	uint32 ret;
	uint8 *pBuf = NULL;
	uint32 sector_num;
	uint32 sector_cnt;
	uint32 dram_addr;
	uint32 dram_addr1;
	uint32 size;
	uint32 stick, etick;
	uint32 i;
	SPI_MEM_FR_INFO fr_info;
	uint32 u32MemSize;

	if(argc < 2){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1], "init") == 0){


		memset(&fr_info, 0, sizeof(fr_info));

		fr_info.flash_type = 1;		// 0: nor, 1: nand
		fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
		fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_0;
		fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_NONE;
		fr_info.mode_value = 0;
		fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;
		fr_info.addr_4byte = 0;
		spi_memctrl_set(0, 4, 0xEB, fr_info);
		//spi_memctrl_set(4, 2, 0xEB, fr_info);

		ret = PPAPI_FTL_Initialize();
		if(ret){
			LOG_DEBUG("FTL Init Fail(%d)\n", ret);
			return(-1);
		}
		ret = PPAPI_FTL_GetMem(&u32MemSize);
		if(ret)LOG_DEBUG("FTL getmem fail(%d)\n", ret);

		gpu8Mem = (uint8 *)OSAL_malloc(u32MemSize);

		ret = PPAPI_FTL_SetMem((uint32 *)gpu8Mem);
		if(ret)LOG_DEBUG("FTL setmem fail(%d)\n", ret);

		ret = PPAPI_FTL_InitFunction(2 | (1<<31));
		if(ret){
			LOG_DEBUG("FTL init func fail(%d). Format\n", ret);

			ret = PPAPI_FTL_Format();
			if(ret)LOG_DEBUG("FTL format fail\n");

			ret = PPAPI_FTL_InitFunction(2 | (1<<31));
			if(ret){
				LOG_DEBUG("FTL init function fail(%d) Return\n", ret);
				OSAL_free(gpu8Mem);
				return (-1);
			}
		}

		ret = PPAPI_FTL_GetPhy(&gstPhy);
		if(ret){
			LOG_DEBUG("FTL Get Phy Error(%d)\n", ret);
		}else{
			LOG_DEBUG("FTL Sectors: %d\n", gstPhy.u32Sectors);
			LOG_DEBUG("FTL Bytes per Sector: %d\n", gstPhy.u32BytesPerSector);
		}

	}else if(strcmp(argv[1], "info") == 0){
		ret = PPAPI_FTL_GetPhy(&gstPhy);
		if(ret){
			LOG_DEBUG("FTL Get Phy Error(%d)\n", ret);
		}else{
			LOG_DEBUG("FTL Sectors: %d\n", gstPhy.u32Sectors);
			LOG_DEBUG("FTL Bytes per Sector: %d\n", gstPhy.u32BytesPerSector);
		}
	}else if(strcmp(argv[1], "write_sector") == 0){
		sector_num = simple_strtoul(argv[2], 0);
		sector_cnt = simple_strtoul(argv[3], 0);
		dram_addr = simple_strtoul(argv[4], 0);
		pBuf = (uint8 *)dram_addr;

		LOG_DEBUG("ftl writing\n");
		stick = GetTickCount();
		ret = PPAPI_FTL_WriteMultipleSector(pBuf, sector_num, sector_cnt);
		etick = GetTickCount();
		size = sector_cnt * gstPhy.u32BytesPerSector;
		LOG_DEBUG("ftl write size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);
	}else if(strcmp(argv[1], "read_sector") == 0){
		sector_num = simple_strtoul(argv[2], 0);
		sector_cnt = simple_strtoul(argv[3], 0);
		dram_addr = simple_strtoul(argv[4], 0);
		pBuf = (uint8 *)dram_addr;

		LOG_DEBUG("ftl reading\n");
		stick = GetTickCount();
		ret = PPAPI_FTL_ReadMultipleSector(pBuf, sector_num, sector_cnt);
		etick = GetTickCount();
		size = sector_cnt * gstPhy.u32BytesPerSector;
		LOG_DEBUG("ftl read size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		print_hex(pBuf, size > 64 ? 64 : size);
	}else if(strcmp(argv[1], "fattest") == 0){
		ret = PPAPI_FTL_FATTest();
		if(ret)LOG_DEBUG("fat test fail(%d)\n", ret);
	}else if(strcmp(argv[1], "test") == 0){
		sector_num = 0;
		//sector_cnt = 0x4000000/512;
		sector_cnt = 0x100000/512;
		dram_addr = 0x28000000;
		dram_addr1 = 0x2c000000;

		LOG_DEBUG("ftl writing. sector: 0x%x, cnt: 0x%x, dram: 0x%x\n", sector_num, sector_cnt, dram_addr);
		pBuf = (uint8 *)dram_addr;
		stick = GetTickCount();
		ret = PPAPI_FTL_WriteMultipleSector(pBuf, sector_num, sector_cnt);
		etick = GetTickCount();
		size = sector_cnt * gstPhy.u32BytesPerSector;
		LOG_DEBUG("ftl write size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		pBuf = (uint8 *)dram_addr1;
		LOG_DEBUG("ftl reading. sector: 0x%x, cnt: 0x%x, dram: 0x%x\n", sector_num, sector_cnt, dram_addr1);
		stick = GetTickCount();
		ret = PPAPI_FTL_ReadMultipleSector(pBuf, sector_num, sector_cnt);
		etick = GetTickCount();
		size = sector_cnt * gstPhy.u32BytesPerSector;
		LOG_DEBUG("ftl read size: %d, %d ms(%d KBytes/s)\n", size, (etick-stick), (etick - stick) ?  size / (etick-stick) * 1000 / 1024 : 0);

		LOG_DEBUG("Compare result\n");
		for(i=0;i<size;i++){
			if(*(uint8 *)(dram_addr+i) != *(uint8 *)(dram_addr1+i)){
				 LOG_DEBUG("Fail!!! Different data at %d\n", i);
				 print_hex((uint8 *)(dram_addr+i), (size-i) > 32 ? 32 : (size-i));
				 print_hex((uint8 *)(dram_addr1+i), (size-i) > 32 ? 32 : (size-i));
				 break;
			}
		}
		if(i == size){
			LOG_DEBUG("Success\n");
		}


	}

	return 1;
}
#endif

#include "dma.h"
int _dramest(int argc, const char **argv)
{
	uint32 test_size = 1024*1024;
	uint8 *pBuf = NULL;
	uint8 *pBuf1 = NULL;
	uint32 stick, etick;
	uint32 rdata[2];
	uint8 u8data[2];
	uint32 i;

	pBuf = OSAL_malloc(test_size);
	if(!pBuf){
		LOG_DEBUG("malloc fail\n");
		return 0;
	}

	pBuf1 = OSAL_malloc(test_size);
	if(!pBuf1){
		LOG_DEBUG("malloc fail\n");
		return 0;
	}


	LOG_DEBUG("===========================================\n");
	LOG_DEBUG(" 4 Byte Access\n");
	LOG_DEBUG("===========================================\n");
	test_size /= 4;

	LOG_DEBUG("1. DRAM Initial\n");
    stick = GetTickCount();
	for (i = 0; i <test_size;i++) { // 64Kbyte
        *(vuint32 *) (pBuf+(0x4*i))    = i;
    }
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size*4, test_size * 4 * 1000 / (etick - stick)/1024);

	LOG_DEBUG("2. DRAM read, DRAM write\n");
    stick = GetTickCount();
	for (i = 0; i <test_size;i++) { // 64Kbyte
        rdata[0] = *(vuint32 *)(pBuf+(0x4*i));
		*(vuint32 *) (pBuf1+(0x4*i))    = rdata[0];
    }
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size*4, test_size * 4 * 1000 / (etick - stick)/1024);

	LOG_DEBUG("3. DRAM read, DRAM read, Compare\n");
    stick = GetTickCount();
	for (i = 0; i <test_size;i++) {
        rdata[0] = *(vuint32 *)(pBuf+(0x4*i));
        rdata[1] = *(vuint32 *)(pBuf1+(0x4*i));

        if(rdata[0] != rdata[1]){
        	LOG_DEBUG("Error!!! Wrong dram data\n");
        	break;
        }
    }
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size*4, test_size * 4 * 1000 / (etick - stick)/1024);



	LOG_DEBUG("===========================================\n");
	LOG_DEBUG(" DMA read mem to mem\n");
	LOG_DEBUG("===========================================\n");
	test_size = 1024*1024;
	stick = GetTickCount();
	if(PPDRV_DMA_M2M_Word(0, (PP_U32 *)pBuf, (PP_U32 *)pBuf1, test_size, SYS_OS_SUSPEND)){
		LOG_DEBUG("DMA Error\n");
	}
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size, test_size * 1000 / (etick - stick)/1024);



	LOG_DEBUG("===========================================\n");
	LOG_DEBUG(" 1 Byte Access\n");
	LOG_DEBUG("===========================================\n");
	test_size = 1024*1024;

	LOG_DEBUG("1. DRAM Initial\n");
    stick = GetTickCount();
	for (i = 0; i <test_size;i++) { // 64Kbyte
        *(vuint8 *) (pBuf+(i))    = (i&0xff);
    }
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size, test_size * 1000 / (etick - stick) / 1024);

	LOG_DEBUG("2. DRAM read, DRAM write\n");
    stick = GetTickCount();
	for (i = 0; i <test_size;i++) {
        u8data[0] = *(vuint8 *)(pBuf+(i));
		*(vuint8 *) (pBuf1+(i))    = u8data[0];
    }
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size, test_size * 1000 / (etick - stick) / 1024);

	LOG_DEBUG("3. DRAM read, DRAM read, Compare\n");
    stick = GetTickCount();
	for (i = 0; i <test_size;i++) {
        u8data[0] = *(vuint8 *)(pBuf+(i));
        u8data[1] = *(vuint8 *)(pBuf1+(i));

        if(u8data[0] != u8data[1]){
        	LOG_DEBUG("Error!!! Wrong dram data\n");
        	break;
        }
    }
	etick = GetTickCount();
	LOG_DEBUG("Time: %d ms, Size: %d, %d KBPS\n\n", etick - stick, test_size, test_size * 1000 / (etick - stick) / 1024);


    if(pBuf)OSAL_free(pBuf);
    if(pBuf1)OSAL_free(pBuf1);

    return 1;
}

int _compare(int argc, const char **argv)
{
	uint32 size;
	uint8 *pBuf1, *pBuf2;
	uint32 diffcnt = 0;
	uint32 init_index = 0;
	uint32 i;

	if(argc < 4){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	pBuf1 = (uint8 *)simple_strtoul(argv[1], 0);
	pBuf2 = (uint8 *)simple_strtoul(argv[2], 0);
	size = simple_strtoul(argv[3], 0);

	for(i=0;i<size;i++){
		if(pBuf1[i] != pBuf2[i]){
			if(diffcnt == 0)init_index = i;
			diffcnt++;
		}
	}

	if(diffcnt == 0){
		LOG_DEBUG("Same.\n");
	}else{
		LOG_DEBUG("Different at 0x%x. Count: %d\n", (uint32)(pBuf1+init_index), diffcnt);
	}

	return 1;

}

#include "dma.h"
int _spitest(int argc, const char **argv)
{
	int size;
	uint8 *pMemTx = NULL;
	uint8 *pMemRx = NULL;
	int dev_ch = 1;
	int dma_tx_ch = 2;
	int dma_rx_ch = 3;
	int isSlave = 0;
	uint32 loop = 0;
	uint32 max_loop;


	if(argc < 5){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1],"m") == 0){
		isSlave = 0;
		PPDRV_SPI_Initialize(dev_ch, isSlave, 37125000/8, 0, 8);
	}else{
		isSlave = 1;
		PPDRV_SPI_Initialize(dev_ch, isSlave, 37125000/8, 0, 8);
	}


	size = simple_strtoul(argv[3], 0);
	max_loop = simple_strtoul(argv[4], 0);

	pMemTx = (uint8 *)OSAL_malloc(size);
	pMemRx = (uint8 *)OSAL_malloc(size);

	//pMemTx = (uint8 *)0x10000;
	//pMemRx = (uint8 *)0x11000;

	memset(pMemTx, 0xff, size);
	memset(pMemRx, 0xff, size);
	PPAPI_SYS_CACHE_Writeback((PP_U32 *)pMemTx, size);
	PPAPI_SYS_CACHE_Writeback((PP_U32 *)pMemRx, size);

	if(strcmp(argv[2], "tx") == 0){


		make_pattern((uint32 *)pMemTx, size, 0);
		PPAPI_SYS_CACHE_Writeback((PP_U32 *)pMemTx, size);

		while(loop++ < max_loop){
			if(isSlave == 0){
				LOG_DEBUG("SPI Master DMA Tx. Size: %d\n", size);
				if(PPDRV_SPI_TxDMA(dev_ch, pMemTx, size, dma_tx_ch, 10000)){
					LOG_DEBUG("Tx Fail\n");
				}else{
					//while(PPDRV_SPI_DMAGetDone(dev_ch, dma_tx_ch) != eSUCCESS);
					LOG_DEBUG("Done\n");
				}
			}else{
				LOG_DEBUG("SPI Slave DMA Tx. Size: %d\n", size);
				if(PPDRV_SPI_TxSlaveDMA(dev_ch, pMemTx, size, dma_tx_ch, 10000)){
					LOG_DEBUG("Tx Fail\n");
				}else{
					//while(PPDRV_SPI_DMAGetDone(dev_ch, dma_tx_ch) != eSUCCESS);
					LOG_DEBUG("Done\n");
				}

			}
		}


	}else if(strcmp(argv[2], "rx") == 0){
		int err_pos;
		max_loop = simple_strtoul(argv[4], 0);
		while(loop++ < max_loop){
			if(isSlave == 0){
				LOG_DEBUG("SPI Master DMA Rx. Size: %d\n", size);
				if(PPDRV_SPI_RxDMA(dev_ch, pMemRx, size, dma_rx_ch, 10000)){
					LOG_DEBUG("Rx Fail\n");
				}else{
					//while(PPDRV_SPI_DMAGetDone(dev_ch, dma_rx_ch) != eSUCCESS);
					LOG_DEBUG("Done\n");
				}
			}else{


				LOG_DEBUG("SPI Slave DMA Rx. Size: %d\n", size);
				if(PPDRV_SPI_RxSlaveDMA(dev_ch, pMemRx, size, dma_rx_ch, SYS_OS_SUSPEND)){
				//if(PPDRV_SPI_RxSlave(dev_ch, pMemRx, size)){
					LOG_DEBUG("Rx Fail\n");
				}else{
					//while(PPDRV_SPI_DMAGetDone(dev_ch, dma_rx_ch) != eSUCCESS);
					LOG_DEBUG("Done\n");
				}
                PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pMemRx, size);
#if 0
                {
                    uint32 stick, etick;
                    int i;

                    stick = GetTickCount();
                    for(i=0;i<100;i++){
                        PPDRV_SPI_RxSlaveTest(dev_ch, pMemRx, size);
                    }
                    etick = GetTickCount();

                    LOG_DEBUG("elapsed time: %d ms\n", etick - stick);
                }
#endif
			}

			if((err_pos = check_pattern((uint32 *)pMemRx, size, 0)) != (uint32)-1){
				LOG_DEBUG("Error!!! Data is not valid. index: %d, addr: 0x%x\n", err_pos, (uint32)(pMemRx + err_pos*4));
			}
		}
	}else if(strcmp(argv[2], "xfer") == 0){
		max_loop = simple_strtoul(argv[4], 0);
		make_pattern((uint32 *)pMemTx, size, 0);
		PPAPI_SYS_CACHE_Writeback((PP_U32 *)pMemTx, size);

		while(loop++ < max_loop){
			if(isSlave == 0){
				LOG_DEBUG("SPI Master DMA Xfer. Size: %d\n", size);
				if(PPDRV_SPI_TxRxDMA(dev_ch, pMemTx, pMemRx, size, dma_tx_ch, dma_rx_ch, 10000)){
					LOG_DEBUG("Rx Fail\n");
				}else{
					//while( (PPDRV_SPI_DMAGetDone(dev_ch, dma_rx_ch) != eSUCCESS) && (PPDRV_SPI_DMAGetDone(dev_ch, dma_tx_ch) != eSUCCESS));
					LOG_DEBUG("Done\n");
				}

			}else{
				LOG_DEBUG("SPI Slave DMA Xfer. Size: %d\n", size);
				if(PPDRV_SPI_TxRxSlaveDMA(dev_ch, pMemTx, pMemRx, size, dma_tx_ch, dma_rx_ch, 10000)){
					LOG_DEBUG("Rx Fail\n");
				}else{
					//while( (PPDRV_SPI_DMAGetDone(dev_ch, dma_rx_ch) != eSUCCESS) && (PPDRV_SPI_DMAGetDone(dev_ch, dma_tx_ch) != eSUCCESS));
					LOG_DEBUG("Done\n");
				}

			}

			PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pMemRx, size);
			if(check_pattern((uint32 *)pMemRx, size, 0) != (uint32)-1){
				LOG_DEBUG("Error!!! Data is not valid\n");
			}

		}
	}else if(strcmp(argv[2], "test") == 0){

		while(1){
			size = 12;
			LOG_DEBUG("SPI Slave DMA Rx. Size: %d\n", size);
			if(PPDRV_SPI_RxSlaveDMA(dev_ch, pMemRx, size, dma_rx_ch, SYS_OS_SUSPEND)){
			//if(PPDRV_SPI_RxSlave(dev_ch, pMemRx, size)){
				LOG_DEBUG("Rx Fail\n");
			}else{
				//while(PPDRV_SPI_DMAGetDone(dev_ch, dma_rx_ch) != eSUCCESS);
				LOG_DEBUG("Done\n");
			}
			PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pMemRx, size);
			print_hex(pMemRx, size);

			size = 16;
			make_pattern((uint32 *)pMemTx, size, 0);
			LOG_DEBUG("SPI Slave Tx. Size: %d\n", size);
			PPDRV_SPI_TxSlave(dev_ch, pMemTx, size);
		}
	}

	if(pMemTx)OSAL_free(pMemTx);
	if(pMemRx)OSAL_free(pMemRx);

	return 1;
}


#include "i2c.h"

// slave buff
static uint8 gI2CSSendBuf[4];
static uint8 gI2CSRecvBuf[4];

static void i2c_slave_onreceive(uint32 ch, uint8 *buf, uint32 size)
{
	uint32 i;

	LOG_DEBUG("slave receive size: %d\n", size);
	for(i=0;i<size;i++)
		gI2CSRecvBuf[i] = buf[i];
}

static uint32 i2c_slave_onsend(uint32 ch, uint8 *buf, uint32 max_size)
{
	uint32 i;
	uint32 size = sizeof(gI2CSSendBuf);


	if(size > max_size)size = max_size;
	for(i=0;i<size;i++)buf[i] = gI2CSSendBuf[i];

	LOG_DEBUG("slave send size: %d\n", size);

	return i;
}

int _i2c_test(int argc, const char **argv)
{
//	int test_cnt = 3;
	int i;
	int chm = 0;
	int chs = 1;
	uint32 dev_addr = 0x10;
	PP_RESULT_E ret;

    // master buff
    static uint8 gI2CMSendBuf[4];
    static uint8 gI2CMRecvBuf[4];

	for(i=0;i<4;i++){
		gI2CMSendBuf[i] = i;
		gI2CSSendBuf[i] = 128 + i;
		gI2CMRecvBuf[i] = 0;
		gI2CSRecvBuf[i] = 0;
	}

	PPDRV_I2C_Initialize();
	PPDRV_I2C_Reset(chm);
	PPDRV_I2C_Reset(chs);

	PPDRV_I2C_Setup(chm, dev_addr, eI2C_SPEED_NORMAL, eI2C_MODE_MASTER, eI2C_ADDRESS_7BIT);	// master, 100kb/s
	PPDRV_I2C_Setup(chs, dev_addr, eI2C_SPEED_NORMAL, eI2C_MODE_SLAVE, eI2C_ADDRESS_7BIT); // slave

	PPDRV_I2C_SlaveSetCallback(chs, i2c_slave_onreceive, i2c_slave_onsend);

	PPDRV_I2C_Enable(chm, 1);
	PPDRV_I2C_Enable(chs, 1);

	while (!drv_uart_is_kbd_hit()){
		LOG_DEBUG("Master Write\n");
		ret = PPDRV_I2C_Write(chm, gI2CMSendBuf, sizeof(gI2CMSendBuf), 1000);
		if(ret != eSUCCESS){
			LOG_DEBUG("I2C Write fail(%d)\n", ret);
		}

		ret = PPDRV_I2C_Read(chm, gI2CMRecvBuf, sizeof(gI2CMRecvBuf), 1000);
		if(ret != eSUCCESS){
			LOG_DEBUG("I2C Read fail(%d)\n", ret);
		}else{
			LOG_DEBUG("Master Read.\n");
		}

		LOG_DEBUG("Master write buffer\n");
		print_hex(gI2CMSendBuf, sizeof(gI2CMSendBuf));

		LOG_DEBUG("Slave read buffer\n");
		print_hex(gI2CSRecvBuf, sizeof(gI2CSRecvBuf));

		LOG_DEBUG("Master read buffer\n");
		print_hex(gI2CMRecvBuf, sizeof(gI2CMRecvBuf));

		LOG_DEBUG("Slave send buffer\n");
		print_hex(gI2CSSendBuf, sizeof(gI2CSSendBuf));

		OSAL_sleep(500);
	}
    return(0);
}

#include "cache.h"
int _cache_test(int argc, const char **argv)
{
	int i;
	int size = 32*1024;
	uint32 *ptr = (uint32 *)0x28000000;
	uint32 *ptr1 = (uint32 *)0xa8000000;

	LOG_DEBUG("Write cache region. Cache wb all & compare ...");
	memset(ptr, 0, size);
	cache_wb_all();
	for(i=0;i<size/4;i++){
		if(*(ptr+i) != *(ptr1+i)){
			LOG_DEBUG("Different. @0x%x, 0x%x / 0x%x\n", i, *(ptr+i), *(ptr1+i));
			break;
		}
	}
	if(i==size/4)
		LOG_DEBUG("Same\n");


	LOG_DEBUG("Write uncached region. compare ...");
	for(i=0;i<size/4;i++){
		*(ptr1 + i) = i*4;
	}

	for(i=0;i<size/4;i++){
		if(*(ptr+i) != *(ptr1+i)){
			LOG_DEBUG("Different. @0x%x, 0x%x / 0x%x\n", i, *(ptr+i), *(ptr1+i));
			break;
		}
	}
	if(i==size/4)
		LOG_DEBUG("Same\n");

	LOG_DEBUG("Cache inv all & compare ...");

	cache_inv_all();
	for(i=0;i<size/4;i++){
		if(*(ptr+i) != *(ptr1+i)){
			LOG_DEBUG("Different. @0x%x, 0x%x / 0x%x\n", i, *(ptr+i), *(ptr1+i));
			break;
		}
	}
	if(i==size/4)
		LOG_DEBUG("Same\n");

	return 0;

}

int _isp_test(int argc, const char **argv)
{
	LOG_DEBUG("ISP TP set\n");

#if 0
    LOG_DEBUG("Cli TEST #!!!! TEST_ISP_TP 1080p30\n");
    //                      1080p25, 1080p30
    SetRegValue(0xF0600FB8, 0x81); //0x81	
    SetRegValue(0xF0600FBC, 0x00); //0x00	
    SetRegValue(0xF0600FC0, 0xFF); //0xFF	
    SetRegValue(0xF0600FC4, 0xF0); //0xF0	
    SetRegValue(0xF0600FC8, 0x00); //0x00	
    SetRegValue(0xF0600FCC, 0xFF); //0xFF	
    SetRegValue(0xF0600FD0, 0xF0); //0xF0	
    SetRegValue(0xF0600FD4, 0x00); //0x00	
    SetRegValue(0xF0600FD8, 0xFF); //0xFF	
    SetRegValue(0xF0600FDC, 0xF0); //0xF0	
    SetRegValue(0xF0600FE0, 0x00); //0x00	
    SetRegValue(0xF0600FE4, 0xFF); //0xFF	
    SetRegValue(0xF0600FE8, 0xF0); //0xF0	
    SetRegValue(0xF0600FEC, 0x07); //0x07	
    SetRegValue(0xF0600FF0, 0x80); //0x80	
    SetRegValue(0xF0600C38, 0x07); //0x07	
    SetRegValue(0xF0600C3C, 0x80); //0x80	
    SetRegValue(0xF0600C40, 0x04); //0x04	
    SetRegValue(0xF0600C44, 0x38); //0x38	
    SetRegValue(0xF0601360, 0x00); //0x00	
    SetRegValue(0xF0601364, 0x00); //0x00	
    SetRegValue(0xF0601368, 0x07); //0x07	
    SetRegValue(0xF060136C, 0x80); //0x80	
    SetRegValue(0xF0601370, 0x00); //0x00	
    SetRegValue(0xF0601374, 0x00); //0x00	
    SetRegValue(0xF0601378, 0x04); //0x04	
    SetRegValue(0xF060137C, 0x38); //0x38	
    SetRegValue(0xF0600C24, 0x04); //0x04	
    SetRegValue(0xF0600C28, 0x64); //0x64	
    SetRegValue(0xF0600C2C, 0x04); //0x04	
    SetRegValue(0xF0600C30, 0x64); //0x64	
    SetRegValue(0xF0600C14, 0x08); //0x0A	
    SetRegValue(0xF0600C18, 0x97); //0x4F	
    SetRegValue(0xF0600D24, 0x04); //0x04	
    SetRegValue(0xF0600D28, 0x59); //0x59	
    SetRegValue(0xF0600DB4, 0x10); //0x10	
    SetRegValue(0xF0602754, 0x00); //0x00	
    SetRegValue(0xF0602758, 0x0F); //0x0F	
    SetRegValue(0xF060275C, 0xFF); //0xFF	
#endif

#if 1
    LOG_DEBUG("Cli TEST #!!!! TEST_ISP_TP 720p30\n");
    {
        int i;
        for(i = 0; i < 4; i++)
        {/*{{{*/
            //                       720p30,  720p25
            SetRegValue(0xF0600FB8+(i*0x4000), 0x81); //0x0x81
            SetRegValue(0xF0600FBC+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FC0+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FC4+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FC8+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FCC+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FD0+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FD4+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FD8+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FDC+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FE0+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FE4+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FE8+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FEC+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF0600FF0+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600C38+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF0600C3C+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600C40+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C44+(i*0x4000), 0xD0); //0x0xD0
            SetRegValue(0xF0601360+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601364+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601368+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF060136C+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601370+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601374+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601378+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF060137C+(i*0x4000), 0xD0); //0x0xD0
            SetRegValue(0xF0600C24+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C28+(i*0x4000), 0xED); //0x0xED
            SetRegValue(0xF0600C2C+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C30+(i*0x4000), 0xED); //0x0xED
            SetRegValue(0xF0600C14+(i*0x4000), 0x06); //0x0x07
            SetRegValue(0xF0600C18+(i*0x4000), 0x71); //0x0xBB
            SetRegValue(0xF0600D24+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600D28+(i*0x4000), 0xE2); //0x0xE2
            SetRegValue(0xF0600DB4+(i*0x4000), 0x10); //0x0x10
            SetRegValue(0xF0602754+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0602758+(i*0x4000), 0x0F); //0x0x0F
            SetRegValue(0xF060275C+(i*0x4000), 0xFF); //0x0xFF
        }/*}}}*/
    }
#endif

#if 0
     //Test Core1 F/W loading on running.
     {/*{{{*/
         /* deserializer reset(PDB). gpio pin gpio1[18] (EVM2.0) */
         PP_U32 u32GPIODev = 1;
         PP_U32 u32GPIOCh = 18;

         // pinmux sel
         set_pinmux(gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val);
         LOG_DEBUG("Dual flash pinmux sel. group: %d, bitpos: %d, pinsel: %d\n", gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val );
         PPDRV_GPIO_SetDir(u32GPIODev, u32GPIOCh, eDIR_OUT, 0);


         LOG_DEBUG("Reset Core1\n");
         SetRegValue(0xF0100080, 0x0); //Reset Core1

         LOG_DEBUG("Reset deserializer\n");
         PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 1);

         LOG_DEBUG("load isp F/W\n");
         {
             PP_U32 u32FlashAddr;
             PP_U32 u32LoadAddr;
             PP_S32 s32Size;

             /* Core1 F/W loading DRAM Address: 0x20000000 */
             /* Read Core1 F/W flash address: 0x    , size:0x     */
             u32FlashAddr = gstFlashFTLHeader.stSect[eFLASH_SECT_IFW].u32FlashAddr;
             u32LoadAddr = gstFlashFTLHeader.stSect[eFLASH_SECT_IFW].u32LoadAddr;
             s32Size = gstFlashFTLHeader.stSect[eFLASH_SECT_IFW].u32Size;

             LOG_DEBUG(" Flash:0x%08x, Load:0x%08x, size:0x%x\n",
                     u32FlashAddr,
                     u32LoadAddr,
                     s32Size);

             PPAPI_FLASH_Read((PP_U8 *)u32LoadAddr, u32FlashAddr, s32Size);
             PPAPI_SYS_CACHE_Writeback((PP_U32 *)u32LoadAddr, s32Size);
         }

         LOG_DEBUG("Release Reset deserializer\n");
         PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 0);
        _mdelay(100);

         LOG_DEBUG("Set Core1 init vector address(0x2000)\n");
         SetRegValue(0xF1500084, 0x2000);

         LOG_DEBUG("Release Reset Core1\n");
         SetRegValue(0xF0100080, 0x1); //Release Core1 reset
     }/*}}}*/
#endif

     return(0);
}

#ifdef BD_SLT
const uint8_t pr2000_mipi4lane_1280x720_30fps[][2] = {
//                      Page0 sys
    {0xff, 0x00},
    {0x10, 0x92},
    {0x11, 0x07},
    {0x12, 0x00},
    {0x13, 0x00},
    {0x14, 0x21},    //b[1:0] => Select Camera Input. VinP(1), VinN(3), Differ(0).
    {0x15, 0x44},
    {0x16, 0x0d},
    {0x40, 0x00},
    {0x41, 0x08},
    {0x42, 0x00},
    {0x43, 0x00},
    {0x47, 0x3a},
    {0x48, 0x00},
    {0x49, 0x0a},
    {0x4e, 0x40},

    {0x54, 0x0a}, //test append
    {0x55, 0x19}, //test append

    {0x80, 0x56},
    {0x81, 0x0e},
    {0x82, 0x0d},
    {0x84, 0x30},
    {0x86, 0x20},
    {0x87, 0x00},
    {0x8a, 0x00},
    {0x90, 0x00},
    {0x91, 0x00},
    {0x92, 0x00},
    {0x94, 0xff},
    {0x95, 0xff},
    {0x96, 0xff},
    {0xa0, 0x00},
    {0xa1, 0x20},
    {0xa4, 0x01},
    {0xa5, 0xe3},
    {0xa6, 0x00},
    {0xa7, 0x12},
    {0xa8, 0x00},
    {0xd0, 0x30},
    {0xd1, 0x08},
    {0xd2, 0x21},
    {0xd3, 0x00},
    {0xd8, 0x37},
    {0xd9, 0x08},
    {0xda, 0x21},
    {0xe0, 0x19},
    {0xe1, 0x00},
    {0xe2, 0x18},
    {0xe3, 0x00},
    {0xe4, 0x00},
    {0xe5, 0x0c},
    {0xe6, 0x00},
    {0xea, 0x00},
    {0xeb, 0x00},
    {0xf1, 0x44},
    {0xf2, 0x01},

//  {          },   Page1 vdec
    {0xff, 0x01},
    {0x00, 0xe4},
    {0x01, 0x61},
    {0x02, 0x00},
    {0x03, 0x57},
    {0x04, 0x0c},
    {0x05, 0x88},
    {0x06, 0x04},
    {0x07, 0xb2},
    {0x08, 0x44},
    {0x09, 0x34},
    {0x0a, 0x02},
    {0x0b, 0x14},
    {0x0c, 0x04},
    {0x0d, 0x08},
    {0x0e, 0x5e},
    {0x0f, 0x5e},
    {0x10, 0x26},
    {0x11, 0x00},
    {0x12, 0x45},
    {0x13, 0xfc},
    {0x14, 0x00},
    {0x15, 0x18},
    {0x16, 0xd0},
    {0x17, 0x00},
    {0x18, 0x41},
    {0x19, 0x46},
    {0x1a, 0x22},
    {0x1b, 0x05},
    {0x1c, 0xea},
    {0x1d, 0x45},
    {0x1e, 0x40},
    {0x1f, 0x00},
    {0x20, 0x80},
    {0x21, 0x80},
    {0x22, 0x90},
    {0x23, 0x80},
    {0x24, 0x80},
    {0x25, 0x80},
    {0x26, 0x84},
    {0x27, 0x82},
    {0x28, 0x00},
    {0x29, 0x7b},
    {0x2a, 0xa6},
    {0x2b, 0x00},
    {0x2c, 0x00},
    {0x2d, 0x00},
    {0x2e, 0x00},
    {0x2f, 0x00},
    {0x30, 0x00},
    {0x31, 0x00},
    {0x32, 0xc0},
    {0x33, 0x14},
    {0x34, 0x14},
    {0x35, 0x80},
    {0x36, 0x80},
    {0x37, 0xaa},
    {0x38, 0x48},
    {0x39, 0x08},
    {0x3a, 0x27},
    {0x3b, 0x02},
    {0x3c, 0x01},
    {0x3d, 0x23},
    {0x3e, 0x02},
    {0x3f, 0xc4},
    {0x40, 0x05},
    {0x41, 0x55},
    {0x42, 0x01},
    {0x43, 0x33},
    {0x44, 0x6a},
    {0x45, 0x00},
    {0x46, 0x09},
    {0x47, 0xdc},
    {0x48, 0xa0},
    {0x49, 0x00},
    {0x4a, 0x7b},
    {0x4b, 0x60},
    {0x4c, 0x00},
    {0x4d, 0x4a},
    {0x4e, 0x00},
    {0x4f, 0x04},
    {0x50, 0x01},
    {0x51, 0x28},
    {0x52, 0x40},
    {0x53, 0x0c},
    {0x54, 0x0f},
    {0x55, 0x8d},
    {0x70, 0x06},
    {0x71, 0x08},
    {0x72, 0x0a},
    {0x73, 0x0c},
    {0x74, 0x0e},
    {0x75, 0x10},
    {0x76, 0x12},
    {0x77, 0x14},
    {0x78, 0x06},
    {0x79, 0x08},
    {0x7a, 0x0a},
    {0x7b, 0x0c},
    {0x7c, 0x0e},
    {0x7d, 0x10},
    {0x7e, 0x12},
    {0x7f, 0x14},
    {0x80, 0x00},
    {0x81, 0x09},
    {0x82, 0x00},
    {0x83, 0x07},
    {0x84, 0x00},
    {0x85, 0x17},
    {0x86, 0x03},
    {0x87, 0xe5},
    {0x88, 0x08},
    {0x89, 0x91},
    {0x8a, 0x08},
    {0x8b, 0x91},
    {0x8c, 0x0b},
    {0x8d, 0xe0},
    {0x8e, 0x05},
    {0x8f, 0x47},
    {0x90, 0x05},
    {0x91, 0xa0},
    {0x92, 0x73},
    {0x93, 0xe8},
    {0x94, 0x0f},
    {0x95, 0x5e},
    {0x96, 0x07},
    {0x97, 0x90},
    {0x98, 0x17},
    {0x99, 0x34},
    {0x9a, 0x13},
    {0x9b, 0x56},
    {0x9c, 0x0b},
    {0x9d, 0x9a},
    {0x9e, 0x09},
    {0x9f, 0xab},
    {0xa0, 0x01},
    {0xa1, 0x74},
    {0xa2, 0x01},
    {0xa3, 0x6b},
    {0xa4, 0x00},
    {0xa5, 0xba},
    {0xa6, 0x00},
    {0xa7, 0xa3},
    {0xa8, 0x01},
    {0xa9, 0x39},
    {0xaa, 0x01},
    {0xab, 0x39},
    {0xac, 0x00},
    {0xad, 0xc1},
    {0xae, 0x00},
    {0xaf, 0xc1},
    {0xb0, 0x09},
    {0xb1, 0xaa},
    {0xb2, 0x0f},
    {0xb3, 0xae},
    {0xb4, 0x00},
    {0xb5, 0x17},
    {0xb6, 0x08},
    {0xb7, 0xe8},
    {0xb8, 0xb0},
    {0xb9, 0xce},
    {0xba, 0x90},

    {0xbb, 0x0b}, //test append

    {0xbc, 0x00},
    {0xbd, 0x04},
    {0xbe, 0x05},
    {0xbf, 0x00},
    {0xc0, 0x00},
    {0xc1, 0x18},
    {0xc2, 0x02},
    {0xc3, 0xd0},


    {0xff, 0x01},
    {0x54, 0x0e},
    {0xff, 0x01},
    {0x54, 0x0f},

#if 0
//  {          },   Stop mipi 4lane
    {0xff, 0x00},
    {0x47, 0x3a},
    {0xff, 0x00},
    {0x40, 0x00},
    {0xff, 0x00},
    {0x4e, 0x7f},

//  {          },   Start mipi 4lane
    {0xff, 0x00},
    {0x40, 0x00},
    {0xff, 0x00},
    {0x4e, 0x40},
    {0xff, 0x00},
    {0x40, 0x40},
    {0xff, 0x00},
    {0x4e, 0x60},
    {0xff, 0x00},
    {0x4e, 0x40},
    {0xff, 0x00},
    {0x47, 0xba},
#endif

///////////////////////////////////////////////
};

const uint8_t pr2000_tp[][2] = {
//  {          },   Page1 vdec
    {0xff, 0x01},
    {0x4f, 0x14}, //0x04, BD_SLT:0x14
    {0xbb, 0x23}, //0x03, BD_SLT:0x23
//  {          },   Page2 vdec
    {0xff, 0x02},
    {0x80, 0x38}, //0x00, BD_SLT:0x38
};

const uint8_t pr2000_mipi4lane_stop_delay[][2] = {
    {0xff, 0x00},
    {0x40, 0x00},
};

const uint8_t pr2000_mipi4lane_stop[][2] = {
//  {          },   Stop mipi 4lane
    {0xff, 0x00},
    {0x47, 0x3a},
    {0xff, 0x00},
    {0x4e, 0x7f},
};

const uint8_t pr2000_mipi4lane_start[][2] = {
//  {          },   Start mipi 4lane
    {0xff, 0x00},
    {0x40, 0x00},
    {0xff, 0x00},
    {0x4e, 0x40},
    {0xff, 0x00},
    {0x40, 0x40},
    {0xff, 0x00},
    {0x4e, 0x60},
    {0xff, 0x00},
    {0x4e, 0x40},
    {0xff, 0x00},
    {0x47, 0xba},
///////////////////////////////////////////////
};

const uint8_t pr2000_mipi4lane_start_dummy[][2] = {
//  {          },   Start mipi 4lane
    {0xff, 0x00},
    {0x47, 0xba},
///////////////////////////////////////////////
};

int _slt_test_auto_mipi_rx(void)
{
    int chm = 0;
    int i = 0;
    int sz = 0;
    PP_U8 (*wbuf)[2] = NULL;
    PP_U8 addr = 0, data = 0, count;
    PP_RESULT_E ret = eSUCCESS;
    uint32_t crc_value[2];
    uint32_t status_value[6];
	PP_U32 cnt;
	PP_U32 ch;
	PP_U32 u32pBufPAddr[4] = {0};
	PP_U32 u32pRdVAddr[4] = {0};
	PP_U32 u32pBufSize[4] = {0};
	PP_U32 w = 1280, h = 720;
	PP_U32 bufOffset = (w*h*2);
	PP_U32 totalPixel = (w*h);
	PP_U8 *t0 = NULL, *t1 = NULL;
	PP_U32 diff = 0, diffCnt = 0;

	LOG_DEBUG(">>>>> SLT Test - Auto <<<<<\n");

    chm = I2CH_M0;

    PPDRV_I2C_Initialize();
    PPDRV_I2C_Reset(chm);
    PPDRV_I2C_Setup(chm, 0x5C, eI2C_SPEED_FAST, eI2C_MODE_MASTER, eI2C_ADDRESS_7BIT);	// Master, 400kb/s(FAST)
    PPDRV_I2C_Enable(chm, 1);

    //PR2000 stop mipi lane.
    {/*{{{*/
        LOG_DEBUG("PR2000 stop mipi lane.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
        sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
        /* stop mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
		_mdelay(1);
		wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
        /* stop mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
        _mdelay(300);
    }/*}}}*/

    //PI5008 Start MIPI.
    {/*{{{*/
        LOG_DEBUG("PI5008 Start MIPI.\n");

        // default init
        LOG_DEBUG("   set reset init value.\n");
        SetRegValue(0xF1700000, 0x00000001); 
        SetRegValue(0xF1700160, 0x0000001b); // 8��d6 : 250 ~ 300MHz, 8��0E : 500 ~ 600MHz
        SetRegValue(0xF17001E0, 0x0000001b); 
        SetRegValue(0xF1700260, 0x0000001b); 
        SetRegValue(0xF17002E0, 0x0000001b); 
        SetRegValue(0xF1700360, 0x0000001b); 
        SetRegValue(0xF17003E0, 0x00000000); 
        SetRegValue(0xF1700128, 0x0000000f); 
        SetRegValue(0xF0E0019C, 0x00000001); //0x0F:4lane, 0x03:2lane, 0x01:1lane
        SetRegValue(0xF0E00180, 0x00000000); //0x0F:4lane, 0x03:2lane, 0x01:1lane

        LOG_DEBUG("   start mipi.\n");
        SetRegValue(0xF1700000, 0x7D); 
        SetRegValue(0xF1700160, 0x06); // 8��d6 : 250 ~ 300MHz, 8��0E : 500 ~ 600MHz
        SetRegValue(0xF17001E0, 0x06); 
        SetRegValue(0xF1700260, 0x06); 
        SetRegValue(0xF17002E0, 0x06); 
        SetRegValue(0xF1700360, 0x06); 
        SetRegValue(0xF17003E0, 0x01); 
        SetRegValue(0xF1700128, 0x3F); 

        SetRegValue(0xF170000C, 0x03); 
        SetRegValue(0xF1700038, 0x36); 

        SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
        SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane

    }/*}}}*/

    //PR2000 Set table.
    {/*{{{*/
        /* set table */
        LOG_DEBUG("PR2000 set table\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_1280x720_30fps[0];
        sz = sizeof(pr2000_mipi4lane_1280x720_30fps) / sizeof(pr2000_mipi4lane_1280x720_30fps[0]);
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PR2000 Set TP.
    {/*{{{*/
        LOG_DEBUG("PR2000 set tp.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_tp[0];
        sz = sizeof(pr2000_tp) / sizeof(pr2000_tp[0]);
        /* start mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PR2000 Start MIPI lane.
    {/*{{{*/
        LOG_DEBUG("PR2000 start mipi lane.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
        sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
        /* start mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PI5008 Change MIPI input
    {/*{{{*/
        uint32_t temp;

        LOG_DEBUG("PI5008 Change mipi input\n");

        temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFFFFF0;
        temp |= 0x00000008;
        SetRegValue(0xF0F0000C, temp); 

        temp = GetRegValue(0xF0F00014); 
        temp &= 0xFFFFFFF0;
        temp |= 0x00000000;
        SetRegValue(0xF0F00014, temp); 

        temp = GetRegValue(0xF0F00044); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00044, temp); 

        _mdelay(300);
    }/*}}}*/

    //PI5008 Wait MIPI input valid
    {/*{{{*/
        LOG_DEBUG("PI5008 Wait mipi input valid\n");

        status_value[0] = GetRegValue(0xF0F00400); 
        LOG_DEBUG("0xF0F00400:0x%08x\n", status_value[0]);

        if(status_value[0] & 0x10000000)
        {/*{{{*/
            int loop = 0;
            do
            {
                LOG_DEBUG("try sync cnt(%d)\n", loop+1);

                LOG_DEBUG("stop mipi lane\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
				_mdelay(1);
				wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
		        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
		        /* stop mipi lane */
		        for (i = 0; i < sz; i++) {
		            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
		            if(ret != eSUCCESS){
		                LOG_DEBUG("I2C Write fail(%d)\n", ret);
		            }
		        }
                _mdelay(300);

                LOG_DEBUG("start mipi lane\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
                _mdelay(300);

                status_value[0] = GetRegValue(0xF0F00400); 
                LOG_DEBUG("400:0x%08x\n", status_value[0]);
            } while( (status_value[0] & 0x10000000) && (loop++ < 10) );
        }/*}}}*/
    }/*}}}*/

    //PI5008 Set Quad display to PVI Tx
    {/*{{{*/
        LOG_DEBUG("PI5008 Set Quad display to PVI Tx\n");

		SetRegValue(0xF0900004, 0x1F0); ///< DU - QUAD Image for PVI Tx
        SetRegValue(0xF0F00018, 0x00);	///< QUAD0~3_CH Port Selection (VIN0->QUAD0, VIN0->QUAD1, VIN0->QUAD2, VIN0->QUAD3)
    }/*}}}*/

    //PI5008 Get CRC value. and output GPIO
    {/*{{{*/
        /* gpio pin gpio1[7] ) */
        PP_U32 u32GPIODev = 0;
        PP_U32 u32GPIOCh = 7;

        LOG_DEBUG("Get CRC value. and output GPIO.\n");

        // pinmux sel
        set_pinmux(gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val);
        LOG_DEBUG("pinmux sel. group: %d, bitpos: %d, pinsel: %d\n", gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val );
        PPDRV_GPIO_SetDir(u32GPIODev, u32GPIOCh, eDIR_OUT, 0);
        PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 0);

        _mdelay(500);

        crc_value[0] = GetRegValue(0xF0F00284); //0x8000DFE5
        crc_value[1] = GetRegValue(0xF0F002D4); //0x80002C75

        LOG_DEBUG("CRC value..0xF0F00284(0x8000DFE5):0x%08x, 0xF0F002D4(0x80002C75):0x%08x\n", crc_value[0], crc_value[1]);

        LOG_DEBUG("TODO! Compare value.. If ok, Gpio7 Low(default)->High\n");
        if( (crc_value[0] == 0x8000DFE5) && (crc_value[1] == 0x80002C75) )
        {

            LOG_DEBUG("OK, Gpio7 Low(default)->High\n");
            PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 1);
        }
    }/*}}}*/

    return(0);
}

typedef union ppIPC_CMD_U
{
	PP_U32 var;
	struct{
		PP_U32 	Rsv		:16,
				Cmd		:8,
				Attr	:4,
				RecvId	:2,
				SendId	:2;
	};
}IPC_CMD_U;

int _core1_test(int argc, const char **argv)
{
#define CORE1_D0_IDX	3
#define CORE1_D1_IDX	4
#define CORE1_D2_IDX	5
#define IPC_C0_CMD_EXEC_EXTERN_FUNC	0x0C

	IPC_CMD_U stCmd;
	PP_U32 data = 0x0;
	PP_U32 u32TimeOut = 0x0;
	PP_U32 u32TimeOutCnt = 0;
	PP_CHAR *strArgv = NULL;

	if ((argc <= 0) || (argv == NULL))
	{
		return(-1);
	}

	strArgv = (char *)argv[1];
	data = strtoul(strArgv, NULL, 16);

	while(PPDRV_MBOX_GetIRQMsg(1)){
		if(u32TimeOut != 0xffffffff && (++u32TimeOutCnt > u32TimeOut))
			break;
	}

	PPDRV_MBOX_SetData(CORE1_D0_IDX, (uint32)data);

	stCmd.SendId = PI5008_CORE_ID;
	stCmd.RecvId = PI5008_CORE_1;
	stCmd.Cmd    = IPC_C0_CMD_EXEC_EXTERN_FUNC;
	stCmd.Attr   = 0;

	PPDRV_MBOX_SetIRQMsg(1, (PP_U32)stCmd.var);

	return 0;
}

int _slt_test_auto_pvi_rx(void)
{
    int chm = 0;
    int sz = 0;
    PP_U8 (*wbuf)[2] = NULL;
    PP_U8 addr = 0, data = 0, count;
    PP_RESULT_E ret = eSUCCESS;
    uint32_t crc_value[2];
    uint32_t status_value[6];
	PP_U32 cnt, ch, i, j, p;
	PP_U32 u32pBufPAddr[4][2] = {{0}};
	PP_U32 u32pRdVAddr[4][2] = {{0}};
	PP_U32 u32pBufSize[4] = {0};
	PP_U32 w = 640, h = 360;
	PP_U32 bufOffset = (w*h*2);
	PP_U32 totalPixel = (w*h);
   	PP_U32 line = (640*2);
 	PP_U8 *t0 = NULL, *t1 = NULL;
	PP_U8 *t0_0 = NULL, *t0_1 = NULL;
	PP_U8 *t1_0 = NULL, *t1_1 = NULL;
	PP_U8 *t2_0 = NULL, *t2_1 = NULL;
	PP_U8 *t3_0 = NULL, *t3_1 = NULL;
	PP_U32 diff = 0, diffCnt[4] = {0};

	LOG_DEBUG(">>>>> SLT Test - Auto <<<<<\n");

    chm = I2CH_M0;

    PPDRV_I2C_Initialize();
    PPDRV_I2C_Reset(chm);
    PPDRV_I2C_Setup(chm, 0x5C, eI2C_SPEED_FAST, eI2C_MODE_MASTER, eI2C_ADDRESS_7BIT);	// Master, 400kb/s(FAST)
    PPDRV_I2C_Enable(chm, 1);

    //PR2000 stop mipi lane.
    {/*{{{*/
        LOG_DEBUG("PR2000 stop mipi lane.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
        sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
        /* stop mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
		_mdelay(1);
		wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
        /* stop mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
        _mdelay(300);
    }/*}}}*/

    //PI5008 Start MIPI.
    {/*{{{*/
        LOG_DEBUG("PI5008 Start MIPI.\n");

        // default init
        LOG_DEBUG("   set reset init value.\n");
        SetRegValue(0xF1700000, 0x00000001); 
        SetRegValue(0xF1700160, 0x0000001b); // 8��d6 : 250 ~ 300MHz, 8��0E : 500 ~ 600MHz
        SetRegValue(0xF17001E0, 0x0000001b); 
        SetRegValue(0xF1700260, 0x0000001b); 
        SetRegValue(0xF17002E0, 0x0000001b); 
        SetRegValue(0xF1700360, 0x0000001b); 
        SetRegValue(0xF17003E0, 0x00000000); 
        SetRegValue(0xF1700128, 0x0000000f); 
        SetRegValue(0xF0E0019C, 0x00000001); //0x0F:4lane, 0x03:2lane, 0x01:1lane
        SetRegValue(0xF0E00180, 0x00000000); //0x0F:4lane, 0x03:2lane, 0x01:1lane

        LOG_DEBUG("   start mipi.\n");
        SetRegValue(0xF1700000, 0x7D); 
        SetRegValue(0xF1700160, 0x06); // 8��d6 : 250 ~ 300MHz, 8��0E : 500 ~ 600MHz
        SetRegValue(0xF17001E0, 0x06); 
        SetRegValue(0xF1700260, 0x06); 
        SetRegValue(0xF17002E0, 0x06); 
        SetRegValue(0xF1700360, 0x06); 
        SetRegValue(0xF17003E0, 0x01); 
        SetRegValue(0xF1700128, 0x3F); 

        SetRegValue(0xF170000C, 0x03); 
        SetRegValue(0xF1700038, 0x36); 

        SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
        SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane

    }/*}}}*/

	//PI5008 Quad HLPF Mode Selection for Y/C
    {/*{{{*/
        uint32_t temp;

        temp = GetRegValue(0xF0F00100);
        temp &= 0xFFFFF000;
        temp |= 0x00000040;
        SetRegValue(0xF0F00100, temp);

        temp = GetRegValue(0xF0F00110);
        temp &= 0xFFFFF000;
        temp |= 0x00000040;
        SetRegValue(0xF0F00110, temp);

        temp = GetRegValue(0xF0F00120);
        temp &= 0xFFFFF000;
        temp |= 0x00000040;
        SetRegValue(0xF0F00120, temp);

		temp = GetRegValue(0xF0F00130);
        temp &= 0xFFFFF000;
        temp |= 0x00000040;
        SetRegValue(0xF0F00130, temp);
    }/*}}}*/

    //PR2000 Set table.
    {/*{{{*/
        /* set table */
        LOG_DEBUG("PR2000 set table\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_1280x720_30fps[0];
        sz = sizeof(pr2000_mipi4lane_1280x720_30fps) / sizeof(pr2000_mipi4lane_1280x720_30fps[0]);
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PR2000 Start MIPI lane.
    {/*{{{*/
        LOG_DEBUG("PR2000 start mipi lane.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
        sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
        /* start mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PI5008 Change MIPI input
    {/*{{{*/
        uint32_t temp;

        LOG_DEBUG("PI5008 Change mipi input\n");

        temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFFFFF0;
        temp |= 0x00000008;
        SetRegValue(0xF0F0000C, temp); 

        temp = GetRegValue(0xF0F00014); 
        temp &= 0xFFFFFFF0;
        temp |= 0x00000000;
        SetRegValue(0xF0F00014, temp); 

        temp = GetRegValue(0xF0F00044); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00000000;
        SetRegValue(0xF0F00044, temp); 

        _mdelay(300);
    }/*}}}*/

    //PI5008 Wait MIPI input valid
    {/*{{{*/
        LOG_DEBUG("PI5008 Wait mipi input valid\n");

        status_value[0] = GetRegValue(0xF0F00400); 
        LOG_DEBUG("0xF0F00400:0x%08x\n", status_value[0]);

        if(status_value[0] & 0x10000000)
        {/*{{{*/
            int loop = 0;
            do
            {
                LOG_DEBUG("try sync cnt(%d)\n", loop+1);

                LOG_DEBUG("stop mipi lane\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
				_mdelay(1);
				wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
		        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
		        /* stop mipi lane */
		        for (i = 0; i < sz; i++) {
		            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
		            if(ret != eSUCCESS){
		                LOG_DEBUG("I2C Write fail(%d)\n", ret);
		            }
		        }
                _mdelay(300);

                LOG_DEBUG("start mipi lane\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
                _mdelay(300);

                status_value[0] = GetRegValue(0xF0F00400); 
                LOG_DEBUG("400:0x%08x\n", status_value[0]);
            } while( (status_value[0] & 0x10000000) && (loop++ < 10) );
        }/*}}}*/
    }/*}}}*/

    //PI5008 Set Quad display to PVI Tx
    {/*{{{*/
        LOG_DEBUG("PI5008 Set Quad display to PVI Tx\n");

		//SetRegValue(0xF0F0000C, 0xFFF88888);	///< VIN0~3 - MIPI Rx
		//SetRegValue(0xF0F0000C, 0xFFFEEEEE);	///< VIN2 - PVI Rx
		SetRegValue(0xF0F0000C, 0xFFFEFEDC);	///< VIN0~3 - PVI Rx
        SetRegValue(0xF0F00018, 0x3210);		///< DU - QUAD0~3_CH Port Selection (VIN0->QUAD0, VIN1->QUAD1, VIN2->QUAD2, VIN3->QUAD3)
		SetRegValue(0xF0900004, 0x1F0); 		///< QUAD Image for PVI Tx
        //SetRegValue(0xF0F00018, 0x00);		///< QUAD0_CH Port Selection (VIN0->QUAD0)
        //SetRegValue(0xF0900004, 0x1F0); 		///< QUAD Image for PVI Tx
    }/*}}}*/

	_mdelay(1000);

#if 1
	//PI5008 Capture Two QUAD0~3_CH output images and compare
    {/*{{{*/
    	void *pfHandle = NULL;
        void *pBuf = NULL;
        uint32 bufSize = 0;
		char sdFileName[80] = {'\0'};

		for(ch = 0; ch < 4; ch++) {
	    	for(i = 0; i < 2; i++) {
	            u32pBufPAddr[ch][i] = (uint32)gpVPUConfig->pBufCapture[0] + (bufOffset * i);
	            u32pRdVAddr[ch][i]  = (uint32)gpVPUConfig->pVBufCapture[0] + (bufOffset * i);
	            //LOG_DEBUG("Buf[%d][%d] P:0x%08x, V:0x%08x\n", ch, i, u32pBufPAddr[ch][i], u32pRdVAddr[ch][i]);

				///< 0: UYVY, PP_FALSE: 4Ch Quad Mode
				if (PPAPI_VIN_GetCaptureImage(BD_VIN_FMT, 0, u32pBufPAddr[ch][i], &u32pBufSize[ch], ch, PP_FALSE) != eSUCCESS) {
		        	LOG_WARNING("[VIN(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
		       		return eERROR_INVALID_ARGUMENT;
		    	}
				//LOG_DEBUG("pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32pBufPAddr[ch][i], u32pBufPAddr[ch][i]+u32pBufSize[ch], u32pBufSize[ch]);
#if 0
				//PI5008 SD card write.
				//for(j = 0; j < 2; j++)
				{
					pBuf = (void *)(uint32)u32pRdVAddr[ch][i];
					bufSize = u32pBufSize[ch];
					sprintf(sdFileName, "test_ch%d_%d.yuv", ch, i);
					LOG_DEBUG("sd write: %s, size:%d\n", sdFileName, bufSize);
		
					PPAPI_FATFS_DelVolume();
					if (!PPAPI_FATFS_InitVolume()) {
						pfHandle = PPAPI_FATFS_Open(sdFileName,"w");
						if ((PPAPI_FATFS_Write(pBuf, 1, bufSize, pfHandle) != bufSize))
							LOG_DEBUG("Error can't write(%s)\n", sdFileName);
		
						PPAPI_FATFS_Close(pfHandle);
					}
					else
						LOG_DEBUG("Error can't open(%s)\n", sdFileName);
		
					LOG_DEBUG("Done write(%s)\n", sdFileName);
		
				}
#endif
	        }

			t0 = (PP_U8 *)u32pBufPAddr[ch][0];
			t1 = (PP_U8 *)u32pBufPAddr[ch][1];

			for (cnt = 0; cnt < bufOffset; cnt++) {
				diff = abs(t0[cnt] - t1[cnt]);
				if (diff > 20) {
					++diffCnt[ch];
				}
			}

			LOG_DEBUG(">>>>> CH_%d Diff: %d <<<<<\n", ch, diffCnt[ch]);
		}
#endif
    }/*}}}*/

    //PI5008 Get CRC value. and output GPIO
    {/*{{{*/
        /* gpio pin gpio1[7] ) */
        PP_U32 u32GPIODev = 0;
        PP_U32 u32GPIOCh = 7;

        // pinmux sel
        set_pinmux(gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val);
        LOG_DEBUG("pinmux sel. group: %d, bitpos: %d, pinsel: %d\n", gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val );
        PPDRV_GPIO_SetDir(u32GPIODev, u32GPIOCh, eDIR_OUT, 0);
        PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 0);

        _mdelay(500);

		if (diffCnt[0] <= 100 && diffCnt[1] <= 100 &&
			diffCnt[2] <= 100 && diffCnt[3] <= 100) {
			LOG_DEBUG("SLT Result: SUCCESS\n");
	        LOG_DEBUG("OK, Gpio7 Low(default)->High\n");
    	    PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 1);
		}
		else
			LOG_DEBUG("SLT Result: FAILE\n");
    }/*}}}*/

    return(0);
}

int _slt_test_manual(void)
{
    int chm = 0;
    int i = 0;
    int sz = 0;
    PP_U8 (*wbuf)[2] = NULL;
    PP_U8 addr = 0, data = 0, count;
    PP_RESULT_E ret = eSUCCESS;
    uint32_t crc_value[2];
    uint32_t status_value[6];

	LOG_NOTICE(">>>>> SLT Test - Manual <<<<<\n");

    chm = I2CH_M0;

    PPDRV_I2C_Initialize();
    PPDRV_I2C_Reset(chm);
    PPDRV_I2C_Setup(chm, 0x5C, eI2C_SPEED_FAST, eI2C_MODE_MASTER, eI2C_ADDRESS_7BIT);	// master, 100kb/s
    PPDRV_I2C_Enable(chm, 1);

    //PR2000 stop mipi lane.
    {/*{{{*/
        LOG_DEBUG("PR2000 stop mipi lane.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
        sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
        /* stop mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
		_mdelay(1);
		wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
        /* stop mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
        _mdelay(300);
    }/*}}}*/

    //PI5008 Start MIPI.
    {/*{{{*/
        LOG_DEBUG("PI5008 Start MIPI.\n");

        // default init
        LOG_DEBUG("   set reset init value.\n");
        SetRegValue(0xF1700000, 0x00000001); 
        SetRegValue(0xF1700160, 0x0000001b); // 8��d6 : 250 ~ 300MHz, 8��0E : 500 ~ 600MHz
        SetRegValue(0xF17001E0, 0x0000001b); 
        SetRegValue(0xF1700260, 0x0000001b); 
        SetRegValue(0xF17002E0, 0x0000001b); 
        SetRegValue(0xF1700360, 0x0000001b); 
        SetRegValue(0xF17003E0, 0x00000000); 
        SetRegValue(0xF1700128, 0x0000000f); 
        SetRegValue(0xF0E0019C, 0x00000001); //0x0F:4lane, 0x03:2lane, 0x01:1lane
        SetRegValue(0xF0E00180, 0x00000000); //0x0F:4lane, 0x03:2lane, 0x01:1lane

        LOG_DEBUG("   start mipi.\n");
        SetRegValue(0xF1700000, 0x7D); 
        SetRegValue(0xF1700160, 0x06); // 8��d6 : 250 ~ 300MHz, 8��0E : 500 ~ 600MHz
        SetRegValue(0xF17001E0, 0x06); 
        SetRegValue(0xF1700260, 0x06); 
        SetRegValue(0xF17002E0, 0x06); 
        SetRegValue(0xF1700360, 0x06); 
        SetRegValue(0xF17003E0, 0x01); 
        SetRegValue(0xF1700128, 0x3F); 

        SetRegValue(0xF170000C, 0x03); 
        SetRegValue(0xF1700038, 0x36); 

        SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
        SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane

    }/*}}}*/

	_mdelay(10);

    //PR2000 Set table.
    {/*{{{*/
        /* set table */
        LOG_DEBUG("PR2000 set table\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_1280x720_30fps[0];
        sz = sizeof(pr2000_mipi4lane_1280x720_30fps) / sizeof(pr2000_mipi4lane_1280x720_30fps[0]);
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PR2000 start mipi lane.
    {/*{{{*/
        LOG_DEBUG("PR2000 start mipi lane.\n");
        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
        sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
        /* start mipi lane */
        for (i = 0; i < sz; i++) {
            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
            if(ret != eSUCCESS){
                LOG_DEBUG("I2C Write fail(%d)\n", ret);
            }
        }
    }/*}}}*/

    //PI5008 Change mipi input
    {/*{{{*/
        uint32_t temp;

        LOG_DEBUG("PI5008 Change mipi input\n");

        temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFFFFF0;
        temp |= 0x00000008;
        SetRegValue(0xF0F0000C, temp); 

        temp = GetRegValue(0xF0F00014); 
        temp &= 0xFFFFFFF0;
        temp |= 0x00000000;
        SetRegValue(0xF0F00014, temp); 

        temp = GetRegValue(0xF0F00044); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00044, temp); 

        _mdelay(300);
    }/*}}}*/

    //PI5008 Wait mipi input valid
    {/*{{{*/
        LOG_DEBUG("PI5008 Wait mipi input valid\n");
        status_value[0] = GetRegValue(0xF0F00400); 
        LOG_DEBUG("0xF0F00400:0x%08x\n", status_value[0]);

        if(status_value[0] & 0x10000000)
        {/*{{{*/
            int loop = 0;
            do
            {
                LOG_DEBUG("try sync cnt(%d)\n", loop+1);

                LOG_DEBUG("stop mipi lane\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
				_mdelay(1);
				wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
		        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
		        /* stop mipi lane */
		        for (i = 0; i < sz; i++) {
		            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
		            if(ret != eSUCCESS){
		                LOG_DEBUG("I2C Write fail(%d)\n", ret);
		            }
		        }
                _mdelay(300);

                LOG_DEBUG("start mipi lane\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
                _mdelay(300);

                status_value[0] = GetRegValue(0xF0F00400); 
                LOG_DEBUG("400:0x%08x\n", status_value[0]);
            } while( (status_value[0] & 0x10000000) && (loop++ < 10) );
        }/*}}}*/
    }/*}}}*/

    //Set ISP 4ch TP 
    {/*{{{*/
        int i;
        LOG_DEBUG("Set ISP 4ch TP\n"); 
        for(i = 0; i < 4; i++)
        {/*{{{*/
#if 1
            //                       720p30,  720p25
			SetRegValue(0xF0600FB8+(i*0x4000), (0x91 + i));
#if 0
			SetRegValue(0xF0600FBC+(i*0x4000), 0xFF); //0x0x00
			SetRegValue(0xF0600FC0+(i*0x4000), 0xFF); //0x0xFF
			SetRegValue(0xF0600FC4+(i*0x4000), 0xF0); //0x0xF0
			SetRegValue(0xF0600FC8+(i*0x4000), 0xFF); //0x0x00
			SetRegValue(0xF0600FCC+(i*0x4000), 0xFF); //0x0xFF
			SetRegValue(0xF0600FD0+(i*0x4000), 0xF0); //0x0xF0
			SetRegValue(0xF0600FD4+(i*0x4000), 0xFF); //0x0x00
			SetRegValue(0xF0600FD8+(i*0x4000), 0xFF); //0x0xFF
			SetRegValue(0xF0600FDC+(i*0x4000), 0xF0); //0x0xF0
			SetRegValue(0xF0600FE0+(i*0x4000), 0xFF); //0x0x00
			SetRegValue(0xF0600FE4+(i*0x4000), 0xFF); //0x0xFF
			SetRegValue(0xF0600FE8+(i*0x4000), 0xF0); //0x0xF0
			SetRegValue(0xF0600FEC+(i*0x4000), 0x07); //0x0x05
			SetRegValue(0xF0600FF0+(i*0x4000), 0x80); //0x0x00
#endif
			SetRegValue(0xF0600C38+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF0600C3C+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600C40+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C44+(i*0x4000), 0xD0); //0x0xD0
            SetRegValue(0xF0601360+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601364+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601368+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF060136C+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601370+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601374+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601378+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF060137C+(i*0x4000), 0xD0); //0x0xD0
            SetRegValue(0xF0600C24+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C28+(i*0x4000), 0xED); //0x0xED
            SetRegValue(0xF0600C2C+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C30+(i*0x4000), 0xED); //0x0xED
            SetRegValue(0xF0600C14+(i*0x4000), 0x06); //0x0x07
            SetRegValue(0xF0600C18+(i*0x4000), 0x71); //0x0xBB
            SetRegValue(0xF0600D24+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600D28+(i*0x4000), 0xE2); //0x0xE2
            SetRegValue(0xF0600DB4+(i*0x4000), 0x10); //0x0x10
            SetRegValue(0xF0602754+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0602758+(i*0x4000), 0x0F); //0x0x0F
            SetRegValue(0xF060275C+(i*0x4000), 0xFF); //0x0xFF
            _mdelay(100);
#else
            //                       720p30,  720p25
            SetRegValue(0xF0600FB8+(i*0x4000), 0x81); //0x0x81
            SetRegValue(0xF0600FBC+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FC0+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FC4+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FC8+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FCC+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FD0+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FD4+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FD8+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FDC+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FE0+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600FE4+(i*0x4000), 0xFF); //0x0xFF
            SetRegValue(0xF0600FE8+(i*0x4000), 0xF0); //0x0xF0
            SetRegValue(0xF0600FEC+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF0600FF0+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600C38+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF0600C3C+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0600C40+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C44+(i*0x4000), 0xD0); //0x0xD0
            SetRegValue(0xF0601360+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601364+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601368+(i*0x4000), 0x05); //0x0x05
            SetRegValue(0xF060136C+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601370+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601374+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0601378+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF060137C+(i*0x4000), 0xD0); //0x0xD0
            SetRegValue(0xF0600C24+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C28+(i*0x4000), 0xED); //0x0xED
            SetRegValue(0xF0600C2C+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600C30+(i*0x4000), 0xED); //0x0xED
            SetRegValue(0xF0600C14+(i*0x4000), 0x06); //0x0x07
            SetRegValue(0xF0600C18+(i*0x4000), 0x71); //0x0xBB
            SetRegValue(0xF0600D24+(i*0x4000), 0x02); //0x0x02
            SetRegValue(0xF0600D28+(i*0x4000), 0xE2); //0x0xE2
            SetRegValue(0xF0600DB4+(i*0x4000), 0x10); //0x0x10
            SetRegValue(0xF0602754+(i*0x4000), 0x00); //0x0x00
            SetRegValue(0xF0602758+(i*0x4000), 0x0F); //0x0x0F
            SetRegValue(0xF060275C+(i*0x4000), 0xFF); //0x0xFF
#endif
        }/*}}}*/
    }/*}}}*/

    //Display PVI 4ch
    {/*{{{*/
        uint32_t temp;

        LOG_DEBUG("Display PVI 4ch input\n");

		temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFFF000;
        temp |= 0x000001F0;
		SetRegValue(0xF0900004, temp); 		///< QUAD Image for PVI Tx
		
        temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFF0000;
        temp |= 0x0000FEDC;
        SetRegValue(0xF0F0000C, temp); 

        temp = GetRegValue(0xF0F00014); 
        temp &= 0xFFFF0000;
        temp |= 0x00003210;
        SetRegValue(0xF0F00014, temp); 

        temp = GetRegValue(0xF0F00044); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00000000;
        SetRegValue(0xF0F00044, temp); 

        temp = GetRegValue(0xF0F00054); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00000000;
        SetRegValue(0xF0F00054, temp); 

        temp = GetRegValue(0xF0F00064); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00000000;
        SetRegValue(0xF0F00064, temp); 

        temp = GetRegValue(0xF0F00074); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00000000;
        SetRegValue(0xF0F00074, temp); 
    }/*}}}*/

    _mdelay(3000);

    //Display mipi 1ch
    {/*{{{*/
        uint32_t temp;

        LOG_DEBUG("Display MIPI 1ch input\n");

		temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFFF000;
        temp |= 0x000001F0;
		SetRegValue(0xF0900004, temp); 		///< DU - QUAD Image for PVI Tx

        temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFF00000;
        temp |= 0x00088888;
        SetRegValue(0xF0F0000C, temp); 

        temp = GetRegValue(0xF0F00014); 
        temp &= 0xFFFF0000;
        temp |= 0x00003210;
        SetRegValue(0xF0F00014, temp); 

        temp = GetRegValue(0xF0F00044); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00044, temp);

		temp = GetRegValue(0xF0F00054); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00054, temp);

		temp = GetRegValue(0xF0F00064); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00064, temp);

		temp = GetRegValue(0xF0F00074); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00074, temp);
    }/*}}}*/

    _mdelay(3000);

    //Display isp 4ch
    {/*{{{*/
    	uint32_t temp;

        LOG_DEBUG("Display ISP 4ch input\n");

		temp = GetRegValue(0xF0F00010);
        temp &= 0xFF000000;
        temp |= 0x00F0F0AE;
		SetRegValue(0xF0F00010, temp);		///< REC sync gen input path (MUX, ISP Output0~3)

		temp = GetRegValue(0xF0900004);
        temp &= 0xFFFFF000;
        temp |= 0x000001F0;
		SetRegValue(0xF0900004, temp); 		///< DU - QUAD Image for PVI Tx

        temp = GetRegValue(0xF0F0000C); 
        temp &= 0xFFFF0000;
        temp |= 0x0000BA98;
        SetRegValue(0xF0F0000C, temp); 

        temp = GetRegValue(0xF0F00014); 
        temp &= 0xFFFF0000;
        temp |= 0x0000FEDC;
        SetRegValue(0xF0F00014, temp); 

		temp = GetRegValue(0xF0F00018); 
        temp &= 0xFFFF0000;
        temp |= 0x00003210;
        SetRegValue(0xF0F00018, temp); 		///< QUAD0~3_CH Port Selection (VIN0->QUAD0, VIN1->QUAD1, VIN2->QUAD2, VIN3->QUAD3)

        temp = GetRegValue(0xF0F00044); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00044, temp); 

        temp = GetRegValue(0xF0F00054); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00054, temp); 

        temp = GetRegValue(0xF0F00064); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00064, temp); 

        temp = GetRegValue(0xF0F00074); 
        temp &= 0xFFC0FFFF;
        temp |= 0x00100000;
        SetRegValue(0xF0F00074, temp); 
    }/*}}}*/

    return(0);
}

int _slt_test(int argc, const char **argv)
{
	char *strArgv = NULL;
	PP_U32 cnt = 0;

    int chm = 0;
    int i = 0;
    int sz = 0;
    PP_U8 (*wbuf)[2] = NULL;
    PP_U8 addr = 0, data = 0, count;
    PP_RESULT_E ret = eSUCCESS;
    uint32_t crc_value[2];
    uint32_t status_value[6];


	if( (argc <= 0) || (argv == NULL) )
	{
		return(-1);
	}

	LOG_DEBUG("SLT Test set\n");

    chm = I2CH_M0;

    PPDRV_I2C_Initialize();
    PPDRV_I2C_Reset(chm);
    PPDRV_I2C_Setup(chm, 0x5C, eI2C_SPEED_FAST, eI2C_MODE_MASTER, eI2C_ADDRESS_7BIT);	// master, 100kb/s
    PPDRV_I2C_Enable(chm, 1);


    strArgv = (char *)argv[1];

	if (argc >= 3 && (!strncmp(strArgv, "init", sizeof("init"))
						|| !strncmp(strArgv, "manual", sizeof("manual"))
						|| !strncmp(strArgv, "auto", sizeof("auto"))))
	{
 		if ((char *)argv[2] != NULL)
			cnt = atoi(argv[2]);	
	}

    if(argc >= 3 && (strncmp(strArgv, "init", sizeof("init"))
						&& strncmp(strArgv, "manual", sizeof("manual"))
						&& strncmp(strArgv, "auto", sizeof("auto"))))
    {
        if( !strncmp(strArgv, "r", sizeof("r")) )
        {/*{{{*/
            if(argc == 3) 
            {/*{{{*/
                addr = simple_strtoul(argv[2], 16);
                count = 1;
            }/*}}}*/
            else if(argc == 4) 
            {/*{{{*/
                addr = simple_strtoul(argv[2], 16);
                count = simple_strtoul(argv[3], 10);
            }/*}}}*/

            LOG_DEBUG("0x%02X: ", addr);
            for(i = 0; i < count; i++)
            {
                PPDRV_I2C_Write(0, &addr, 1, 1000); //address
                PPDRV_I2C_Read(0, &data, 1, 1000); //read data
                LOG_DEBUG("0x%02X ", data);
                addr += 0x1;
                if( (addr & 0xF) == 0 )
                {
                    LOG_DEBUG("\r\n0x%02X: ", addr);
                }
            }
            LOG_DEBUG("\n");

        }/*}}}*/
        else if( !strncmp(strArgv, "w", sizeof("w")) )
        {/*{{{*/
            if(argc == 4) //write
            {/*{{{*/
                PP_U8 temp[2] = {0, };
                addr = simple_strtoul(argv[2], 16);
                data = simple_strtoul(argv[3], 16);
                temp[0] = addr;
                temp[1] = data;

                PPDRV_I2C_Write(0, temp, 2, 1000); //address
                LOG_DEBUG("write address:0x%02X, data:0x%02X\n", addr, data);
            }/*}}}*/
        }/*}}}*/
    }
    else
    {/*{{{*/
        if( !strncmp(strArgv, "mipi", sizeof("mipi")) )
        {/*{{{*/
            uint32_t temp;

            LOG_DEBUG("Display MIPI 1ch input\n");

            temp = GetRegValue(0xF0F0000C); 
            temp &= 0xFFFFFFF0;
            temp |= 0x00000008;
            SetRegValue(0xF0F0000C, temp); 

            temp = GetRegValue(0xF0F00014); 
            temp &= 0xFFFF0000;
            temp |= 0x00003210;
            SetRegValue(0xF0F00014, temp); 

            temp = GetRegValue(0xF0F00044); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00044, temp); 

        }/*}}}*/
        else if( !strncmp(strArgv, "pvi", sizeof("pvi")) )
        {/*{{{*/
            uint32_t temp;

            LOG_DEBUG("Display PVI 4ch input\n");

            temp = GetRegValue(0xF0F0000C); 
            temp &= 0xFFFF0000;
            temp |= 0x0000FEDC;
            SetRegValue(0xF0F0000C, temp); 

            temp = GetRegValue(0xF0F00014); 
            temp &= 0xFFFF0000;
            temp |= 0x00003210;
            SetRegValue(0xF0F00014, temp); 

            temp = GetRegValue(0xF0F00044); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00000000;
            SetRegValue(0xF0F00044, temp); 

            temp = GetRegValue(0xF0F00054); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00000000;
            SetRegValue(0xF0F00054, temp); 

            temp = GetRegValue(0xF0F00064); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00000000;
            SetRegValue(0xF0F00064, temp); 

            temp = GetRegValue(0xF0F00074); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00000000;
            SetRegValue(0xF0F00074, temp); 
        }/*}}}*/
        else if( !strncmp(strArgv, "isp", sizeof("isp")) )
        {/*{{{*/
            uint32_t temp;

            LOG_DEBUG("Display ISP 4ch input\n");

            temp = GetRegValue(0xF0F0000C); 
            temp &= 0xFFFF0000;
            temp |= 0x0000BA98;
            SetRegValue(0xF0F0000C, temp); 

            temp = GetRegValue(0xF0F00014); 
            temp &= 0xFFFF0000;
            temp |= 0x0000FEDC;
            SetRegValue(0xF0F00014, temp); 

            temp = GetRegValue(0xF0F00044); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00044, temp); 

            temp = GetRegValue(0xF0F00054); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00054, temp); 

            temp = GetRegValue(0xF0F00064); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00064, temp); 

            temp = GetRegValue(0xF0F00074); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00074, temp); 
        }/*}}}*/
        else if( !strncmp(strArgv, "init", sizeof("init")) )
        {/*{{{*/
        	while (cnt-- > 0) {/*{{{*/
			LOG_DEBUG("[%d]SLT_TEST - init \n", cnt);
            //PR2000 stop mipi lane.
            {/*{{{*/
                LOG_DEBUG("PR2000 stop mipi lane.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
				_mdelay(1);
				wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
		        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
		        /* stop mipi lane */
		        for (i = 0; i < sz; i++) {
		            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
		            if(ret != eSUCCESS){
		                LOG_DEBUG("I2C Write fail(%d)\n", ret);
		            }
		        }
                _mdelay(300);
            }/*}}}*/
			
            //PR2000 Set table.
            {/*{{{*/
                /* set table */
                LOG_DEBUG("PR2000 set table\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_1280x720_30fps[0];
                sz = sizeof(pr2000_mipi4lane_1280x720_30fps) / sizeof(pr2000_mipi4lane_1280x720_30fps[0]);
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/

#if 0
            //PI5008 Start MIPI.
            {/*{{{*/
                LOG_DEBUG("PI5008 Start MIPI.\n");

                // default init
                LOG_DEBUG("   set reset/release init value.\n");
                SetRegValue(0xF1700000+(0x20<<2), 0x1C); // Enable Internal Reset Digital & Analog Logic 
                SetRegValue(0xF1700000+(0x20<<2), 0x1F); // Disable Internal Reset Digital & Analog Logic 
#if 0
                SetRegValue(0xF1700000, 0x00000001); 
                SetRegValue(0xF1700160, 0x0000001b); // 8?�d6 : 250 ~ 300MHz, 8??E : 500 ~ 600MHz
                SetRegValue(0xF17001E0, 0x0000001b); 
                SetRegValue(0xF1700260, 0x0000001b); 
                SetRegValue(0xF17002E0, 0x0000001b); 
                SetRegValue(0xF1700360, 0x0000001b); 
                SetRegValue(0xF17003E0, 0x00000000); 
                SetRegValue(0xF1700128, 0x0000000f); 
                SetRegValue(0xF0E0019C, 0x00000001); //0x0F:4lane, 0x03:2lane, 0x01:1lane
                SetRegValue(0xF0E00180, 0x00000000); //0x0F:4lane, 0x03:2lane, 0x01:1lane
#endif

                LOG_DEBUG("   start mipi.\n");
                SetRegValue(0xF1700000, 0x7D); 
                SetRegValue(0xF1700160, 0x06); // 8?�d6 : 250 ~ 300MHz, 8??E : 500 ~ 600MHz
                SetRegValue(0xF17001E0, 0x06); 
                SetRegValue(0xF1700260, 0x06); 
                SetRegValue(0xF17002E0, 0x06); 
                SetRegValue(0xF1700360, 0x06); 
                SetRegValue(0xF17003E0, 0x01); 
                SetRegValue(0xF1700128, 0x3F); 

                SetRegValue(0xF170000C, 0x03); 
                SetRegValue(0xF1700038, 0x36); 

#if 0
                SetRegValue(0xF1700100, 0x8A); 
                SetRegValue(0xF1700180, 0x8A); 
                SetRegValue(0xF1700200, 0x8A); 
                SetRegValue(0xF1700280, 0x8A); 
                SetRegValue(0xF1700300, 0x8A); 

                //SetRegValue(0xF1700038, 0x00); 
                //SetRegValue(0xF170003C, 0x00); 
                SetRegValue(0xF1700038, 0x0F); 
                SetRegValue(0xF170003C, 0xFF); 
#endif

                SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
                SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane

            }/*}}}*/
#endif

#define TEST_MIPI
#ifdef TEST_MIPI
//Step1: Normal initialization
SetRegValue(0xF1700000+(0x20<<2), 0x1C); // Enable Internal Reset Digital & Analog Logic 
SetRegValue(0xF1700000+(0x20<<2), 0x1F); // Disable Internal Reset Digital & Analog Logic 
_mdelay(1); //wait 1us;
SetRegValue(0xF1700000+(0x00<<2), 0x7D); // PHY_EN
SetRegValue(0xF1700000+(0x58<<2), 0x04);  // settle for lane ck: 0x04 for 250Mbps per lane
SetRegValue(0xF1700000+(0x78<<2), 0x04); // settle for lane 0
SetRegValue(0xF1700000+(0x98<<2), 0x04); // settle for lane 1
SetRegValue(0xF1700000+(0xB8<<2), 0x04); // settle for lane 2
SetRegValue(0xF1700000+(0xD8<<2), 0x04); // settle for lane 3
SetRegValue(0xF1700000+(0x4A<<2), 0x3F); // For Clock Lane : continuous clock
SetRegValue(0xF1700000+(0x03<<2), 0x03); // Open clock delay switch
SetRegValue(0xF1700000+(0x0e<<2), 0x36); // bit[6:4] defines the 8 phase(0~7) of internal clock, default 3.  
SetRegValue(0xF1700000+(0x21<<2), 0x04); // Invert clock 

SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane
#endif // TEST_MIPI

            //PR2000 start mipi lane.
            {/*{{{*/
                LOG_DEBUG("PR2000 start mipi lane.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                /* start mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/
_mdelay(100);
#ifdef TEST_MIPI
//Step2: Read internal status registers
//Wait until DPHY TX works
//SetRegValue(0xF1700000+(0x72<<2), 0xB2); //  Keep just for lane 0 test
//SetRegValue(0xF1700000+(0x92<<2), 0xB2); //  Keep just for lane 1 test
//SetRegValue(0xF1700000+(0xB2<<2), 0xB2); //  Keep just for lane 2 test
//SetRegValue(0xF1700000+(0xD2<<2), 0xB2); // Keep just for lane 3 test

//Read the registers value as below
LOG_DEBUG("0x74:0x%08x\n", GetRegValue(0xF1700000+(0x74<<2))); //  Read from lane 0
LOG_DEBUG("0x94:0x%08x\n", GetRegValue(0xF1700000+(0x94<<2))); //  Read from lane 1
LOG_DEBUG("0xB4:0x%08x\n", GetRegValue(0xF1700000+(0xB4<<2))); //  Read from lane 2
LOG_DEBUG("0xD4:0x%08x\n", GetRegValue(0xF1700000+(0xD4<<2))); //  Read from lane 3
#endif // TEST_MIPI

            //PI5008 Change mipi input
            {/*{{{*/
                uint32_t temp;

                LOG_DEBUG("PI5008 Change mipi input\n");

                temp = GetRegValue(0xF0F0000C); 
                temp &= 0xFFFFFFF0;
                temp |= 0x00000008;
                SetRegValue(0xF0F0000C, temp); 

                temp = GetRegValue(0xF0F00014); 
                temp &= 0xFFFFFFF0;
                temp |= 0x00000000;
                SetRegValue(0xF0F00014, temp); 

                temp = GetRegValue(0xF0F00044); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00100000;
                SetRegValue(0xF0F00044, temp); 

                _mdelay(300);
            }/*}}}*/

#if 0
            //PI5008 Wait mipi input valid
            {/*{{{*/

                LOG_DEBUG("PI5008 Wait mipi input valid\n");
                status_value[0] = GetRegValue(0xF0F00400); 
                LOG_DEBUG("0xF0F00400:0x%08x\n", status_value[0]);

                if(status_value[0] & 0x10000000)
                {/*{{{*/
                    int loop = 0;
                    do
                    {
                        LOG_DEBUG("try sync cnt(%d)\n", loop+1);

                        LOG_DEBUG("stop mipi lane\n");
                        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                        sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                        /* stop mipi lane */
                        for (i = 0; i < sz; i++) {
                            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                            if(ret != eSUCCESS){
                                LOG_DEBUG("I2C Write fail(%d)\n", ret);
                            }
                        }
                        _mdelay(300);

                        LOG_DEBUG("start mipi lane\n");
                        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                        sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                        /* stop mipi lane */
                        for (i = 0; i < sz; i++) {
                            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                            if(ret != eSUCCESS){
                                LOG_DEBUG("I2C Write fail(%d)\n", ret);
                            }
                        }
                        _mdelay(300);

                        status_value[0] = GetRegValue(0xF0F00400); 
                        LOG_DEBUG("400:0x%08x\n", status_value[0]);
                    } while( (status_value[0] & 0x10000000) && (loop++ < 10) );
                }/*}}}*/
            }/*}}}*/
#endif

#if 0
            //PI5008 Print debug 
            {/*{{{*/

                //0xF0E00184 ~ 0xF0E0018C (3 Address)
                status_value[0] = GetRegValue(0xF0E00184); 
                status_value[1] = GetRegValue(0xF0E00188); 
                status_value[2] = GetRegValue(0xF0E0018C); 
                LOG_DEBUG("0- status:0x%08x, 0x%08x, 0x%08x\n", 
                        status_value[0], status_value[1], status_value[2]);
                LOG_DEBUG("delay 200msec\n");
                _mdelay(200);
                status_value[0] = GetRegValue(0xF0E00184); 
                status_value[1] = GetRegValue(0xF0E00188); 
                status_value[2] = GetRegValue(0xF0E0018C); 
                LOG_DEBUG("1- status:0x%08x, 0x%08x, 0x%08x\n", 
                        status_value[0], status_value[1], status_value[2]);
            }/*}}}*/
#endif

#if 0
            //PI5008 Set Quad display to PVI Tx
            {/*{{{*/
                LOG_DEBUG("PI5008 Set Quad display to PVI Tx\n");

                SetRegValue(0xF0900004, 0x1F0); 
                SetRegValue(0xF0F00018, 0x00); 
            }/*}}}*/


            //PI5008 Get CRC value. and output GPIO
            {/*{{{*/
                LOG_DEBUG("Get CRC value. and output GPIO.\n");

                _mdelay(500);

                crc_value[0] = GetRegValue(0xF0F00284); //0x8000DFE5
                crc_value[1] = GetRegValue(0xF0F002D4); //0x80002C75

                LOG_DEBUG("CRC value..0xF0F00284(0x8000DFE5):0x%08x, 0xF0F002D4(0x80002C75):0x%08x\n", crc_value[0], crc_value[1]);

                LOG_DEBUG("TODO! Compare value.. If ok, Gpio7 Low(default)->High\n");
                if( (crc_value[0] == 0x8000DFE5) && (crc_value[1] == 0x80002C75) )
                {
                    LOG_DEBUG("TODO! OK, Gpio7 Low(default)->High\n");
                }
            }/*}}}*/
#endif
        }/*}}}*/
        }/*}}}*/
        else if( !strncmp(strArgv, "manual", sizeof("manual")) )
        {/*{{{*/
        	while (cnt-- > 0) {/*{{{*/
			LOG_DEBUG("[%d]SLT_TEST - manual \n", cnt);
            //PR2000 stop mipi lane.
            {/*{{{*/
                LOG_DEBUG("PR2000 stop mipi lane.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
				_mdelay(1);
				wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
		        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
		        /* stop mipi lane */
		        for (i = 0; i < sz; i++) {
		            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
		            if(ret != eSUCCESS){
		                LOG_DEBUG("I2C Write fail(%d)\n", ret);
		            }
		        }
                _mdelay(300);
            }/*}}}*/

            //PR2000 Set table.
            {/*{{{*/
                /* set table */
                LOG_DEBUG("PR2000 set table\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_1280x720_30fps[0];
                sz = sizeof(pr2000_mipi4lane_1280x720_30fps) / sizeof(pr2000_mipi4lane_1280x720_30fps[0]);
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/

#if 1
            //PI5008 Start MIPI.
            {/*{{{*/
                LOG_DEBUG("PI5008 Start MIPI.\n");

                // default init
                LOG_DEBUG("   set reset init value.\n");
                SetRegValue(0xF1700000, 0x00000001); 
                SetRegValue(0xF1700160, 0x0000001b); // 8?�d6 : 250 ~ 300MHz, 8??E : 500 ~ 600MHz
                SetRegValue(0xF17001E0, 0x0000001b); 
                SetRegValue(0xF1700260, 0x0000001b); 
                SetRegValue(0xF17002E0, 0x0000001b); 
                SetRegValue(0xF1700360, 0x0000001b); 
                SetRegValue(0xF17003E0, 0x00000000); 
                SetRegValue(0xF1700128, 0x0000000f); 
                SetRegValue(0xF0E0019C, 0x00000001); //0x0F:4lane, 0x03:2lane, 0x01:1lane
                SetRegValue(0xF0E00180, 0x00000000); //0x0F:4lane, 0x03:2lane, 0x01:1lane

                LOG_DEBUG("   start mipi.\n");
                SetRegValue(0xF1700000, 0x7D); 
                SetRegValue(0xF1700160, 0x06); // 8?�d6 : 250 ~ 300MHz, 8??E : 500 ~ 600MHz
                SetRegValue(0xF17001E0, 0x06); 
                SetRegValue(0xF1700260, 0x06); 
                SetRegValue(0xF17002E0, 0x06); 
                SetRegValue(0xF1700360, 0x06); 
                SetRegValue(0xF17003E0, 0x01); 
                SetRegValue(0xF1700128, 0x3F); 

                SetRegValue(0xF170000C, 0x03); 
                SetRegValue(0xF1700038, 0x36); 

#if 0
                SetRegValue(0xF1700100, 0x8A); 
                SetRegValue(0xF1700180, 0x8A); 
                SetRegValue(0xF1700200, 0x8A); 
                SetRegValue(0xF1700280, 0x8A); 
                SetRegValue(0xF1700300, 0x8A); 

                //SetRegValue(0xF1700038, 0x00); 
                //SetRegValue(0xF170003C, 0x00); 
                SetRegValue(0xF1700038, 0x0F); 
                SetRegValue(0xF170003C, 0xFF); 
#endif

                SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
                SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane

            }/*}}}*/
#endif

            //PR2000 start mipi lane.
            {/*{{{*/
                LOG_DEBUG("PR2000 start mipi lane.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                /* start mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/

#if 1
            //PI5008 Change mipi input
            {/*{{{*/
                uint32_t temp;

                LOG_DEBUG("PI5008 Change mipi input\n");

                temp = GetRegValue(0xF0F0000C); 
                temp &= 0xFFFFFFF0;
                temp |= 0x00000008;
                SetRegValue(0xF0F0000C, temp); 

                temp = GetRegValue(0xF0F00014); 
                temp &= 0xFFFFFFF0;
                temp |= 0x00000000;
                SetRegValue(0xF0F00014, temp); 

                temp = GetRegValue(0xF0F00044); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00100000;
                SetRegValue(0xF0F00044, temp); 

                _mdelay(300);
            }/*}}}*/

            //PI5008 Wait mipi input valid
            {/*{{{*/

                LOG_DEBUG("PI5008 Wait mipi input valid\n");
                status_value[0] = GetRegValue(0xF0F00400); 
                LOG_DEBUG("0xF0F00400:0x%08x\n", status_value[0]);

                if(status_value[0] & 0x10000000)
                {/*{{{*/
                    int loop = 0;
                    do
                    {
                        LOG_DEBUG("try sync cnt(%d)\n", loop+1);

                        LOG_DEBUG("stop mipi lane\n");
                        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                        sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                        /* stop mipi lane */
                        for (i = 0; i < sz; i++) {
                            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                            if(ret != eSUCCESS){
                                LOG_DEBUG("I2C Write fail(%d)\n", ret);
                            }
                        }
                        _mdelay(300);

                        LOG_DEBUG("start mipi lane\n");
                        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                        sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                        /* stop mipi lane */
                        for (i = 0; i < sz; i++) {
                            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                            if(ret != eSUCCESS){
                                LOG_DEBUG("I2C Write fail(%d)\n", ret);
                            }
                        }
                        _mdelay(300);

                        status_value[0] = GetRegValue(0xF0F00400); 
                        LOG_DEBUG("400:0x%08x\n", status_value[0]);
                    } while( (status_value[0] & 0x10000000) && (loop++ < 10) );
                }/*}}}*/
            }/*}}}*/
#endif

            //Set ISP 4ch TP 
            {/*{{{*/
                int i;
                LOG_DEBUG("Set ISP 4ch TP\n"); 
                for(i = 0; i < 4; i++)
                {/*{{{*/
                    //                       720p30,  720p25
                    SetRegValue(0xF0600FB8+(i*0x4000), 0x81); //0x0x81
                    SetRegValue(0xF0600FBC+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0600FC0+(i*0x4000), 0xFF); //0x0xFF
                    SetRegValue(0xF0600FC4+(i*0x4000), 0xF0); //0x0xF0
                    SetRegValue(0xF0600FC8+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0600FCC+(i*0x4000), 0xFF); //0x0xFF
                    SetRegValue(0xF0600FD0+(i*0x4000), 0xF0); //0x0xF0
                    SetRegValue(0xF0600FD4+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0600FD8+(i*0x4000), 0xFF); //0x0xFF
                    SetRegValue(0xF0600FDC+(i*0x4000), 0xF0); //0x0xF0
                    SetRegValue(0xF0600FE0+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0600FE4+(i*0x4000), 0xFF); //0x0xFF
                    SetRegValue(0xF0600FE8+(i*0x4000), 0xF0); //0x0xF0
                    SetRegValue(0xF0600FEC+(i*0x4000), 0x05); //0x0x05
                    SetRegValue(0xF0600FF0+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0600C38+(i*0x4000), 0x05); //0x0x05
                    SetRegValue(0xF0600C3C+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0600C40+(i*0x4000), 0x02); //0x0x02
                    SetRegValue(0xF0600C44+(i*0x4000), 0xD0); //0x0xD0
                    SetRegValue(0xF0601360+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0601364+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0601368+(i*0x4000), 0x05); //0x0x05
                    SetRegValue(0xF060136C+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0601370+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0601374+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0601378+(i*0x4000), 0x02); //0x0x02
                    SetRegValue(0xF060137C+(i*0x4000), 0xD0); //0x0xD0
                    SetRegValue(0xF0600C24+(i*0x4000), 0x02); //0x0x02
                    SetRegValue(0xF0600C28+(i*0x4000), 0xED); //0x0xED
                    SetRegValue(0xF0600C2C+(i*0x4000), 0x02); //0x0x02
                    SetRegValue(0xF0600C30+(i*0x4000), 0xED); //0x0xED
                    SetRegValue(0xF0600C14+(i*0x4000), 0x06); //0x0x07
                    SetRegValue(0xF0600C18+(i*0x4000), 0x71); //0x0xBB
                    SetRegValue(0xF0600D24+(i*0x4000), 0x02); //0x0x02
                    SetRegValue(0xF0600D28+(i*0x4000), 0xE2); //0x0xE2
                    SetRegValue(0xF0600DB4+(i*0x4000), 0x10); //0x0x10
                    SetRegValue(0xF0602754+(i*0x4000), 0x00); //0x0x00
                    SetRegValue(0xF0602758+(i*0x4000), 0x0F); //0x0x0F
                    SetRegValue(0xF060275C+(i*0x4000), 0xFF); //0x0xFF
                }/*}}}*/
            }/*}}}*/

            //Display PVI 4ch
            {/*{{{*/
                uint32_t temp;

                LOG_DEBUG("Display PVI 4ch input\n");

                temp = GetRegValue(0xF0F0000C); 
                temp &= 0xFFFF0000;
                temp |= 0x0000FEDC;
                SetRegValue(0xF0F0000C, temp); 

                temp = GetRegValue(0xF0F00014); 
                temp &= 0xFFFF0000;
                temp |= 0x00003210;
                SetRegValue(0xF0F00014, temp); 

                temp = GetRegValue(0xF0F00044); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00000000;
                SetRegValue(0xF0F00044, temp); 

                temp = GetRegValue(0xF0F00054); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00000000;
                SetRegValue(0xF0F00054, temp); 

                temp = GetRegValue(0xF0F00064); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00000000;
                SetRegValue(0xF0F00064, temp); 

                temp = GetRegValue(0xF0F00074); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00000000;
                SetRegValue(0xF0F00074, temp); 
            }/*}}}*/

            _mdelay(3000);

            //Display mipi 1ch
            {/*{{{*/
                uint32_t temp;

                LOG_DEBUG("Display MIPI 1ch input\n");

                temp = GetRegValue(0xF0F0000C); 
                temp &= 0xFFFFFFF0;
                temp |= 0x00000008;
                SetRegValue(0xF0F0000C, temp); 

                temp = GetRegValue(0xF0F00014); 
                temp &= 0xFFFF0000;
                temp |= 0x00003210;
                SetRegValue(0xF0F00014, temp); 

                temp = GetRegValue(0xF0F00044); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00100000;
                SetRegValue(0xF0F00044, temp); 

            }/*}}}*/

            _mdelay(3000);

            //Display isp 4ch
        {/*{{{*/
            uint32_t temp;

            LOG_DEBUG("Display ISP 4ch input\n");

            temp = GetRegValue(0xF0F0000C); 
            temp &= 0xFFFF0000;
            temp |= 0x0000BA98;
            SetRegValue(0xF0F0000C, temp); 

            temp = GetRegValue(0xF0F00014); 
            temp &= 0xFFFF0000;
            temp |= 0x0000FEDC;
            SetRegValue(0xF0F00014, temp); 

            temp = GetRegValue(0xF0F00044); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00044, temp); 

            temp = GetRegValue(0xF0F00054); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00054, temp); 

            temp = GetRegValue(0xF0F00064); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00064, temp); 

            temp = GetRegValue(0xF0F00074); 
            temp &= 0xFFC0FFFF;
            temp |= 0x00100000;
            SetRegValue(0xF0F00074, temp); 
        }/*}}}*/
        }/*}}}*/

        }/*}}}*/
        else if( !strncmp(strArgv, "auto", sizeof("auto")) )
        {/*{{{*/
        	while (cnt-- > 0) {/*{{{*/
			LOG_DEBUG("[%d]SLT_TEST - auto \n", cnt);
            //PR2000 stop mipi lane.
            {/*{{{*/
                LOG_DEBUG("PR2000 stop mipi lane.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                /* stop mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
				_mdelay(1);
				wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
		        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
		        /* stop mipi lane */
		        for (i = 0; i < sz; i++) {
		            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
		            if(ret != eSUCCESS){
		                LOG_DEBUG("I2C Write fail(%d)\n", ret);
		            }
		        }
                _mdelay(300);
            }/*}}}*/

			//PR2000 Set table.
            {/*{{{*/
                /* set table */
                LOG_DEBUG("PR2000 set table\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_1280x720_30fps[0];
                sz = sizeof(pr2000_mipi4lane_1280x720_30fps) / sizeof(pr2000_mipi4lane_1280x720_30fps[0]);
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/

#if 1
            //PI5008 Start MIPI.
            {/*{{{*/
                LOG_DEBUG("PI5008 Start MIPI.\n");

                // default init
                LOG_DEBUG("   set reset init value.\n");
                SetRegValue(0xF1700000, 0x00000001); 
                SetRegValue(0xF1700160, 0x0000001b); // 8?�d6 : 250 ~ 300MHz, 8??E : 500 ~ 600MHz
                SetRegValue(0xF17001E0, 0x0000001b); 
                SetRegValue(0xF1700260, 0x0000001b); 
                SetRegValue(0xF17002E0, 0x0000001b); 
                SetRegValue(0xF1700360, 0x0000001b); 
                SetRegValue(0xF17003E0, 0x00000000); 
                SetRegValue(0xF1700128, 0x0000000f); 
                SetRegValue(0xF0E0019C, 0x00000001); //0x0F:4lane, 0x03:2lane, 0x01:1lane
                SetRegValue(0xF0E00180, 0x00000000); //0x0F:4lane, 0x03:2lane, 0x01:1lane

                LOG_DEBUG("   start mipi.\n");
                SetRegValue(0xF1700000, 0x7D); 
                SetRegValue(0xF1700160, 0x06); // 8?�d6 : 250 ~ 300MHz, 8??E : 500 ~ 600MHz
                SetRegValue(0xF17001E0, 0x06); 
                SetRegValue(0xF1700260, 0x06); 
                SetRegValue(0xF17002E0, 0x06); 
                SetRegValue(0xF1700360, 0x06); 
                SetRegValue(0xF17003E0, 0x01); 
                SetRegValue(0xF1700128, 0x3F); 

                SetRegValue(0xF170000C, 0x03); 
                SetRegValue(0xF1700038, 0x36); 

#if 0
                SetRegValue(0xF1700100, 0x8A); 
                SetRegValue(0xF1700180, 0x8A); 
                SetRegValue(0xF1700200, 0x8A); 
                SetRegValue(0xF1700280, 0x8A); 
                SetRegValue(0xF1700300, 0x8A); 

                //SetRegValue(0xF1700038, 0x00); 
                //SetRegValue(0xF170003C, 0x00); 
                SetRegValue(0xF1700038, 0x0F); 
                SetRegValue(0xF170003C, 0xFF); 
#endif

                SetRegValue(0xF0E0019C, 0x0F); //0x0F:4lane, 0x03:2lane, 0x01:1lane
                SetRegValue(0xF0E00180, 0x03); //0x0F:4lane, 0x03:2lane, 0x01:1lane

            }/*}}}*/
#endif

            //PR2000 Set TP.
            {/*{{{*/
                LOG_DEBUG("PR2000 set tp.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_tp[0];
                sz = sizeof(pr2000_tp) / sizeof(pr2000_tp[0]);
                /* start mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/

            //PR2000 start mipi lane.
            {/*{{{*/
                LOG_DEBUG("PR2000 start mipi lane.\n");
                wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                /* start mipi lane */
                for (i = 0; i < sz; i++) {
                    ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                    if(ret != eSUCCESS){
                        LOG_DEBUG("I2C Write fail(%d)\n", ret);
                    }
                }
            }/*}}}*/

#if 1
            //PI5008 Change mipi input
            {/*{{{*/
                uint32_t temp;

                LOG_DEBUG("PI5008 Change mipi input\n");

                temp = GetRegValue(0xF0F0000C); 
				temp &= 0xFFFFFFF0;
                temp |= 0x00000008;
				//temp &= 0xFFF00000;
                //temp |= 0x00088888;
                SetRegValue(0xF0F0000C, temp); 

                temp = GetRegValue(0xF0F00014); 
                temp &= 0xFFFFFFF0;
                temp |= 0x00000000;
                SetRegValue(0xF0F00014, temp); 

                temp = GetRegValue(0xF0F00044); 
                temp &= 0xFFC0FFFF;
                temp |= 0x00100000;
                SetRegValue(0xF0F00044, temp); 

                _mdelay(300);
            }/*}}}*/

            //PI5008 Wait mipi input valid
            {/*{{{*/

                LOG_DEBUG("PI5008 Wait mipi input valid\n");
                status_value[0] = GetRegValue(0xF0F00400); 
                LOG_DEBUG("0xF0F00400:0x%08x\n", status_value[0]);

                if(status_value[0] & 0x10000000)
                {/*{{{*/
                    int loop = 0;
                    do
                    {
                        LOG_DEBUG("try sync cnt(%d)\n", loop+1);

                        LOG_DEBUG("stop mipi lane\n");
                        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop[0];
                        sz = sizeof(pr2000_mipi4lane_stop) / sizeof(pr2000_mipi4lane_stop[0]);
                        /* stop mipi lane */
                        for (i = 0; i < sz; i++) {
                            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                            if(ret != eSUCCESS){
                                LOG_DEBUG("I2C Write fail(%d)\n", ret);
                            }
                        }
						_mdelay(1);
						wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_stop_delay[0];
				        sz = sizeof(pr2000_mipi4lane_stop_delay) / sizeof(pr2000_mipi4lane_stop_delay[0]);
				        /* stop mipi lane */
				        for (i = 0; i < sz; i++) {
				            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
				            if(ret != eSUCCESS){
				                LOG_DEBUG("I2C Write fail(%d)\n", ret);
				            }
				        }
                        _mdelay(300);

                        LOG_DEBUG("start mipi lane\n");
                        wbuf = (PP_U8 (*)[2])&pr2000_mipi4lane_start[0];
                        sz = sizeof(pr2000_mipi4lane_start) / sizeof(pr2000_mipi4lane_start[0]);
                        /* stop mipi lane */
                        for (i = 0; i < sz; i++) {
                            ret = PPDRV_I2C_Write(0, wbuf[i], sizeof(wbuf[i]), 1000);
                            if(ret != eSUCCESS){
                                LOG_DEBUG("I2C Write fail(%d)\n", ret);
                            }
                        }
                        _mdelay(300);

                        status_value[0] = GetRegValue(0xF0F00400); 
                        LOG_DEBUG("400:0x%08x\n", status_value[0]);
                    } while( (status_value[0] & 0x10000000) && (loop++ < 10) );
                }/*}}}*/
            }/*}}}*/
#endif

            //PI5008 Set Quad display to PVI Tx
            {/*{{{*/
                LOG_DEBUG("PI5008 Set Quad display to PVI Tx\n");

                SetRegValue(0xF0900004, 0x1F0); 
                SetRegValue(0xF0F00018, 0x00); 
            }/*}}}*/


            //PI5008 Get CRC value. and output GPIO
            {/*{{{*/

                /* gpio pin gpio1[7] ) */
                PP_U32 u32GPIODev = 0;
                PP_U32 u32GPIOCh = 7;

                LOG_DEBUG("Get CRC value. and output GPIO.\n");

                // pinmux sel
                set_pinmux(gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val);
                LOG_DEBUG("pinmux sel. group: %d, bitpos: %d, pinsel: %d\n", gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Group , gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32BitPos, gstGPIOPinMux[u32GPIODev][u32GPIOCh].u32Val );
                PPDRV_GPIO_SetDir(u32GPIODev, u32GPIOCh, eDIR_OUT, 0);
                PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 0);

                _mdelay(500);

                crc_value[0] = GetRegValue(0xF0F00284); //0x8000DFE5
                crc_value[1] = GetRegValue(0xF0F002D4); //0x80002C75

                LOG_DEBUG("CRC value..0xF0F00284(0x8000DFE5):0x%08x, 0xF0F002D4(0x80002C75):0x%08x\n", crc_value[0], crc_value[1]);

                LOG_DEBUG("TODO! Compare value.. If ok, Gpio7 Low(default)->High\n");
                if( (crc_value[0] == 0x8000DFE5) && (crc_value[1] == 0x80002C75) )
                {

                    LOG_DEBUG("OK, Gpio7 Low(default)->High\n");
                    PPDRV_GPIO_SetValue(u32GPIODev, u32GPIOCh, 1);
                }
            }/*}}}*/
	        }/*}}}*/

        }/*}}}*/

    }/*}}}*/

    return(0);
}
#endif // BD_SLT

static uint32 g_i2c_ch = (uint32)-1; 
static uint32 g_i2c_init = 0; 
int _i2c_reg(int argc, const char **argv) 
{ 
	uint32 dev_addr; 
	uint8 buf[4]; 
	PP_RESULT_E ret; 
	uint32 read_cnt; 
	uint32 i; 
 
 
	if(g_i2c_init == 0){ 
		PPDRV_I2C_Initialize(); 
		g_i2c_init = 1; 
	} 
 
	if(strcmp(argv[1],"setup") == 0){ 
		if(argc < 3){ 
			LOG_DEBUG("Invalid usage\n"); 
			return(-1); 
		} 
		g_i2c_ch = simple_strtoul(argv[2], 0); 
		dev_addr = simple_strtoul(argv[3], 0); 
 
		PPDRV_I2C_Setup(g_i2c_ch, dev_addr, eI2C_SPEED_NORMAL, eI2C_MODE_MASTER, eI2C_ADDRESS_7BIT);	// master, 100kb/s 
		PPDRV_I2C_Enable(g_i2c_ch, 1); 
 
	}else if(strcmp(argv[1],"read") == 0){ 
		if(argc < 3){ 
			LOG_DEBUG("Invalid usage\n"); 
			return(-1); 
		}else{ 
			if(argc == 4) 
				read_cnt = simple_strtoul(argv[3], 0); 
			else 
				read_cnt = 1; 
		} 
 
		buf[0] = simple_strtoul(argv[2], 0); 
 
		for(i=0;i<read_cnt;i++){ 
			ret = PPDRV_I2C_Write(g_i2c_ch, &buf[0], 1, 1000); 
			if(ret != eSUCCESS){ 
				LOG_DEBUG("I2C Write fail(%d)\n", ret); 
			} 
 
			ret = PPDRV_I2C_Read(g_i2c_ch, &buf[1], 1, 1000); 
			if(ret != eSUCCESS){ 
				LOG_DEBUG("I2C Read fail(%d)\n", ret); 
			}else{ 
				LOG_DEBUG("Read value @ 0x%x: 0x%x.\n", buf[0], buf[1]); 
			} 
 
			buf[0]++; 
		} 
 
	}else if(strcmp(argv[1],"write") == 0){ 
		if(argc < 4){ 
			LOG_DEBUG("Invalid usage\n"); 
			return(-1); 
		} 
		buf[0] = simple_strtoul(argv[2], 0); 
		buf[1] = simple_strtoul(argv[3], 0); 
 
		ret = PPDRV_I2C_Write(g_i2c_ch, &buf[0], 2, 1000); 
		if(ret != eSUCCESS){ 
			LOG_DEBUG("I2C Write fail(%d)\n", ret); 
		} 
 	}
#if 0
 	else if(strcmp(argv[1],"test") == 0)
 	{ /*{{{*/
        LOG_DEBUG("================================\n");
        //0x00~0x1B
		for(buf[0] = 0x00 ; buf[0] <=0x1B ; buf[0]++)
		{ 
			ret = PPDRV_I2C_Write(g_i2c_ch, &buf[0], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Write fail(%d)\n", ret); 
			} 
 
			ret = PPDRV_I2C_Read(g_i2c_ch, &buf[1], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Read fail(%d)\n", ret); 
			}else
			{ 
				LOG_DEBUG("0x%x: 0x%x\n", buf[0], buf[1]); 
			} 
		} 

        //0x64~0x67
		for(buf[0] = 0x64 ; buf[0] <=0x67 ; buf[0]++)
		{ 
			ret = PPDRV_I2C_Write(g_i2c_ch, &buf[0], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Write fail(%d)\n", ret); 
			} 
 
			ret = PPDRV_I2C_Read(g_i2c_ch, &buf[1], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Read fail(%d)\n", ret); 
			}else
			{ 
				LOG_DEBUG("0x%x: 0x%x\n", buf[0], buf[1]); 
			} 
		} 

        //0xC6~0xC7
		for(buf[0] = 0xC6 ; buf[0] <=0xC7 ; buf[0]++)
		{ 
			ret = PPDRV_I2C_Write(g_i2c_ch, &buf[0], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Write fail(%d)\n", ret); 
			} 
 
			ret = PPDRV_I2C_Read(g_i2c_ch, &buf[1], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Read fail(%d)\n", ret); 
			}else
			{ 
				LOG_DEBUG("0x%x: 0x%x\n", buf[0], buf[1]); 
			} 
		} 
        //0xF0~0xF5
		for(buf[0] = 0xF0 ; buf[0] <=0xF5 ; buf[0]++)
		{ 
			ret = PPDRV_I2C_Write(g_i2c_ch, &buf[0], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Write fail(%d)\n", ret); 
			} 
 
			ret = PPDRV_I2C_Read(g_i2c_ch, &buf[1], 1, 1000); 
			if(ret != eSUCCESS)
			{ 
				LOG_DEBUG("I2C Read fail(%d)\n", ret); 
			}else
			{ 
				LOG_DEBUG("0x%x: 0x%x\n", buf[0], buf[1]); 
			} 
		} 
	}/*}}}*/
#endif
	else{ 
		LOG_DEBUG("unknown command\n"); 
		return(-1); 
	} 
 
    return(0); 
} 

#include "standby.h"

int _wboot_test(int argc, const char **argv)
{
	EnterStandbyMode();
	return 0;
}

#if 0
int _pd_test(int argc, const char **argv)
{
	if(argc < 2){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1], "on") == 0){

		*(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00000080;  //Qspi 0:en, 1:dis
//        *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00008000;  //timers 0:en, 1:dis
        *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00800000;  //wdt 0:en, 1:dis
//        *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x80000000;  //uart 0:en, 1:dis
        *(volatile unsigned int *)0xf0000008 &= (unsigned int)~0x00000080;  //gadc 0:en, 1:dis
        *(volatile unsigned int *)0xf0000008 &= (unsigned int)~0x00800000;  //svm 0:en, 1:dis


        *(volatile unsigned int *)0xf0700054 |= (unsigned int)0x00000000;  // power up
		*(volatile unsigned int *)0xf0701054 |= (unsigned int)0x00000000;  // power up
		*(volatile unsigned int *)0xf0702054 |= (unsigned int)0x00000000;  // power up
		*(volatile unsigned int *)0xf0703054 |= (unsigned int)0x00000000;  // power up

		*(volatile unsigned int *)0xf000000C &= (unsigned int)~0x80808080;  //pvirx 0:en, 1:dis
        *(volatile unsigned int *)0xf0000010 &= (unsigned int)~0x80808080;  //vadc 0:en, 1:dis

        PPAPI_PVIRX_Initialize();


        *(volatile unsigned int *)0xf0000014 &= (unsigned int)~0x80808080;  //isp 0:en, 1:dis
        *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00000080;  //vin 0:en, 1:dis
        *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00008000;  //vpu 0:en, 1:dis
        *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00800000;  //du 0:en, 1:dis
        *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x80000000;  //pvitx rd 0:en, 1:dis
        *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x80000000;  //pvitx wr 0:en, 1:dis
        *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00000080;  //pvitx fifo 0:en, 1:dis
        *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00008000;  //pvitx vdac 0:en, 1:dis
		*(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00800000;  //recfmt 0:en, 1:dis
		*(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00000080;  //recfmt_2x_clock 0:en, 1:dis
		*(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00008000;  //recfmt_2x_mph0 0:en, 1:dis
		*(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00800000;  //recfmt_2x_mph1 0:en, 1:dis

        *(volatile unsigned int *)0xf0100054 = (unsigned int)0x02020202;  //set pad output.
		*(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x80000000;  //quad 0:en, 1:dis
		*(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x00000080;  //quad_2x 0:en, 1:dis
		*(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x00008000;  //disfmt 0:en, 1:dis
		*(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x80000000;  //disfmt_2x 0:en, 1:dis
		*(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x00008000;  //rxbyteclkhs 0:en, 1:dis
		*(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x00800000;  //mipi_lane 0:en, 1:dis
		*(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x80000000;  //mipi_pixel 0:en, 1:dis
		*(volatile unsigned int *)0xf000002C &= (unsigned int)~0x00000080;  //mipi cci 0:en, 1:dis
        *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00008000;  //i2S 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00800000;  //par0 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x80000000;  //par1 0:en, 1:dis

		*(vuint32 *)(0xf0800000) = 0x203;	//svm enable
		*(vuint32 *)(0xf0800014) |= 0x701;	//svm input enable	[0]= 0, [10:8]=0, enable: [0]=1, [10:8]=0x7

		*(vuint32 *)(0xf0904500) = 0x1;		//du enable
		*(vuint32 *)(0xf0f00154) = 0xf0000000;		//quad write enable
		*(vuint32 *)(0xf0f00158) = 0x8f000003;		//quad read enable

		cache_enable();
		PPDRV_TIMER_Start (0, 0);
		GIE_ENABLE();

	}else if(strcmp(argv[1], "off") == 0){

		*(vuint32 *)(0xf0800000) = 0x202;	//svm disable
		*(vuint32 *)(0xf0800014) &= ~0x701;	//svm input disable	[0]= 0, [10:8]=0, enable: [0]=1, [10:8]=0x7

		*(vuint32 *)(0xf0904500) = 0x0;		//du disable
		*(vuint32 *)(0xf0f00154) = 0x0;		//quad write disable
		*(vuint32 *)(0xf0f00158) = 0x0;		//quad read disable

		cache_disable();
	    GIE_DISABLE();
		PPDRV_TIMER_Stop (0, 0);


		*(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00000080;  //Qspi 0:en, 1:dis
//		*(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00008000;  //timers 0:en, 1:dis
		*(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00800000;  //wdt 0:en, 1:dis
//		*(volatile unsigned int *)0xf0000004 |= (unsigned int)0x80000000;  //uart 0:en, 1:dis
		*(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00000080;  //gadc 0:en, 1:dis
		*(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00800000;  //svm 0:en, 1:dis
		*(volatile unsigned int *)0xf000000C |= (unsigned int)0x80808080;  //pvirx 0:en, 1:dis
		*(volatile unsigned int *)0xf0000010 |= (unsigned int)0x80808080;  //vadc 0:en, 1:dis

		/* power down adc */
		*(volatile unsigned int *)0xf0700054 |= (unsigned int)0x00000010;  // power down
		*(volatile unsigned int *)0xf0701054 |= (unsigned int)0x00000010;  // power down
		*(volatile unsigned int *)0xf0702054 |= (unsigned int)0x00000010;  // power down
		*(volatile unsigned int *)0xf0703054 |= (unsigned int)0x00000010;  // power down

		*(volatile unsigned int *)0xf0000014 |= (unsigned int)0x80808080;  //isp 0:en, 1:dis
		*(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00000080;  //vin 0:en, 1:dis
		*(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00008000;  //vpu 0:en, 1:dis
		*(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00800000;  //du 0:en, 1:dis
		*(volatile unsigned int *)0xf0000018 |= (unsigned int)0x80000000;  //pvitx rd 0:en, 1:dis
		*(volatile unsigned int *)0xf000001C |= (unsigned int)0x80000000;  //pvitx wr 0:en, 1:dis
		*(volatile unsigned int *)0xf000001C |= (unsigned int)0x00000080;  //pvitx fifo 0:en, 1:dis
		*(volatile unsigned int *)0xf000001C |= (unsigned int)0x00008000;  //pvitx vdac 0:en, 1:dis

		/* power down dac */
//		*(volatile unsigned int *)0xf0100070 |= (unsigned int)0x00100000;  // power down

		*(volatile unsigned int *)0xf000001C |= (unsigned int)0x00800000;  //recfmt 0:en, 1:dis
		*(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00000080;  //recfmt_2x_clock 0:en, 1:dis
		*(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00008000;  //recfmt_2x_mph0 0:en, 1:dis
		*(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00800000;  //recfmt_2x_mph1 0:en, 1:dis

		*(volatile unsigned int *)0xf0100054 = (unsigned int)0x01010101;  //set pad input.
		*(volatile unsigned int *)0xf0000020 |= (unsigned int)0x80000000;  //quad 0:en, 1:dis
		*(volatile unsigned int *)0xf0000024 |= (unsigned int)0x00000080;  //quad_2x 0:en, 1:dis
		*(volatile unsigned int *)0xf0000024 |= (unsigned int)0x00008000;  //disfmt 0:en, 1:dis
		*(volatile unsigned int *)0xf0000024 |= (unsigned int)0x80000000;  //disfmt_2x 0:en, 1:dis
		*(volatile unsigned int *)0xf0000028 |= (unsigned int)0x00008000;  //rxbyteclkhs 0:en, 1:dis
		*(volatile unsigned int *)0xf0000028 |= (unsigned int)0x00800000;  //mipi_lane 0:en, 1:dis
		*(volatile unsigned int *)0xf0000028 |= (unsigned int)0x80000000;  //mipi_pixel 0:en, 1:dis
		*(volatile unsigned int *)0xf000002C |= (unsigned int)0x00000080;  //mipi cci 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00008000;  //i2S 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00800000;  //par0 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 |= (unsigned int)0x80000000;  //par1 0:en, 1:dis

	}

	return 0;
}
#endif



typedef enum ppCLOCK_BLOCK_E{
	eCLK_SCU00 = 0,					// apb, ddr_axi, cpu_axi, cpu 		- 0xf0000000
	eCLK_SCU01,						// Qspi, timer, wdt, uart			- 0xf0000004
	eCLK_SCU02,						// gadc, svm					- 0xf0000008
	eCLK_SCU03,						// pvirx						- 0xf000000c
	eCLK_SCU04,						// vadc						- 0xf0000010
	eCLK_SCU05,						// isp						- 0xf0000014
	eCLK_SCU06,						// vin, vpu, du, pvi tx rd			- 0xf0000018
	eCLK_SCU07,						// pvitx wr, pvitx fifo, pvitx vdac, recfmt0 						- 0xf000001c
	eCLK_SCU08,						// recfmt_2x_clock 0, recfmt_2x_mph0, recfmt_2x_mph1, quad 0 	- 0xf0000020
	eCLK_SCU09,						// quad_2x, disfmt, disfmt_2x		- 0xf0000024
	eCLK_SCU10,						//0 rxbyteclkhs, mipi_lane 0, mipi_pixel 0 					- 0xf0000028
	eCLK_SCU11,						// mipi cci 0:en, 1:dis			- 0xf000002c
	eCLK_SCU12,						// tck						- 0xf0000030
	eCLK_ADC0_PD,					// vadc0 power down			- 0xf0700054
	eCLK_ADC1_PD,					// vadc1 power down			- 0xf0701054
	eCLK_ADC2_PD,					// vadc2 power down			- 0xf0702054
	eCLK_ADC3_PD,					// vadc3 power down			- 0xf0703054
	eCLK_DAC_PD,					// vdac power down				- 0xf0100070
	eCLK_MAX,
}PP_CLOCK_BLOCK_E;

uint32 gClockBackup[eCLK_MAX];

int _pd_test(int argc, const char **argv)
{
	if(argc < 2){
		LOG_DEBUG("Invalid usage\n");
		return(-1);
	}

	if(strcmp(argv[1], "on") == 0){

		// clock power up
		*(volatile unsigned int *)0xf0000004 = (unsigned int)gClockBackup[eCLK_SCU01];  //Qspi, timer, wdt
		*(volatile unsigned int *)0xf0000008 = (unsigned int)gClockBackup[eCLK_SCU02];  //gadc, svm
		*(volatile unsigned int *)0xf000000C = (unsigned int)gClockBackup[eCLK_SCU03];  //pvirx 0:en, 1:dis
		*(volatile unsigned int *)0xf0000010 = (unsigned int)gClockBackup[eCLK_SCU04];  //vadc 0:en, 1:dis

		/* power down adc */
		*(volatile unsigned int *)0xf0700054 = (unsigned int)gClockBackup[eCLK_ADC0_PD];	// power down
		*(volatile unsigned int *)0xf0701054 = (unsigned int)gClockBackup[eCLK_ADC1_PD];	// power down
		*(volatile unsigned int *)0xf0702054 = (unsigned int)gClockBackup[eCLK_ADC2_PD];	// power down
		*(volatile unsigned int *)0xf0703054 = (unsigned int)gClockBackup[eCLK_ADC3_PD];	// power down

		*(volatile unsigned int *)0xf0000014 = (unsigned int)gClockBackup[eCLK_SCU05];  //isp
		*(volatile unsigned int *)0xf0000018 = (unsigned int)gClockBackup[eCLK_SCU06];  //vin, vpu, du, pvi tx rd
		*(volatile unsigned int *)0xf000001C = (unsigned int)gClockBackup[eCLK_SCU07];  //pvitx wr, pvitx fifo, pvitx vdac, recfmt0

		PPAPI_PVIRX_Initialize();

		/* power down dac */
		*(volatile unsigned int *)0xf0100070 = (unsigned int)gClockBackup[eCLK_ADC0_PD];	// power down
		*(volatile unsigned int *)0xf0000020 = (unsigned int)gClockBackup[eCLK_SCU08];  //recfmt_2x_clock 0, recfmt_2x_mph0, recfmt_2x_mph1, quad 0

		*(volatile unsigned int *)0xf0000024 = (unsigned int)gClockBackup[eCLK_SCU09];  //quad_2x, disfmt, disfmt_2x
		*(volatile unsigned int *)0xf0000028 = (unsigned int)gClockBackup[eCLK_SCU10];  //rxbyteclkhs, mipi_lane 0, mipi_pixel 0
		*(volatile unsigned int *)0xf000002C = (unsigned int)gClockBackup[eCLK_SCU11];  //mipi cci 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 = (unsigned int)gClockBackup[eCLK_SCU12];  //tck

		//*(volatile unsigned int *)0xf0000000 = gClockBackup[eCLK_SCU00];	// apb, ddr_axi, cpu_axi, cpu


		*(vuint32 *)(0xf0800000) = 0x203;	//svm enable
		*(vuint32 *)(0xf0800014) |= 0x701;	//svm input enable	[0]= 0, [10:8]=0, enable: [0]=1, [10:8]=0x7

		*(vuint32 *)(0xf0904500) = 0x1;		//du enable
		*(vuint32 *)(0xf0f00154) = 0xf0000000;		//quad write enable
		*(vuint32 *)(0xf0f00158) = 0x8f000003;		//quad read enable

		cache_enable();
		PPDRV_TIMER_Start (0, 0);
		GIE_ENABLE();

	}else if(strcmp(argv[1], "off") == 0){

		*(vuint32 *)(0xf0800000) = 0x202;	//svm disable
		*(vuint32 *)(0xf0800014) &= ~0x701;	//svm input disable	[0]= 0, [10:8]=0, enable: [0]=1, [10:8]=0x7

		*(vuint32 *)(0xf0904500) = 0x0;		//du disable
		*(vuint32 *)(0xf0f00154) = 0x0;		//quad write disable
		*(vuint32 *)(0xf0f00158) = 0x0;		//quad read disable

		cache_disable();
	    GIE_DISABLE();
		PPDRV_TIMER_Stop (0, 0);


		// backup clock setting
		gClockBackup[eCLK_SCU00] = *(vuint32 *)(0xf0000000); // apb, ddr_axi, cpu_axi, cpu (X-tal In / 16)
		gClockBackup[eCLK_SCU01] = *(vuint32 *)(0xf0000004); //Qspi, timer, wdt, uart
		gClockBackup[eCLK_SCU02] = *(vuint32 *)(0xf0000008); // gadc, svm,
		gClockBackup[eCLK_SCU03] = *(vuint32 *)(0xf000000c); //pvirx 0:en, 1:dis
		gClockBackup[eCLK_SCU04] = *(vuint32 *)(0xf0000010); //vadc 0:en, 1:dis
		gClockBackup[eCLK_SCU05] = *(vuint32 *)(0xf0000014); //isp
		gClockBackup[eCLK_SCU06] = *(vuint32 *)(0xf0000018); //vin, vpu, du, pvi tx rd
		gClockBackup[eCLK_SCU07] = *(vuint32 *)(0xf000001c); //pvitx wr, pvitx fifo, pvitx vdac, recfmt0
		gClockBackup[eCLK_SCU08] = *(vuint32 *)(0xf0000020); //recfmt_2x_clock 0, recfmt_2x_mph0, recfmt_2x_mph1, quad 0
		gClockBackup[eCLK_SCU09] = *(vuint32 *)(0xf0000024); //quad_2x, disfmt, disfmt_2x
		gClockBackup[eCLK_SCU10] = *(vuint32 *)(0xf0000028); //rxbyteclkhs, mipi_lane 0, mipi_pixel 0
		gClockBackup[eCLK_SCU11] = *(vuint32 *)(0xf000002c); //mipi cci 0:en, 1:dis
		gClockBackup[eCLK_SCU12] = *(vuint32 *)(0xf0000030); //tck
		gClockBackup[eCLK_ADC0_PD] = *(vuint32 *)(0xf0700054); //vadc0 power down
		gClockBackup[eCLK_ADC1_PD] = *(vuint32 *)(0xf0701054); //vadc1 power down
		gClockBackup[eCLK_ADC2_PD] = *(vuint32 *)(0xf0702054); //vadc2 power down
		gClockBackup[eCLK_ADC3_PD] = *(vuint32 *)(0xf0703054); //vadc3 power down
		gClockBackup[eCLK_DAC_PD] = *(vuint32 *)(0xf0100070); //vdac power down

		// SET PWDN mode
		// 3. clock down (apb, ddr_axi, cpu_axi, cpu)
		//*(volatile unsigned int *) 0xf0000000 = 0x0D0F0C0F; // apb, ddr_axi, cpu_axi, cpu (X-tal In / 16)

		*(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00808080;  //Qspi, timer, wdt
		*(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00000080;  //gadc
		*(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00800000;  //svm
		*(volatile unsigned int *)0xf000000C |= (unsigned int)0x80808080;  //pvirx 0:en, 1:dis
		*(volatile unsigned int *)0xf0000010 |= (unsigned int)0x80808080;  //vadc 0:en, 1:dis

		/* power down adc */
		*(volatile unsigned int *)0xf0700054 |= (unsigned int)0x00000010;  // power down
		*(volatile unsigned int *)0xf0701054 |= (unsigned int)0x00000010;  // power down
		*(volatile unsigned int *)0xf0702054 |= (unsigned int)0x00000010;  // power down
		*(volatile unsigned int *)0xf0703054 |= (unsigned int)0x00000010;  // power down

		*(volatile unsigned int *)0xf0000014 |= (unsigned int)0x80808080;  //isp
		*(volatile unsigned int *)0xf0000018 |= (unsigned int)0x80808080;  //vin, vpu, du, pvi tx rd
		*(volatile unsigned int *)0xf000001C |= (unsigned int)0x80808080;  //pvitx wr, pvitx fifo, pvitx vdac, recfmt0

		/* power down dac */
		*(volatile unsigned int *)0xf0100070 |= (unsigned int)0x00100000;  // power down

		*(volatile unsigned int *)0xf0000020 |= (unsigned int)0x80808080;  //recfmt_2x_clock 0, recfmt_2x_mph0, recfmt_2x_mph1, quad 0

		*(volatile unsigned int *)0xf0000024 |= (unsigned int)0x80008080;  //quad_2x, disfmt, disfmt_2x
		*(volatile unsigned int *)0xf0000028 |= (unsigned int)0x80808000;  //rxbyteclkhs, mipi_lane 0, mipi_pixel 0
		*(volatile unsigned int *)0xf000002C |= (unsigned int)0x00000080;  //mipi cci 0:en, 1:dis
		*(volatile unsigned int *)0xf0000030 |= (unsigned int)0x80808000;  //tck

	}

	return 0;
}



#endif // SUPPORT_DEBUG_CLI

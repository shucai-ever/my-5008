#include <stdio.h>
#include "type.h"
#include "debug.h"
#include "uart_drv.h"


//unsigned char log_run_level = LOG_LVL_CRITICAL;
unsigned char log_run_level = LOG_LVL_DEBUG;
//unsigned char log_run_level = LOG_LVL_NONE;

const char * log_level_strings [] = {
    "NONE", // 0
    "CRIT", // 1
    "WARN", // 2
    "NOTI", // 3
    " LOG", // 4
    "DEBG" // 5
};



void print_hex(uint8 *buf, uint32 size)
{	
	uint32 i;

	for(i=0;i<size;i++){		
		if((i&0xf) == 0)LOG_DEBUG("0x%08x  ", (unsigned int)(buf + i));
		LOG_DEBUG("%02x ", buf[i]);
		if(((i+1)&0xf) == 0)LOG_DEBUG("\n");
	}	
	LOG_DEBUG("\n");
}

void print_hexw(uint32 *buf, uint32 size)
{	
	uint32 i;

	size = ((size + 3) & (~0x3))/2;
	for(i=0;i<size;i++){		
		if((i&0x3) == 0)LOG_DEBUG("0x%08x  ", (unsigned int)(buf + i));
		LOG_DEBUG("%08x ", (unsigned int)buf[i]);
		if(((i+1)&0x3) == 0)LOG_DEBUG("\n");
	}	
	LOG_DEBUG("\n");
}

//==================================================
// overriding function for printf redirection -> uart
//==================================================
#undef putchar
inline int putchar(int c)
{

    /* LF -> CR+LF due to some terminal programs */
    if ( c == '\n' )
        putchar( '\r' );

    PPDRV_UART_OutByte(UART_PNT_CH, c);

	return c;
}


__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{	
	int    i;

	for (i = 0; i < size; i++)		
		putchar(buf[i]);

}




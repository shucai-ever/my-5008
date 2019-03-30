#include "..\common\board.h"
#include "..\common\common.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "..\Common\i2c.h"
//#include "pi5008k_test.h"
//#include "pi5008k_remocon.h"
//#include "pi5008k_func.h"
#include "pi5008k_func.h"
#define CMD_PROMPT      ">>"
#include <stdlib.h>     //NULL pointer definition
#include <stdarg.h>     // (...) parameter handling
#include <ctype.h>

int pi5008k_usart1_buffer_cnt;
int pi5008k_usart2_buffer_cnt;
char pi5008k_usart1_get_buffer[PI5008K_USART_BUFFER_SIZE];
char pi5008k_usart2_get_buffer[PI5008K_USART_BUFFER_SIZE];
char pi5008k_usart1_get_cmd[PI5008K_USART_BUFFER_SIZE];
char pi5008k_usart2_get_cmd[PI5008K_USART_BUFFER_SIZE];
char pi5008k_usart1_get_cmd_arg[20][20];
char pi5008k_usart2_get_cmd_arg[20][20];
int pi5008k_usart1_arg_div_point_cnt;
int pi5008k_usart2_arg_div_point_cnt;
int pi5008k_usart_arg_cnt_for_test;
char pi5008k_usart1_get_cmd_arg_div_point[20];// cmd arguments number
char pi5008k_usart2_get_cmd_arg_div_point[20];// cmd arguments number
unsigned int pi5008k_usart1_cmd_exist_flag; //exist flag
unsigned int pi5008k_usart2_cmd_exist_flag; //exist flag
static char gTempCmdBuffer1[PI5008K_USART_BUFFER_SIZE];
static char gTempCmdBuffer2[PI5008K_USART_BUFFER_SIZE];

extern uint8_t KeyNumber;
extern CanTxMsg TxMessage;



extern uint32_t g_old_KeyVal;
extern uint32_t g_start_key_tim ;
extern uint32_t g_long_key_flag;
extern uint32_t g_input_key_flag;
extern uint32_t g_repeat_cnt;
extern uint32_t g_repeat_cnt_prv;


#define IR_KEY_REPEAT_TERM		12000



void PutUart1Con(char ch) 
{
        if(ch == '\n')
        {   
                USART_SendData(USART1, (char) '\r');

                while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { } 
        }   

        USART_SendData(USART1, (char) ch);

        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { } 
}
void PutUart2Con(char ch) 
{
        if(ch == '\n')
        {   
                USART_SendData(USART2, (char) '\r');

                while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { } 
        }   

        USART_SendData(USART2, (char) ch);

        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) { } 
}

void UART1_PRINTF(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vfprintf_((&PutUart1Con), format, arg);
	va_end(arg);
}
void UART2_PRINTF(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vfprintf_((&PutUart2Con), format, arg);
	va_end(arg);
}

static void long_itoa (long val, int radix, int len, void (*putc) (char))
{
	char c, sgn = 0, pad = ' ';
	char s[20];
	int  i = 0;


	if (radix < 0) {
		radix = -radix;
		if (val < 0) {
			val = -val;
			sgn = '-';
		}
	}
	if (len < 0) {
		len = -len;
		pad = '0';
	}
	if (len > 20) return;
	do {
		c = (char)((unsigned long)val % radix); //cast!
		//if (c >= 10) c += ('A'-10); //ABCDEF
		if (c >= 10) c += ('a'-10); //abcdef
		else c += '0';            //0123456789
		s[i++] = c;
		val = (unsigned long)val /radix; //cast!
	} while (val);
	if (sgn) s[i++] = sgn;
	while (i < len)
		s[i++] = pad;
	do
		(*putc)(s[--i]);
	while (i);
}

static int vfprintf_(void (*putc)(char), const char* str,  va_list arp)
{
	int d, r, w, s, l;  //d=char, r = radix, w = width, s=zeros, l=long
	char *c;            // for the while loop only
	//const char* p;
#ifdef INCLUDE_FLOAT
	float f;
	long int m, w2;
#endif



	while ((d = *str++) != 0) {
		if (d != '%') {//if it is not format qualifier
			(*putc)(d);
			continue;//get out of while loop
		}
		d = *str++;//if it is '%'get next char
		w = r = s = l = 0;
		if (d == '%') {//if it is % print silmpy %
			(*putc)(d);
			d = *str++;
		}
		if (d == '0') {
			d = *str++; s = 1;  //padd with zeros
		}
		while ((d >= '0')&&(d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}
		if (s) w = -w;      //padd with zeros if negative

#ifdef INCLUDE_FLOAT
		w2 = 0;
		if (d == '.')
			d = *str++;
		while ((d >= '0')&&(d <= '9')) {
			w2 += w2 * 10 + (d - '0');
			d = *str++;
		}
#endif

		if (d == 's') {// if string
			c = va_arg(arp, char*); //get buffer addres
			//p = c;//debug
			while (*c)
				(*putc)(*(c++));//write the buffer out
			continue;
		}


		//debug

		//while(*p) PutUartCon(*p++);

		if (d == 'c') {
			(*putc)((char)va_arg(arp, int));
			continue;
		}
		if (d == 'u') {     // %ul
			r = 10;
			d = *str++;
		}
		if (d == 'l') {     // long =32bit
			l = 1;
			if (r==0) r = -10;
			d = *str++;
		}
		//		if (!d) break;
		if (d == 'u') r = 10;//     %lu,    %llu
		else if (d == 'd' || d == 'i') {if (r==0) r = -10;}  //can be 16 or 32bit int
		else if (d == 'X' || d == 'x') r = 16;               // 'x' added by mthomas
		else if (d == 'b') r = 2;
		else str--;                                         // normal character

#ifdef INCLUDE_FLOAT
		if (d == 'f' || d == 'F') {
			f=va_arg(arp, double);
			if (f>0) {
				r=10;
				m=(int)f;
			}
			else {
				r=-10;
				f=-f;
				m=(int)(f);
			}
			long_itoa(m, r, w, (putc));
			f=f-m; m=f*(10^w2); w2=-w2;
			long_itoa(m, r, w2, (putc));
			l=3; //do not continue with long
		}
#endif

		if (!r) continue;  //
		if (l==0) {
			if (r > 0){      //unsigned
				unsigned long temp = (unsigned long)va_arg(arp, unsigned);
				//long_itoa((unsigned long)va_arg(arp, int), r, w, (putc)); //needed for 16bit int, no harm to 32bit int
				long_itoa(temp, r, w, (putc)); //needed for 16bit int, no harm to 32bit int
			}
			else            //signed
			{
				//long_itoa((long)va_arg(arp, int), r, w, (putc));
				long temp = (long)va_arg(arp, int);
				long_itoa(temp, r, w, (putc));
			}
		} else if (l==1){  // long =32bit
			//long_itoa((long)va_arg(arp, long), r, w, (putc));        //no matter if signed or unsigned
			long temp = (long)va_arg(arp, long);
			long_itoa(temp, r, w, (putc));        //no matter if signed or unsigned
		}
	}

	return 0;
}

void PI5008K_RDK_PrintHelp(void)
{
	UART1_PRINTF("==============================================================================================\r\n");	
	UART1_PRINTF("\"Hello PI5008K RDK\"\r\n");	
	UART1_PRINTF("\"reg\"sp\"r\"sp[addr]sp[data]sp[cnt]\\n\r\n");	
	UART1_PRINTF("\"reg\"sp\"w\"sp[addr]sp[data]\\n\r\n");	
	UART1_PRINTF("\"reg\"sp\"cw\"sp[addr]sp[data 1]sp[data 2]sp[data 3]sp[data 4]sp[data ...]sp[data N]\\n\r\n");	
	UART1_PRINTF("\"can\"sp\"tx\"sp[data]sp[loopcnt]\\n\r\n");	
	UART1_PRINTF("==============================================================================================\r\n");	
	//UART2_PRINTF("==============================================================================================\r\n");	
	//UART2_PRINTF("\"Hello PI5008K RDK\"\r\n");	
	//UART2_PRINTF("\"reg\"sp\"r\"sp[addr]sp[data]sp[cnt]\\n\r\n");	
	//UART2_PRINTF("\"reg\"sp\"w\"sp[addr]sp[data]\\n\r\n");	
	//UART2_PRINTF("\"reg\"sp\"cw\"sp[addr]sp[data 1]sp[data 2]sp[data 3]sp[data 4]sp[data ...]sp[data N]\\n\r\n");	
	//UART2_PRINTF("\"can\"sp\"tx\"sp[data]sp[loopcnt]\\n\r\n");	
	//UART2_PRINTF("==============================================================================================\r\n");	

}
void PI5008K_UART_BUF_INIT(void)
{
	memset(pi5008k_usart1_get_buffer,0,sizeof(pi5008k_usart1_get_buffer));
	memset(pi5008k_usart2_get_buffer,0,sizeof(pi5008k_usart2_get_buffer));
	memset(pi5008k_usart1_get_cmd,0,sizeof(pi5008k_usart1_get_cmd));
	memset(pi5008k_usart2_get_cmd,0,sizeof(pi5008k_usart2_get_cmd));
	memset(pi5008k_usart1_get_cmd_arg,0,sizeof(pi5008k_usart1_get_cmd_arg));
	memset(pi5008k_usart2_get_cmd_arg,0,sizeof(pi5008k_usart2_get_cmd_arg));
	memset(pi5008k_usart1_get_cmd_arg_div_point,0,sizeof(pi5008k_usart1_get_cmd_arg_div_point));
	memset(pi5008k_usart2_get_cmd_arg_div_point,0,sizeof(pi5008k_usart2_get_cmd_arg_div_point));
	memset(gTempCmdBuffer1,0,sizeof(gTempCmdBuffer1));
	memset(gTempCmdBuffer2,0,sizeof(gTempCmdBuffer2));

}
void PI5008K_Init(void)
{

	UART1_PRINTF("#####################################################\n\r");
	UART1_PRINTF("#                  PI5008K RDK                  #\n\r");
	UART1_PRINTF("#####################################################\n\r");
	PI5008K_RDK_PrintHelp();
	PI5008K_UART_BUF_INIT();
	PI5008K_Reset_Con();	

}


void PI5008K_Usart1CmdClearBufferAll(void)
{
	memset(pi5008k_usart1_get_buffer,0,PI5008K_USART_BUFFER_SIZE); 
	memset(pi5008k_usart1_get_cmd,0,PI5008K_USART_BUFFER_SIZE);
	memset(pi5008k_usart1_get_cmd_arg,0,10*10);
	memset(pi5008k_usart1_get_cmd_arg_div_point,0,10);	

}
void PI5008K_Usart2CmdClearBufferAll(void)
{
	
	memset(pi5008k_usart2_get_buffer,0,PI5008K_USART_BUFFER_SIZE); 
	memset(pi5008k_usart2_get_cmd,0,PI5008K_USART_BUFFER_SIZE);
	memset(pi5008k_usart2_get_cmd_arg,0,10*10);
	memset(pi5008k_usart2_get_cmd_arg_div_point,0,10);	

}


void PI5008K_Usart1CmdExistFlagClear(void)
{
	pi5008k_usart1_cmd_exist_flag=0;
	UART1_PRINTF(">> ");
	PI5008K_Usart1CmdClearBufferAll();
}
unsigned int PI5008K_Usart1CmdExistFlagGet(void)
{
	return pi5008k_usart1_cmd_exist_flag;
}
void PI5008K_Usart2CmdExistFlagClear(void)
{
	pi5008k_usart2_cmd_exist_flag=0;
	UART2_PRINTF(">> ");
	PI5008K_Usart2CmdClearBufferAll();
}
unsigned int PI5008K_Usart2CmdExistFlagGet(void)
{
	return pi5008k_usart2_cmd_exist_flag;
}
int PI5008K_UsartCmdArgHexGet(char *str)
{
	char arg_temp[10];
	int arg_temp_size;
	int arg;
	
	//printf("\r\n in the PR1000K_UsartCmdArgHexGet %s \r\n ",str); 		
	
	arg=(int)strtol(str, 0, 16);

	return arg;
	
}


int PI5008K_Usart1GetCh(void)
{
	uint8_t ch;
	while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==RESET);
	ch=USART_ReceiveData(USART1);
	return ch;
}
int PI5008K_Usart1Push(void)
{
	char *usart_bf;
	int cmd_arg_buffer_size;
	usart_bf = &pi5008k_usart1_get_buffer[0]; 
	int i=0;


	
	usart_bf[pi5008k_usart1_buffer_cnt] = PI5008K_Usart1GetCh(); //recieve
	
	//printf("\r\ndebug0:%s",usart_bf);
	USART_SendData(USART1, usart_bf[pi5008k_usart1_buffer_cnt]); //feedback function

	//check "Enter Button"
	if(usart_bf[pi5008k_usart1_buffer_cnt]=='\r')
	{
		/*last argu check & init cnt*/
		if(pi5008k_usart1_arg_div_point_cnt!=0)
		{
			pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt]=pi5008k_usart1_buffer_cnt;

			//cmd_arg_buffer_size = pi5008k_usart1_buffer_cnt-pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+1;
			cmd_arg_buffer_size = pi5008k_usart1_buffer_cnt-pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1];
			// "+1" meaning is NULL character.
			
			memcpy(pi5008k_usart1_get_cmd_arg[pi5008k_usart1_arg_div_point_cnt-1], \
			usart_bf+pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+1, \
			cmd_arg_buffer_size-1); //add NULL			
			//strcat(pi5008k_usart1_get_cmd_arg[pi5008k_usart1_arg_div_point_cnt-1],"\n");
		}
		else
		{
			memcpy(pi5008k_usart1_get_cmd,usart_bf,pi5008k_usart1_buffer_cnt);
			//printf("\r\n test = %s \r\n",pi5008k_usart1_get_cmd);
		}
		pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt]=pi5008k_usart1_buffer_cnt;

		//memcpy(pi5008k_usart_get_cmd,usart_bf,pi5008k_usart_buffer_cnt);
		//printf("\r\narg0 = %s \r\n",pi5008k_usart1_get_cmd);
		//printf("arg1 = 0x%02x \r\n",*(usart_bf+pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+1));
		//printf("arg2 = 0x%02x \r\n",*(usart_bf+pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+2));
		//printf("arg3 = 0x%02x \r\n",*(usart_bf+pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+3));
		//printf("arg4 = 0x%02x \r\n",*(usart_bf+pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+4));
		pi5008k_usart_arg_cnt_for_test=pi5008k_usart1_arg_div_point_cnt;
		pi5008k_usart1_arg_div_point_cnt=0;
		pi5008k_usart1_buffer_cnt=0;			
		
		USART_SendData(USART1, '\r');
		return 0;
	}
	//check "Back Space Button" /*TODO*/
	else if(usart_bf[pi5008k_usart1_buffer_cnt]==127 || usart_bf[pi5008k_usart1_buffer_cnt]==8)
	{
		if(pi5008k_usart1_buffer_cnt>0)
		{

				gTempCmdBuffer1[pi5008k_usart1_buffer_cnt-1]='\0';
				UART1_PRINTF("%c[1A", 0x1B); // 1line up
				UART1_PRINTF("\n\r%c[2K%s %s", 0x1B, CMD_PROMPT, gTempCmdBuffer1); 
				pi5008k_usart1_buffer_cnt = strlen(gTempCmdBuffer1);
				
				//printf("\r\ndebug1:%s\r\n",gTempCmdBuffer);
				//printf("\r\ndebug2:%d,%d\r\n",pi5008k_usart_buffer_cnt,strlen(gTempCmdBuffer));
				
		}
	}
	else 
	{
		//check "Space Button" and get token
		if(usart_bf[pi5008k_usart1_buffer_cnt]== 0x20)
		{
			/* argu div check & increase cnt*/
			pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt]=pi5008k_usart1_buffer_cnt;
			if(pi5008k_usart1_arg_div_point_cnt==0)
			{
				memcpy(pi5008k_usart1_get_cmd,usart_bf,pi5008k_usart1_buffer_cnt);
				//printf("\r\n test = %s \r\n",pr1000_usart_get_cmd);
			}
			else
			{

					
					memcpy(pi5008k_usart1_get_cmd_arg[pi5008k_usart1_arg_div_point_cnt-1], \
					usart_bf+pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+1, \
					pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt]- \
					//pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]+1); //add NULL
					pi5008k_usart1_get_cmd_arg_div_point[pi5008k_usart1_arg_div_point_cnt-1]-1); 
				
					//printf("\r\n space argargarg \r\n");
					//printf("\r\n test+pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1] = %s \r\n",test+pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1]);
					//printf("\r\n test = %s \r\n",test);
					//printf("\r\n pr1000_usart_arg_div_point_cnt = %d \r\n",pr1000_usart_arg_div_point_cnt);
					//printf("\r\n pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1] = %d \r\n",pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1]);
					//printf("\r\n pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt] = %d \r\n",pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt]);
					//printf("\r\n space argargarg \r\n");

					
			}
			pi5008k_usart1_arg_div_point_cnt++;
		}
		pi5008k_usart1_buffer_cnt++;
		strcpy(gTempCmdBuffer1, usart_bf);
		
	}

	return 1;
}
int PI5008K_Usart2GetCh(void)
{
	uint8_t ch;
	while(USART_GetFlagStatus(USART2,USART_FLAG_RXNE)==RESET);
	ch=USART_ReceiveData(USART2);
	return ch;
}
int PI5008K_Usart2_Get_Response(void)
{
	char *usart_bf;
	int cmd_arg_buffer_size;
	usart_bf = &pi5008k_usart2_get_buffer[0]; 

	
	usart_bf[pi5008k_usart2_buffer_cnt] = PI5008K_Usart2GetCh(); //recieve
	
	//printf("\r\ndebug0:%s",usart_bf);
	//USART_SendData(USART1, usart_bf[pi5008k_usart2_buffer_cnt]); //feedback function
	//UART1_PRINTF("%s",usart_bf[pi5008k_usart2_buffer_cnt]); 

	//check "Enter Button" //LF code
	if(usart_bf[pi5008k_usart2_buffer_cnt]==0x0A)
	{
		memcpy(pi5008k_usart2_get_cmd,usart_bf,pi5008k_usart2_buffer_cnt);
		return 0;		
	}
	else 
	{		
		pi5008k_usart2_buffer_cnt++;
	}

	return 1;
}

int PI5008K_Usart2Push(void)
{
	char *usart_bf;
	int cmd_arg_buffer_size;
	usart_bf = &pi5008k_usart2_get_buffer[0]; 



	
	usart_bf[pi5008k_usart2_buffer_cnt] = PI5008K_Usart2GetCh(); //recieve
	
	//printf("\r\ndebug0:%s",usart_bf);
	USART_SendData(USART2, usart_bf[pi5008k_usart2_buffer_cnt]); //feedback function

	//check "Enter Button"
	if(usart_bf[pi5008k_usart2_buffer_cnt]=='\r')
	{
		/*last argu check & init cnt*/
		if(pi5008k_usart2_arg_div_point_cnt!=0)
		{
			pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt]=pi5008k_usart2_buffer_cnt;

			cmd_arg_buffer_size = pi5008k_usart2_buffer_cnt-pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt-1]+1;
			// "+1" meaning is NULL character.
			
			memcpy(pi5008k_usart2_get_cmd_arg[pi5008k_usart2_arg_div_point_cnt-1], \
			usart_bf+pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt-1], \
			cmd_arg_buffer_size); //add NULL						
		}
		else
		{
			memcpy(pi5008k_usart2_get_cmd_arg,usart_bf,pi5008k_usart2_buffer_cnt);
			//printf("\r\n test = %s \r\n",pr1000_usart_get_cmd);
		}
		pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt]=pi5008k_usart2_buffer_cnt;

		//memcpy(pi5008k_usart2_get_cmd,usart_bf,pi5008k_usart2_buffer_cnt);
		//printf("\r\n test = %s \r\n",pi5008k_usart2_get_cmd);

		pi5008k_usart2_arg_div_point_cnt=0;
		pi5008k_usart2_buffer_cnt=0;			
		
		USART_SendData(USART2, '\r');
		return 0;
	}
	//check "Back Space Button" /*TODO*/
	else if(usart_bf[pi5008k_usart2_buffer_cnt]==127 || usart_bf[pi5008k_usart2_buffer_cnt]==8)
	{
		if(pi5008k_usart2_buffer_cnt>0)
		{

				gTempCmdBuffer2[pi5008k_usart2_buffer_cnt-1]='\0';
				UART2_PRINTF("%c[1A", 0x1B); // 1line up
				UART2_PRINTF("\n\r%c[2K%s %s", 0x1B, CMD_PROMPT, gTempCmdBuffer2); 
				pi5008k_usart2_buffer_cnt = strlen(gTempCmdBuffer2);
				
				//printf("\r\ndebug1:%s\r\n",gTempCmdBuffer);
				//printf("\r\ndebug2:%d,%d\r\n",pi5008k_usart_buffer_cnt,strlen(gTempCmdBuffer));
				
		}
	}
	else 
	{
		//check "Space Button" and get token
		if(usart_bf[pi5008k_usart2_buffer_cnt]== 0x20)
		{
			/* argu div check & increase cnt*/
			pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt]=pi5008k_usart2_buffer_cnt;
			if(pi5008k_usart2_arg_div_point_cnt==0)
			{
				memcpy(pi5008k_usart2_get_cmd,usart_bf,pi5008k_usart2_buffer_cnt);
				//printf("\r\n test = %s \r\n",pr1000_usart_get_cmd);
			}
			else
			{

					
					memcpy(pi5008k_usart2_get_cmd_arg[pi5008k_usart2_arg_div_point_cnt-1], \
					usart_bf+pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt-1], \
					pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt]- \
					pi5008k_usart2_get_cmd_arg_div_point[pi5008k_usart2_arg_div_point_cnt-1]+1); //add NULL
					
				
					//printf("\r\n space argargarg \r\n");
					//printf("\r\n test+pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1] = %s \r\n",test+pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1]);
					//printf("\r\n test = %s \r\n",test);
					//printf("\r\n pr1000_usart_arg_div_point_cnt = %d \r\n",pr1000_usart_arg_div_point_cnt);
					//printf("\r\n pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1] = %d \r\n",pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt-1]);
					//printf("\r\n pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt] = %d \r\n",pr1000_usart_get_cmd_arg_div_point[pr1000_usart_arg_div_point_cnt]);
					//printf("\r\n space argargarg \r\n");

					
			}
			pi5008k_usart2_arg_div_point_cnt++;
		}
		pi5008k_usart2_buffer_cnt++;
		strcpy(gTempCmdBuffer2, usart_bf);
		
	}

	return 1;
}




void swap(char*  a, char*  b)
{
    char temp;
 
    temp=*a;
    *a=*b;
    *b=temp;
    
}

void reverse(char *str, int i)
{
	int start =0;
	int end=i-1;
	while(start <end)
	{
		swap((str+start), (str+end));
		start++;
		end--;
	}
}

// Implementation of itoa()
char* itoa(unsigned int num, char* str, int base)
{
    int i = 0;
    bool isNegative = FALSE;
 
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled only with 
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = TRUE;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}




char * PI5008K_UartcmdGetCheckSum(unsigned char *buf,unsigned int buf_size)
{
	unsigned int check_sum=0;
	char str_cs_buf[100]="";
	char str_0x[3]="0x";
	char ascii_us_code[1]=" ";
	unsigned int i=0;

	check_sum=ascii_us_code[0];//add sp code in checksum
	str_cs_buf[0]=ascii_us_code[0];
	strcpy(&str_cs_buf[1],str_0x);
	
	//UART1_PRINTF("buf_size = 0x%4x \r\n",buf_size); 
	for(i=0;i<buf_size;i++)
	{
		check_sum+=buf[i];
		//UART1_PRINTF("buf[%d] = 0x%8x \r\n",i,buf[i]); 
		
	}
	check_sum = check_sum&0xFFFFFFFF;
	
	itoa(check_sum,&str_cs_buf[3],16);
	#if 0
	UART1_PRINTF("cs hex = 0x%8x \r\n",check_sum); 
	UART1_PRINTF("cs str = %s \r\n",str_cs_buf); 
	for(i=0;i<strlen(str_cs_buf);i++)
	{
		UART1_PRINTF("0x%2x ",str_cs_buf[i]); 
		
	}
	UART1_PRINTF(" \r\n"); 
	#endif
	
	return str_cs_buf;
	
}

void PI5008K_Uart1_CmdParsing(void)
{
	char send_cmd_buf[100];
	char temp;
	int i=0;
	int loopcnt=0;
	
	if(!strcmp(pi5008k_usart1_get_cmd,"t"))
	{
		if(!strcmp(pi5008k_usart1_get_cmd_arg[0],"con"))
		{
			if(!strcmp(pi5008k_usart1_get_cmd_arg[1],"rem"))
			{
		
				//UART1_PRINTF("hello remote world \r\n"); 
				if(!strcmp(pi5008k_usart1_get_cmd_arg[2],"up"))
				{
					strcpy(send_cmd_buf,"t con rem up");					
				}
				else if(!strcmp(pi5008k_usart1_get_cmd_arg[2],"down"))
				{
					strcpy(send_cmd_buf,"t con rem down");		
				}
				else if(!strcmp(pi5008k_usart1_get_cmd_arg[2],"left")) 
				{
					strcpy(send_cmd_buf,"t con rem left");		
				}
				else if(!strcmp(pi5008k_usart1_get_cmd_arg[2],"right")) 
				{
					strcpy(send_cmd_buf,"t con rem right");		
				}
				else if(!strcmp(pi5008k_usart1_get_cmd_arg[2],"center")) 
				{
					strcpy(send_cmd_buf,"t con rem center");		
				}
				
				strcat(send_cmd_buf,PI5008K_UartcmdGetCheckSum(send_cmd_buf,strlen(send_cmd_buf)));
				strcat(send_cmd_buf,"\n");
				
				//UART1_PRINTF(" debug \r\n"); 
				//UART1_PRINTF("send uart data = \"%s\"",send_cmd_buf); 
				for(i=0;i<strlen(send_cmd_buf);i++)
				{
					//UART1_PRINTF("0x%02x ",send_cmd_buf[i]); 
					
				}	
				//UART1_PRINTF(" \r\n"); 
				//UART1_PRINTF(" debug end \r\n"); 
				
				UART2_PRINTF("%s",send_cmd_buf); //send data
			}

		}
		else if(!strcmp(pi5008k_usart1_get_cmd_arg[0],"reg")) //add space
		{

			if(!strcmp(pi5008k_usart1_get_cmd_arg[1],"sw")) //add space
			{
				UART1_PRINTF("hello reg world \r\n"); 

				strcpy(send_cmd_buf,"t reg sw ");					
				temp=strlen(send_cmd_buf)+strlen(pi5008k_usart1_get_cmd_arg[2])+\
					strlen(pi5008k_usart1_get_cmd_arg[3])+1;//add sp

				strcat(send_cmd_buf,pi5008k_usart1_get_cmd_arg[2]);
				strcat(send_cmd_buf," ");					
				strcat(send_cmd_buf,pi5008k_usart1_get_cmd_arg[3]);
				strcat(send_cmd_buf,PI5008K_UartcmdGetCheckSum(send_cmd_buf,temp));
				strcat(send_cmd_buf,"\n");
				
				UART1_PRINTF(" debug \r\n"); 
				UART1_PRINTF("send uart data = %s\r\n",send_cmd_buf); 
				for(i=0;i<strlen(send_cmd_buf);i++)
				{
					UART1_PRINTF("0x%02x ",send_cmd_buf[i]); 
					
				}	
				UART1_PRINTF(" \r\n"); 
				UART1_PRINTF(" debug end \r\n"); 
				
				UART2_PRINTF("%s",send_cmd_buf); //send data
			}
		}

		
		
	}
	else if(!strcmp(pi5008k_usart1_get_cmd,"can"))
	{

		if(!strcmp(pi5008k_usart1_get_cmd_arg[0],"tx"))
		{
			loopcnt = PI5008K_UsartCmdArgHexGet(pi5008k_usart1_get_cmd_arg[2]);
			UART1_PRINTF("loopcnt = %d \r\n",loopcnt); 

			KeyNumber = PI5008K_UsartCmdArgHexGet(pi5008k_usart1_get_cmd_arg[1]);
			UART1_PRINTF("test can tx [0x%2x] \r\n",KeyNumber); 
			TxMessage.Data[0] = KeyNumber;

			
			//GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_RESET); 
			//Delay_ms(200);	// delay 200ms
			//GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_SET); 
			//GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);
			for(i=0;i<loopcnt;i++)
			{
				UART1_PRINTF("loopcnt = [%d/%d] \r\n",i,loopcnt); 
				CAN_Transmit(CAN1, &TxMessage);
				Delay_ms(200);	// delay 200ms
			}
		
		}

	}
	else
	{
		UART1_PRINTF("unvaild cmd \r\n"); 
	}
	
	UART1_PRINTF("end remote world \r\n"); 

}
void PI5008K_Uart_Reg_Single_Write(unsigned int  addr,unsigned int data)
{
	char send_cmd_buf[100]=0;
	char temp;
	int i=0;
	char str_add[100]=0;
	char str_data[100]=0;
	char str_0x[2]="0x";
	
	strcpy(&str_add[0],str_0x);
	strcpy(&str_data[0],str_0x);

	

	
	itoa(addr,&str_add[2],16);
	itoa(data,&str_data[2],16);

	
	strcpy(send_cmd_buf,"t reg sw ");					
	temp=strlen(send_cmd_buf)+strlen(str_add)+\
		strlen(str_data)+1;//add sp
	
	strcat(send_cmd_buf,str_add);
	strcat(send_cmd_buf," ");					
	strcat(send_cmd_buf,str_data);
	strcat(send_cmd_buf,PI5008K_UartcmdGetCheckSum(send_cmd_buf,temp));
	strcat(send_cmd_buf,"\n");
	
	UART1_PRINTF(" debug \r\n"); 
	UART1_PRINTF(" str_add = [0x%x] \r\n",addr); 
	UART1_PRINTF(" str_data = [0x%x] \r\n",data); 
	UART1_PRINTF(" str_add = [%s] \r\n",str_add); 
	UART1_PRINTF(" str_data = [%s] \r\n",str_data); 
	UART1_PRINTF("send uart data = %s\r\n",send_cmd_buf); 
	for(i=0;i<strlen(send_cmd_buf);i++)
	{
		UART1_PRINTF("0x%02x ",send_cmd_buf[i]); 
		
	}	
	UART1_PRINTF(" \r\n"); 
	UART1_PRINTF(" debug end \r\n"); 
	UART2_PRINTF("%s",send_cmd_buf); //send data to pi5008k

}
void PI5008K_Uart_Con_Remote_Cmd(unsigned int dir)
{
	char send_cmd_buf[100];
	int i=0;
	
	//UART1_PRINTF(" dir  = %d \r\n",dir); 
	switch (dir)
	{
		case REM_DIR_UP:
		{
			strcpy(send_cmd_buf,"t con rem up");					
		}
			break;
		case REM_DIR_UP_LONG:
		{
			strcpy(send_cmd_buf,"t con rem uplong");					
		}
			break;
		
		case REM_DIR_DOWN:
		{
			strcpy(send_cmd_buf,"t con rem down");		
		}
			break;
		case REM_DIR_DOWN_LONG:
		{
			strcpy(send_cmd_buf,"t con rem downlong");		
		}
			break;
		case REM_DIR_LEFT:
		{
			strcpy(send_cmd_buf,"t con rem left");		
		}
			break;
		case REM_DIR_LEFT_LONG:
		{
			strcpy(send_cmd_buf,"t con rem leftlong");		
		}
			break;
		case REM_DIR_RIGHT:
		{
			strcpy(send_cmd_buf,"t con rem right"); 	
		}
			break;
		case REM_DIR_RIGHT_LONG:
		{
			strcpy(send_cmd_buf,"t con rem rightlong"); 	
		}
			break;
		case REM_DIR_CENTER:
		{
			strcpy(send_cmd_buf,"t con rem center");		
		}
			break;
		case REM_DIR_CENTER_LONG:
		{
			strcpy(send_cmd_buf,"t con rem centerlong");					
		}
			break;
		case REM_MENU:
		{			
			strcpy(send_cmd_buf,"t con rem menu");					
		}
			break;
		case REM_MENU_LONG:
		{			
			strcpy(send_cmd_buf,"t con rem menulong");					
		}
			break;
		case REM_NAVI:
		{			
			strcpy(send_cmd_buf,"t con rem navi");					
		}
			break;
		case REM_NAVI_LONG:
		{			
			strcpy(send_cmd_buf,"t con rem navilong");					
		}
			break;
		default:
		{
			UART1_PRINTF("unvaild cmd \r\n"); 
		}
			break;
	}
	strcat(send_cmd_buf,PI5008K_UartcmdGetCheckSum(send_cmd_buf,strlen(send_cmd_buf)));
	strcat(send_cmd_buf,"\n");
	
	//UART1_PRINTF(" debug \r\n"); 
	//UART1_PRINTF("send uart data = %s",send_cmd_buf); 
	for(i=0;i<strlen(send_cmd_buf);i++)
	{
		//UART1_PRINTF("0x%02x ",send_cmd_buf[i]); 
		
	}	
	//UART1_PRINTF(" \r\n"); 
	//UART1_PRINTF(" debug end \r\n"); 
	UART1_PRINTF("send uart data = %s",send_cmd_buf); //send data to pi5008k
	UART2_PRINTF("%s",send_cmd_buf); //send data to pi5008k
	//UART2_PRINTF("test"); //send data to pi5008k
	//UART2_PRINTF("\n"); //send data to pi5008k

	
}

void PI5008K_UartCmdProcessing(void)
{
	char i2cData=0;
	int i=0;
	
	if(PI5008K_Usart1CmdExistFlagGet()) //with external(pc)
	{
		UART1_PRINTF("\r\n>> cmd :%s \r\n",pi5008k_usart1_get_cmd);	

		for(i=0;i<pi5008k_usart_arg_cnt_for_test;i++)
		{
			UART1_PRINTF(">> arg %d : %s \r\n",i,pi5008k_usart1_get_cmd_arg[i]); 		
		}
		pi5008k_usart_arg_cnt_for_test=0;
		//UART1_PRINTF(">> %s \r\n ",pi5008k_usart1_get_cmd);	
		//UART1_PRINTF("[[remote controller command set]] command in %s \r\n ",pi5008k_usart1_get_cmd);			
		//UART1_PRINTF("[[remote controller command set]] arg1:[%s] \r\n ",pi5008k_usart1_get_cmd_arg[0]);	
		//UART1_PRINTF("[[remote controller command set]] arg2 :[%s] \r\n ",pi5008k_usart1_get_cmd_arg[1]);	
		//UART1_PRINTF("[[remote controller command set]] arg3 :[%s] \r\n ",pi5008k_usart1_get_cmd_arg[2]);
		//UART1_PRINTF("[[remote controller command set]] arg4: [%s] \r\n ",pi5008k_usart1_get_cmd_arg[3]);	
		//UART1_PRINTF("[[remote controller command set]] arg5 : [%s] \r\n ",pi5008k_usart1_get_cmd_arg[4]);	
		//UART1_PRINTF("[[remote controller command set]] arg6 : [%s] \r\n ",pi5008k_usart1_get_cmd_arg[5]);
		PI5008K_Uart1_CmdParsing();




			//UART1_PRINTF("unvaild cmd \r\n");	
			PI5008K_RDK_PrintHelp();
			PI5008K_Usart1CmdExistFlagClear();		
	}
	else if(PI5008K_Usart2CmdExistFlagGet()) //with pi5008k
	{
		UART2_PRINTF(">> %s \r\n ",pi5008k_usart2_get_cmd);	
		UART1_PRINTF(">>[UART2] %s \r\n ",pi5008k_usart2_get_cmd);	
		#if 0
		if(strcmp(pi5008k_usart_get_cmd,"i2cw")==0)
		{
			printf("[[i2cWrite]] command in %s \r\n ",pi5008k_usart_get_cmd);			
			printf("[[i2cWrite]] page : 0x%x \r\n ",PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[0]));	
			printf("[[i2cWrite]] addr : 0x%x \r\n ",PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[1]));	
			printf("[[i2cWrite]] data : 0x%x \r\n ",PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[2]));
			
			PI5008K_PageWrite(0, (0x5C<<1), PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[0]), (unsigned char)PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[1]),\
							(unsigned char)PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[2]));
			
		}
		else if(strcmp(pi5008k_usart_get_cmd,"i2cr")==0)
		{
			printf("[[i2cRead]] command in : %s \r\n ",pi5008k_usart_get_cmd);	
			printf("[[i2cRead]] page : 0x%x \r\n ",PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[0]));	
			printf("[[i2cRead]] addr : 0x%x \r\n ",PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[1]));	
			printf("[[i2cRead]] cnt : %d \r\n ",PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[2]));	


			for(i=0;i<PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[2]);i++)
			{
				PI5008K_PageRead(0, (0x5C<<1), PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[0]), \
				(unsigned char)PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[1])+i,&i2cData);
				printf("[[i2cRead]] {0x%02x , 0x%02x}, \r\n ",(unsigned char)PI5008K_UsartCmdArgHexGet(pi5008k_usart_get_cmd_arg[1])+i,i2cData);	
			}
		}
		else if(strcmp(pi5008k_usart_get_cmd,"loopstop")==0)
		{
			printf("[[loopstop]] \r\n");	
			loopstopflag=1;
		}
		else if(strcmp(pi5008k_usart_get_cmd,"loopstart")==0)
		{
			printf("[[loopstart]] \r\n");	
			loopstopflag=0;
		}
		else
		{
			printf("unvaild cmd \r\n");	
			PI5008K_RDK_PrintHelp();
		}
		#endif

			UART2_PRINTF("unvaild cmd \r\n");	
			UART1_PRINTF(">>[UART2] unvaild cmd \r\n");	
			PI5008K_RDK_PrintHelp();
			PI5008K_Usart2CmdExistFlagClear();		
	}
}



void USART1_IRQHandler(void)
{	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		if(PI5008K_Usart1Push()==0)// 0 : get "Enter"
		{
			//memcpy(pr1000_usart_get_cmd,pr1000_usart_get_buffer,PR1000_USART_BUFFER_SIZE);
			//printf("\r\n cmd : %s \r\n ",pr1000_usart_get_cmd);			
			//printf("\r\n arg1_div : %d \r\n ",pr1000_usart_get_cmd_arg_div_point[0]);
			//printf("\r\n arg2_div : %d \r\n ",pr1000_usart_get_cmd_arg_div_point[1]);
			//printf("\r\n arg3_div : %d \r\n ",pr1000_usart_get_cmd_arg_div_point[2]);

			//printf("\r\n arg0 : 0x%2x \r\n ",PR1000K_UsartCmdArgHexGet(pr1000_usart_get_cmd_arg[0]));
			//printf("\r\n arg1 : 0x%2x \r\n ",PR1000K_UsartCmdArgHexGet(pr1000_usart_get_cmd_arg[1]));
			//printf("\r\n arg2 : 0x%2x \r\n ",PR1000K_UsartCmdArgHexGet(pr1000_usart_get_cmd_arg[2]));
			
			memset(pi5008k_usart1_get_cmd_arg_div_point,0x00,10);
			memset(pi5008k_usart1_get_buffer,0x00,PI5008K_USART_BUFFER_SIZE);
			pi5008k_usart1_cmd_exist_flag=1;
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}	
}

void USART2_IRQHandler(void)
{	
	//UART2_PRINTF("\r\n USART2_IRQHandler \r\n "); 		
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		//normal sequence like usart1
		#if 0
			if(PI5008K_Usart2Push()==0)// 0 : get "Enter"
			{
				memset(pi5008k_usart2_get_cmd_arg_div_point,0x00,10);
				memset(pi5008k_usart2_get_buffer,0x00,PI5008K_USART_BUFFER_SIZE);
				pi5008k_usart2_cmd_exist_flag=1;
				UART1_PRINTF("\r\n uart2 cmd : %s \r\n ",pi5008k_usart2_get_cmd);	
			}		
		#else
			//pi5008k's response
			if(PI5008K_Usart2_Get_Response()==0)//get '\n' LF code
			{
				UART1_PRINTF("uart2 cmd : %s \r\n ",pi5008k_usart2_get_cmd);	
				memset(pi5008k_usart2_get_cmd,0x00,PI5008K_USART_BUFFER_SIZE);			
				memset(pi5008k_usart2_get_buffer,0x00,PI5008K_USART_BUFFER_SIZE);			
				pi5008k_usart2_buffer_cnt =0;
				
			}
			//UART1_PRINTF("test %s",pi5008k_usart2_get_cmd); 
		#endif
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}	
}
uint16_t g_TimerVal_previous = 0;
uint16_t cnt_10us=0;
uint16_t cnt_100us=0;
uint16_t cnt_1ms=0;
uint16_t cnt_1s=0;

void EXTI0_IRQHandler(void)
{

    uint16_t timVal, diffVal;
    
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
	timVal = cnt_10us;

        if(timVal > g_TimerVal_previous)
        {
            diffVal = timVal - g_TimerVal_previous;
        }
        else
        {
            diffVal = (0xFFFF - g_TimerVal_previous) + timVal;
        }

        IrRxQueue_EnQueue(diffVal);
	#if 1		// for debug
       // UART1_PRINTF("IR-RX, TIM-Val: %d, Diff: %d\r\n", timVal, diffVal);
	#endif

        g_TimerVal_previous = timVal;        
        
        EXTI_ClearITPendingBit(EXTI_Line0);

        

    }  
}
void EXTI2_IRQHandler(void)
{

    uint16_t timVal, diffVal;
    
    if(EXTI_GetITStatus(EXTI_Line2) != RESET) {

        UART1_PRINTF("TEST 2\r\n");

        
        EXTI_ClearITPendingBit(EXTI_Line2);
    }  
}
void EXTI4_IRQHandler(void)
{

    uint16_t timVal, diffVal;
    
    if(EXTI_GetITStatus(EXTI_Line4) != RESET) {

        UART1_PRINTF("TEST 4\r\n");

        
        EXTI_ClearITPendingBit(EXTI_Line4);
    }  
}

uint16_t tick=0;
uint16_t toggle_led_1s=0;

void PI5008K_LED_TOGGLE(void)
{
	// 1 sec led toggle
	if ((tick%100000)==0)
	{
		toggle_led_1s++;
		if(toggle_led_1s%2==0)
		{
			GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET); //Led1 off
			GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET); //Led1 off
			toggle_led_1s=0;
		}
		else
		{
			GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET); //Led1 on
			GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET); //Led1 on
	
		}
	}


}




void TIM4_IRQHandler(void) //10us timer
{ 
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) 
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//		TimeService_10ms();
//		Check_Timer();
		tick++;
		cnt_10us++;
		//if((cnt_10us%10)==0) cnt_100us++;
		//if((cnt_100us%10)==0) cnt_1ms++;
		if((cnt_1ms%1000)==0) cnt_1s++;

		PI5008K_LED_TOGGLE();



		/* To distinguish between longkey and normal keystrokes.*/
		if(g_old_KeyVal)
		{
			 if(g_repeat_cnt!=g_repeat_cnt_prv)
			{
				g_input_key_flag=0;
				g_start_key_tim=0;
				g_repeat_cnt_prv=g_repeat_cnt;
				//UART1_PRINTF("refresh g_repeat_cnt %d\r\n", g_repeat_cnt);
			}

			if(g_start_key_tim>=IR_KEY_REPEAT_TERM)
			{
				if(g_repeat_cnt>=10)
				{
					g_long_key_flag=1;

					//g_old_KeyVal = 0;
					g_start_key_tim = 0;
					g_input_key_flag=0;
					g_repeat_cnt=0;
					g_repeat_cnt_prv=0;
				}
				else
				{
					UART1_PRINTF("refresh g_repeat_cnt %d\r\n", g_repeat_cnt);
					g_input_key_flag=1;

					g_long_key_flag=0;
					//g_old_KeyVal = 0;
					g_start_key_tim = 0;
					g_repeat_cnt=0;
					g_repeat_cnt_prv=0;

					

				}
			}
			else
			{
				g_start_key_tim++;
			}

		}


	}
}

unsigned int jogkey_cnt;
unsigned int jogkey_pre_cnt;
unsigned int jogkey_cur_val;
int PI5008K_5Dir_Key_Read(void)
{
	
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14)==0x00)
	{
		jogkey_cur_val=1;
		jogkey_cnt++;
		jogkey_pre_cnt=jogkey_cnt;
	}
	else if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==0x00)
	{	
		jogkey_cur_val=2;
		jogkey_cnt++;
		jogkey_pre_cnt=jogkey_cnt;
	}
	else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)==0x00)
	{
		jogkey_cur_val=3;
		jogkey_cnt++;
		jogkey_pre_cnt=jogkey_cnt;

	}
	else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)==0x00)
	{
		jogkey_cur_val=4;
		jogkey_cnt++;
		jogkey_pre_cnt=jogkey_cnt;

	}
	else if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0x00)
	{
		jogkey_cur_val=5;
		jogkey_cnt++;
		jogkey_pre_cnt=jogkey_cnt;

	}
	else
	{
		if(jogkey_cur_val)
		{
			if(jogkey_cnt == jogkey_pre_cnt)
			{
				if(jogkey_cur_val==1)
				{
					UART1_PRINTF("LEFT\r\n");
					PI5008K_Uart_Con_Remote_Cmd(REM_DIR_LEFT);
				}
				else if(jogkey_cur_val==2)
				{
					UART1_PRINTF("RIGHT\r\n");
					PI5008K_Uart_Con_Remote_Cmd(REM_DIR_RIGHT);
				}
				else if(jogkey_cur_val==3)
				{
					UART1_PRINTF("UP\r\n");
					PI5008K_Uart_Con_Remote_Cmd(REM_DIR_UP);
				}
				else if(jogkey_cur_val==4)
				{
					UART1_PRINTF("DOWN\r\n");
					PI5008K_Uart_Con_Remote_Cmd(REM_DIR_DOWN);
				}
				else if(jogkey_cur_val==5)
				{
					if(jogkey_cnt>=100000)
					{
						UART1_PRINTF("LONG CENTER\r\n");
						PI5008K_Uart_Con_Remote_Cmd(REM_MENU);

					}
					else
					{
						UART1_PRINTF("CENTER\r\n");
						PI5008K_Uart_Con_Remote_Cmd(REM_DIR_CENTER);

					}
				}
		
				jogkey_cnt=0;
				jogkey_pre_cnt=0;
				jogkey_cur_val=0;
			}
		}

	}






	

}

//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <type.h>
#include "pi5008k_remocon.h"
#include "pi5008k_func.h"


/*global values*/	
//key
unsigned int g_old_KeyVal = 0;
unsigned int g_start_key_tim = 0;
unsigned int g_long_key_flag=0;
unsigned int g_input_key_flag=0;
unsigned int g_repeat_cnt=0;
unsigned int g_repeat_cnt_prv=0;

//led
unsigned int toggle_led=0;


static void ir_rx_queue_increase_point_value(unsigned int * data_p)
{
	(* data_p) ++;
	if(IR_RX_QUEUE_BUF_SIZE == (* data_p))
	{
		(* data_p) = 0;
	}
}

bool IrRxQueue_Is_Empty(void)
{
	if(ir_rx_queue_point_head == ir_rx_queue_point_tail)
	{
		return TRUE;
	}
	return FALSE;
}

void IrRxQueue_EnQueue(unsigned short data)
{
	ir_rx_queue_buf[ir_rx_queue_point_head] = data;
	ir_rx_queue_increase_point_value(&ir_rx_queue_point_head);
}

unsigned short IrRxQueue_DeQueue(void)
{
	unsigned short retVal = ir_rx_queue_buf[ir_rx_queue_point_tail];
	ir_rx_queue_increase_point_value(&ir_rx_queue_point_tail);
	return retVal;
}

static bool check_time_is_start(unsigned short diffVal)
{
    if((diffVal > (IR_RX_TIME_VAL_START - IR_RX_TIME_VAL_GUARD))
        && (diffVal < (IR_RX_TIME_VAL_START + IR_RX_TIME_VAL_GUARD)))
    {
        return TRUE;
    }
    else
    {
		//UART1_PRINTF("diffVal = %d\n",diffVal);
        return FALSE;
    }
}
static bool check_time_is_repeat(unsigned short diffVal)
{
    if((diffVal > (IR_RX_TIME_VAL_REPEAT_START - IR_RX_TIME_VAL_GUARD))
        && (diffVal < (IR_RX_TIME_VAL_REPEAT_START + IR_RX_TIME_VAL_GUARD)))
    {
        return TRUE;
    }
    else
    {
		//UART1_PRINTF("diffVal = %d\n",diffVal);
        return FALSE;
    }
}




static unsigned int check_time_value(unsigned short diffVal)
{
    if((diffVal > (IR_RX_TIME_VAL_0 - IR_RX_TIME_VAL_GUARD))
        && (diffVal < (IR_RX_TIME_VAL_0 + IR_RX_TIME_VAL_GUARD)))
    {
        return 0;
    }
    else if((diffVal > (IR_RX_TIME_VAL_1 - IR_RX_TIME_VAL_GUARD))
            && (diffVal < (IR_RX_TIME_VAL_1 + IR_RX_TIME_VAL_GUARD)))
    {
        return 1;
    }
    else
    {
        return IR_RX_ABNORMAL_KEY;
    }
}
static unsigned int get_current_ir_key_value(void)
{
    unsigned int currKeyVal = 0, i, currBit;
    unsigned short diffVal;

    for(i = 0; i < IR_RX_1KEY_COUNT; i ++)
    {
        U10secDelay(300);

        if(TRUE == IrRxQueue_Is_Empty())
        {
            UART1_PRINTF("[ERROR] Queue empty, index: %d\n", i);
            return IR_RX_ABNORMAL_KEY;
        }

        diffVal = IrRxQueue_DeQueue();
        currBit = check_time_value(diffVal);
        //UART1_PRINTF("IR-RX, Diff: %d\n", diffVal);

        if(IR_RX_ABNORMAL_KEY == currBit)
        {
            // UART1_PRINTF("[ERROR] currBit: %d\n", currBit);        
            return IR_RX_ABNORMAL_KEY;
        }
        
        currKeyVal = currKeyVal | (currBit << (IR_RX_1KEY_COUNT - 1 - i));
        
    }

    //UART1_PRINTF("currKeyVal: 0x%08X\n", currKeyVal);

    if(currKeyVal==IR_KEY_DIR_CENTER || currKeyVal==TG_ENTER)
    {
    	currKeyVal = KEY_CENTER;
    }

    return currKeyVal;
}
void receive_toggle_led(void)
{
	unsigned char i=0;
	
	for(i=0;i<5;i++)
	{
		MDelay(50);
		toggle_led++;
		if(toggle_led%2==0)
		{
			GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET); //Led1 off
			toggle_led=0;
		}
		else
		{
			GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET); //Led1 on
		
		}

	}
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET); //Led1 off

}


static void send_current_key_value(unsigned int currKeyVal)
{
    switch(currKeyVal)
    {
	case IR_KEY_TV_POWER:
	{	
		IR_KEY_PRINT(IR_KEY_TV_POWER);
	} break;
 	case IR_KEY_DIR_UP:
 	{
		IR_KEY_PRINT(IR_KEY_DIR_UP);
		PI5008K_Uart_Con_Remote_Cmd(REM_DIR_UP);

 	}break;
 	case IR_KEY_DIR_DOWN:
 	{
		IR_KEY_PRINT(IR_KEY_DIR_DOWN);
		PI5008K_Uart_Con_Remote_Cmd(REM_DIR_DOWN);

 	}break;
 	case IR_KEY_DIR_LEFT:
 	{
		IR_KEY_PRINT(IR_KEY_DIR_LEFT);
		PI5008K_Uart_Con_Remote_Cmd(REM_DIR_LEFT);

 	}break; 	
 	case IR_KEY_DIR_RIGHT:
 	{
		IR_KEY_PRINT(IR_KEY_DIR_RIGHT);
		PI5008K_Uart_Con_Remote_Cmd(REM_DIR_RIGHT);

 	}break; 	
 	case IR_KEY_DIR_CENTER:
 	{
		if(g_long_key_flag)
		{
			IR_KEY_PRINT(IR_KEY_DIR_CENTER);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_CENTER_LONG);
		}
		else
		{
			IR_KEY_PRINT(IR_KEY_DIR_CENTER);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_CENTER);
		}

 	}break; 	
 	case IR_KEY_REG_TEST_NUM1:
 	{
		IR_KEY_PRINT(IR_KEY_REG_TEST_NUM1);
		UART1_PRINTF("[KEY_NUM1]test reg sw\n\r");
		PI5008K_Uart_Reg_Single_Write(0xffffffff,0xdeadface);

 	}break; 	
 	case IR_KEY_REG_TEST_NUM2:
 	{
		IR_KEY_PRINT(IR_KEY_REG_TEST_NUM2);
		UART1_PRINTF("[KEY_NUM2]test reg cw\n\r");
		PI5008K_Uart_Reg_Single_Write(0xEEEEEEEE,0xceedcafe);

 	}break; 	




 	//add_tg_remocon
 	
	case TG_LCD_ON_OFF:
	{	
		IR_KEY_PRINT(IR_KEY_TV_POWER);
	} break;
 	case TG_UP:
 	{
		if(g_long_key_flag)
		{
			IR_KEY_PRINT(IR_KEY_DIR_UP);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_UP_LONG);
		}
		else
		{
			IR_KEY_PRINT(IR_KEY_DIR_UP);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_UP);		
		}

 	}break;
 	case TG_DOWN:
 	{
		if(g_long_key_flag)
		{
			IR_KEY_PRINT(IR_KEY_DIR_DOWN);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_DOWN_LONG);
		}
		else
		{
			IR_KEY_PRINT(IR_KEY_DIR_DOWN);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_DOWN);
		}

 	}break;
 	case TG_LEFT:
 	{
 		if(g_long_key_flag)
		{
			IR_KEY_PRINT(IR_KEY_DIR_LEFT);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_LEFT_LONG);
		}
		else
		{
			IR_KEY_PRINT(IR_KEY_DIR_LEFT);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_LEFT);
		}

 	}break; 	
 	case TG_RIGHT:
 	{
 		if(g_long_key_flag)
		{
			IR_KEY_PRINT(IR_KEY_DIR_RIGHT);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_RIGHT_LONG);
		}
		else
		{
			IR_KEY_PRINT(IR_KEY_DIR_RIGHT);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_RIGHT);
		}

 	}break; 	
 	case TG_ENTER:
 	case KEY_CENTER:
 	{
		if(g_long_key_flag)
		{
			IR_KEY_PRINT(IR_KEY_DIR_CENTER);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_CENTER_LONG);
		}
		else
		{
			IR_KEY_PRINT(IR_KEY_DIR_CENTER);
			PI5008K_Uart_Con_Remote_Cmd(REM_DIR_CENTER);
		}

 	}break; 	
 	case TG_NAVI:
 	{
 		if(g_long_key_flag)
		{
			IR_KEY_PRINT(TG_NAVI);
			PI5008K_Uart_Con_Remote_Cmd(REM_NAVI_LONG);
		}
		else
		{
			IR_KEY_PRINT(TG_NAVI);
			PI5008K_Uart_Con_Remote_Cmd(REM_NAVI);
		}
 	
 	}

 	break;
 	case TG_MENU:
 	{
 		if(g_long_key_flag)
		{
			IR_KEY_PRINT(TG_NAVI);
			PI5008K_Uart_Con_Remote_Cmd(REM_MENU_LONG);
		}
		else
		{
			IR_KEY_PRINT(TG_MENU);
			PI5008K_Uart_Con_Remote_Cmd(REM_MENU);
		}
 	
 	}
 	break;

    case IR_KEY_TV_NUM_1:
        IR_KEY_PRINT(IR_KEY_TV_NUM_1);   break;
    case IR_KEY_TV_NUM_2:
        IR_KEY_PRINT(IR_KEY_TV_NUM_2);   break;
    case IR_KEY_TV_NUM_3:
        IR_KEY_PRINT(IR_KEY_TV_NUM_3);   break;
    case IR_KEY_TV_NUM_4:
        IR_KEY_PRINT(IR_KEY_TV_NUM_4);   break;
    case IR_KEY_TV_NUM_5:
        IR_KEY_PRINT(IR_KEY_TV_NUM_5);   break;
    case IR_KEY_TV_NUM_6:
        IR_KEY_PRINT(IR_KEY_TV_NUM_6);   break;
    case IR_KEY_TV_NUM_7:
        IR_KEY_PRINT(IR_KEY_TV_NUM_7);   break;
    case IR_KEY_TV_NUM_8:
        IR_KEY_PRINT(IR_KEY_TV_NUM_8);   break;
    case IR_KEY_TV_NUM_9:
        IR_KEY_PRINT(IR_KEY_TV_NUM_9);   break;
    case IR_KEY_TV_NUM_0:
        IR_KEY_PRINT(IR_KEY_TV_NUM_0);   break;
    
    case IR_KEY_TV_MEM_DEL:
        IR_KEY_PRINT(IR_KEY_TV_MEM_DEL);  break;
    case IR_KEY_TV_SLEEP_RESERV:
        IR_KEY_PRINT(IR_KEY_TV_SLEEP_RESERV);  break;
    case IR_KEY_TV_CONFIG_MEMORY:
        IR_KEY_PRINT(IR_KEY_TV_CONFIG_MEMORY);  break;
    case IR_KEY_TV_AUTO_CHANNEL:
        IR_KEY_PRINT(IR_KEY_TV_AUTO_CHANNEL);  break;
    
    case IR_KEY_TV_VOL_PLUS:
        IR_KEY_PRINT(IR_KEY_TV_VOL_PLUS);  break;
    case IR_KEY_TV_VOL_MINUS:
        IR_KEY_PRINT(IR_KEY_TV_VOL_MINUS);  break;
    case IR_KEY_TV_CHANNEL_UP:
        IR_KEY_PRINT(IR_KEY_TV_CHANNEL_UP);  break;
    case IR_KEY_TV_CHANNEL_DOWN:
        IR_KEY_PRINT(IR_KEY_TV_CHANNEL_DOWN);  break;
    case IR_KEY_TV_SELECT_LIST:
        IR_KEY_PRINT(IR_KEY_TV_SELECT_LIST);  break;

    case IR_KEY_TV_MUTE:
        IR_KEY_PRINT(IR_KEY_TV_MUTE);  break;
    case IR_KEY_TV_CONTROL:
        IR_KEY_PRINT(IR_KEY_TV_CONTROL);  break;
    case IR_KEY_TV_TV_VIDEO:
        IR_KEY_PRINT(IR_KEY_TV_TV_VIDEO);  break;
    case IR_KEY_TV_TV_CABLE:
        IR_KEY_PRINT(IR_KEY_TV_TV_CABLE);  break;

    case IR_KEY_VIDEO_POWER:
        IR_KEY_PRINT(IR_KEY_VIDEO_POWER);  break;
    case IR_KEY_VIDEO_TV_VCR:
        IR_KEY_PRINT(IR_KEY_VIDEO_TV_VCR);  break;
    
    case IR_KEY_VIDEO_REWIND:
        IR_KEY_PRINT(IR_KEY_VIDEO_REWIND);  break;
    case IR_KEY_VIDEO_PLAY:
        IR_KEY_PRINT(IR_KEY_VIDEO_PLAY);  break;
    case IR_KEY_VIDEO_FASTFORWARD:
        IR_KEY_PRINT(IR_KEY_VIDEO_FASTFORWARD);  break;

    case IR_KEY_VIDEO_VOL_UP:
        IR_KEY_PRINT(IR_KEY_VIDEO_VOL_UP);  break;
    case IR_KEY_VIDEO_VOL_DOWN:
        IR_KEY_PRINT(IR_KEY_VIDEO_VOL_DOWN);  break;
    case IR_KEY_VIDEO_REC_STOP:
        IR_KEY_PRINT(IR_KEY_VIDEO_REC_STOP);  break;
    case IR_KEY_VIDEO_REC_PAUSE:
        IR_KEY_PRINT(IR_KEY_VIDEO_REC_PAUSE);  break;
    case IR_KEY_VIDEO_CH_UP:
        IR_KEY_PRINT(IR_KEY_VIDEO_CH_UP);  break;
    case IR_KEY_VIDEO_CH_DOWN:
        IR_KEY_PRINT(IR_KEY_VIDEO_CH_DOWN);  break;

    default:
		UART1_PRINTF("value = 0x%08x\n\r",currKeyVal);
        break;
    }
	//receive_toggle_led();
}


void PI5008K_remocon_test(void)
{
	static unsigned int currKeyVal;
	unsigned short diffVal;
	if(FALSE == IrRxQueue_Is_Empty())    
	{        
		diffVal = IrRxQueue_DeQueue();    
		if(check_time_is_start(diffVal))
		{
			U10secDelay(10);
			//UART1_PRINTF("check_time_is_start   diffVal = %d\\n",diffVal);
			currKeyVal = get_current_ir_key_value();    
			send_current_key_value(currKeyVal);             
			return;
		}
		else if (check_time_is_repeat(diffVal))
		{
			send_current_key_value(currKeyVal);    
			UART1_PRINTF("repeat\n");
			return;
		}
		else
		{
			//UART1_PRINTF("[error] diffVal = %d\\n",diffVal);
			return;
		}        
	}
}


void PI5008K_remocon_test_longkey(void)
{
	unsigned int currKeyVal;
	unsigned short diffVal;

	if(FALSE == IrRxQueue_Is_Empty())    
	{        
		diffVal = IrRxQueue_DeQueue();    
		if(check_time_is_start(diffVal))
		{
			U10secDelay(10);
			//UART1_PRINTF("check_time_is_start   diffVal = %d\\n",diffVal);
			currKeyVal = get_current_ir_key_value();
			g_old_KeyVal = currKeyVal;
			g_repeat_cnt = 1;
			return;
		}
		else if (check_time_is_repeat(diffVal))
		{
			//UART1_PRINTF("repeat\n");
			//UART1_PRINTF("g_repeat_cnt = 0x%x\r\n",g_repeat_cnt);			
			//UART1_PRINTF("g_repeat_cnt = 0x%x\r\n",g_repeat_cnt_prv);
			g_repeat_cnt++;
			return;
		}
	}
	if(g_long_key_flag==1 || g_input_key_flag==1)
	{
		send_current_key_value(g_old_KeyVal);
		g_start_key_tim = 0;
		g_long_key_flag=0;
		g_input_key_flag=0;
		g_repeat_cnt=0;
		g_repeat_cnt_prv=0;			
		g_old_KeyVal=0;
		return;
	}
}

void PI5008K_remocon_test_modif(void)
{
	static unsigned int currKeyVal;
	unsigned short diffVal;
	if(FALSE == IrRxQueue_Is_Empty())    
	{        
		diffVal = IrRxQueue_DeQueue();    
		if(check_time_is_start(diffVal))
		{
			U10secDelay(10);
			//UART1_PRINTF("check_time_is_start   diffVal = %d\\n",diffVal);
			currKeyVal = get_current_ir_key_value();    
			send_current_key_value(currKeyVal);             
			return;
		}
		else if (check_time_is_repeat(diffVal))
		{
			send_current_key_value(currKeyVal);    
			UART1_PRINTF("repeat\n");
			return;
		}
		else
		{
			UART1_PRINTF("[error] diffVal = %d\\n",diffVal);
			return;
		}        
	}
}



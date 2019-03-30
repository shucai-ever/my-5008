/*
 * (C) COPYRIGHT 2009 CRZ
 *
 * File Name : main.c
 * 
 * History 
 *
 * when         who       what, where, why
 *--------------------------------------------------------------
 * 08/12/2009   POOH      Initial release
 * 09/29/2011   sikim     Modify for 3 axis sensor
 */

/* includes */

#include "hw_config.h"

/* global variables */

RCC_ClocksTypeDef  rcc_clocks;

bool g_TestProcessState = FALSE;

/* functions */

void System_Information()
{
    printf("SYSCLK_Frequency = %d\n",rcc_clocks.SYSCLK_Frequency );
    printf("HCLK_Frequency = %d\n",rcc_clocks.HCLK_Frequency );
    printf("PCLK1_Frequency = %d\n",rcc_clocks.PCLK1_Frequency );
    printf("PCLK2_Frequency = %d\n",rcc_clocks.PCLK2_Frequency );
    printf("ADCCLK_Frequency = %d\n",rcc_clocks.ADCCLK_Frequency );
}

void USB_Test_Start (void)
{
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
}

/*
 * Name   : main
 * Input  : None
 * Output : None
 * Return : None
 */
int IR_Sample_main(void)
{
    uint8_t ch;

    /* System Clocks Configuration */
    RCC_Configuration();
    
    RCC_GetClocksFreq(&rcc_clocks);

    /* NVIC configuration */
    NVIC_Configuration();

    /* Board Initialize */
    USART1_Init();        
    USB_Initialize();    
    LED_Initialize(); 
    Seven_Segment_Initilaze();
    KEY_Initialize();
    board_detect_initialzie();
    remocon_initialzie();
    
    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(rcc_clocks.SYSCLK_Frequency / 1000))
    { 
        /* Capture error */ 
        while (1);
    }

    USB_Cable_Config(DISABLE);        
    
    Delay(500);

    while(1)
    {
        printf("\n---------------------\n");
        printf("Mango M32 test start...\n");
        printf("Press menu key\n");
        printf("---------------------\n");
        printf("0> System Information\n");
        printf("---------------------\n");
        printf("1> LED Test\n");
        printf("2> KEY Test\n");
        printf("3> 7-Segment Test\n");
        printf("4> USB HID Test\n");
        printf("5> Auto Sensor Board Detect\n"); 
        printf("6> Remocon Test\n");        
        printf("7> \n");                
        printf("---------------------\n");
        printf("x> quit\n\n");

        ch = USART_GetCharacter(USART1);
        printf(" is selected\n\n");

        switch((char)ch)
        {
        case '0':
            System_Information();
            break;

        case '1':
            LED_Test();
            break;

        case '2':
            KEY_Test();
            break;

        case '3':
            Seven_Segment_Test();
            break;

        case '4':
            g_TestProcessState = TRUE;

            /* USB initialization */
            USB_Test_Start();
            Delay(500);
            USB_Cable_Config(ENABLE);
            break;

        case '5':
            board_detect();
            break;
            
        case '6':
            while(1)
            {
                remocon_test();
            }
            break;

        case '7':
            break;            
        }

        if('x' == (char)ch)
        {
            break;
        }
    }

    return 0;
}

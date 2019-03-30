/*
 * (C) COPYRIGHT 2011 CRZ
 *
 * File Name : board_detect.c
 * 
 * History 
 *
 * when         who       what, where, why
 *--------------------------------------------------------------
 * 09/29/2011   sikim     Initial release
 */

/* includes */

#include "hw_config.h"

/* Local Defines */
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define BOARD_TYPE_MAX      31

/* Local Variables */

/*
    12 bit adc resolution is 4096
    3.3V / 4096 = 0.0008056640625 V

    J Class resistance 9000 ohm -> 9450 ~ 8550
    J Class resistance 1000 ohm -> 950  ~ 1050 

    3.3V X ( 1050 / 10500 ) = 0.33V
    3.3V X ( 950  / 10400 ) = 0.301V
    3.3V X ( 1050 / 9600  ) = 0.3609V
    3.3V X ( 950  / 9500  ) = 0.33V

    Max Error -0.03V ~ + 0.03V

    Max Sensor Board is 30 Class

    0   V	device 1    0     ~ 0.03V
    0.11V	device 2    0.08V ~ 0.14V
    0.22V	device 3    0.19V ~ 0.25V
     .	 .
     .	 .
     .	 .
     
*/
static float BoardClass[BOARD_TYPE_MAX][3];

/* Local Global Variables */
__IO uint16_t BoardDetectValue;
char *BoardType[BOARD_TYPE_MAX] = { 
                                       "TEST", // 0    0 V
                                        "TEST",      // 1    0.11V
                                        "TEST",      // 2    0.22V
                                        "3-Axis ACC Sensor Board with Temp",     // 3    0.33 V
                                        "TEST",     // 4                                        
                                        "TEST",     // 5                                        
                                        "Pressure Sensor Board with Temp",     // 6     0.66 V
                                        "TEST",     // 7                                       
                                        "Light Sensor Board",     // 8        0.88 V
                                        "TEST",     // 9                                        
                                        "CAN Board",     // 10
                                        "TEST",     // 11
                                        "Remocon Sensor Board",     // 12
                                        "TEST",     // 13    
                                        "TEST",     // 14
                                        "TEST",     // 15                                        
                                        "TEST",     // 16                                        
                                        "TEST",     // 17
                                        "TEST",     // 18                                       
                                        "TEST",     // 19                                        
                                        "TEST",     // 20                                        
                                        "TEST",     // 21                                       
                                        "TEST",     // 22
                                        "TEST",     // 23 
                                        "TEST",     // 24    
                                        "TEST",     // 25
                                        "TEST",     // 26                                        
                                        "TEST",     // 27                                        
                                        "TEST",     // 28
                                        "TEST",     // 29                                       
                                        "TEST"      // 30                                        
                                        };

/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
void board_detect(void)
{
    uint32_t nLoop;
    float volt=0.0;

    /* Initialize Board Class */
    for( nLoop = 0; nLoop < BOARD_TYPE_MAX; nLoop++)
    {
        BoardClass[nLoop][0] = 0 + (0.11*nLoop);
        BoardClass[nLoop][1] = BoardClass[nLoop][0] - 0.03;        
        BoardClass[nLoop][2] = BoardClass[nLoop][0] + 0.03;        
    }
    BoardClass[0][1] = 0.0;        
    BoardClass[BOARD_TYPE_MAX-1][2] = (float)3.3;        

    /* Convert AD value to Volt */
    volt = (float)(BoardDetectValue * 3.3)/(float) 4095;
#if 0
    printf("volt [ %f ]\n", volt);    
#endif

    for( nLoop = 0; nLoop < BOARD_TYPE_MAX; nLoop++)
    {
        if( BoardClass[nLoop][1] < volt )
            if( volt < BoardClass[nLoop][2] )
            {
                printf("Board Type : %s\n", BoardType[nLoop]);
                break;
            }
    }
}


/*
 * Description : none.
 * Argument(s) : none.
 * Return(s)   : none.
 * Note(s)     : none.
 */
void board_detect_initialzie(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;  
    DMA_InitTypeDef DMA_InitStructure;    
    ADC_InitTypeDef ADC_InitStructure;    

    /******************** Clock Enable ********************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        
    /******************** GPIO configure ********************/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);          

    /******************** DMA configure ********************/        
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&BoardDetectValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    /******************** Fuction configure ********************/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channel14 configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_55Cycles5);    
    
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);    

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);    

    /* Enable ADC1 reset calibaration register */   
    ADC_ResetCalibration(ADC1);
    
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));
     
    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);    
}




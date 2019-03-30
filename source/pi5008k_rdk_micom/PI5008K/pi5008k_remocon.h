#ifndef __PI5008K_REMOTE_H__
#define __PI5008K_REMOTE_H__

/*
 * (C) COPYRIGHT 2011 CRZ
 *
 * File Name : remocon.c
 * 
 * History 
 *
 * when         who       what, where, why
 *--------------------------------------------------------------
 * 04/04/2012   sikim     Initial release
 */

/* includes */
#include <type.h>
//#include "hw_config.h"

/* Defines */
// Queue for IR Rx
#define  IR_RX_QUEUE_BUF_SIZE  200
uint16_t  ir_rx_queue_buf[IR_RX_QUEUE_BUF_SIZE];
uint32_t  ir_rx_queue_point_head = 0;
uint32_t  ir_rx_queue_point_tail = 0;

#define IR_RX_TIME_VAL_START (1350) //10us
#define IR_RX_TIME_VAL_0     (113)
#define IR_RX_TIME_VAL_1     (IR_RX_TIME_VAL_0*2)

#define IR_RX_TIME_VAL_REPEAT_START (1125) //10us


#define IR_RX_TIME_VAL_GUARD 20

#define IR_RX_1KEY_COUNT     32
#define IR_RX_ABNORMAL_KEY   0xFFFF

// _SEND_VAL define generation macro

#define IR_KEY_GEN(def_data) def_data##_SEND_VAL
#define IR_KEY_PRINT(def_data) UART1_PRINTF("%s\n", #def_data)

// IR KEY Received Value

#define IR_KEY_TV_POWER  0x40be629d

#define IR_KEY_TV_NUM_1  0xE0E020DF
#define IR_KEY_TV_NUM_2  0xE0E0A05F
#define IR_KEY_TV_NUM_3  0xE0E0609F
#define IR_KEY_TV_NUM_4  0xE0E010EF
#define IR_KEY_TV_NUM_5  0xE0E0906F
#define IR_KEY_TV_NUM_6  0xE0E050AF
#define IR_KEY_TV_NUM_7  0xE0E030CF
#define IR_KEY_TV_NUM_8  0xE0E0B04F
#define IR_KEY_TV_NUM_9  0xE0E0708F
#define IR_KEY_TV_NUM_0  0xE0E08877

#define IR_KEY_TV_MEM_DEL       0xE0E018E7
#define IR_KEY_TV_SLEEP_RESERV  0xE0E0C03F
#define IR_KEY_TV_CONFIG_MEMORY 0xE0E09867
#define IR_KEY_TV_AUTO_CHANNEL  0xE0E05CA3

#define IR_KEY_TV_VOL_PLUS      0xE0E0E01F
#define IR_KEY_TV_VOL_MINUS     0xE0E0D02F
#define IR_KEY_TV_CHANNEL_UP    0xE0E048B7
#define IR_KEY_TV_CHANNEL_DOWN  0xE0E008F7
#define IR_KEY_TV_SELECT_LIST   0xE0E058A7

#define IR_KEY_TV_MUTE     0xE0E0F00F
#define IR_KEY_TV_CONTROL  0xE0E0B847

#define IR_KEY_TV_TV_VIDEO 0xE0E0807F
#define IR_KEY_TV_TV_CABLE 0xE0E0E41B

#define IR_KEY_VIDEO_POWER   0x040440BF
#define IR_KEY_VIDEO_TV_VCR  0x0404807F

#define IR_KEY_VIDEO_REWIND       0x040418E7
#define IR_KEY_VIDEO_PLAY         0x04049867
#define IR_KEY_VIDEO_FASTFORWARD  0x040458A7

#define IR_KEY_VIDEO_VOL_UP       0x0404E01F
#define IR_KEY_VIDEO_VOL_DOWN     0x0404D02F
#define IR_KEY_VIDEO_REC_STOP     0x0404A857
#define IR_KEY_VIDEO_REC_PAUSE    0x04046897
#define IR_KEY_VIDEO_CH_UP        0x040448B7
#define IR_KEY_VIDEO_CH_DOWN      0x040408F7







#define IR_KEY_DIR_UP			0x40be12ed
#define IR_KEY_DIR_DOWN		0x40beb24d
#define IR_KEY_DIR_LEFT			0x40be728d
#define IR_KEY_DIR_RIGHT			0x40be926d
#define IR_KEY_DIR_CENTER		0x40be52ad


//add smoh tg remocon
#define TG_UP       0x0877d02f
#define TG_DOWN     0x0877b04f
#define TG_ENTER     0x087730cf
#define TG_LEFT    0x087728d7
#define TG_RIGHT       0x087720df
#define TG_LCD_ON_OFF     0x0877b24d
#define TG_NAVI     0x0877b847
#define TG_MENU     0x087718e7

#define KEY_CENTER 0xCE

#define IR_KEY_REG_TEST_NUM1		0x40be807f
#define IR_KEY_REG_TEST_NUM2		0x40be40bf


extern void IrRxQueue_EnQueue(uint16_t data);

bool IrRxQueue_Is_Empty(void);

void IrRxQueue_EnQueue(uint16_t data);
uint16_t IrRxQueue_DeQueue(void);
void remocon_temp(void);
void PI5008K_remocon_test(void);
void PI5008K_remocon_test_longkey(void);
void remocon_initialzie(void);
#endif /* __PI5008K_REMOTE_H__ */


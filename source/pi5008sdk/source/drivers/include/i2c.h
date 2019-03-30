/* i2c.h */
#ifndef __I2C_H__
#define __I2C_H__

#include "system.h"
#include "type.h"
#include "error.h"

#define MAX_I2C_DEV_NUM			2

// Interrupt Enable Register(RW)
#define INT_CMPL			((unsigned int)1<<9)
#define INT_BYTE_RECV		((unsigned int)1<<8)
#define INT_BYTE_TRANS		((unsigned int)1<<7)
#define INT_START			((unsigned int)1<<6)
#define INT_STOP			((unsigned int)1<<5)
#define INT_ARB_LOSE		((unsigned int)1<<4)
#define INT_ADDR_HIT		((unsigned int)1<<3)
#define INT_FIFO_HALF		((unsigned int)1<<2)
#define INT_FIFO_FULL		((unsigned int)1<<1)
#define INT_FIFO_EMPTY		((unsigned int)1<<0)

// Status Register //
#define STAT_LINE_SDA		((unsigned int)1<<14)
#define STAT_LINE_SCL		((unsigned int)1<<13)
#define STAT_GEN_CALL		((unsigned int)1<<12)
#define STAT_BUSY			((unsigned int)1<<11)
#define STAT_ACK			((unsigned int)1<<10)
#define STAT_CMPL			((unsigned int)1<<9)
#define STAT_BYTE_RCV		((unsigned int)1<<8)
#define STAT_BYTE_TRANS		((unsigned int)1<<7)
#define STAT_START			((unsigned int)1<<6)
#define STAT_STOP			((unsigned int)1<<5)
#define STAT_ARB_LOSE		((unsigned int)1<<4)
#define STAT_ADDR_HIT		((unsigned int)1<<3)
#define STAT_FIFO_HALF		((unsigned int)1<<2)
#define STAT_FIFO_FULL		((unsigned int)1<<1)
#define STAT_FIFO_EMPTY		((unsigned int)1<<0)

// Control Register //
#define PHASE_START			((unsigned int)1<<12)
#define PHASE_ADDR			((unsigned int)1<<11)
#define PHASE_DATA			((unsigned int)1<<10)
#define PHASE_STOP			((unsigned int)1<<9)
#define PHASE_DIR_MST_TRS	((unsigned int)0<<8)
#define PHASE_DIR_MST_RCV	((unsigned int)1<<8)
#define PHASE_DIR_SLV_TRS	((unsigned int)1<<8)
#define PHASE_DIR_SLV_RCV	((unsigned int)0<<8)

// Command Register //
#define CMD_NO_ACTION		((unsigned int)0x0)
#define CMD_TRANSACTION		((unsigned int)0x1)
#define CMD_RSP_ACK			((unsigned int)0x2)
#define CMD_RSP_NACK		((unsigned int)0x3)
#define CMD_CLEAR			((unsigned int)0x4)
#define CMD_RESET			((unsigned int)0x5)

// Setup Register //
#define DMA_DISABLE			((unsigned int)0<<3)
#define DMA_ENABLE			((unsigned int)1<<3)

#define SLAVE_MODE			((unsigned int)0<<2)
#define MASTER_MODE			((unsigned int)1<<2)

#define ADDRESSING_7BIT		((unsigned int)0<<1)
#define ADDRESSING_10BIT	((unsigned int)1<<1)

#define I2C_DISABLE			((unsigned int)0<<0)
#define I2C_ENABLE			((unsigned int)1<<0)

//-------------------------------------------------------------
typedef enum ppI2C_SPEED_E
{
	eI2C_SPEED_NORMAL = 0,
	eI2C_SPEED_FAST,
} PP_I2C_SPEED_E;

typedef enum ppI2C_MODE_E
{
	eI2C_MODE_MASTER = 0,
	eI2C_MODE_SLAVE,
}PP_I2C_MODE_E;

typedef enum ppI2C_ADDRBIT_E
{
	eI2C_ADDRESS_7BIT = 0,
	eI2C_ADDRESS_10BIT,
}PP_I2C_ADDRBIT_E;


typedef PP_VOID (*I2C_ISR_CALLBACK) (PP_U32 s32Ch, PP_U32 u32Status);
typedef PP_VOID (*I2C_SLAVE_READ) (PP_U32 s32Ch, PP_U8 *pu8Buf, PP_U32 u32Size);
typedef PP_U32 (*I2C_SLAVE_WRITE) (PP_U32 s32Ch, PP_U8 *pu8Buf, PP_U32 u32MaxSize);

PP_VOID PPDRV_I2C_Initialize(PP_VOID);
PP_VOID PPDRV_I2C_Setup(PP_S32 IN s32Ch, PP_U32 IN u32DevAddr, PP_I2C_SPEED_E IN enSpeed, PP_I2C_MODE_E IN enMode, PP_I2C_ADDRBIT_E IN enAddrBit);
PP_VOID PPDRV_I2C_SetISR(PP_S32 IN s32Ch, I2C_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_I2C_Enable(PP_S32 IN s32Ch, PP_U32 IN u32Enable);
PP_RESULT_E PPDRV_I2C_Write(PP_S32 IN s32Ch, PP_U8 IN *pu8Data, PP_U8 IN u8DataSize, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_I2C_Read(PP_S32 IN s32Ch, PP_U8 OUT *pu8Data, PP_U8 IN u8DataSize, PP_U32 IN u32Timeout);
PP_U32 PPDRV_I2C_GetDone(PP_S32 IN s32Ch, PP_RESULT_E OUT *penResult);
PP_VOID PPDRV_I2C_SlaveSetCallback(PP_S32 IN s32Ch, I2C_SLAVE_READ IN cbRecv, I2C_SLAVE_WRITE IN cbSend);
PP_VOID PPDRV_I2C_Reset(PP_S32 IN s32Ch);




#endif // __I2C_H__


#ifndef _GPIO_I2C_H
#define _GPIO_I2C_H

#define PIXELPLUS_PR1000 	0

typedef struct hiI2C_DATA_S
{
	unsigned char dev_addr; 
#ifdef PIXELPLUS_PR1000
	unsigned char reg_page;
#endif /* PIXELPLUS_PR1000 */
	unsigned int reg_addr; 
	unsigned int addr_byte; 
	unsigned int data; 
	unsigned int data_byte;
    
} I2C_DATA_S;

#define GPIO_I2C_READ   0x01
#define GPIO_I2C_WRITE  0x03
typedef unsigned char		byte;

unsigned char gpio_i2c_read(unsigned char devaddress, unsigned char address);
void gpio_i2c_write(unsigned char devaddress, unsigned char address, unsigned char value);
byte siiReadSegmentBlockEDID(byte SlaveAddr, byte Segment, byte Offset, byte *Buffer, byte Length);

#ifdef PIXELPLUS_PR1000
#define I2C_CMD_PR1000_WRITE      0x05
#define I2C_CMD_PR1000_READ       0x07
int PR1000_SetPage(const int fd, unsigned char slave, unsigned char page);
int PR1000_Read(const int fd, unsigned char slave, unsigned char reg, unsigned char *pRet);
int PR1000_Write(const int fd, unsigned char slave, unsigned char reg, unsigned char value);
int PR1000_PageRead(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char *pRet);
int PR1000_PageWrite(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char value);
int PR1000_PageReadBurst(const int fd, unsigned char slave, int page, unsigned char reg, unsigned short length, unsigned char *pRetData);
int PR1000_PageWriteBurst(const int fd, unsigned char slave, int page, unsigned char reg, unsigned short length, const unsigned char *pData);
int PR1000_ReadMaskBit(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char regMaskBit, unsigned char *pRet);
int PR1000_WriteMaskBit(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char regMaskBit, unsigned char value);
#endif /* PIXELPLUS_PR1000 */

#endif


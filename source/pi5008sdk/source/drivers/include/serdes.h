/*
 * serdes.h
 *
 *  Created on: 2017. 5. 17.
 *      Author: ihkong
 */

#ifndef SOURCE_APPLICATIONS_SERDES_H_
#define SOURCE_APPLICATIONS_SERDES_H_

#include "type.h"

//=================================================
//	I2C device address define
//=================================================
#define I2C_DES_0_SLV_ADDR			0x48
#define I2C_DES_1_SLV_ADDR			0x4A
#define I2C_DES_2_SLV_ADDR			0x4C
#define I2C_DES_3_SLV_ADDR			0x4E

#define I2C_SER_DEF_SLV_ADDR		0x40
#define I2C_SER_0_SLV_ADDR			0x50
#define I2C_SER_1_SLV_ADDR			0x54
#define I2C_SER_2_SLV_ADDR			0x58
#define I2C_SER_3_SLV_ADDR			0x5C

#define I2C_SENSOR_DEF_SLV_ADDR		0x36
#define I2C_SENSOR_0_SLV_ADDR		0x60
#define I2C_SENSOR_1_SLV_ADDR		0x64
#define I2C_SENSOR_2_SLV_ADDR		0x68
#define I2C_SENSOR_3_SLV_ADDR		0x6C

typedef enum {
	I2C_DES_0_DEV = 0,
	I2C_DES_1_DEV 	 ,
	I2C_DES_2_DEV	 ,
	I2C_DES_3_DEV	 ,
	I2C_SER_0_DEV    ,
	I2C_SER_1_DEV 	 ,
	I2C_SER_2_DEV	 ,
	I2C_SER_3_DEV	 ,
	I2C_SENSOR_0_DEV ,
	I2C_SENSOR_1_DEV ,
	I2C_SENSOR_2_DEV ,
	I2C_SENSOR_3_DEV ,
	I2C_DUMMY_DEV	 ,
	I2C_SER_DEV		 ,		// 4
	I2C_SENSOR_DEV	 ,		// 4
	I2C_LAST_DEV
} ENUM_I2C_SENSOR_DEV;

extern uint32 i2c_slave_addr[];

void delay (int time);
void init_vstream(void);

#endif /* SOURCE_APPLICATIONS_SERDES_H_ */

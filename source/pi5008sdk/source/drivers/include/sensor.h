/*
 * sensor.h
 *
 *  Created on: 2017. 5. 17.
 *      Author: ihkong
 */
#include "type.h"

#ifndef PI5008_SENSOR_H_
#define PI5008_SENSOR_H_

typedef struct {
	uint16 addr;
	uint8 data;
} SENSOR_INIT;

void init_sensor(sint32 ch);



#endif

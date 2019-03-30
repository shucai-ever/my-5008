/*
 * standby.c
 *
 *  Created on: 2018. 11. 15.
 *      Author: ihkong
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include <ctype.h>

#include "type.h"
#include "error.h"

#include "system.h"
#include "debug.h"
#include "osal.h"
#include "api_flash.h"
#include "gpio.h"
#include "task_manager.h"
#include "timer.h"
#include "cache.h"
#include "standby.h"



PP_VOID InitStandbyMode(PP_VOID)
{

}
PP_RESULT_E EnterStandbyMode(PP_VOID)
{
	return eSUCCESS;
}

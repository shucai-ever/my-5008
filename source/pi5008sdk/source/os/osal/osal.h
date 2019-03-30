/**
 * \file
 *
 * \brief	System API
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */


#ifndef OSAL_H
#define OSAL_H


/*---------------------------------------------------------------------------*/
/* Includes */
/*---------------------------------------------------------------------------*/
//#include "os_cpu.h"
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "error.h"

#include "system.h"
#include "utils.h"

/*---------------------------------------------------------------------------*/
/* Defines */
/*---------------------------------------------------------------------------*/
#define ADDR_CACHEABLE(va) 		( va  & (~(0x3<<30)) )
#define ADDR_NON_CACHEABLE(va) 	( va  | (0x2<<30) )

#define IRQ_EDGE_TRIGGER    1
#define IRQ_LEVEL_TRIGGER   0

#define IRQ_ACTIVE_HIGH     1
#define IRQ_ACTIVE_LOW      0

#ifndef SR_CLRB32
#define SR_CLRB32(reg, bit)     \
{ \
    int mask = __nds32__mfsr(reg)& ~(1<<bit);\
    __nds32__mtsr(mask, reg);    \
    __nds32__dsb();              \
}
#endif

#ifndef SR_SETB32
#define SR_SETB32(reg,bit) \
{ \
    int mask = __nds32__mfsr(reg)|(1<<bit); \
    __nds32__mtsr(mask, reg);           \
    __nds32__dsb();                     \
}
#endif


/* Call by HISR.
 * Since our mask/unmask are not atomic. 
 * And HISR is task level ISR in RTOS, we need make sure it is atomic. 
 *
 * TODO remove gie if atomic 
 */
#define HAL_INTC_IRQ_ATOMIC_DISABLE(_irq_)	do {					\
							unsigned long _gie_;		\
							GIE_SAVE(&_gie_);		\
							SYSAPI_INTC_irq_disable(_irq_);	\
							GIE_RESTORE(_gie_);		\
						}while(0)

#define HAL_INTC_IRQ_ATOMIC_ENABLE(_irq_)	do{					\
							unsigned long _gie_;		\
							GIE_SAVE(&_gie_);		\
							SYSAPI_INTC_irq_enable(_irq_);	\
							GIE_RESTORE(_gie_);		\
						}while(0)


#define SYS_OS_DISABLE_INTERRUPTS       (0)
#define SYS_OS_ENABLE_INTERRUPTS        (1)

#define SYS_OS_NULL                     (0)
#define SYS_OS_TRUE                     (1)
#define SYS_OS_FALSE                    (0)

#define SYS_OS_SUSPEND                  (-1)
#define SYS_OS_DEL_NO_PEND              (0)
#define SYS_OS_DEL_ALWAYS               (1)

#define SYS_OS_TIMER_DEV                (eTIMER_DEV_0)
#define SYS_OS_TIMER_CH                 (eTIMER_CH_0)

typedef struct sys_os_semaphore {

    void    *obj;

} sys_os_semaphore_t;

typedef struct sys_os_mutex {

    void    *obj;

} sys_os_mutex_t;

typedef struct sys_os_mbox {

    void    *obj;

} sys_os_mbox_t;

typedef struct sys_os_queue {

    void    *obj;
    void    **start;
    void    *msg;
    uint32  size;
    uint32  opt;
    uint32  timeout;
    void    *event;
    sint32  err;

} sys_os_queue_t;

typedef struct sys_os_message {

    void    *obj;

} sys_os_message_t;

typedef struct sys_os_thread {

    void        (*fn)(void *);
    void        *pvParameters;
    sint8       prio;
    uint32      stack_size; /* in bytes */
    char        *name;
    void        *phTask;

} sys_os_thread_t;

typedef struct sys_os_bh {

    sys_os_semaphore_t sem;
    sys_os_thread_t    th;

} sys_os_bh_t;

typedef struct sys_os_event {

    void    *obj;

} sys_os_event_t;

typedef isr_t       sys_os_isr_t;

typedef void (*sys_os_idle_task_t) (void* arg);

#define SYS_DMA_MAX_CHANNEL		(8)

typedef void (*SYSDMA_CALLBACK) (uint8 ch, uint32 event);


#define UART_PACKET_SIZE_MAX    (261) /* SOF(1) + Command(1) + Length(1) + Data(max:256) + CRC(2) */

/* SPI , UART packet data */
typedef struct
{
    uint16 length;
    uint8 *buffer;
}UartMsg_t;

/* SPI, UART1(To CAN MCU) communication Message Header */
typedef struct
{
    uint8 sof;
    uint8 command;
    uint8 length;
    uint8 *data;
}SerialHeaderInfo_t;

/*---------------------------------------------------------------------------*/
/* Interfaces */
/*---------------------------------------------------------------------------*/

/********************************
 * UART
 ********************************/
PP_RESULT_E SYSAPI_UART_initialize(uint8 IN ch, uint32 IN baudRate, uint32 IN dataBit, uint32 IN stopBit, uint32 IN parity);
PP_RESULT_E SYSAPI_UART_packetParser(uint8 IN ch, CONST uint8 IN *rxBuffer, uint16 IN rxLength, SerialHeaderInfo_t OUT *headerInfo);

/********************************
 * OS
 ********************************/
PP_RESULT_E OSAL_register_isr(sint32 IN vector, sys_os_isr_t IN isr, sys_os_isr_t* OUT old);
void OSAL_register_idle_task (sys_os_idle_task_t IN idleTask);
void  OSAL_system_panic(uint32 IN err);
uint32 OSAL_global_int_ctl(uint32 IN int_op);
void* OSAL_current(void);
uint32 OSAL_ms2ticks(uint32 IN timeout);
uint32 OSAL_tick2ms(uint32 IN tick);
PP_RESULT_E OSAL_sleep(uint32 IN ms);
PP_RESULT_E OSAL_delay(uint32 IN ms, bool IN isFromISR);
void* OSAL_malloc(uint32 IN size);
void OSAL_free(void* IN ptr);
void* OSAL_get_pid_current(void);
PP_RESULT_E OSAL_create_semaphore(sys_os_semaphore_t* OUT sem, uint32 IN num, CONST void* IN param);
PP_RESULT_E OSAL_destroy_semaphore(sys_os_semaphore_t *sem);
PP_RESULT_E OSAL_pend_semaphore(sys_os_semaphore_t* OUT sem, uint32 IN timeout);
PP_RESULT_E OSAL_post_semaphore(sys_os_semaphore_t* OUT sem);
PP_RESULT_E OSAL_post_semaphore_in_isr(sys_os_semaphore_t* OUT sem);
PP_RESULT_E OSAL_post_queue_from_lisr(sys_os_queue_t* OUT p_queue);
PP_RESULT_E OSAL_create_mutex(sys_os_mutex_t* OUT mutex);
PP_RESULT_E OSAL_destroy_mutex(sys_os_mutex_t* OUT mutex);
PP_RESULT_E OSAL_wait_for_mutex(sys_os_mutex_t* OUT mutex);
PP_RESULT_E OSAL_release_mutex(sys_os_mutex_t* OUT mutex);
PP_RESULT_E OSAL_create_queue(sys_os_queue_t* OUT queue);
PP_RESULT_E OSAL_destroy_queue(sys_os_queue_t* OUT queue);
PP_RESULT_E OSAL_pend_queue(sys_os_queue_t* OUT queue);
PP_RESULT_E OSAL_post_queue(sys_os_queue_t* OUT queue);

EventGroupHandle_t* OSAL_EVENTGROUP_CMD_Create(void);
PP_RESULT_E OSAL_EVENTGROUP_CMD_Destroy(EventGroupHandle_t *phEventGroup);
EventBits_t OSAL_EVENTGROUP_CMD_WaitBits(EventGroupHandle_t *phEventGroup, const EventBits_t eventBits, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits, const int timeOut);
EventBits_t OSAL_EVENTGROUP_CMD_SetBits(EventGroupHandle_t *phEventGroup, const EventBits_t eventBits);
EventBits_t OSAL_EVENTGROUP_CMD_SetBitsFromISR(EventGroupHandle_t *phEventGroup, const EventBits_t eventBits, BaseType_t *p);
QueueHandle_t* OSAL_QUEUE_CMD_Create(const int maxQueueCnt, const uint32 sizeQueue);
PP_RESULT_E OSAL_QUEUE_CMD_Destroy(QueueHandle_t *phQueue);
int OSAL_QUEUE_CMD_GetCount(QueueHandle_t *phQueue);
PP_RESULT_E OSAL_QUEUE_CMD_Receive(QueueHandle_t *phQueue, void *pvParameters, const int timeOut);
PP_RESULT_E OSAL_QUEUE_CMD_Send(QueueHandle_t *phQueue, void *pvParameters, const int timeOut);
PP_RESULT_E OSAL_QUEUE_CMD_ReceiveFromISR(QueueHandle_t *phQueue, void *pvParameters, BaseType_t *p);
PP_RESULT_E OSAL_QUEUE_CMD_SendFromISR(QueueHandle_t *phQueue, void *pvParameters, BaseType_t *p);

PP_RESULT_E OSAL_create_thread(sys_os_thread_t* OUT th);
PP_RESULT_E OSAL_destroy_thread(sys_os_thread_t* OUT th);
PP_RESULT_E OSAL_create_bh(sys_os_bh_t* OUT bh);
PP_RESULT_E OSAL_destroy_bh(sys_os_bh_t* OUT bh);
PP_RESULT_E OSAL_register_bh(sys_os_bh_t* IN bh, void* IN param);
PP_RESULT_E OSAL_raise_bh(sys_os_bh_t* OUT bh);
void OSAL_init_os(void);
void OSAL_start_os(void);
bool OSAL_get_start_os(void);
uint32 GetTickCount (void);
void OSAL_SetHeapConfig(uint8 *pu8HeapBuf, uint32 u32HeapSize);
#endif /* SYS_API_H */
/*---------------------------------------------------------------------------*/
/* End Of File */
/*---------------------------------------------------------------------------*/


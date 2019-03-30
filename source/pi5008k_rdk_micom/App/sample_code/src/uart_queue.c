
/* Includes */

#include "hw_config.h"

/* Defines */

#define  UART_QUEUE_BUFFER_SIZE  200
#define  IR_RX_QUEUE_BUFFER_SIZE  200

/* Variables */

// UART Queue for COM
uint16_t  uart_queue_com_buffer[UART_QUEUE_BUFFER_SIZE];
uint32_t  uart_queue_com_point_head = 0;
uint32_t  uart_queue_com_point_tail = 0;

// UART Queue for Serial
uint16_t  uart_queue_debug_buffer[UART_QUEUE_BUFFER_SIZE];
uint32_t  uart_queue_debug_point_head = 0;
uint32_t  uart_queue_debug_point_tail = 0;

// Queue for IR Rx
uint16_t  ir_rx_queue_buffer[IR_RX_QUEUE_BUFFER_SIZE];
uint32_t  ir_rx_queue_point_head = 0;
uint32_t  ir_rx_queue_point_tail = 0;

/* Static Functions */

static void uart_queue_increase_point_value(uint32_t * data_p)
{
    (* data_p) ++;
    if(UART_QUEUE_BUFFER_SIZE == (* data_p))
    {
        (* data_p) = 0;
    }
}

static void ir_rx_queue_increase_point_value(uint32_t * data_p)
{
    (* data_p) ++;
    if(IR_RX_QUEUE_BUFFER_SIZE == (* data_p))
    {
        (* data_p) = 0;
    }
}

/* Global Functions */

void UartQueue_Initialize(void)
{
    uart_queue_com_point_head = uart_queue_com_point_tail = 0;
    uart_queue_debug_point_head = uart_queue_debug_point_tail = 0;
}

void IrRxQueue_Initialize(void)
{
    ir_rx_queue_point_head = ir_rx_queue_point_tail = 0;
    uart_queue_debug_point_head = uart_queue_debug_point_tail = 0;
}

// UART Queue for COM

bool UartQueue_COM_Is_Empty(void)
{
    if(uart_queue_com_point_head == uart_queue_com_point_tail)
    {
        return TRUE;
    }
    return FALSE;
}

void UartQueue_COM_EnQueue(uint16_t data)
{
    uart_queue_com_buffer[uart_queue_com_point_head] = data;
    uart_queue_increase_point_value(&uart_queue_com_point_head);
}

uint16_t UartQueue_COM_DeQueue(void)
{
    uint16_t retVal = uart_queue_com_buffer[uart_queue_com_point_tail];
    uart_queue_increase_point_value(&uart_queue_com_point_tail);
    return retVal;
}

// UART Queue for Debug

bool UartQueue_Debug_Is_Empty(void)
{
    if(uart_queue_debug_point_head == uart_queue_debug_point_tail)
    {
        return TRUE;
    }
    return FALSE;
}

void UartQueue_Debug_EnQueue(uint16_t data)
{
    uart_queue_debug_buffer[uart_queue_debug_point_head] = data;
    uart_queue_increase_point_value(&uart_queue_debug_point_head);
}

uint16_t UartQueue_Debug_DeQueue(void)
{
    uint16_t retVal = uart_queue_debug_buffer[uart_queue_debug_point_tail];
    uart_queue_increase_point_value(&uart_queue_debug_point_tail);
    return retVal;
}

// IR Rx Queue

bool IrRxQueue_Is_Empty(void)
{
    if(ir_rx_queue_point_head == ir_rx_queue_point_tail)
    {
        return TRUE;
    }
    return FALSE;
}

void IrRxQueue_EnQueue(uint16_t data)
{
    ir_rx_queue_buffer[ir_rx_queue_point_head] = data;
    ir_rx_queue_increase_point_value(&ir_rx_queue_point_head);
}

uint16_t IrRxQueue_DeQueue(void)
{
    uint16_t retVal = ir_rx_queue_buffer[ir_rx_queue_point_tail];
    ir_rx_queue_increase_point_value(&ir_rx_queue_point_tail);
    return retVal;
}


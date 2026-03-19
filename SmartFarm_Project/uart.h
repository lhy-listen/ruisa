#ifndef UART_H
#define UART_H

#include "hal_data.h"

extern uint8_t rx_buf;
extern uint8_t rx_buf_u2; 

void uart7_callback(uart_callback_args_t * p_args);
void uart2_callback(uart_callback_args_t * p_args);

extern volatile uint8_t g_uart7_tx_busy;
extern volatile uint8_t g_uart2_tx_busy;

#endif
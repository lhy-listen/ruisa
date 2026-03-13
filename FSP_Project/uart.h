#ifndef UART_H
#define UART_H

#include "hal_data.h"

extern uint8_t rx_buf;

void uart7_callback(uart_callback_args_t * p_args);
void uart7_wait_for_tx(void);
void uart7_wait_for_rx(void);

#endif
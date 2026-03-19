#ifndef UART2_H_
#define UART2_H_
#include "hal_data.h"
void UART2_Init(void);
void uart2_wait_for_tx(void);
void uart2_wait_for_rx(void);

#endif 
#include "hal_data.h"

static volatile int g_uart7_tx_complete = 0;
static volatile int g_uart7_rx_complete = 0;

uint8_t rx_buf;

void uart7_callback(uart_callback_args_t * p_args)
{
    if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        uint8_t c = rx_buf;   // 从缓冲区取数据

        c++;

        g_uart7.p_api->write(g_uart7.p_ctrl, &c, 1);

        g_uart7.p_api->read(g_uart7.p_ctrl, &rx_buf, 1); // 重新启动接收
    }
}

void uart7_wait_for_tx(void)
{
    while (!g_uart7_tx_complete);
    g_uart7_tx_complete = 0;
}

void uart7_wait_for_rx(void)
{
    while (!g_uart7_rx_complete);
    g_uart7_rx_complete = 0;
}
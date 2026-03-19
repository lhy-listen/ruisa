#include "uart2.h"
#include "hal_data.h"

static volatile int g_uart2_tx_complete = 0;
static volatile int g_uart2_rx_complete = 0;

void UART2_Init(void)
{
	    // ´®¿Ú³õÊ¼»¯ ...
    g_uart2.p_api->open(g_uart2.p_ctrl, g_uart2.p_cfg);
    //uint8_t msg[] = "100ask\r\n";
	  //uint8_t msg1[] = "100ast\r\n";
    //g_uart2.p_api->write(g_uart2.p_ctrl, msg, sizeof(msg)-1);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
}

void uart2_wait_for_tx(void)
{
    while (!g_uart2_tx_complete);
    g_uart2_tx_complete = 0;
}

void uart2_wait_for_rx(void)
{
    while (!g_uart2_rx_complete);
    g_uart2_rx_complete = 0;
}
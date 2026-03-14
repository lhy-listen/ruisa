#include "uart_thread.h"
#include "uart.h"
/* uart_thread entry function */
/* pvParameters contains TaskHandle_t */
void uart_thread_entry(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
		g_uart7.p_api->open(g_uart7.p_ctrl, g_uart7.p_cfg);
    uint8_t msg[] = "100ask\r\n";

/* 发送字符串 */
    g_uart7.p_api->write(g_uart7.p_ctrl, msg, sizeof(msg)-1);

/* 启动第一次接收 */
    g_uart7.p_api->read(g_uart7.p_ctrl, &rx_buf, 1);

/* TODO: add your own code here */
    while(1)
    {
      vTaskDelay(10);
    }
}

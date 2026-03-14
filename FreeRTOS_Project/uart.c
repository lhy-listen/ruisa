#include "hal_data.h"

static volatile int g_uart7_tx_complete = 0;
static volatile int g_uart7_rx_complete = 0;

/* 串口接收缓冲区 */
uint8_t rx_buf; 

void uart7_callback(uart_callback_args_t * p_args)
{
    // 用于通知内核是否需要进行任务切换
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        /* 1. 将收到的字符转为 uint32_t，匹配你的 Item Size (4) */
        uint32_t received_char = (uint32_t)rx_buf;

        /* 2. 发送到队列 (中断安全版) */
        xQueueSendFromISR(LEDqueue, &received_char, &xHigherPriorityTaskWoken);

        /* 3. 回显收到的字符 (可选，方便你在串口助手看到反馈) */
        g_uart7.p_api->write(g_uart7.p_ctrl, &rx_buf, 1);

        /* 4. 继续监听下一个字符 (这一步绝对不能丢) */
        g_uart7.p_api->read(g_uart7.p_ctrl, &rx_buf, 1);

        /* 5. 退出中断并检查是否需要唤醒高优先级任务 */
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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


#include "hal_data.h"
#include "Tpye.h"

volatile bool uart_send_complete_flag = false;
volatile uint8_t g_uart7_tx_busy = 0;
volatile uint8_t g_uart2_tx_busy = 0;

/* 串口接收缓冲区 */
uint8_t rx_buf; 
uint8_t rx_buf_u2; 

void uart7_callback(uart_callback_args_t * p_args)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // --- 情况 A: 收到数据 ---
    if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        uint32_t received_char = (uint32_t)rx_buf;
			  g_uart7.p_api->write(g_uart7.p_ctrl, &rx_buf, 1);
        xQueueSendFromISR(LEDqueue, &received_char, &xHigherPriorityTaskWoken);
        g_uart7.p_api->read(g_uart7.p_ctrl, &rx_buf, 1);
        
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    // --- 情况 B: 数据发送完成 (重点修复这里!) ---
    else if (p_args->event == UART_EVENT_TX_COMPLETE)
    {
        /* 硬件告诉你：这一串字符发完了，现在不忙了 */
        g_uart7_tx_busy = 0; 
    }
}

void uart5_callback(uart_callback_args_t * p_args)
{
    if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        
    }
}


void uart2_callback(uart_callback_args_t * p_args)
{
    // 用于通知内核是否需要进行任务切换
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // --- 情况 A: UART2 收到数据 ---
    if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        /* 1. 将收到的字符转为 uint32_t (根据你的队列 Item Size 匹配) */
        uint32_t received_val = (uint32_t)rx_buf_u2;

        /* 2. 发送到队列 (这里建议发往控制 LED 或 舵机的队列) */
        // 如果是蓝牙控制，可以发往 LEDqueue 共享逻辑
        xQueueSendFromISR(LEDqueue, &received_val, &xHigherPriorityTaskWoken);

        /* 3. 继续监听下一个字符 (这一步绝对不能丢，否则只接收一次就停了) */
        g_uart2.p_api->read(g_uart2.p_ctrl, &rx_buf_u2, 1);

        /* 4. 退出中断并检查是否需要执行上下文切换 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    // --- 情况 B: UART2 数据发送完成 ---
    else if (p_args->event == UART_EVENT_TX_COMPLETE)
    {
        /* 标记 UART2 硬件现在空闲了 */
        g_uart2_tx_busy = 0; 
    }
}
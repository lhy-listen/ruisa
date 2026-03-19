#include "BLT_thread.h"
#include "uart.h"
#include "Tpye.h"
/* BLT_thread entry function */
/* pvParameters contains TaskHandle_t */

void BLT_thread_entry(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    
    /* 1. 初始化：打开两个串口并开启接收监听 */
    g_uart7.p_api->open(g_uart7.p_ctrl, g_uart7.p_cfg);
    g_uart2.p_api->open(g_uart2.p_ctrl, g_uart2.p_cfg);
    
    g_uart7.p_api->read(g_uart7.p_ctrl, &rx_buf, 1);
    g_uart2.p_api->read(g_uart2.p_ctrl, &rx_buf_u2, 1);

    sensor_packet_t data;
    alarm_packet_t alarm;
    char sensormsg[100];
    char alarmmsg[100];

    while (1)
    {
        // --- 1. 处理传感器数据 ---
        if (xQueueReceive(sensorQueue, &data, pdMS_TO_TICKS(50))) 
        {
            // 先格式化一次数据
            sprintf(sensormsg, "Soil:%.1f Rain:%.1f Light:%.1f\r\n", 
                    data.soil, data.rain, data.light);

            // 发送给 UART7
            while (g_uart7_tx_busy == 1) { vTaskDelay(1); }
            g_uart7_tx_busy = 1;
            g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t *)sensormsg, strlen(sensormsg));

            // 发送给 UART2
            while (g_uart2_tx_busy == 1) { vTaskDelay(1); }
            g_uart2_tx_busy = 1;
            g_uart2.p_api->write(g_uart2.p_ctrl, (uint8_t *)sensormsg, strlen(sensormsg));
        }

        // --- 2. 处理报警数据 ---
        if (xQueueReceive(alarmQueue, &alarm, pdMS_TO_TICKS(50)))
        {
            // 先格式化一次数据
            sprintf(alarmmsg, "ALARM S:%d R:%d L:%d\r\n", 
                    alarm.alarm_soil, alarm.alarm_rain, alarm.alarm_light);

            // 发送给 UART7
            while (g_uart7_tx_busy == 1) { vTaskDelay(1); }
            g_uart7_tx_busy = 1;
            g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t *)alarmmsg, strlen(alarmmsg));

            // 发送给 UART2
            while (g_uart2_tx_busy == 1) { vTaskDelay(1); }
            g_uart2_tx_busy = 1;
            g_uart2.p_api->write(g_uart2.p_ctrl, (uint8_t *)alarmmsg, strlen(alarmmsg));
        }
    }
}
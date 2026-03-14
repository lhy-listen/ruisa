#include "led_thread.h"
#include "uart.h"
                /* led_thread entry function */
                /* pvParameters contains TaskHandle_t */

void toggle_led(bsp_io_port_pin_t pin)
{
    bsp_io_level_t level;
    R_IOPORT_PinRead(&g_ioport_ctrl, pin, &level);
    R_IOPORT_PinWrite(&g_ioport_ctrl, pin, (level == BSP_IO_LEVEL_LOW) ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
}

void led_thread_entry(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    
    uint32_t queue_data = 0;

    /* 建议在这里先确保串口开始接收第一次 */
    // g_uart7.p_api->read(g_uart7.p_ctrl, &rx_buf, 1);

    while (1)
    {
        /* 阻塞等待消息：队列空时，线程进入休眠状态 */
        if (pdPASS == xQueueReceive(LEDqueue, &queue_data, portMAX_DELAY))
        {
            /* 逻辑判断：
               如果你串口助手输入 '1'，ASCII 码是 49
               如果你串口助手输入 '2'，ASCII 码是 50
            */
            if (queue_data == '1') 
            {
                // 收到字符 '1'，点亮 (假设低电平亮)
                R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_00, BSP_IO_LEVEL_LOW);
            }
            else if (queue_data == '0')
            {
                // 收到字符 '0'，熄灭
                R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_00, BSP_IO_LEVEL_HIGH);
            }
            else
            {
                // 收到其他任何字符，反转电平
                toggle_led(BSP_IO_PORT_04_PIN_00);
            }
        }
    }
}
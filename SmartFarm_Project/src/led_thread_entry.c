#include "led_thread.h"
#include "uart.h"
#include "Servo.h"
#include "BEEP.h"
#include "pump.h"

/* led_thread entry function */
/* pvParameters contains TaskHandle_t */

volatile uint32_t g_servo_ticks = 0;

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
    
    // 1. 初始化定时器
    R_GPT_Open(&g_timer6_ctrl, &g_timer6_cfg);
    R_GPT_OutputEnable(&g_timer6_ctrl, GPT_IO_PIN_GTIOCA); // 确保开启输出
    R_GPT_Start(&g_timer6_ctrl);

    R_GPT_Open(&g_timer5_ctrl, &g_timer5_cfg);
    R_GPT_OutputEnable(&g_timer5_ctrl, GPT_IO_PIN_GTIOCA); // 确保开启输出
    R_GPT_Start(&g_timer5_ctrl);


    while (1)
    {
        // 阻塞等待消息
        if (pdPASS == xQueueReceive(LEDqueue, &queue_data, portMAX_DELAY))
        {
            switch(queue_data)
            {
                case '1': // 设为 90 度
                    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_00, BSP_IO_LEVEL_LOW);
								    g_servo_ticks = pdMS_TO_TICKS(1000);
                    greenhouse_open();
                    buzzer_beep(50); // 鸣叫反馈
                    break;

                case '0': // 设为 0 度
                    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_00, BSP_IO_LEVEL_HIGH);
								    g_servo_ticks = pdMS_TO_TICKS(1000);
                    greenhouse_close();
                    buzzer_beep(50);
                    break;

                case 'A': // 舵机1 左转
                case 'a':
                    break;

                case 'D': // 舵机1 右转
                case 'd':
                    break;

                case 'W': // 舵机2 增加
                case 'w':
                    break;

                case 'S': // 舵机2 减少
                case 's':
                    break;
								case 'P': // 舵机2 减少
                case 'p':
                    Pump_On();
								    buzzer_alarm(2);
                    break;
								case 'N': // 舵机2 减少
                case 'n':
                    Pump_On();
								    buzzer_alarm(2);
                    break;

                default:
                    toggle_led(BSP_IO_PORT_04_PIN_00);
                    buzzer_alarm(2); // 其他字符报错
                    break;
            }
        }
    }
}
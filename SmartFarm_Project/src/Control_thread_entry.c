#include "hal_data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "Tpye.h"
#include "Servo.h"
#include "pump.h"
#include "BEEP.h"


/* ===== Control线程入口 ===== */
void Control_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    alarm_packet_t alarm;

    while (1)
    {
        /* 阻塞等待报警数据 */
        if (xQueueReceive(controlQueue, &alarm, portMAX_DELAY) == pdPASS)
        {
            /* ===== 逻辑 A：大棚遮盖（舵机） ===== 
               安全策略：下雨(Rain) 或 阳光过强(Light) 都要关闭大棚保护植物 */
					if (g_servo_ticks > 0)
            {
                // 减去当前经历的时间
                if (g_servo_ticks > 100)
                    g_servo_ticks -= 100;
                else
                    g_servo_ticks = 0;
                
                goto PUMP_LOGIC; 
            }
						
            if (alarm.alarm_rain || alarm.alarm_light)
            {
                greenhouse_close(); // 执行关闭（内部已含状态判断）
							  buzzer_alarm(2);
            }
            else
            {
                /* 只有既没雨，阳光也不强时，才打开大棚 */
                greenhouse_open();
            }

            /* ===== 逻辑 B：灌溉系统（水泵） ===== 
               安全策略：土壤干就浇水，不干就停 */
						PUMP_LOGIC:
            if (alarm.alarm_soil)
            {
                Pump_On();
							  buzzer_alarm(2);
            }
            else
            {
                Pump_Off();
            }
            
            /* 可选：为了防止继电器频繁吸合，可以在此处添加一个极短的逻辑死区延时 */
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
    }
}

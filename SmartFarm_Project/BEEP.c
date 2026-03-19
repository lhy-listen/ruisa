#include "hal_data.h"
#include "BEEP.h"

/**
 * @brief 蜂鸣器开关控制
 * @param state : BUZZER_ON (响), BUZZER_OFF (静音)
 */
void buzzer_set(bsp_io_level_t state)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, BUZZER_PIN, state);
}

/**
 * @brief 蜂鸣器短鸣一次
 * @param ms : 鸣叫持续的时间（毫秒）
 */
void buzzer_beep(uint32_t ms)
{
    buzzer_set(BUZZER_ON);
    vTaskDelay(pdMS_TO_TICKS(ms));
    buzzer_set(BUZZER_OFF);
}

/**
 * @brief 蜂鸣器报警模式
 * @param times : 鸣叫次数
 */
void buzzer_alarm(uint32_t times)
{
    for(uint32_t i = 0; i < times; i++)
    {
        buzzer_set(BUZZER_ON);
        vTaskDelay(pdMS_TO_TICKS(200));
        buzzer_set(BUZZER_OFF);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
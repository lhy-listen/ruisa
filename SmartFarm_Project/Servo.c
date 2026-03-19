#include "hal_data.h"

#define SERVO_PERIOD_US         20000.0f
#define SERVO_MIN_PULSE_US      500.0f
#define SERVO_MAX_PULSE_US      2500.0f

// 舵机 1 控制函数 (g_timer6)
void set_servo1_angle(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    timer_info_t info;
    R_GPT_InfoGet(&g_timer6_ctrl, &info);
    
    float target_pulse_us = SERVO_MIN_PULSE_US + (angle / 180.0f) * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US);
    uint32_t duty_counts = (uint32_t)((target_pulse_us / SERVO_PERIOD_US) * (float)info.period_counts);

    R_GPT_DutyCycleSet(&g_timer6_ctrl, duty_counts, GPT_IO_PIN_GTIOCA);
}

// 舵机 2 控制函数 (g_timer5)
void set_servo2_angle(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    timer_info_t info;
    R_GPT_InfoGet(&g_timer5_ctrl, &info);
    
    float target_pulse_us = SERVO_MIN_PULSE_US + (angle / 180.0f) * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US);
    uint32_t duty_counts = (uint32_t)((target_pulse_us / SERVO_PERIOD_US) * (float)info.period_counts);

    R_GPT_DutyCycleSet(&g_timer5_ctrl, duty_counts, GPT_IO_PIN_GTIOCA);
}

/* ===== 状态记录（防止反复执行） ===== */
uint8_t servo_state = 0;   // 0=关闭(0度)，1=打开(180度)
uint8_t pump_state  = 0;   // 0=关闭，1=打开

void greenhouse_open(void)
{
    if (servo_state != 1)
    {
      set_servo1_angle(90.0f);   // 打开大棚
			set_servo2_angle(90.0f);   // 打开大棚
      servo_state = 1;
    }
}

void greenhouse_close(void)
{
    if (servo_state != 0)
    {
        set_servo1_angle(0.0f);   // 关闭大棚
			  set_servo2_angle(0.0f);   // 打开大棚
        servo_state = 0;
    }
}
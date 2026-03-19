#ifndef SERVO_H
#define SERVO_H

#include "hal_data.h"

void set_servo1_angle(float angle);
void set_servo2_angle(float angle);
void gpt_timer6_callback(timer_callback_args_t *p_args);
/* ===== 状态记录（防止反复执行） ===== */
extern uint8_t servo_state;   // 0=关闭(0度)，1=打开(180度)
extern uint8_t pump_state;   // 0=关闭，1=打开
void greenhouse_open(void);
void greenhouse_close(void);
	
#endif
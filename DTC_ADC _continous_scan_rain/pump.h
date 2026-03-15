#ifndef __PUMP_H
#define __PUMP_H

// 瑞萨 FSP 核心头文件
#include "hal_data.h"

// 水泵引脚定义：P400
#define PUMP_PIN        BSP_IO_PORT_04_PIN_14   // 瑞萨官方标准名称

// 水泵控制函数（接口和你原来的完全一样！）
void Pump_Init(void);    // 初始化 P400
void Pump_On(void);      // 开水泵
void Pump_Off(void);     // 关水泵

#endif
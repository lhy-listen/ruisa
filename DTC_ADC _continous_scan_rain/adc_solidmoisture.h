#ifndef ADC_SOLIDMOISTURE_H_
#define ADC_SOLIDMOISTURE_H_
#include <stdio.h>  // 必须包含此头文件
#include <string.h>
#include "hal_data.h"
// 定义一个结构体，用于一次性返回所有传感器数据
typedef struct {
    uint16_t raw_data;  // ADC 原始值 (0-4095)
    double   voltage;   // 换算后的电压值 (V)
    double   humidity;  // 计算出的湿度值 (%)
} sensor_data_t;

// 函数声明
void bsp_adc_init(void);
void bsp_adc_get_data(sensor_data_t *p_data);
float data_handle(uint16_t data_solidmoisture);
#endif
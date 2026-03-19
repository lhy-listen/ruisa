#include "hal_data.h"
#include "task.h"
#include "queue.h"
#include "adc0.h"
#include "adc_solidmoisture.h"
#include "Tpye.h"   // 你定义的结构体
#include <stdio.h>
#include "uart.h"

/* ===== 外部变量 ===== */
extern uint16_t adc_result_buffer[4];

/* ===== 阈值（你可以根据实际改） ===== */
#define SOIL_THRESHOLD_LOW     30.0f   // 土壤湿度低于30%
#define RAIN_THRESHOLD_HIGH    70.0f   // 雨量高于70%
#define LIGHT_THRESHOLD_HIGH   80.0f   // 光照高于80%

#include "hal_data.h"

// 传感器实测阈值定义
#define RAIN_DRY_ADC    4060.0f   // 无雨
#define RAIN_WET_ADC    2300.0f   // 满水

#define SOIL_DRY_ADC    3900.0f   // 干土壤
#define SOIL_WET_ADC    2800.0f   // 湿土壤

static float soil_filter_mem = 0.0f;
static float rain_filter_mem = 0.0f;
static float light_filter_mem = 0.0f;

/**
 * 线性映射函数：将 ADC 范围映射到 0-100%
 */
float map_sensor(float current, float dry, float wet)
{
    // 1. 限幅：防止超出实测范围导致百分比溢出
    if (current > dry) current = dry;
    if (current < wet) current = wet;

    // 2. 计算百分比 (注意：ADC 越大越干，所以是 dry - current)
    float result = ((dry - current) / (dry - wet)) * 100.0f;
    return result;
}

/**
 * 独立滤波函数 (解决之前数据混淆的问题)
 * @param input 当前计算出的百分比
 * @param last_val 指向该传感器专属的静态变量
 */
float sensor_filter(float input, float *last_val)
{
    // 滤波系数 0.2 (越小越平滑，越大响应越快)
    *last_val = (0.2f * input) + (0.8f * (*last_val));
    return *last_val;
}

/* ===== 线程入口 ===== */
void Senor_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    fsp_err_t err;

    /* ===== 1?? ADC初始化（只执行一次） ===== */
    ADC0_Init(err);

    DTC_Init(adc_result_buffer[0], err);

    adc0_Scan_Start(err);

    /* ===== 结构体 ===== */
    sensor_packet_t sensor_data;
    alarm_packet_t alarm;

    while (1)
    {
        /* ===== 2 读取ADC数据 ===== */
        uint16_t raw_light = adc_result_buffer[3];
        uint16_t raw_soil  = adc_result_buffer[1];
        uint16_t raw_rain  = adc_result_buffer[2];
			
			char msg[100];
			sprintf(msg,"%d,%d\r\n",(int)raw_soil,(int)raw_rain);
			
			g_uart7.p_api->write(g_uart7.p_ctrl,msg,strlen(msg));
			vTaskDelay(30);
			

        /* ===== 3 转换为百分比 ===== */
			
			/* 1. 映射逻辑：将 2800~3900 映射到 0~100 */
        float soil_pct = map_sensor((float)raw_soil, SOIL_DRY_ADC, SOIL_WET_ADC);
        float rain_pct = map_sensor((float)raw_rain, RAIN_DRY_ADC, RAIN_WET_ADC);
        float light_pct = ((float)raw_light / 4095.0f) * 100.0f;

        /* 2. 独立滤波：传入对应的内存地址 */
        sensor_data.soil  = sensor_filter(soil_pct, &soil_filter_mem);
        sensor_data.rain  = sensor_filter(rain_pct, &rain_filter_mem);
        sensor_data.light = sensor_filter(light_pct, &light_filter_mem);

				
//				sprintf(msg, "Soil:%.1f Rain:%.1f Light:%.1f\r\n",
//                    sensor_data.soil, sensor_data.rain, sensor_data.light);

//        g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t *)msg, strlen(msg));

        /* ===== 4 发送传感器数据 ===== */
        xQueueSend(sensorQueue, &sensor_data, portMAX_DELAY);

        /* ===== 5 报警判断（核心） ===== */
        alarm.alarm_soil  = 0;
        alarm.alarm_rain  = 0;
        alarm.alarm_light = 0;

        if (sensor_data.soil < SOIL_THRESHOLD_LOW)
        {
            alarm.alarm_soil = 1;
        }

        if (sensor_data.rain > RAIN_THRESHOLD_HIGH)
        {
            alarm.alarm_rain = 1;
        }

        if (sensor_data.light > LIGHT_THRESHOLD_HIGH)
        {
            alarm.alarm_light = 1;
        }

        /* ===== 6 发送报警信息 ===== */
        xQueueSend(alarmQueue, &alarm, portMAX_DELAY);
				xQueueOverwrite(controlQueue, &alarm);

        /* ===== 7 周期 ===== */
        vTaskDelay(2000);
    }
}
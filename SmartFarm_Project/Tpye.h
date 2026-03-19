#ifndef COMMON_H
#define COMMON_H

#include "hal_data.h"

typedef struct
{
    float soil;     // 土壤湿度 %
    float rain;     // 雨量 %
    float light;    // 光照 %
} sensor_packet_t;

typedef struct
{
    uint8_t alarm_soil;
    uint8_t alarm_rain;
    uint8_t alarm_light;
} alarm_packet_t;

extern volatile uint32_t g_servo_ticks;

#endif
#ifndef FARM_ANALYSIS_H
#define FARM_ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>

// 【1】集中管理所有农场数据的“数据库”结构体
typedef struct {
    // --- 传感器实时数据 ---
    float    temperature;   // 空气温度 (℃)
    float    air_humidity;  // 空气湿度 (%)
    float    light_lux;     // 光照强度 (Lux)
    uint16_t co2_ppm;       // CO2浓度 (ppm)
    float    soil_moisture; // 土壤湿度
    float    rain_level;    // 降雨量/状态
    
    // --- 执行器当前状态 (false:关, true:开) ---
    bool     water_pump;    // 水泵状态
    bool     air_pump;      // 气泵状态 (已改纯手动，仅供记录)
    bool     roof_servo;    // 【新增】大棚顶窗舵机状态 (false:tingzhi, true:kaishi)
    bool     alarm_sys;     // 报警系统状态
} FarmData_t;

extern FarmData_t g_farm_data; 
extern void Hmi_SendCmd(const char *cmd_str);

void Farm_Data_Init(void);
void Farm_Data_Analysis(void);

#endif // FARM_ANALYSIS_H
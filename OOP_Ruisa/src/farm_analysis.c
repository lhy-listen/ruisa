#include "farm_analysis.h"
#include "driver_uart.h" 
#include <stdio.h>
#include <string.h>

FarmData_t g_farm_data;
extern UartDevice_t Uart8; // LoRa 通信接口

void Farm_Data_Init(void) {
    g_farm_data.temperature = 0.0f;
    g_farm_data.air_humidity = 0.0f;
    g_farm_data.light_lux = 0.0f;
    g_farm_data.co2_ppm = 0;
    g_farm_data.soil_moisture = 0.0f;
    g_farm_data.rain_level = 0.0f;
    
    g_farm_data.water_pump = false;
    g_farm_data.air_pump = false;
    g_farm_data.roof_servo = false; // 默认大棚关闭
    g_farm_data.alarm_sys = false;
}

void Farm_Data_Analysis(void) {
    Uart_Printf(&UartPc, "\r\n=== [Farm Brain] System Snapshot ===\r\n");
    Uart_Printf(&UartPc, "Temp: %.1f C | Hum: %.1f %% | Light: %.1f lx | CO2: %d ppm\r\n", 
                g_farm_data.temperature, g_farm_data.air_humidity, 
                g_farm_data.light_lux, g_farm_data.co2_ppm);
    Uart_Printf(&UartPc, "Soil: %.2f | Rain: %.2f\r\n", 
                g_farm_data.soil_moisture, g_farm_data.rain_level);
    Uart_Printf(&UartPc, "Status -> WaterPump:%d | Roof:%d | AirPump:%d\r\n", 
                g_farm_data.water_pump, g_farm_data.roof_servo, g_farm_data.air_pump);

    // ======================================================================
    // 1. 【新增联动】大棚顶窗舵机逻辑 (温度 + 降雨)
    // ======================================================================
    if (g_farm_data.rain_level > 10.0f) {
        // 下雨了，强制关窗避雨！
        if (g_farm_data.roof_servo == true) {
            g_farm_data.roof_servo = false;
            Uart_Printf(&UartPc, ">>> [Action] Raining! Roof closed (tingzhi)\r\n");
            char *cmd = "tingzhi\r\n";
            Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd));
        }
    } 
    else {
        // 没下雨时，根据温度开关窗
        if (g_farm_data.temperature > 30.0f && g_farm_data.roof_servo == false) {
            g_farm_data.roof_servo = true;
            Uart_Printf(&UartPc, ">>> [Action] Too Hot! Roof opened (kaishi) to cool down\r\n");
            char *cmd = "kaishi\r\n";
            Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd));
        } 
        else if (g_farm_data.temperature < 15.0f && g_farm_data.roof_servo == true) {
            g_farm_data.roof_servo = false;
            Uart_Printf(&UartPc, ">>> [Action] Too Cold! Roof closed (tingzhi) to keep warm\r\n");
            char *cmd = "tingzhi\r\n";
            Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd));
        }
    }

    // ======================================================================
    // 2. 【土-天】水泵控制逻辑 (已修正发往 LoRa 的指令为 PUMP_ON/OFF)
    // ======================================================================
    if (g_farm_data.soil_moisture < 30.0f && g_farm_data.rain_level < 10.0f) {
        if (!g_farm_data.water_pump) {
            g_farm_data.water_pump = true;
            Uart_Printf(&UartPc, ">>> [Action] Soil dry & No Rain: Water Pump ON!\r\n");
            char *cmd = "PUMP_ON\r\n"; // 水泵用独立指令
            Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd));
        }
    } 
    else if (g_farm_data.soil_moisture > 60.0f || g_farm_data.rain_level > 30.0f) {
        if (g_farm_data.water_pump) {
            g_farm_data.water_pump = false;
            Uart_Printf(&UartPc, ">>> [Action] Soil wet OR Raining: Water Pump OFF!\r\n");
            char *cmd = "PUMP_OFF\r\n"; // 水泵用独立指令
            Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd));
        }
    }

    // ======================================================================
    // 3. 【光-碳】气泵控制逻辑 (仅打印日志，不再往 UART8 发送指令)
    // ======================================================================
    if (g_farm_data.light_lux > 1000.0f) {
        if (g_farm_data.co2_ppm > 0 && g_farm_data.co2_ppm < 1000) { 
            if (!g_farm_data.air_pump) {
                g_farm_data.air_pump = true;
                Uart_Printf(&UartPc, ">>> [Log] Need CO2, please turn on Air Pump manually.\r\n");
            }
        } else if (g_farm_data.co2_ppm > 1200) {
            if (g_farm_data.air_pump) {
                g_farm_data.air_pump = false;
                Uart_Printf(&UartPc, ">>> [Log] CO2 Sufficient.\r\n");
            }
        }
    } else { 
        if (g_farm_data.air_pump) {
            g_farm_data.air_pump = false;
            Uart_Printf(&UartPc, ">>> [Log] Night time, CO2 disabled.\r\n");
        }
    }

    Uart_Printf(&UartPc, "======================================\r\n");
}
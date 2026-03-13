#include "pump.h"

// 初始化 P400 为输出模式
void Pump_Init(void)
{
    // 初始化已经由 FSP 配置工具自动完成
    // 这里只需要设置默认关闭水泵
    Pump_Off();
}

// 打开水泵（低电平有效，继电器常用）
void Pump_On(void)
{
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, PUMP_PIN, BSP_IO_LEVEL_LOW);
}

// 关闭水泵
void Pump_Off(void)
{
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, PUMP_PIN, BSP_IO_LEVEL_HIGH);
}
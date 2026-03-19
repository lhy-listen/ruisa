#include "pump.h"

// 初始化 P400 为输出模式
/* 假设 PUMP_PIN 已经在头文件定义 */
#define PUMP_ON_LEVEL  BSP_IO_LEVEL_HIGH  // 根据你的描述，HIGH 为开
#define PUMP_OFF_LEVEL BSP_IO_LEVEL_LOW

// 获取水泵当前开关状态
uint8_t Get_Pump_State(void)
{
    bsp_io_level_t level;
    R_IOPORT_PinRead(&g_ioport_ctrl, PUMP_PIN, &level);
    return (level == PUMP_ON_LEVEL) ? 1 : 0;
}

// 开启水泵
void Pump_On(void)
{
    if (Get_Pump_State() == 0) // 只有关着的时候才执行写操作
    {
        R_IOPORT_PinWrite(&g_ioport_ctrl, PUMP_PIN, PUMP_ON_LEVEL);
    }
}

// 关闭水泵
void Pump_Off(void)
{
    if (Get_Pump_State() == 1) // 只有开着的时候才执行写操作
    {
        R_IOPORT_PinWrite(&g_ioport_ctrl, PUMP_PIN, PUMP_OFF_LEVEL);
    }
}
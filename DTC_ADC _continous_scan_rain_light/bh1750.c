#include "bh1750.h"

#define BH1750_POWER_ON          0x01
#define BH1750_RESET             0x07
#define BH1750_CONT_HIGH_RES     0x10

// SCI I2C 正确写入函数
static fsp_err_t BH1750_WriteCmd(uint8_t cmd)
{
    return g_i2c4.p_api->write(g_i2c4.p_ctrl, BH1750_ADDR, &cmd, 1);
}

// SCI I2C 正确读取函数
static fsp_err_t BH1750_ReadData(uint8_t *p_data, uint32_t len)
{
    return g_i2c4.p_api->read(g_i2c4.p_ctrl, BH1750_ADDR, p_data, len);
}

// 初始化
void BH1750_Init(void)
{
    BH1750_WriteCmd(BH1750_POWER_ON);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    BH1750_WriteCmd(BH1750_RESET);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    BH1750_WriteCmd(BH1750_CONT_HIGH_RES);
    R_BSP_SoftwareDelay(180, BSP_DELAY_UNITS_MILLISECONDS);
}

// 读取照度
float BH1750_ReadLux(void)
{
    uint8_t buf[2] = {0};
    uint16_t raw = 0;

    BH1750_ReadData(buf, 2);

    raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (float)raw / 1.2f;
}
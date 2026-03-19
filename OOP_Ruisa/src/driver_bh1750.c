#include "driver_bh1750.h"

/* BH1750 指令 */
#define BH1750_CMD_POWER_DOWN            (0x00)
#define BH1750_CMD_POWER_ON              (0x01)
#define BH1750_CMD_RESET                 (0x07)
#define BH1750_CMD_CONT_H_RES_MODE       (0x10)

extern volatile bool g_i2c_completed;
extern volatile bool g_i2c_aborted;

static fsp_err_t bh1750_write_cmd(Bh1750_t * dev, uint8_t cmd)
{
    if ((NULL == dev) || (NULL == dev->p_i2c)) return FSP_ERR_INVALID_ARGUMENT; 

    g_i2c_completed = false;
    g_i2c_aborted   = false;

    // 先动态设置要通信的从机地址
    dev->p_i2c->p_api->slaveAddressSet(dev->p_i2c->p_ctrl, dev->addr_7bit, I2C_MASTER_ADDR_MODE_7BIT);
    
    // 写入命令，第四个参数是 false，表示传输结束后发送 STOP 停止位
    fsp_err_t err = dev->p_i2c->p_api->write(dev->p_i2c->p_ctrl, &cmd, 1, false);                                                                          
    if (err == FSP_SUCCESS) {
        uint32_t timeout = 5000000;
        // 等待发送完成或者超时
        while (!g_i2c_completed && timeout > 0) { timeout--; }

        if (timeout == 0) return FSP_ERR_TIMEOUT;
        if (g_i2c_aborted) return FSP_ERR_ABORTED; // 如果硬件拒绝，立刻报错    
    }
    return err;
}

static fsp_err_t bh1750_read_2bytes(Bh1750_t * dev, uint8_t data[2])
{
    if ((NULL == dev) || (NULL == dev->p_i2c) || (NULL == data)) return FSP_ERR_INVALID_ARGUMENT;                                                               
    
    g_i2c_completed = false;
    g_i2c_aborted   = false;

    // 先动态设置要通信的从机地址
    dev->p_i2c->p_api->slaveAddressSet(dev->p_i2c->p_ctrl, dev->addr_7bit, I2C_MASTER_ADDR_MODE_7BIT);

    // 读取数据，第四个参数是 false，表示传输结束后发送 STOP 停止位
    fsp_err_t err = dev->p_i2c->p_api->read(dev->p_i2c->p_ctrl, data, 2, false);                                                                           
    if (err == FSP_SUCCESS) {
        uint32_t timeout = 5000000;
        while (!g_i2c_completed && timeout > 0) { timeout--; }

        if (timeout == 0) return FSP_ERR_TIMEOUT;
        if (g_i2c_aborted) return FSP_ERR_ABORTED;
    }
    return err;
}

fsp_err_t Bh1750_Init(Bh1750_t * dev, i2c_master_instance_t const * p_i2c, uint8_t addr_7bit)                                                                   
{
    if ((NULL == dev) || (NULL == p_i2c)) return FSP_ERR_INVALID_ARGUMENT;      

    dev->p_i2c       = p_i2c;
    dev->addr_7bit   = addr_7bit;
    dev->initialized = false;

    fsp_err_t err = bh1750_write_cmd(dev, BH1750_CMD_POWER_ON);
    if (FSP_SUCCESS != err) return err;

    err = bh1750_write_cmd(dev, BH1750_CMD_RESET);
    if (FSP_SUCCESS != err) return err;

    err = bh1750_write_cmd(dev, BH1750_CMD_CONT_H_RES_MODE);
    if (FSP_SUCCESS != err) return err;

    dev->initialized = true;
    return FSP_SUCCESS;
}

fsp_err_t Bh1750_ReadLux(Bh1750_t * dev, float * lux_out)
{
    if ((NULL == dev) || (NULL == lux_out)) return FSP_ERR_INVALID_ARGUMENT;    
    if (!dev->initialized) return FSP_ERR_NOT_OPEN;

    uint8_t raw[2] = {0, 0};
    fsp_err_t err = bh1750_read_2bytes(dev, raw);
    if (FSP_SUCCESS != err) return err; 
    
    // 如果读取成功，转换光照度                                               
    uint16_t raw_u16 = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    *lux_out = ((float)raw_u16) / 1.2f;

    return FSP_SUCCESS;
}
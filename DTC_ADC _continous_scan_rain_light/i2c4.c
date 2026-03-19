#include "hal_data.h"
#include "i2c4.h"
void I2C4_Init(fsp_err_t err1)
{
	  err1 = g_i2c4.p_api->open(g_i2c4.p_ctrl, g_i2c4.p_cfg);
    assert(FSP_SUCCESS == err1);
}


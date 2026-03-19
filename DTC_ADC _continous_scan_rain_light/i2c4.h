#ifndef I2C4_H_
#define I2C4_H_
#include "hal_data.h"

void I2C4_Init(fsp_err_t err1);
void sci_i2c4_master_callback(i2c_master_callback_args_t *p_args);


#endif
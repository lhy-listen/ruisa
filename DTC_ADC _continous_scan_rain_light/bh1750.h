#ifndef BH1750_H
#define BH1750_H

#include "hal_data.h"
#include <stdint.h>

#define BH1750_ADDR      0x23    // 7Œªµÿ÷∑

void BH1750_Init(void);
float BH1750_ReadLux(void);

#endif
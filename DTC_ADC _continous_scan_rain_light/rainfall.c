#include "rainfall.h"
float Get_Rainfall(uint16_t adc_val) {
    //u16 adc_val = ADC_ReadValue(ADC_Channel_0); // 读取通道0 ADC值
    // 步骤1：ADC值转电压（STM32参考电压3.3V，12位ADC：电压=ADC值*3.3/4095）
    float voltage = adc_val * 3.3f / 4095.0f;
    
    // 步骤2：电压→降雨量（需根据传感器手册校准，示例公式）
    // 示例：某传感器0V=0mm/h，3.3V=200mm/h，线性换算
    float rainfall = (voltage / 3.3f) * 200.0f; 
    
    // 异常值过滤（如电压<0.01V视为无降雨）
    if(voltage < 0.01f) rainfall = 0.0f;
    return rainfall;
}

uint16_t rainfall_datahandle(uint16_t data_rainfall)
{
	uint16_t mode=0;
	float rate=(float)data_rainfall*3.3/4095.0;
	if(rate<=3.1 && rate>=2.7)
	{
		mode=1;//小雨
	}
	else if(rate <2.7 && rate>=2.4)
	{
		mode=2;//中雨
	}
	else if(rate<2.4 && rate>1.779)
	{
		mode=3;
	}
	return mode;
}
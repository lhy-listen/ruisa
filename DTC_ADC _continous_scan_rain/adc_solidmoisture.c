#include "adc_solidmoisture.h"
//实际测试得到数据
float	dry_data	=1544.0;
float full_data =730.0;

/*********************************************************************************
 * 函数名：bsp_adc_init
 * 描  述：初始化 ADC 模块并启动连续扫描
 *********************************************************************************/
void bsp_adc_init(void)
{
    fsp_err_t err;

    // 1. 打开 ADC 模块
    err = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    if(FSP_SUCCESS != err) { while(1); } // 初始化失败则卡死

    // 2. 配置扫描通道
    err = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
    if(FSP_SUCCESS != err) { while(1); }

    // 3. 启动扫描 (连续扫描模式下调用一次即可)
    err = R_ADC_ScanStart(&g_adc0_ctrl);
    if(FSP_SUCCESS != err) { while(1); }
}

/*********************************************************************************
 * 函数名：bsp_adc_get_data
 * 描  述：读取 ADC 通道 5 的值，并计算电压和湿度
 * 参  数：p_data - 指向 sensor_data_t 结构体的指针，用于保存结果
 *********************************************************************************/
void bsp_adc_get_data(sensor_data_t *p_data)
{
    // 如果传入的指针为空，直接退出保护
    if (p_data == NULL) return; 

    // 1. 读取硬件原始数据
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_5, &(p_data->raw_data));

    // 2. 计算电压值 (12-bit 分辨率)
    p_data->voltage = (double)(p_data->raw_data * 3.3 / 4096.0);

    // 3. 计算湿度值
    if (p_data->raw_data <= dry_data && p_data->raw_data >= full_data)
    {
        p_data->humidity = (p_data->raw_data - dry_data) / (-dry_data+full_data) * 100.0;
    }
    else if (p_data->raw_data > dry_data)
    {
        p_data->humidity = 0.0;
    }
    else if (p_data->raw_data < full_data)
    {
        p_data->humidity = 100.0;
    }
}

float data_handle(uint16_t data_solidmoisture)
{
	float vol1=0.0f;//电压
	vol1=data_solidmoisture*3.3f/4096.0f;
	
	float humidity=0.0f;
	if(data_solidmoisture<=dry_data && data_solidmoisture>=full_data)
	{
		humidity=(data_solidmoisture - dry_data) / (-dry_data+full_data) * 100.0f;
	}
	else if(data_solidmoisture > dry_data)
    {
       humidity = 0.0f;
    }
    else if (data_solidmoisture < full_data)
    {
        humidity = 100.0f;
    }
		return humidity;

}

//float data_handle(uint16_t data_solidmoisture)
//{
//	float vol1=0.0f;//电压
//	vol1=data_solidmoisture*3.3f/4096.0f;
//	
//	float humidity=0.0f;

//		humidity=(data_solidmoisture - dry_data) / (-dry_data + full_data) * 100.0f;

//		return humidity;

//}

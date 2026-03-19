#include "adc0.h"
#include "fsp_common_api.h"

fsp_err_t err0 = FSP_SUCCESS;
uint16_t adc_result_buffer[4];

// 用于存储 ADC 原始值的变量
void ADC0_Init(fsp_err_t err)
{
	  // ADC 初始化
    err=R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    assert(FSP_SUCCESS==err);

    err=R_ADC_ScanCfg(&g_adc0_ctrl,&g_adc0_channel_cfg);
    assert(FSP_SUCCESS==err);
}

void adc0_Scan_Start(fsp_err_t err)
{
    err = R_ADC_ScanStart(&g_adc0_ctrl);
    assert(FSP_SUCCESS == err);	
}

/* Callback function */

void adc_callback(adc_callback_args_t *p_args)
{
    if (p_args->event == ADC_EVENT_SCAN_COMPLETE)
    {
        /**
         * 重点修复：R_DTC_Reset 的最后一个参数
         * 如果你采集 3 个通道（ADDR0, ADDR1, ADDR2），
         * 那么传输次数应该是 3，而不是 1。
         */
        err0 = R_DTC_Reset(&g_transfer0_ctrl,
                          (void*)&R_ADC0->ADDR[0],  // 源地址：ADC数据寄存器0
                          &adc_result_buffer[0],    // 目标地址：你的数组
                          4);                       // 【关键】传输次数必须与通道数一致！
        
        if (FSP_SUCCESS != err0)
        {
            // 如果重置失败，不要用 __BKPT，可以先尝试忽略或者记录错误
            // __BKPT(1); 
        }

        /**
         * 如果你的 ADC 配置是 "Single Scan" (单次扫描)
         * 搬运完后需要手动再次启动，才能进行下一次采集。
         */
        R_ADC_ScanStart(&g_adc0_ctrl);
    }
}

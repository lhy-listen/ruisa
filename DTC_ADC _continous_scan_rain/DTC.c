#include "DTC.h"
void DTC_Init(uint16_t adc_result_buffer[0],fsp_err_t err)
{
    g_transfer0_cfg.p_info->p_src = (void*)&R_ADC0->ADDR[0];
    g_transfer0_cfg.p_info->p_dest = &adc_result_buffer[0];

    err = R_DTC_Open(&g_transfer0_ctrl,&g_transfer0_cfg);
    assert(FSP_SUCCESS == err);

    err = R_DTC_Enable(&g_transfer0_ctrl);
    assert(FSP_SUCCESS == err);	
}
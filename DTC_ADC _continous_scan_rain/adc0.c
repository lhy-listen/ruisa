#include "adc0.h"
#include "fsp_common_api.h"
void ADC0_Init(fsp_err_t err)
{
	  // ADC ≥ı ºªØ
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
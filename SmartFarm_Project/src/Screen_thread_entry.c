#include "Screen_thread.h"
                /* Screen_thread entry function */
                /* pvParameters contains TaskHandle_t */
                void Screen_thread_entry(void * pvParameters)
                {
                    FSP_PARAMETER_NOT_USED(pvParameters);

                    /* TODO: add your own code here */
                    while(1)
                    {
                        vTaskDelay(100);
                    }
                }

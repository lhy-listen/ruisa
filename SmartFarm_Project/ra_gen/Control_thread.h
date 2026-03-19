/* generated thread header file - do not edit */
#ifndef CONTROL_THREAD_H_
#define CONTROL_THREAD_H_
#include "bsp_api.h"
                #include "FreeRTOS.h"
                #include "task.h"
                #include "semphr.h"
                #include "hal_data.h"
                #ifdef __cplusplus
                extern "C" void Control_thread_entry(void * pvParameters);
                #else
                extern void Control_thread_entry(void * pvParameters);
                #endif
FSP_HEADER
FSP_FOOTER
#endif /* CONTROL_THREAD_H_ */

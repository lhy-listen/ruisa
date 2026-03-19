/* generated thread header file - do not edit */
#ifndef SENOR_THREAD_H_
#define SENOR_THREAD_H_
#include "bsp_api.h"
                #include "FreeRTOS.h"
                #include "task.h"
                #include "semphr.h"
                #include "hal_data.h"
                #ifdef __cplusplus
                extern "C" void Senor_thread_entry(void * pvParameters);
                #else
                extern void Senor_thread_entry(void * pvParameters);
                #endif
FSP_HEADER
FSP_FOOTER
#endif /* SENOR_THREAD_H_ */

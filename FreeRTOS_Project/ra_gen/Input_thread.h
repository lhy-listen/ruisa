/* generated thread header file - do not edit */
#ifndef INPUT_THREAD_H_
#define INPUT_THREAD_H_
#include "bsp_api.h"
                #include "FreeRTOS.h"
                #include "task.h"
                #include "semphr.h"
                #include "hal_data.h"
                #ifdef __cplusplus
                extern "C" void Input_thread_entry(void * pvParameters);
                #else
                extern void Input_thread_entry(void * pvParameters);
                #endif
FSP_HEADER
FSP_FOOTER
#endif /* INPUT_THREAD_H_ */

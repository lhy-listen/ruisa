/* generated common source file - do not edit */
#include "common_data.h"
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
        {
            .p_api = &g_ioport_on_ioport,
            .p_ctrl = &g_ioport_ctrl,
            .p_cfg = &g_bsp_pin_cfg,
        };
QueueHandle_t LEDqueue;
                #if 0
                StaticQueue_t LEDqueue_memory;
                uint8_t LEDqueue_queue_memory[4 * 20];
                #endif
                void rtos_startup_err_callback(void * p_instance, void * p_data);
QueueHandle_t sensorQueue;
                #if 0
                StaticQueue_t sensorQueue_memory;
                uint8_t sensorQueue_queue_memory[12 * 20];
                #endif
                void rtos_startup_err_callback(void * p_instance, void * p_data);
QueueHandle_t alarmQueue;
                #if 0
                StaticQueue_t alarmQueue_memory;
                uint8_t alarmQueue_queue_memory[8 * 20];
                #endif
                void rtos_startup_err_callback(void * p_instance, void * p_data);
QueueHandle_t controlQueue;
                #if 0
                StaticQueue_t controlQueue_memory;
                uint8_t controlQueue_queue_memory[8 * 1];
                #endif
                void rtos_startup_err_callback(void * p_instance, void * p_data);
SemaphoreHandle_t g_uart7_tx_sem;
                #if 0
                StaticSemaphore_t g_uart7_tx_sem_memory;
                #endif
                void rtos_startup_err_callback(void * p_instance, void * p_data);
void g_common_init(void) {
LEDqueue =
                #if 0
                xQueueCreateStatic(
                #else
                xQueueCreate(
                #endif
                20,
                4
                #if 0
                , &LEDqueue_queue_memory[0],
                &LEDqueue_memory
                #endif
                );
                if (NULL == LEDqueue) {
                rtos_startup_err_callback(LEDqueue, 0);
                }
sensorQueue =
                #if 0
                xQueueCreateStatic(
                #else
                xQueueCreate(
                #endif
                20,
                12
                #if 0
                , &sensorQueue_queue_memory[0],
                &sensorQueue_memory
                #endif
                );
                if (NULL == sensorQueue) {
                rtos_startup_err_callback(sensorQueue, 0);
                }
alarmQueue =
                #if 0
                xQueueCreateStatic(
                #else
                xQueueCreate(
                #endif
                20,
                8
                #if 0
                , &alarmQueue_queue_memory[0],
                &alarmQueue_memory
                #endif
                );
                if (NULL == alarmQueue) {
                rtos_startup_err_callback(alarmQueue, 0);
                }
controlQueue =
                #if 0
                xQueueCreateStatic(
                #else
                xQueueCreate(
                #endif
                1,
                8
                #if 0
                , &controlQueue_queue_memory[0],
                &controlQueue_memory
                #endif
                );
                if (NULL == controlQueue) {
                rtos_startup_err_callback(controlQueue, 0);
                }
g_uart7_tx_sem =
                #if 0
                xSemaphoreCreateBinaryStatic(&g_uart7_tx_sem_memory);
                #else
                xSemaphoreCreateBinary();
                #endif
                if (NULL == g_uart7_tx_sem) {
                rtos_startup_err_callback(g_uart7_tx_sem, 0);
                }
}

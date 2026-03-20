#include "hal_data.h"
#include <stdio.h>
#include <string.h>

#include "driver_uart.h"
#include "driver_dht22.h"
#include "driver_bh1750.h"
#include "farm_analysis.h" // 【新增】引入农场数据分析大脑

/* ====================================================================================
 * 【0】全局对象与中断
 * ==================================================================================== */
static Bh1750_t g_bh1750;
volatile bool g_i2c_completed = false;
volatile bool g_i2c_aborted   = false; 

void sci_i2c_master_callback(i2c_master_callback_args_t * p_args) {
    if (p_args->event == I2C_MASTER_EVENT_TX_COMPLETE ||
        p_args->event == I2C_MASTER_EVENT_RX_COMPLETE) {
        g_i2c_aborted = false; g_i2c_completed = true;
    } else if (p_args->event == I2C_MASTER_EVENT_ABORTED) {
        g_i2c_aborted = true; g_i2c_completed = true; 
    }
}

void uart2_callback(uart_callback_args_t * p_args) { if (p_args->event == UART_EVENT_RX_COMPLETE) UartSensor.rx_complete = true; }
void uart4_callback(uart_callback_args_t * p_args) { if (p_args->event == UART_EVENT_RX_COMPLETE) UartHmi.rx_complete = true; else if (p_args->event == UART_EVENT_TX_COMPLETE) UartHmi.tx_complete = true; }
void uart7_callback(uart_callback_args_t * p_args) { if (p_args->event == UART_EVENT_TX_COMPLETE) UartPc.tx_complete = true; }
void uart9_callback(uart_callback_args_t * p_args) { if (p_args->event == UART_EVENT_RX_COMPLETE) UartFace.rx_complete = true; else if (p_args->event == UART_EVENT_TX_COMPLETE) UartFace.tx_complete = true; }
void uart8_callback(uart_callback_args_t * p_args) { if (p_args->event == UART_EVENT_RX_COMPLETE) Uart8.rx_complete = true; else if (p_args->event == UART_EVENT_TX_COMPLETE) Uart8.tx_complete = true; }

/* ====================================================================================
 * 【2】公共业务助手
 * 注意：去掉了 static，这样 farm_analysis.c 也能调它了
 * ==================================================================================== */
void Hmi_SendCmd(const char *cmd_str) {
    uint8_t buffer[128];
    int len = snprintf((char *)buffer, sizeof(buffer), "%s", cmd_str);
    buffer[len++] = 0xFF; buffer[len++] = 0xFF; buffer[len++] = 0xFF;
    Uart_Send(&UartHmi, buffer, (uint32_t)len);
}

/* ====================================================================================
 * 【3】系统任务区 (所有任务只管采集，把数据塞入 g_farm_data)
 * ==================================================================================== */

// --- 任务 1：温湿度模块 ---
static void Task_DHT22(void) {
    static uint32_t dht22_read_tick = 0;
    dht22_read_tick++;
    if (dht22_read_tick >= 10000000) {
        float hum, temp;
        if (Dht22_Read(&Dht22, &hum, &temp)) {
            // 【写入数据库】
            g_farm_data.temperature = temp;
            g_farm_data.air_humidity = hum;

            int temp_int = (int) temp; int temp_dec = (int) (temp * 10) % 10;
            int hum_int  = (int) hum;  int hum_dec  = (int) (hum * 10) % 10;
            char cmd[64];
            snprintf(cmd, sizeof(cmd), "t4.txt=\"%d.%d\"", temp_int, temp_dec);
            Hmi_SendCmd(cmd);
            R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
            snprintf(cmd, sizeof(cmd), "t5.txt=\"%d.%d\"", hum_int, hum_dec);
            Hmi_SendCmd(cmd);
        }
        dht22_read_tick = 0;
    }
}

// --- 任务 2：CO2 传感器模块 ---
static void Task_CO2(void) {
    #define CO2_RX_MAX_LEN 64
    static uint8_t rx_frame[CO2_RX_MAX_LEN];
    static uint8_t rx_idx = 0;
    static uint32_t rx_idle_tick = 0; 
    static uint16_t low_co2_count = 0;
    static uint8_t add_rest_count = 0;
    static uint16_t add_send_count = 0;

    if (UartSensor.rx_complete) {
        UartSensor.rx_complete = false;
        if (rx_idx < CO2_RX_MAX_LEN) rx_frame[rx_idx++] = UartSensor.rx_byte;
        rx_idle_tick = 0; 
        Uart_StartRx(&UartSensor);
    }

    if (rx_idx > 0) {
        rx_idle_tick++;
        if (rx_idle_tick > 50000) { 
            for (int i = 0; i <= rx_idx - 16; i++) {
                if (rx_frame[i] == 0xFE) {
                    uint16_t real_co2 = (uint16_t)((((uint16_t)rx_frame[i+7] << 8) | rx_frame[i+8]));
                    if (real_co2 > 0 && real_co2 <= 5000) {
                        // 【写入数据库】
                        g_farm_data.co2_ppm = real_co2;

                        uint16_t co2_val_scaled = real_co2 / 25; 
                        if (co2_val_scaled < (1000 / 25)) {
                            low_co2_count++;
                            if (low_co2_count >= 50) { Hmi_SendCmd("page box"); low_co2_count = 0; }
                        } else { low_co2_count = 0; }

                        char cmd_core[32];
                        snprintf(cmd_core, sizeof(cmd_core), "add 1,1,%d", co2_val_scaled);
                        Hmi_SendCmd(cmd_core);

                        add_rest_count++;
                        if (add_rest_count >= 50) {
                            R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);
                            add_rest_count = 0;
                        }
                        if (++add_send_count >= 300) { Hmi_SendCmd("cle 1,1"); add_send_count = 0; }
                    }
                    break; 
                }
            }
            rx_idx = 0; rx_idle_tick = 0;
        }
    }
}

// --- 任务 3：HMI 屏幕指令处理 (含 LoRa 透传与人脸识别) ---
typedef enum { FACE_IDLE = 0, FACE_WAIT_RESET } FaceWorkState_e;
static FaceWorkState_e g_face_state = FACE_IDLE;
static uint32_t g_face_delay_tick = 0;

static void Task_HMI(void) {
#define HMI_RX_MAX_LEN 64
    static uint8_t hmi_rx_frame[HMI_RX_MAX_LEN];
    static uint16_t hmi_rx_idx = 0;
    static uint32_t hmi_rx_idle_tick = 0;

    if (UartHmi.rx_complete) {
        UartHmi.rx_complete = false;
        uint8_t c = UartHmi.rx_byte;
        if (c >= 0x20 && c <= 0x7E) {
            if (hmi_rx_idx == 0 && c == 'p') { } 
            else if (hmi_rx_idx < (HMI_RX_MAX_LEN - 1)) { hmi_rx_frame[hmi_rx_idx++] = c; }
        }
        hmi_rx_idle_tick = 0; Uart_StartRx(&UartHmi);
    }

    if (hmi_rx_idx > 0) {
        hmi_rx_idle_tick++;
        if (hmi_rx_idle_tick > 50000) {
            hmi_rx_frame[hmi_rx_idx] = '\0';
            Uart_Printf(&UartPc, "[Status] Screen CMD: %s\r\n", hmi_rx_frame);

            if (strcmp((char *) hmi_rx_frame, "START") == 0) {
                static const uint8_t reset_cmd[] = {0xEF, 0xAA, 0x10, 0x00, 0x00, 0x10};
                Uart_Send(&UartFace, (uint8_t *) reset_cmd, sizeof(reset_cmd));
                g_face_state = FACE_WAIT_RESET; g_face_delay_tick = 0;
            }
                        // ... 前面的代码保持不变 ...
            else if (strcmp((char *) hmi_rx_frame, "zhuce") == 0) {
                static const uint8_t enroll_cmd[] = {0xEF, 0xAA, 0x1D, 0x00, 0x00, 0x1D};
                Uart_Send(&UartFace, (uint8_t *) enroll_cmd, sizeof(enroll_cmd));
            }
            // ================= 【微调这里】 =================
            else if (strcmp((char *) hmi_rx_frame, "kaishi") == 0) {
                Uart_Printf(&UartPc, "[HMI Manual] Sending kaishi...\r\n");
                char *cmd = "kaishi\r\n"; 
                Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd)); 
                // 同步更新大脑状态，告诉它现在窗户是开着的
                g_farm_data.roof_servo = true; 
            }
            else if (strcmp((char *) hmi_rx_frame, "tingzhi") == 0) {
                Uart_Printf(&UartPc, "[HMI Manual] Sending tingzhi...\r\n");
                char *cmd = "tingzhi\r\n";
                Uart_Send(&Uart8, (uint8_t *)cmd, strlen(cmd));
                // 同步更新大脑状态，告诉它现在窗户是关着的
                g_farm_data.roof_servo = false; 
            }
            // ===============================================
            hmi_rx_idx = 0; hmi_rx_idle_tick = 0;
        }
    }
}

// --- 任务 4：人脸识别交互 ---
static void Task_Face(void) {
#define SCI9_RX_MAX_LEN 256
    static uint8_t sci9_rx_frame[SCI9_RX_MAX_LEN];
    static uint16_t sci9_rx_idx = 0;
    static uint32_t sci9_rx_idle_tick = 0;

    if (g_face_state == FACE_WAIT_RESET) {
        g_face_delay_tick++;
        if (g_face_delay_tick >= 2000000) {
            static const uint8_t reco_cmd[] = {0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x05, 0x15};
            Uart_Send(&UartFace, (uint8_t *) reco_cmd, sizeof(reco_cmd));
            g_face_state = FACE_IDLE;
        }
    }

    if (UartFace.rx_complete) {
        UartFace.rx_complete = false;
        if (sci9_rx_idx < SCI9_RX_MAX_LEN) sci9_rx_frame[sci9_rx_idx++] = UartFace.rx_byte;
        sci9_rx_idle_tick = 0; Uart_StartRx(&UartFace);
    }

    if (sci9_rx_idx > 0) {
        sci9_rx_idle_tick++;
        if (sci9_rx_idle_tick > 100000) {
            for (int i = 0; i <= (int)sci9_rx_idx - 9; i++) {
                if (sci9_rx_frame[i] == 0xEF && sci9_rx_frame[i + 1] == 0xAA) {
                    uint8_t cmd_type = sci9_rx_frame[i + 5];
                    if (cmd_type == 0x12) {
                        uint8_t status = sci9_rx_frame[i + 6];
                        if (status == 0x00) Hmi_SendCmd("t9.txt=\"识别成功！\"");
                        else Hmi_SendCmd("t9.txt=\"识别失败！\"");
                        break;
                    }
                    else if (cmd_type == 0x1D) {
                        uint8_t status = sci9_rx_frame[i + 6];
                        if (status == 0x00) Hmi_SendCmd("t10.txt=\"注册成功！\"");
                        else Hmi_SendCmd("t10.txt=\"注册失败！\"");
                        break;
                    }
                }
            }
            sci9_rx_idx = 0; sci9_rx_idle_tick = 0;
        }
    }
}

// --- 任务 5：LoRa 数据解析 (Soil & Rain) ---
static void Task_Uart8(void) {
    #define UART8_RX_MAX_LEN 512
    static uint8_t rx_frame[UART8_RX_MAX_LEN];
    static uint16_t rx_idx = 0;
    static uint32_t rx_idle_tick = 0;

    if (Uart8.rx_complete) {
        Uart8.rx_complete = false;
        if (rx_idx < UART8_RX_MAX_LEN - 1) rx_frame[rx_idx++] = Uart8.rx_byte;
        rx_idle_tick = 0; Uart_StartRx(&Uart8);
    }

    if (rx_idx > 0) {
        rx_idle_tick++;
        if (rx_idle_tick > 50000) {
            rx_frame[rx_idx] = '\0'; 
            
            float soil_val = 0.0f; float rain_val = 0.0f; char cmd_buf[64];

            char *p_soil = strstr((char *)rx_frame, "Soil:");
            if (p_soil != NULL && sscanf(p_soil + 5, "%f", &soil_val) == 1) {
                // 【写入数据库】
                g_farm_data.soil_moisture = soil_val;

                int soil_int = (int)soil_val; int soil_dec = (int)(soil_val * 10) % 10;
                snprintf(cmd_buf, sizeof(cmd_buf), "t14.txt=\"%d.%d\"", soil_int, soil_dec);
                Hmi_SendCmd(cmd_buf);
                R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
            }

            char *p_rain = strstr((char *)rx_frame, "Rain:");
            if (p_rain != NULL && sscanf(p_rain + 5, "%f", &rain_val) == 1) {
                // 【写入数据库】
                g_farm_data.rain_level = rain_val;

                int rain_int = (int)rain_val; int rain_dec = (int)(rain_val * 10) % 10;
                snprintf(cmd_buf, sizeof(cmd_buf), "t15.txt=\"%d.%d\"", rain_int, rain_dec);
                Hmi_SendCmd(cmd_buf);
            }
            rx_idx = 0; rx_idle_tick = 0;
        }
    }
}

// --- 任务 6：BH1750 光照 ---
static void Task_Light(void) {
    static uint32_t light_tick = 0;
    light_tick++;
    if (light_tick >= 10000000) {
        float lux = 0.0f;
        if (FSP_SUCCESS == Bh1750_ReadLux(&g_bh1750, &lux)) {
            // 【写入数据库】
            g_farm_data.light_lux = lux;

            char cmd[64];
            snprintf(cmd, sizeof(cmd), "t8.txt=\"%d\"", (int)lux);
            Hmi_SendCmd(cmd);
        }
        light_tick = 0;
    }
}

// ====================================================================================
// 【新增任务】任务 7：全局数据分析大脑 (定时执行)
// ====================================================================================
static void Task_Analysis(void) {
    static uint32_t analysis_tick = 0;
    analysis_tick++;
    
    // 大约每隔 5~10 秒执行一次全局数据快照和分析
    if (analysis_tick >= 3000000) {
        Farm_Data_Analysis();
        analysis_tick = 0;
    }
}

/* ====================================================================================
 * 【4】大老板总办公室
 * ==================================================================================== */
void hal_entry(void)
{
    Uart_Init(&UartSensor); Uart_Init(&UartHmi); Uart_Init(&UartPc);
    Uart_Init(&UartFace); Uart_Init(&Uart8);
    Uart_StartRx(&UartSensor); Uart_StartRx(&UartHmi); Uart_StartRx(&UartFace); Uart_StartRx(&Uart8);

    /* 初始化数据库 */
    Farm_Data_Init();

    fsp_err_t err = g_i2c5.p_api->open(g_i2c5.p_ctrl, g_i2c5.p_cfg);
    if (FSP_SUCCESS == err) {
        err = Bh1750_Init(&g_bh1750, &g_i2c5, 0x23);
        if (FSP_SUCCESS == err) R_BSP_SoftwareDelay(180, BSP_DELAY_UNITS_MILLISECONDS);
    }
    
    Uart_Printf(&UartPc, "\r\n=== System Boot OK (Brain Mode Activated)! ===\r\n");
    R_BSP_SoftwareDelay(2000, BSP_DELAY_UNITS_MILLISECONDS);

    while (1)
    {
        Task_DHT22();
        Task_CO2();
        Task_HMI();
        Task_Face();
        Task_Uart8();
        Task_Light();
        
        // 【调用大脑分析任务】
        Task_Analysis();
    }
}
/*
 * @Author: xingnian
 * @Date: 2025-11-10
 * @Description: 应用管理器 - 处理设备业务逻辑
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "esp_err.h"

/**
 * @brief 初始化应用管理器
 * 
 * @return esp_err_t 
 */
esp_err_t app_manager_init(void);

/**
 * @brief WiFi连接成功回调
 * 
 * 当WiFi连接成功后，启动IoT管理器
 */
void app_on_wifi_connected(void);

/**
 * @brief 启动数据上报任务
 * 
 * 定时上报设备数据到后台
 */
void app_start_report_task(void);

#endif // APP_MANAGER_H


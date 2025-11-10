/*
 * @Author: xingnian
 * @Date: 2025-11-10
 * @Description: 应用管理器实现
 */

#include "app_manager.h"
#include "iot_manager.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

static const char *TAG = "app_manager";

// 设备配置 - 根据实际情况修改
#define APP_DEVICE_ID       "ESP32_001"
#define APP_DEVICE_NAME     "ESP32智能设备"
#define APP_DEVICE_TYPE     "sensor"

// 数据上报间隔（秒）
#define REPORT_INTERVAL_SEC 30

/**
 * @brief MQTT数据接收回调
 * 
 * 处理从后台服务器接收到的命令
 */
static void app_mqtt_data_callback(const char *topic, int topic_len,
                                   const char *data, int data_len)
{
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    ESP_LOGI(TAG, "📥 收到MQTT消息");
    ESP_LOGI(TAG, "主题: %.*s", topic_len, topic);
    ESP_LOGI(TAG, "数据: %.*s", data_len, data);
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // 检查是否是命令主题（使用静态缓冲区）
    static char expected_topic[128];
    snprintf(expected_topic, sizeof(expected_topic), "device/%s/command", APP_DEVICE_ID);
    
    if (strncmp(topic, expected_topic, topic_len) == 0) {
        // 复制数据用于解析
        char *data_copy = strndup(data, data_len);
        if (!data_copy) {
            ESP_LOGE(TAG, "内存分配失败");
            return;
        }
        
        // 解析JSON命令
        cJSON *root = cJSON_Parse(data_copy);
        if (root) {
            cJSON *cmd = cJSON_GetObjectItem(root, "command");
            cJSON *params __attribute__((unused)) = cJSON_GetObjectItem(root, "params");
            
            if (cmd && cJSON_IsString(cmd)) {
                const char *command = cmd->valuestring;
                ESP_LOGI(TAG, "📋 执行命令: %s", command);
                
                // 处理不同的命令
                if (strcmp(command, "get_status") == 0) {
                    // 立即上报状态
                    ESP_LOGI(TAG, "✅ 执行: 获取状态");
                    
                    // 构建状态JSON
                    cJSON *status = cJSON_CreateObject();
                    cJSON_AddStringToObject(status, "device_id", APP_DEVICE_ID);
                    cJSON_AddStringToObject(status, "status", "online");
                    cJSON_AddNumberToObject(status, "uptime", esp_timer_get_time() / 1000000);
                    cJSON_AddNumberToObject(status, "free_heap", esp_get_free_heap_size());
                    
                    char *status_str = cJSON_PrintUnformatted(status);
                    if (status_str) {
                        iot_manager_report_status(status_str);
                        free(status_str);
                    }
                    cJSON_Delete(status);
                    
                } else if (strcmp(command, "restart") == 0) {
                    ESP_LOGW(TAG, "⚠️  收到重启命令，3秒后重启...");
                    vTaskDelay(pdMS_TO_TICKS(3000));
                    esp_restart();
                    
                } else if (strcmp(command, "test") == 0) {
                    ESP_LOGI(TAG, "✅ 执行: 测试命令");
                    // 测试响应
                    
                } else {
                    ESP_LOGW(TAG, "⚠️  未知命令: %s", command);
                }
            }
            
            cJSON_Delete(root);
        } else {
            ESP_LOGE(TAG, "JSON解析失败");
        }
        
        free(data_copy);
    }
}

/**
 * @brief WiFi连接成功后的处理
 */
void app_on_wifi_connected(void)
{
    ESP_LOGI(TAG, "WiFi已连接，启动IoT管理器...");
    
    // 配置IoT管理器
    iot_manager_config_t config = {
        .device_id = APP_DEVICE_ID,
        .device_name = APP_DEVICE_NAME,
        .device_type = APP_DEVICE_TYPE,
        .data_cb = app_mqtt_data_callback
    };
    
    // 初始化IoT管理器
    esp_err_t ret = iot_manager_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IoT管理器初始化失败: %s", esp_err_to_name(ret));
        return;
    }
    
    // 启动IoT管理器
    ret = iot_manager_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IoT管理器启动失败: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "✅ IoT管理器启动成功");
    
    // 启动数据上报任务
    app_start_report_task();
}

/**
 * @brief 数据上报任务
 */
static void report_task(void *pvParameters)
{
    int report_count = 0;
    
    ESP_LOGI(TAG, "数据上报任务已启动");
    
    while (1) {
        // 等待MQTT连接
        if (iot_manager_is_connected()) {
            // 构建设备数据JSON
            cJSON *data = cJSON_CreateObject();
            if (data) {
                cJSON_AddStringToObject(data, "device_id", APP_DEVICE_ID);
                cJSON_AddNumberToObject(data, "timestamp", esp_timer_get_time() / 1000);
                cJSON_AddNumberToObject(data, "uptime", esp_timer_get_time() / 1000000);
                cJSON_AddNumberToObject(data, "free_heap", esp_get_free_heap_size());
                cJSON_AddNumberToObject(data, "report_count", report_count++);
                
                // 这里可以添加你的传感器数据
                // cJSON_AddNumberToObject(data, "temperature", get_temperature());
                // cJSON_AddNumberToObject(data, "humidity", get_humidity());
                
                char *data_str = cJSON_PrintUnformatted(data);
                if (data_str) {
                    int msg_id = iot_manager_report_properties(data_str);
                    if (msg_id >= 0) {
                        ESP_LOGI(TAG, "📤 数据上报成功 #%d (msg_id=%d)", report_count, msg_id);
                    } else {
                        ESP_LOGW(TAG, "数据上报失败");
                    }
                    free(data_str);
                }
                
                cJSON_Delete(data);
            }
        } else {
            ESP_LOGD(TAG, "等待MQTT连接...");
        }
        
        // 等待下一次上报
        vTaskDelay(pdMS_TO_TICKS(REPORT_INTERVAL_SEC * 1000));
    }
}

/**
 * @brief 启动数据上报任务
 */
void app_start_report_task(void)
{
    // 增加栈大小到6KB，避免栈溢出
    xTaskCreate(report_task, "report_task", 6144, NULL, 5, NULL);
    ESP_LOGI(TAG, "数据上报任务已创建（间隔: %d秒）", REPORT_INTERVAL_SEC);
}

/**
 * @brief 初始化应用管理器
 */
esp_err_t app_manager_init(void)
{
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    ESP_LOGI(TAG, "  应用管理器初始化");
    ESP_LOGI(TAG, "  设备ID: %s", APP_DEVICE_ID);
    ESP_LOGI(TAG, "  设备名称: %s", APP_DEVICE_NAME);
    ESP_LOGI(TAG, "  设备类型: %s", APP_DEVICE_TYPE);
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    return ESP_OK;
}


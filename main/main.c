/*
 * @Author: jxingnian j_xingnian@163.com
 * @Date: 2025-01-01 11:27:58
 * @LastEditors: xingnina j_xingnian@163.com
 * @LastEditTime: 2025-11-10
 * @FilePath: \esp_mqtt\main\main.c
 * @Description: WiFi配网主程序
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "wifi_manager.h"
#include "http_server.h"
#include "app/app_manager.h"

static const char *TAG = "main";

// 初始化SPIFFS
static esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,   // 最大打开文件数
        .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}

void app_main(void)
{    
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    ESP_LOGI(TAG, "  ESP32 IoT设备管理系统");
    ESP_LOGI(TAG, "  Author: 星年");
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "✅ NVS初始化完成");

    // 初始化SPIFFS
    ESP_ERROR_CHECK(init_spiffs());
    ESP_LOGI(TAG, "✅ SPIFFS初始化完成");

    // 初始化应用管理器
    ESP_ERROR_CHECK(app_manager_init());
    
    // 设置WiFi连接成功回调
    wifi_set_connected_callback(app_on_wifi_connected);
    
    // 初始化并启动WiFi AP
    ESP_LOGI(TAG, "启动WiFi (AP+STA模式)");
    ESP_ERROR_CHECK(wifi_init_softap());

    // 启动HTTP服务器
    ESP_ERROR_CHECK(start_webserver());
    
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    ESP_LOGI(TAG, "  系统初始化完成");
    ESP_LOGI(TAG, "  Web配置: http://192.168.4.1:8080");
    ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
}
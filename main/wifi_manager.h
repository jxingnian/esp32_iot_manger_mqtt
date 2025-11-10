#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_wifi.h"
#include "esp_event.h"

// WiFi连接成功回调函数类型
typedef void (*wifi_connected_callback_t)(void);

// WiFi初始化函数
esp_err_t wifi_init_softap(void);

// WiFi扫描函数
esp_err_t wifi_scan_networks(wifi_ap_record_t **ap_records, uint16_t *ap_count);

// 设置WiFi连接成功回调函数
void wifi_set_connected_callback(wifi_connected_callback_t callback);

#endif // WIFI_MANAGER_H

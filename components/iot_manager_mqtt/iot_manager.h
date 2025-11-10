/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-11-10
 * @Description: IoT管理组件 - MQTT通信模块
 * 
 * 这是一个通用的IoT设备管理组件，用于通过MQTT协议
 * 与后台服务器进行数据通信，支持数据上报和命令下发。
 */

#ifndef IOT_MANAGER_H
#define IOT_MANAGER_H

#include "esp_err.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MQTT消息回调函数类型
 * 
 * @param topic 消息主题
 * @param topic_len 主题长度
 * @param data 消息数据
 * @param data_len 数据长度
 */
typedef void (*iot_mqtt_data_callback_t)(const char *topic, int topic_len, 
                                          const char *data, int data_len);

/**
 * @brief IoT管理器配置结构
 */
typedef struct {
    const char *device_id;              ///< 设备唯一ID
    const char *device_name;            ///< 设备名称
    const char *device_type;            ///< 设备类型
    iot_mqtt_data_callback_t data_cb;   ///< 数据接收回调函数
} iot_manager_config_t;

/**
 * @brief 初始化IoT管理器
 * 
 * 初始化MQTT客户端并连接到服务器
 * 
 * @param config 配置参数
 * @return esp_err_t 
 *         - ESP_OK: 成功
 *         - ESP_FAIL: 失败
 */
esp_err_t iot_manager_init(const iot_manager_config_t *config);

/**
 * @brief 启动IoT管理器
 * 
 * 启动MQTT客户端，建立连接
 * 
 * @return esp_err_t 
 *         - ESP_OK: 成功
 *         - ESP_FAIL: 失败
 */
esp_err_t iot_manager_start(void);

/**
 * @brief 停止IoT管理器
 * 
 * 断开MQTT连接并停止客户端
 * 
 * @return esp_err_t 
 */
esp_err_t iot_manager_stop(void);

/**
 * @brief 发布数据到指定主题
 * 
 * @param topic 目标主题
 * @param data 数据内容
 * @param len 数据长度（0表示自动计算字符串长度）
 * @param qos QoS级别 (0, 1, 2)
 * @param retain 是否保留消息
 * @return int 消息ID，失败返回-1
 */
int iot_manager_publish(const char *topic, const char *data, int len, int qos, int retain);

/**
 * @brief 订阅主题
 * 
 * @param topic 要订阅的主题
 * @param qos QoS级别 (0, 1, 2)
 * @return int 消息ID，失败返回-1
 */
int iot_manager_subscribe(const char *topic, int qos);

/**
 * @brief 取消订阅主题
 * 
 * @param topic 要取消订阅的主题
 * @return int 消息ID，失败返回-1
 */
int iot_manager_unsubscribe(const char *topic);

/**
 * @brief 获取MQTT客户端句柄
 * 
 * @return esp_mqtt_client_handle_t 客户端句柄
 */
esp_mqtt_client_handle_t iot_manager_get_client(void);

/**
 * @brief 检查MQTT是否已连接
 * 
 * @return true 已连接
 * @return false 未连接
 */
bool iot_manager_is_connected(void);

/**
 * @brief 上报设备状态到后台
 * 
 * 使用JSON格式上报设备状态
 * 
 * @param status_json JSON格式的状态数据
 * @return int 消息ID，失败返回-1
 */
int iot_manager_report_status(const char *status_json);

/**
 * @brief 上报设备属性
 * 
 * @param properties_json JSON格式的属性数据
 * @return int 消息ID，失败返回-1
 */
int iot_manager_report_properties(const char *properties_json);

/**
 * @brief 响应命令执行结果
 * 
 * @param command_id 命令ID
 * @param result 执行结果 (0: 成功, 其他: 失败码)
 * @param message 结果消息
 * @return int 消息ID，失败返回-1
 */
int iot_manager_reply_command(const char *command_id, int result, const char *message);

#ifdef __cplusplus
}
#endif

#endif // IOT_MANAGER_H


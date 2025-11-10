/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-11-10
 * @Description: IoT管理组件实现 - MQTT通信模块
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "iot_manager.h"

static const char *TAG = "IOT_MANAGER";

// MQTT客户端句柄
static esp_mqtt_client_handle_t mqtt_client = NULL;

// 配置信息
static iot_manager_config_t manager_config = {0};

// 连接状态
static bool is_connected = false;

// 用户数据回调函数
static iot_mqtt_data_callback_t user_data_callback = NULL;

/**
 * @brief 记录错误信息
 */
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

#if CONFIG_IOT_MQTT_PROTOCOL_V5
/**
 * @brief 打印MQTT5用户属性
 */
static void print_user_property(mqtt5_user_property_handle_t user_property)
{
    if (user_property) {
        uint8_t count = esp_mqtt5_client_get_user_property_count(user_property);
        if (count) {
            esp_mqtt5_user_property_item_t *item = malloc(count * sizeof(esp_mqtt5_user_property_item_t));
            if (item && esp_mqtt5_client_get_user_property(user_property, item, &count) == ESP_OK) {
                for (int i = 0; i < count; i++) {
                    esp_mqtt5_user_property_item_t *t = &item[i];
                    ESP_LOGI(TAG, "User property: %s = %s", t->key, t->value);
                    free((char *)t->key);
                    free((char *)t->value);
                }
            }
            free(item);
        }
    }
}
#endif

/**
 * @brief MQTT事件处理函数
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                               int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event: base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client __attribute__((unused)) = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT已连接到服务器");
        is_connected = true;
        
        // 自动订阅命令主题（使用动态内存）
        char *command_topic = malloc(128);
        if (command_topic) {
            snprintf(command_topic, 128, 
                    CONFIG_IOT_COMMAND_TOPIC_TEMPLATE, manager_config.device_id);
            iot_manager_subscribe(command_topic, 1);
            ESP_LOGI(TAG, "已订阅命令主题: %s", command_topic);
            free(command_topic);
        }
        
        // 上报设备上线消息（使用动态内存）
        char *online_msg = malloc(256);
        if (online_msg) {
            snprintf(online_msg, 256, 
                    "{\"device_id\":\"%s\",\"status\":\"online\",\"timestamp\":%lld}",
                    manager_config.device_id, esp_timer_get_time() / 1000);
            iot_manager_report_status(online_msg);
            free(online_msg);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT连接断开");
        is_connected = false;
#if CONFIG_IOT_MQTT_PROTOCOL_V5
        print_user_property(event->property->user_property);
#endif
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "订阅成功, msg_id=%d", event->msg_id);
#if CONFIG_IOT_MQTT_PROTOCOL_V5
        print_user_property(event->property->user_property);
#endif
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "取消订阅成功, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(TAG, "消息发布成功, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "收到MQTT消息");
        ESP_LOGI(TAG, "主题: %.*s", event->topic_len, event->topic);
        ESP_LOGD(TAG, "数据: %.*s", event->data_len, event->data);
        
        // 调用用户回调函数
        if (user_data_callback) {
            user_data_callback(event->topic, event->topic_len, 
                             event->data, event->data_len);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT错误");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("esp-tls错误", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("tls栈错误", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("socket错误", event->error_handle->esp_transport_sock_errno);
        }
        break;

    default:
        ESP_LOGD(TAG, "其他事件 id:%d", event->event_id);
        break;
    }
}

/**
 * @brief 初始化IoT管理器
 */
esp_err_t iot_manager_init(const iot_manager_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    // 保存配置
    manager_config.device_id = config->device_id;
    manager_config.device_name = config->device_name;
    manager_config.device_type = config->device_type;
    user_data_callback = config->data_cb;

    ESP_LOGI(TAG, "初始化IoT管理器");
    ESP_LOGI(TAG, "设备ID: %s", manager_config.device_id);
    ESP_LOGI(TAG, "设备名称: %s", manager_config.device_name);
    ESP_LOGI(TAG, "设备类型: %s", manager_config.device_type);

    // 配置MQTT客户端
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_IOT_BROKER_URL,
#if CONFIG_IOT_MQTT_PROTOCOL_V5
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
#else
        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
#endif
        .credentials.client_id = manager_config.device_id,
    };

    // 如果配置了用户名和密码
#ifdef CONFIG_IOT_MQTT_USERNAME
    if (strlen(CONFIG_IOT_MQTT_USERNAME) > 0) {
        mqtt_cfg.credentials.username = CONFIG_IOT_MQTT_USERNAME;
    }
#endif

#ifdef CONFIG_IOT_MQTT_PASSWORD
    if (strlen(CONFIG_IOT_MQTT_PASSWORD) > 0) {
        mqtt_cfg.credentials.authentication.password = CONFIG_IOT_MQTT_PASSWORD;
    }
#endif

    // 配置遗嘱消息（使用静态内存以避免栈溢出）
    static char will_topic[128];
    static char will_message[256];
    snprintf(will_topic, sizeof(will_topic), 
            CONFIG_IOT_STATUS_TOPIC_TEMPLATE, manager_config.device_id);
    snprintf(will_message, sizeof(will_message), 
            "{\"device_id\":\"%s\",\"status\":\"offline\"}",
            manager_config.device_id);
    
    mqtt_cfg.session.last_will.topic = will_topic;
    mqtt_cfg.session.last_will.msg = will_message;
    mqtt_cfg.session.last_will.msg_len = strlen(will_message);
    mqtt_cfg.session.last_will.qos = 1;
    mqtt_cfg.session.last_will.retain = true;

    // 初始化MQTT客户端
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_client) {
        ESP_LOGE(TAG, "MQTT客户端初始化失败");
        return ESP_FAIL;
    }

    // 注册事件处理器
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, 
                                   mqtt_event_handler, NULL);

    ESP_LOGI(TAG, "IoT管理器初始化完成");
    return ESP_OK;
}

/**
 * @brief 启动IoT管理器
 */
esp_err_t iot_manager_start(void)
{
    if (!mqtt_client) {
        ESP_LOGE(TAG, "MQTT客户端未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "启动MQTT客户端...");
    esp_err_t ret = esp_mqtt_client_start(mqtt_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MQTT客户端启动失败");
        return ret;
    }

    ESP_LOGI(TAG, "MQTT客户端启动成功");
    return ESP_OK;
}

/**
 * @brief 停止IoT管理器
 */
esp_err_t iot_manager_stop(void)
{
    if (!mqtt_client) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "停止MQTT客户端...");
    esp_err_t ret = esp_mqtt_client_stop(mqtt_client);
    if (ret == ESP_OK) {
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        is_connected = false;
    }
    return ret;
}

/**
 * @brief 发布数据
 */
int iot_manager_publish(const char *topic, const char *data, int len, int qos, int retain)
{
    if (!mqtt_client || !is_connected) {
        ESP_LOGW(TAG, "MQTT未连接，无法发布消息");
        return -1;
    }

    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, data, len, qos, retain);
    if (msg_id >= 0) {
        ESP_LOGD(TAG, "发布消息到 %s, msg_id=%d", topic, msg_id);
    } else {
        ESP_LOGE(TAG, "发布消息失败");
    }
    return msg_id;
}

/**
 * @brief 订阅主题
 */
int iot_manager_subscribe(const char *topic, int qos)
{
    if (!mqtt_client || !is_connected) {
        ESP_LOGW(TAG, "MQTT未连接，无法订阅主题");
        return -1;
    }

    int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, qos);
    if (msg_id >= 0) {
        ESP_LOGI(TAG, "订阅主题 %s, msg_id=%d", topic, msg_id);
    } else {
        ESP_LOGE(TAG, "订阅主题失败");
    }
    return msg_id;
}

/**
 * @brief 取消订阅
 */
int iot_manager_unsubscribe(const char *topic)
{
    if (!mqtt_client) {
        return -1;
    }

    int msg_id = esp_mqtt_client_unsubscribe(mqtt_client, topic);
    ESP_LOGI(TAG, "取消订阅 %s, msg_id=%d", topic, msg_id);
    return msg_id;
}

/**
 * @brief 获取客户端句柄
 */
esp_mqtt_client_handle_t iot_manager_get_client(void)
{
    return mqtt_client;
}

/**
 * @brief 检查连接状态
 */
bool iot_manager_is_connected(void)
{
    return is_connected;
}

/**
 * @brief 上报设备状态
 */
int iot_manager_report_status(const char *status_json)
{
    // 使用静态缓冲区避免栈使用
    static char topic[128];
    snprintf(topic, sizeof(topic), CONFIG_IOT_STATUS_TOPIC_TEMPLATE, 
            manager_config.device_id);
    return iot_manager_publish(topic, status_json, 0, 1, 0);
}

/**
 * @brief 上报设备属性
 */
int iot_manager_report_properties(const char *properties_json)
{
    // 使用静态缓冲区避免栈使用
    static char topic[128];
    snprintf(topic, sizeof(topic), CONFIG_IOT_PROPERTY_TOPIC_TEMPLATE, 
            manager_config.device_id);
    return iot_manager_publish(topic, properties_json, 0, 1, 0);
}

/**
 * @brief 响应命令结果
 */
int iot_manager_reply_command(const char *command_id, int result, const char *message)
{
    // 使用静态缓冲区避免栈使用
    static char topic[128];
    static char reply_json[512];
    
    snprintf(topic, sizeof(topic), CONFIG_IOT_REPLY_TOPIC_TEMPLATE, 
            manager_config.device_id);
    snprintf(reply_json, sizeof(reply_json), 
            "{\"command_id\":\"%s\",\"result\":%d,\"message\":\"%s\",\"timestamp\":%lld}",
            command_id, result, message, esp_timer_get_time() / 1000);
    
    return iot_manager_publish(topic, reply_json, 0, 1, 0);
}


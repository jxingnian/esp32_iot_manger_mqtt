# IoT Manager MQTT 组件

ESP32物联网管理组件，提供MQTT通信功能，用于设备与后台服务器的双向数据交互。

**Author:** 星年 (xingnian)

---

## 📝 组件简介

`iot_manager_mqtt` 是一个通用的ESP32 IoT管理组件，基于ESP-IDF MQTT客户端封装，提供简洁的API接口用于：
- 设备与后台服务器的MQTT通信
- 设备状态和数据上报
- 接收和处理后台命令
- 灵活的主题配置

## ✨ 功能特性

- ✅ **MQTT协议支持**
  - MQTT v3.1.1 / v5
  - 自动连接和重连
  - QoS 0/1/2 支持

- ✅ **数据通信**
  - 状态上报
  - 属性/数据上报
  - 命令接收
  - 事件上报

- ✅ **灵活配置**
  - 可配置的MQTT服务器
  - 自定义主题模板
  - 用户名密码认证
  - 连接参数调整

- ✅ **回调机制**
  - 数据接收回调
  - 连接状态通知
  - 错误处理

## 📦 使用方法

### 1. 添加组件到项目

将组件目录放置在项目的 `components/` 目录下：

```
your_project/
├── components/
│   └── iot_manager_mqtt/
├── main/
└── CMakeLists.txt
```

### 2. 在 CMakeLists.txt 中添加依赖

```cmake
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES iot_manager_mqtt  # 添加组件依赖
)
```

### 3. 包含头文件

```c
#include "iot_manager.h"
```

### 4. 初始化和使用

```c
#include "iot_manager.h"

// 定义数据接收回调
void mqtt_data_callback(const char *topic, int topic_len,
                        const char *data, int data_len)
{
    ESP_LOGI("APP", "收到消息 主题: %.*s", topic_len, topic);
    ESP_LOGI("APP", "数据: %.*s", data_len, data);
    
    // 处理接收到的数据
}

void app_main(void)
{
    // WiFi连接成功后...
    
    // 配置IoT管理器
    iot_manager_config_t config = {
        .device_id = "ESP32_001",
        .device_name = "智能传感器",
        .device_type = "sensor",
        .data_cb = mqtt_data_callback
    };
    
    // 初始化
    esp_err_t ret = iot_manager_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE("APP", "初始化失败");
        return;
    }
    
    // 启动MQTT客户端
    ret = iot_manager_start();
    if (ret != ESP_OK) {
        ESP_LOGE("APP", "启动失败");
        return;
    }
    
    // 上报设备状态
    const char *status = "{\"status\":\"online\",\"temperature\":25.5}";
    iot_manager_report_status(status);
}
```

## 🔧 配置选项

### 通过 menuconfig 配置

```bash
idf.py menuconfig
```

进入：`Component config → IoT Manager Configuration`

### 配置项说明

#### MQTT服务器配置

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `IOT_BROKER_URL` | `mqtt://win.xingnian.vip:1883` | MQTT服务器地址 |
| `IOT_MQTT_USERNAME` | `esp_xiaoya_cli` | 用户名 |
| `IOT_MQTT_PASSWORD` | 已配置 | 密码 |
| `IOT_MQTT_PROTOCOL_V5` | 否 | 启用MQTT v5 |

#### 主题模板配置

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `IOT_STATUS_TOPIC_TEMPLATE` | `device/%s/status` | 状态上报主题 |
| `IOT_PROPERTY_TOPIC_TEMPLATE` | `device/%s/data` | 数据上报主题 |
| `IOT_COMMAND_TOPIC_TEMPLATE` | `device/%s/command` | 命令接收主题 |
| `IOT_REPLY_TOPIC_TEMPLATE` | `device/%s/reply` | 命令响应主题 |
| `IOT_EVENT_TOPIC_TEMPLATE` | `device/%s/event` | 事件上报主题 |

**注意**: `%s` 会被替换为 `device_id`

#### 高级设置

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `IOT_MQTT_KEEPALIVE` | 120秒 | 心跳间隔 |
| `IOT_MQTT_BUFFER_SIZE` | 4096 | 缓冲区大小 |
| `IOT_ENABLE_AUTO_RECONNECT` | 是 | 自动重连 |

## 📡 API 参考

### 初始化和控制

#### `iot_manager_init()`

初始化IoT管理器

```c
esp_err_t iot_manager_init(const iot_manager_config_t *config);
```

**参数**:
- `config`: 配置结构体指针

**返回**:
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 参数错误
- `ESP_FAIL`: 初始化失败

**配置结构体**:
```c
typedef struct {
    const char *device_id;              // 设备唯一ID (必填)
    const char *device_name;            // 设备名称 (可选)
    const char *device_type;            // 设备类型 (可选)
    iot_mqtt_data_callback_t data_cb;   // 数据回调 (必填)
} iot_manager_config_t;
```

**示例**:
```c
iot_manager_config_t config = {
    .device_id = "ESP32_001",
    .device_name = "客厅传感器",
    .device_type = "sensor",
    .data_cb = my_data_callback
};
esp_err_t ret = iot_manager_init(&config);
```

#### `iot_manager_start()`

启动MQTT客户端

```c
esp_err_t iot_manager_start(void);
```

**返回**:
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_STATE`: 未初始化
- `ESP_FAIL`: 启动失败

#### `iot_manager_stop()`

停止MQTT客户端

```c
esp_err_t iot_manager_stop(void);
```

### 数据发布

#### `iot_manager_publish()`

发布消息到指定主题

```c
int iot_manager_publish(const char *topic, const char *data, 
                        int len, int qos, int retain);
```

**参数**:
- `topic`: 目标主题
- `data`: 消息内容
- `len`: 数据长度（0表示自动计算）
- `qos`: QoS级别 (0, 1, 2)
- `retain`: 是否保留消息

**返回**: 消息ID（>=0成功，<0失败）

**示例**:
```c
int msg_id = iot_manager_publish("device/ESP32_001/data", 
                                  "{\"temp\":25.5}", 0, 1, 0);
```

#### `iot_manager_report_status()`

上报设备状态

```c
int iot_manager_report_status(const char *status_json);
```

**参数**:
- `status_json`: JSON格式的状态数据

**返回**: 消息ID

**示例**:
```c
const char *status = "{\"status\":\"online\",\"uptime\":3600}";
int msg_id = iot_manager_report_status(status);
```

#### `iot_manager_report_properties()`

上报设备属性/数据

```c
int iot_manager_report_properties(const char *properties_json);
```

**参数**:
- `properties_json`: JSON格式的属性数据

**返回**: 消息ID

**示例**:
```c
const char *data = "{\"temperature\":25.5,\"humidity\":60}";
int msg_id = iot_manager_report_properties(data);
```

#### `iot_manager_reply_command()`

响应命令执行结果

```c
int iot_manager_reply_command(const char *command_id, 
                              int result, const char *message);
```

**参数**:
- `command_id`: 命令ID
- `result`: 执行结果码 (0表示成功)
- `message`: 结果描述

**返回**: 消息ID

**示例**:
```c
iot_manager_reply_command("cmd_123", 0, "执行成功");
```

### 主题订阅

#### `iot_manager_subscribe()`

订阅MQTT主题

```c
int iot_manager_subscribe(const char *topic, int qos);
```

**参数**:
- `topic`: 要订阅的主题
- `qos`: QoS级别

**返回**: 消息ID

**示例**:
```c
iot_manager_subscribe("device/ESP32_001/custom", 1);
```

#### `iot_manager_unsubscribe()`

取消订阅

```c
int iot_manager_unsubscribe(const char *topic);
```

### 状态查询

#### `iot_manager_is_connected()`

检查MQTT连接状态

```c
bool iot_manager_is_connected(void);
```

**返回**: `true`已连接，`false`未连接

**示例**:
```c
if (iot_manager_is_connected()) {
    iot_manager_report_status(status);
}
```

#### `iot_manager_get_client()`

获取MQTT客户端句柄

```c
esp_mqtt_client_handle_t iot_manager_get_client(void);
```

**返回**: MQTT客户端句柄

## 📋 使用示例

### 完整示例

```c
#include "iot_manager.h"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "APP";

// 数据接收回调
void mqtt_callback(const char *topic, int topic_len,
                   const char *data, int data_len)
{
    char *data_str = strndup(data, data_len);
    cJSON *root = cJSON_Parse(data_str);
    
    if (root) {
        cJSON *cmd = cJSON_GetObjectItem(root, "command");
        if (cmd && cJSON_IsString(cmd)) {
            if (strcmp(cmd->valuestring, "get_status") == 0) {
                // 处理获取状态命令
                const char *status = "{\"status\":\"ok\"}";
                iot_manager_report_status(status);
            }
        }
        cJSON_Delete(root);
    }
    free(data_str);
}

void app_main(void)
{
    // 初始化配置
    iot_manager_config_t config = {
        .device_id = "ESP32_001",
        .device_name = "测试设备",
        .device_type = "sensor",
        .data_cb = mqtt_callback
    };
    
    // 初始化和启动
    if (iot_manager_init(&config) == ESP_OK) {
        if (iot_manager_start() == ESP_OK) {
            ESP_LOGI(TAG, "IoT管理器启动成功");
        }
    }
}
```

### 定时上报示例

```c
void report_task(void *param)
{
    while (1) {
        if (iot_manager_is_connected()) {
            cJSON *data = cJSON_CreateObject();
            cJSON_AddNumberToObject(data, "temperature", 25.5);
            cJSON_AddNumberToObject(data, "humidity", 60);
            
            char *json_str = cJSON_PrintUnformatted(data);
            iot_manager_report_properties(json_str);
            
            free(json_str);
            cJSON_Delete(data);
        }
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30秒
    }
}

// 创建任务
xTaskCreate(report_task, "report", 4096, NULL, 5, NULL);
```

## 🔌 与后台系统对接

### 主题规则

组件使用以下主题模板与后台通信：

```
设备 → 后台:
  device/{device_id}/status    - 状态上报
  device/{device_id}/data      - 数据上报
  
后台 → 设备:
  device/{device_id}/command   - 命令下发
```

### 消息格式

#### 设备上报 (JSON)

```json
{
  "device_id": "ESP32_001",
  "temperature": 25.5,
  "humidity": 60,
  "timestamp": 1699999999
}
```

#### 命令下发 (JSON)

```json
{
  "command": "get_status",
  "params": {
    "key": "value"
  }
}
```

## ⚠️ 注意事项

1. **初始化顺序**
   - 必须在WiFi连接成功后才能启动MQTT
   - 先调用 `iot_manager_init()` 再调用 `iot_manager_start()`

2. **设备ID**
   - 每个设备的 `device_id` 必须唯一
   - 建议使用MAC地址或序列号作为设备ID

3. **内存管理**
   - 在数据回调中处理的数据需要及时释放
   - JSON字符串使用后要 `free()`

4. **线程安全**
   - API调用是线程安全的
   - 回调函数在MQTT事件任务中执行

5. **QoS选择**
   - QoS 0: 最多一次传输，性能最好
   - QoS 1: 至少一次传输，适合一般数据
   - QoS 2: 仅一次传输，开销最大

## 🐛 故障排查

### 连接失败

**检查项**:
1. MQTT服务器地址和端口是否正确
2. 用户名和密码是否正确
3. 网络连接是否正常
4. 防火墙是否允许连接

**日志示例**:
```
E (12345) IOT_MANAGER: MQTT连接失败
```

### 发布失败

**原因**:
- 未连接到MQTT服务器
- 消息太大超过缓冲区
- QoS设置不正确

**解决方法**:
```c
if (iot_manager_is_connected()) {
    int msg_id = iot_manager_publish(...);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "发布失败");
    }
}
```

### 收不到消息

**检查项**:
1. 是否已订阅对应主题
2. 回调函数是否正确注册
3. 主题格式是否匹配

## 📊 性能指标

- **初始化时间**: <100ms
- **连接建立**: ~2秒
- **消息发送延迟**: <50ms
- **内存占用**: ~30KB
- **支持并发**: 多线程安全

## 🔄 版本历史

- **v1.0.0** - 初始版本
  - 基础MQTT通信功能
  - 支持MQTT v3.1.1/v5
  - 数据上报和命令接收

## 📄 许可证

MIT License

## 👨‍💻 作者

星年 (xingnian) - j_xingnian@163.com

---

**Happy Coding!** 🚀


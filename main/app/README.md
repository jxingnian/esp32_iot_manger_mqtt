# App应用层

## 文件说明

```
app/
├── app_config.h      # 应用配置（设备ID、上报间隔等）
├── app_manager.h     # 应用管理器接口
├── app_manager.c     # 应用管理器实现
└── README.md         # 本文档
```

## 配置设备

### 1. 修改设备信息

编辑 `app_config.h` 或 `app_manager.c`:

```c
#define APP_DEVICE_ID       "ESP32_001"      // 设备唯一ID
#define APP_DEVICE_NAME     "ESP32智能设备"   // 设备名称
#define APP_DEVICE_TYPE     "sensor"         // 设备类型
```

### 2. 修改上报间隔

```c
#define APP_REPORT_INTERVAL_SEC     30       // 30秒上报一次
```

## MQTT主题说明

后台系统使用的主题规则：

| 功能 | 主题 | 说明 |
|------|------|------|
| 设备上报状态 | `device/{device_id}/status` | ESP32 → 后台 |
| 设备上报数据 | `device/{device_id}/data` | ESP32 → 后台 |
| 接收命令 | `device/{device_id}/command` | 后台 → ESP32 |

示例（设备ID为ESP32_001）：
- 上报状态：`device/ESP32_001/status`
- 上报数据：`device/ESP32_001/data`
- 接收命令：`device/ESP32_001/command`

## 添加传感器数据

在 `app_manager.c` 的 `report_task()` 函数中添加：

```c
// 构建设备数据JSON
cJSON *data = cJSON_CreateObject();
cJSON_AddStringToObject(data, "device_id", APP_DEVICE_ID);

// 添加你的传感器数据
cJSON_AddNumberToObject(data, "temperature", get_temperature());
cJSON_AddNumberToObject(data, "humidity", get_humidity());
cJSON_AddNumberToObject(data, "pressure", get_pressure());

char *data_str = cJSON_PrintUnformatted(data);
iot_manager_report_properties(data_str);
free(data_str);
cJSON_Delete(data);
```

## 处理后台命令

在 `app_mqtt_data_callback()` 函数中添加命令处理：

```c
if (strcmp(command, "your_command") == 0) {
    ESP_LOGI(TAG, "执行你的命令");
    // 你的处理代码
}
```

## 与后台系统对接

### 1. 配置MQTT连接

在 `menuconfig` 中配置：
```
Component config → IoT Manager Configuration
```

或直接使用默认配置：
- 服务器：`win.xingnian.vip:1883`
- 用户名：`esp_xiaoya_cli`
- 密码：已配置

### 2. 后台查看数据

访问后台管理系统：
- 设备会自动注册
- 查看实时数据
- 发送命令测试

## 编译和烧录

```bash
# 配置
idf.py menuconfig

# 编译
idf.py build

# 烧录
idf.py flash monitor
```

## 注意事项

1. 每个设备的 `APP_DEVICE_ID` 必须唯一
2. 修改设备ID后需要重新编译
3. 确保后台MQTT服务器地址正确
4. 首次使用需要通过Web配置WiFi


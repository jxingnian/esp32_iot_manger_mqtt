# ESP32 IoT设备管理系统

基于ESP32-S3的物联网设备管理系统，支持WiFi配网、MQTT通信和设备数据上报。

**Author:** 星年 (xingnian)

---

## 📋 项目简介

这是一个完整的ESP32物联网解决方案，包含：
- **WiFi配网**：AP+STA双模式，Web配置界面
- **MQTT通信**：与后台服务器双向通信
- **设备管理**：自动注册、状态上报、命令接收
- **组件化设计**：模块化、易扩展、可复用

## ✨ 功能特性

- 🌐 **WiFi管理**
  - AP热点模式（用于配网）
  - STA模式（连接路由器）
  - 配置持久化存储
  - 自动重连机制

- 📡 **MQTT通信**
  - MQTT v3.1.1 / v5 支持
  - 自动连接和重连
  - 双向数据通信
  - 灵活的主题配置

- 🖥️ **Web配置**
  - 响应式设计界面
  - WiFi扫描和连接
  - 设备状态查看
  - 配置管理

- 🔧 **设备功能**
  - 自动注册到后台
  - 定时数据上报
  - 命令接收和处理
  - 状态监控

## 📁 项目结构

```
esp_mqtt/
├── components/
│   └── iot_manager_mqtt/          # IoT管理组件
│       ├── iot_manager.c          # MQTT通信实现
│       ├── iot_manager.h          # API接口
│       ├── Kconfig                # 组件配置
│       └── CMakeLists.txt
├── main/
│   ├── app/                       # 应用层
│   │   ├── app_manager.c          # 应用管理器
│   │   ├── app_manager.h          
│   │   ├── app_config.h           # 应用配置
│   │   └── README.md              # 应用层说明
│   ├── main.c                     # 主程序入口
│   ├── wifi_manager.c/h           # WiFi管理
│   ├── http_server.c/h            # HTTP服务器
│   └── CMakeLists.txt
├── spiffs/
│   └── index.html                 # Web配置页面
├── partitions.csv                 # 分区表
├── sdkconfig.defaults             # 默认配置
└── README.md                      # 本文档
```

## 🚀 快速开始

### 1. 环境准备

- **ESP-IDF**: v5.0 或更高版本
- **芯片**: ESP32-S3
- **Flash**: 8MB 或更大

### 2. 克隆项目

```bash
git clone <仓库地址>
cd esp_mqtt
```

### 3. 配置项目

#### 修改设备信息

编辑 `main/app/app_manager.c`:

```c
#define APP_DEVICE_ID       "ESP32_001"      // 修改为你的设备ID（必须唯一）
#define APP_DEVICE_NAME     "ESP32智能设备"   // 设备名称
#define APP_DEVICE_TYPE     "sensor"         // 设备类型
```

#### 配置MQTT连接（可选）

```bash
idf.py menuconfig
```

进入：`Component config → IoT Manager Configuration`

默认配置（已配置好）：
- **服务器**: `win.xingnian.vip:1883`
- **用户名**: `esp_xiaoya_cli`
- **密码**: 已配置

### 4. 编译和烧录

```bash
# 编译
idf.py build

# 烧录
idf.py flash

# 监控日志
idf.py monitor
```

### 5. WiFi配网

首次使用或WiFi配置失败时：

1. ESP32启动后创建WiFi热点：`ESP32-ConfigAP`
2. 手机/电脑连接该热点（密码：`12345678`）
3. 浏览器访问：`http://192.168.4.1:8080`
4. 扫描并连接到你的WiFi网络
5. 设备自动连接MQTT服务器

### 6. 查看设备

访问后台管理系统，查看设备自动注册和数据上报！

## ⚙️ 配置说明

### MQTT主题规则

与后台系统对接的主题格式：

| 功能 | 主题模板 | 说明 |
|------|----------|------|
| 设备上报状态 | `device/{device_id}/status` | 在线/离线状态 |
| 设备上报数据 | `device/{device_id}/data` | 传感器数据 |
| 接收命令 | `device/{device_id}/command` | 后台下发命令 |

示例（设备ID为 `ESP32_001`）：
- 状态主题: `device/ESP32_001/status`
- 数据主题: `device/ESP32_001/data`
- 命令主题: `device/ESP32_001/command`

### WiFi AP配置

编辑 `main/Kconfig.projbuild` 或使用 `menuconfig`:

```
WiFi Configuration
├─ WiFi SSID (AP Mode): ESP32-ConfigAP
├─ WiFi Password (AP Mode): 12345678
├─ WiFi Channel: 1
└─ Maximal STA connections: 4
```

### 数据上报间隔

编辑 `main/app/app_manager.c`:

```c
#define REPORT_INTERVAL_SEC 30    // 30秒上报一次
```

## 🔌 后台系统对接

### 后台系统信息

- **MQTT服务器**: `win.xingnian.vip:1883`
- **服务器端用户**: `server_client`
- **设备端用户**: `esp_xiaoya_cli`

### 设备注册

设备连接MQTT后会自动发送上线消息，后台自动注册设备。

### 数据格式

#### 设备上报数据 (JSON)

```json
{
  "device_id": "ESP32_001",
  "timestamp": 1699999999,
  "uptime": 3600,
  "free_heap": 123456,
  "report_count": 10
}
```

#### 后台下发命令 (JSON)

```json
{
  "command": "get_status",
  "params": {}
}
```

支持的命令：
- `get_status` - 获取设备状态
- `restart` - 重启设备
- `test` - 测试命令

## 📝 开发指南

### 添加传感器数据

在 `main/app/app_manager.c` 的 `report_task()` 函数中：

```c
// 添加你的传感器数据
cJSON_AddNumberToObject(data, "temperature", 25.5);
cJSON_AddNumberToObject(data, "humidity", 60);
cJSON_AddNumberToObject(data, "pressure", 1013.25);
```

### 处理自定义命令

在 `main/app/app_manager.c` 的 `app_mqtt_data_callback()` 函数中：

```c
else if (strcmp(command, "your_command") == 0) {
    ESP_LOGI(TAG, "✅ 执行自定义命令");
    // 你的处理代码
}
```

### 添加新功能模块

1. 在 `main/app/` 下创建新的 `.c/.h` 文件
2. 在 `main/CMakeLists.txt` 中添加源文件
3. 在 `app_manager.c` 中调用你的功能

## 🔧 组件说明

### iot_manager_mqtt 组件

通用的IoT管理组件，提供MQTT通信功能。

**API接口**:
- `iot_manager_init()` - 初始化
- `iot_manager_start()` - 启动MQTT
- `iot_manager_stop()` - 停止MQTT
- `iot_manager_publish()` - 发布消息
- `iot_manager_report_status()` - 上报状态
- `iot_manager_report_properties()` - 上报属性
- `iot_manager_subscribe()` - 订阅主题

详细文档：`components/iot_manager_mqtt/`

### app 应用层

设备业务逻辑实现，包括：
- 设备配置管理
- 数据采集和上报
- 命令接收和处理

详细文档：`main/app/README.md`

## 🐛 故障排查

### 编译错误

**Q: 找不到头文件**
```
A: 确保已安装ESP-IDF v5.0+，执行 idf.py fullclean 清理后重新编译
```

**Q: menuconfig中找不到配置项**
```
A: 检查 Kconfig 文件，确保组件在 CMakeLists.txt 中正确添加
```

### 运行问题

**Q: WiFi配网失败**
```
A: 
1. 检查热点是否正确创建
2. 确认手机已连接到ESP32热点
3. 尝试访问 http://192.168.4.1:8080
```

**Q: MQTT连接失败**
```
A:
1. 检查服务器地址是否正确
2. 验证用户名密码
3. 确认网络连接正常
4. 查看串口日志获取详细错误
```

**Q: 设备不上报数据**
```
A:
1. 确认MQTT已连接（查看日志）
2. 检查主题配置是否正确
3. 验证设备ID配置
```

## 📊 性能指标

- **启动时间**: ~3秒
- **WiFi连接**: ~5秒
- **MQTT连接**: ~2秒
- **内存占用**: ~150KB
- **数据上报间隔**: 可配置（默认30秒）

## 🔐 安全建议

1. **修改默认密码**: 生产环境请修改AP热点密码
2. **使用MQTTS**: 生产环境建议使用TLS加密
3. **设备认证**: 为每个设备配置唯一ID和密钥
4. **固件更新**: 定期更新固件修复安全漏洞

## 📄 许可证

MIT License

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📧 联系方式

- **Author**: 星年 (xingnian)
- **Email**: j_xingnian@163.com
- **GitHub**: https://github.com/jxingnian/esp_mqtt

---

**Happy Coding!** 🚀


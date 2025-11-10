/*
 * @Author: xingnian
 * @Date: 2025-11-10
 * @Description: 应用配置文件 - 根据后台系统配置修改
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// ========== 设备信息配置 ==========
// 设备唯一ID - 每个设备应该不同
#define APP_DEVICE_ID       "ESP32_001"

// 设备名称
#define APP_DEVICE_NAME     "ESP32智能设备"

// 设备类型
#define APP_DEVICE_TYPE     "sensor"

// ========== 数据上报配置 ==========
// 数据上报间隔（秒）
#define APP_REPORT_INTERVAL_SEC     30

// 是否启用自动上报
#define APP_AUTO_REPORT_ENABLED     1

// ========== MQTT主题配置 ==========
// 后台系统使用的主题格式：
// - 设备发布状态：device/{device_id}/status
// - 设备发布数据：device/{device_id}/data  
// - 设备订阅命令：device/{device_id}/command
//
// 这些已在 iot_manager 组件的 Kconfig 中配置

// ========== 功能开关 ==========
// 是否启用调试日志
#define APP_DEBUG_ENABLED           1

// 是否启用命令处理
#define APP_COMMAND_ENABLED         1

#endif // APP_CONFIG_H


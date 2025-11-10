# IoT管理组件 - MQTT通信模块
idf_component_register(
    SRCS "iot_manager.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt esp_event esp_timer
)

# 设置编译选项
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

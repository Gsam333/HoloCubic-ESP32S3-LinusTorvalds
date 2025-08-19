#pragma once

//** ESP32-S3 HoloCubic 应用层常量定义
//** Linus原则：消除应用层魔数，提高代码可读性
//** 
//** 这个文件包含应用逻辑相关的常量，与硬件无关的数值定义

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// WiFi应用相关常量
// ========================================

//** WiFi LED指示相关
#define WIFI_LED_UPDATE_INTERVAL_MS    2000    // WiFi状态LED更新间隔 (2秒)

// ========================================
// LED闪烁相关常量
// ========================================

//** LED闪烁时序
#define LED_BLINK_ON_MS                200     // LED点亮持续时间
#define LED_BLINK_OFF_MS               200     // LED熄灭持续时间

//** LED亮度常量
#define LED_DEFAULT_BRIGHTNESS         200     // LED默认亮度 (0-255)

//** LED颜色常量 (RGB值)
#define LED_COLOR_MAX_VALUE            255     // LED颜色最大值
#define LED_COLOR_MIN_VALUE            0       // LED颜色最小值

// ========================================
// 心跳监控相关常量
// ========================================

//** 心跳默认配置
#define HEARTBEAT_DEFAULT_INTERVAL_MS  1000    // 默认心跳间隔 (1秒)

// ========================================
// 命令处理相关常量
// ========================================

//** ASCII字符范围
#define ASCII_PRINTABLE_START          32      // 可打印字符起始 (空格)
#define ASCII_PRINTABLE_END            126     // 可打印字符结束 (~)

// ========================================
// 显示相关常量
// ========================================

//** PWM相关
#define PWM_MAX_VALUE                  255     // PWM最大值 (8位)

//** 百分比转换
#define PERCENTAGE_TO_FLOAT_DIVISOR    100.0f  // 百分比转浮点数除数

//** 时间转换常量
#define MILLISECONDS_TO_SECONDS        1000    // 毫秒转秒的除数
#define SECONDS_TO_MINUTES             60      // 秒转分钟的除数

// ========================================
// 显示示例相关常量
// ========================================

//** 显示坐标常量
#define DISPLAY_CENTER_X               120     // 显示中心X坐标 (240/2)
#define DISPLAY_CENTER_Y               120     // 显示中心Y坐标 (240/2)
#define DISPLAY_TEXT_START_X           10      // 文本起始X坐标
#define DISPLAY_TEXT_START_Y           10      // 文本起始Y坐标
#define DISPLAY_CIRCLE_RADIUS          50      // 圆形半径

// ========================================
// 存储相关常量
// ========================================

//** 存储单位转换
#define BYTES_TO_KB                    1024    // 字节转KB
#define BYTES_TO_MB                    1048576 // 字节转MB (1024*1024)
#define KB_TO_MB_DIVISOR               1024.0  // KB转MB的浮点除数

#ifdef __cplusplus
}
#endif

//** 使用说明：
//** 1. 这个文件包含应用层的数值常量
//** 2. 硬件相关常量仍在hardware_config.h中
//** 3. 系统级常量在system_constants.h中
//** 4. 遵循Linus原则：简洁、清晰、有意义的命名
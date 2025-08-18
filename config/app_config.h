#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*
 * Linus Torvalds 应用配置系统
 * 
 * 设计原则：
 * 1. 单一职责 - 只包含应用级配置
 * 2. 简洁明了 - 不要复杂的条件编译
 * 3. 合理默认值 - 开箱即用
 * 4. 易于调整 - 清晰的分组和注释
 */

// ========================================
// 应用基本配置
// ========================================

// 应用信息
#define APP_NAME            "HoloCubic-Linus"
#define APP_VERSION         "1.0.0"
#define APP_BUILD_DATE      __DATE__
#define APP_BUILD_TIME      __TIME__

// 系统配置
#define SYSTEM_TICK_MS      10      // 主循环间隔
#define WATCHDOG_TIMEOUT_MS 30000   // 看门狗超时

// ========================================
// 显示应用配置
// ========================================

// 显示参数
#define DISPLAY_DEFAULT_BRIGHTNESS  128     // 默认亮度 (0-255)
#define DISPLAY_TIMEOUT_MS          30000   // 屏幕超时时间
#define DISPLAY_ANIMATION_SPEED     100     // 动画速度 (ms)

// 界面配置
#define UI_REFRESH_RATE_MS          50      // UI刷新率
#define UI_TRANSITION_TIME_MS       300     // 界面切换时间

// ========================================
// 网络应用配置
// ========================================

// 连接配置
#define WIFI_CONNECT_TIMEOUT_MS     15000   // WiFi连接超时
#define WIFI_RECONNECT_INTERVAL_MS  30000   // 重连间隔
#define WIFI_MAX_RETRY_COUNT        3       // 最大重试次数

// 网络服务
#define NTP_UPDATE_INTERVAL_MS      3600000 // NTP更新间隔 (1小时)
#define WEATHER_UPDATE_INTERVAL_MS  1800000 // 天气更新间隔 (30分钟)

// ========================================
// 传感器应用配置
// ========================================

// IMU配置
#define IMU_SAMPLE_RATE_HZ          100     // 采样率
#define IMU_GESTURE_THRESHOLD       1000    // 手势识别阈值
#define IMU_CALIBRATION_SAMPLES     100     // 校准样本数

// 手势配置
#define GESTURE_DEBOUNCE_MS         500     // 手势防抖时间
#define GESTURE_TIMEOUT_MS          2000    // 手势超时

// ========================================
// LED应用配置
// ========================================

// LED效果
#define LED_DEFAULT_BRIGHTNESS      200     // 默认亮度
#define LED_ANIMATION_SPEED         50      // 动画速度
#define LED_FADE_TIME_MS            1000    // 渐变时间

// 状态指示
#define LED_STATUS_BLINK_RATE_MS    500     // 状态闪烁频率
#define LED_ERROR_BLINK_RATE_MS     200     // 错误闪烁频率

// ========================================
// 电源管理配置
// ========================================

// 省电模式
#define POWER_SAVE_TIMEOUT_MS       300000  // 进入省电模式时间 (5分钟)
#define DEEP_SLEEP_TIMEOUT_MS       1800000 // 深度睡眠时间 (30分钟)

// 电池监控
#define BATTERY_CHECK_INTERVAL_MS   60000   // 电池检查间隔 (1分钟)
#define LOW_BATTERY_THRESHOLD       20      // 低电量阈值 (%)

// ========================================
// 存储配置
// ========================================

// 配置文件
#define CONFIG_SAVE_INTERVAL_MS     300000  // 配置保存间隔 (5分钟)
#define CONFIG_BACKUP_COUNT         3       // 配置备份数量

// 日志配置
#define LOG_MAX_SIZE_KB             100     // 最大日志大小
#define LOG_ROTATION_COUNT          5       // 日志轮转数量

// ========================================
// 功能开关 - 简单的开关控制
// ========================================

// 核心功能
#define FEATURE_DISPLAY_ENABLED     1
#define FEATURE_NETWORK_ENABLED     1
#define FEATURE_IMU_ENABLED         1
#define FEATURE_LED_ENABLED         1

// 可选功能
#define FEATURE_WEATHER_ENABLED     1
#define FEATURE_CLOCK_ENABLED       1
#define FEATURE_MUSIC_ENABLED       0       // 暂未实现
#define FEATURE_CAMERA_ENABLED      0       // 暂未实现

// 调试功能
#define FEATURE_SERIAL_COMMANDS     1
#define FEATURE_WEB_CONFIG          0       // 可选的Web配置界面
#define FEATURE_OTA_UPDATE          0       // 可选的OTA更新

// ========================================
// 测试代码控制 - 移至 debug_config.h 统一管理
// ========================================

// 测试代码控制已移至 config/debug_config.h 统一管理
// 避免宏重定义冲突，遵循 Linus 原则："Single Source of Truth"

// ========================================
// 配置验证
// ========================================

// 基本参数检查
#if SYSTEM_TICK_MS <= 0
#error "SYSTEM_TICK_MS must be positive"
#endif

#if DISPLAY_DEFAULT_BRIGHTNESS > 255
#error "DISPLAY_DEFAULT_BRIGHTNESS must be <= 255"
#endif

#if WIFI_MAX_RETRY_COUNT <= 0
#error "WIFI_MAX_RETRY_COUNT must be positive"
#endif

// ========================================
// 便捷宏定义
// ========================================

// 时间转换宏
#define SECONDS_TO_MS(s)    ((s) * 1000)
#define MINUTES_TO_MS(m)    ((m) * 60 * 1000)
#define HOURS_TO_MS(h)      ((h) * 60 * 60 * 1000)

// 功能检查宏
#define IS_FEATURE_ENABLED(feature) (FEATURE_##feature##_ENABLED)

// 版本信息宏
#define APP_VERSION_STRING  APP_NAME " v" APP_VERSION " (" APP_BUILD_DATE ")"

#endif // APP_CONFIG_H

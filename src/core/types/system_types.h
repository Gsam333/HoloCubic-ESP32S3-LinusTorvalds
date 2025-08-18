#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Linus Torvalds 系统类型定义
 * 
 * 原则：
 * 1. 使用标准C类型，不要Arduino的垃圾类型
 * 2. 固定大小的数据结构，不要动态分配
 * 3. 简单明了的命名，不要匈牙利命名法
 * 4. 消除特殊情况，统一处理
 * 5. 避免与系统库冲突 - 使用前缀
 */

// ========================================
// 基础类型定义
// ========================================

// 硬件状态枚举 - 简单明了
typedef enum {
    HW_NOT_INITIALIZED = 0,
    HW_INITIALIZING,
    HW_READY,
    HW_ERROR,
    HW_SUSPENDED
} hw_status_t;

// 错误代码 - 统一错误处理，使用SYS前缀避免冲突
typedef enum {
    SYS_ERR_OK = 0,
    SYS_ERR_INIT_FAILED,
    SYS_ERR_INVALID_PARAM,
    SYS_ERR_TIMEOUT,
    SYS_ERR_NO_MEMORY,
    SYS_ERR_HARDWARE_FAULT
} sys_error_t;

// ========================================
// 硬件状态结构
// ========================================

// 通用硬件状态 - 所有驱动都用这个
typedef struct {
    hw_status_t status;
    uint32_t last_update_ms;
    uint32_t error_count;
    sys_error_t last_error;
    const char* error_msg;  // 指向常量字符串，不分配内存
} hw_state_t;

// ========================================
// IMU数据结构 - 消除特殊情况
// ========================================

// IMU原始数据 - 使用数组，不是6个独立变量
typedef struct {
    int16_t accel[3];       // [x, y, z] 加速度
    int16_t gyro[3];        // [x, y, z] 陀螺仪
    uint32_t timestamp_ms;  // 时间戳
    bool valid;             // 数据有效性
} imu_data_t;

// IMU手势类型 - 简化枚举
typedef enum {
    GESTURE_NONE = 0,
    GESTURE_LEFT,
    GESTURE_RIGHT,
    GESTURE_UP,
    GESTURE_DOWN,
    GESTURE_FORWARD,
    GESTURE_SHAKE,
    GESTURE_COUNT  // 用于数组大小
} gesture_t;

// IMU配置 - 固定结构
typedef struct {
    int16_t accel_offset[3];  // 加速度偏移
    int16_t gyro_offset[3];   // 陀螺仪偏移
    uint8_t orientation;      // 方向设置
    bool auto_calibration;    // 自动校准
} imu_config_t;

// ========================================
// 网络数据结构 - 简化状态管理
// ========================================

// 网络连接状态
typedef enum {
    NET_DISCONNECTED = 0,
    NET_CONNECTING,
    NET_CONNECTED,
    NET_FAILED
} net_status_t;

// WiFi凭据 - 固定大小，不用String
typedef struct {
    char ssid[32];
    char password[64];
} wifi_credential_t;

// 网络状态 - 统一信息
typedef struct {
    net_status_t status;
    char current_ssid[32];
    char ip_address[16];      // "192.168.1.100"
    int8_t signal_dbm;        // 信号强度
    uint32_t uptime_ms;       // 连接时长
    uint32_t reconnect_count; // 重连次数
} net_state_t;

// ========================================
// 显示数据结构
// ========================================

// 显示配置
typedef struct {
    uint8_t rotation;         // 旋转角度 (0-3)
    uint8_t brightness;       // 亮度 (0-255)
    bool backlight_on;        // 背光状态
} display_config_t;

// ========================================
// LED数据结构
// ========================================

// RGB颜色 - 简单结构
typedef struct {
    uint8_t r, g, b;
} rgb_color_t;

// HSV颜色
typedef struct {
    uint8_t h, s, v;
} hsv_color_t;

// LED配置
typedef struct {
    uint8_t brightness;       // 整体亮度
    rgb_color_t color;        // 当前颜色
    bool enabled;             // 是否启用
} led_config_t;

// ========================================
// 系统状态 - 单一数据源
// ========================================

// 系统统计信息
typedef struct {
    uint32_t uptime_ms;
    uint32_t free_heap_bytes;
    uint32_t min_free_heap_bytes;
    uint32_t task_count;
    float cpu_usage_percent;
} system_stats_t;

// 完整系统状态 - 所有信息的单一来源
typedef struct {
    // 硬件状态
    hw_state_t display_hw;
    hw_state_t imu_hw;
    hw_state_t led_hw;
    hw_state_t network_hw;
    
    // 传感器数据
    imu_data_t imu_data;
    gesture_t current_gesture;
    
    // 网络状态
    net_state_t network;
    
    // 配置信息
    display_config_t display_config;
    led_config_t led_config;
    imu_config_t imu_config;
    
    // 系统统计
    system_stats_t stats;
    
    // 系统标志
    bool system_ready;
    bool low_power_mode;
    bool debug_enabled;
} system_state_t;

// ========================================
// 常量定义
// ========================================

#define MAX_WIFI_NETWORKS   3
#define IMU_HISTORY_SIZE    8
#define SYSTEM_TICK_MS      10
#define WATCHDOG_TIMEOUT_MS 30000

// 硬件引脚定义 - 不要重复定义，让编译器标志处理
#ifndef TFT_MISO
#define TFT_MISO    13
#endif

#ifndef TFT_MOSI
#define TFT_MOSI    42
#endif

#ifndef TFT_SCLK
#define TFT_SCLK    41
#endif

#ifndef TFT_CS
#define TFT_CS      10
#endif

#ifndef TFT_DC
#define TFT_DC      40
#endif

#ifndef TFT_RST
#define TFT_RST     45
#endif

#ifndef TFT_BL
#define TFT_BL      46
#endif

#define RGB_LED_PIN 39
#define RGB_LED_NUM 2

#define IMU_I2C_SDA 17
#define IMU_I2C_SCL 18
#define IMU_I2C_ADDRESS 0x6B

#endif // SYSTEM_TYPES_H

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <stdint.h>
// 不包含Arduino.h - 保持头文件干净

/*
 * Linus Torvalds 硬件配置系统 - 修正版本
 *
 * 核心设计哲学：
 * "不要为假想的问题创造复杂的解决方案"
 *
 * 设计原则：
 * 1. 纯宏定义 - 零运行时开销
 * 2. 无外部依赖 - 头文件干净
 * 3. 集中配置 - 便于移植
 * 4. 编译时验证 - 错误早发现
 * 5. 库兼容性 - 与TFT_eSPI.h等库完美共存
 *
 * 重要洞察：
 * hardware_config.h (硬件引脚定义) 与 TFT_eSPI.h (操作函数) 完全不冲突！
 * - hardware_config.h: 定义 HW_DISPLAY_MOSI = 42
 * - TFT_eSPI.h: 提供 tft.drawPixel() 等函数
 * - 两者职责分离，互不干扰，可以同时包含
 *
 * 这是"关注点分离"的完美体现：
 * - 硬件配置 = 数据 (引脚号、频率等)
 * - 库函数 = 行为 (绘图、初始化等)
 * - 数据和行为分离，符合Linus的"好品味"原则
 */

// ========================================
// ESP32-S3 HoloCubic 硬件配置
// ========================================

// LED配置 (WS2812)
#define HW_LED_PIN 39
#define HW_LED_COUNT 2
#define HW_LED_BRIGHTNESS 200

// TFT dispaly显示配置 (ST7789 240x240)
// 
// 【Linus式设计决策】
// 引脚配置统一在 lib/TFT_eSPI/User_Setups/Setup24_ST7789.h 中定义
// 这里只定义应用层特有的参数，避免重复定义
//
// 原则："Single Source of Truth" - 每个硬件参数只在一个地方定义
//
#define HW_DISPLAY_WIDTH 240
#define HW_DISPLAY_HEIGHT 240
#define HW_DISPLAY_SPI_FREQ 40000000

// 应用层显示配置
#define HW_DISPLAY_DEFAULT_ROTATION 4    // HoloCubic全息模式
#define HW_DISPLAY_DEFAULT_BRIGHTNESS 80 // 默认亮度百分比

// 显示PWM配置
#define HW_DISPLAY_PWM_CHANNEL 0
#define HW_DISPLAY_PWM_FREQUENCY 5000
#define HW_DISPLAY_PWM_RESOLUTION 8

// IMU配置 (QMI8658)
#define HW_IMU_SDA 17
#define HW_IMU_SCL 18
#define HW_IMU_ADDRESS 0x6B
#define HW_IMU_INT_PIN 19

// 系统配置
#define HW_SYSTEM_CPU_MHZ 240
#define HW_SYSTEM_SERIAL_BAUD 115200

// 系统时间常量 - 消除魔数
#define HW_SYSTEM_STARTUP_DELAY_MS     1000    // ESP32-S3启动稳定时间
#define HW_SYSTEM_HEALTH_CHECK_MS      30000   // 系统健康检查间隔
#define HW_SYSTEM_TFT_UPDATE_MS        5000    // TFT显示更新间隔

// LED系统常量
#define HW_LED_STARTUP_DURATION_MS     200     // 启动指示LED持续时间
#define HW_LED_HEARTBEAT_ON_MS         50      // 心跳LED点亮时间

// WiFi系统常量  
#define HW_WIFI_CONNECT_TIMEOUT_MS     30000   // WiFi连接超时
#define HW_WIFI_STATUS_CHECK_MS        5000    // WiFi状态检查间隔

// 系统恢复常量
#define HW_PANIC_TIMEOUT_MS            10000   // 系统panic超时重启时间

// ========================================
// 第三方库兼容宏 - 不修改库，只提供宏
// ========================================

// FastLED 兼容宏
#define RGB_LED_PIN HW_LED_PIN
#define RGB_LED_NUM HW_LED_COUNT
#define RGB_LED_TYPE WS2812
#define RGB_COLOR_ORDER GRB
#define RGB_DEFAULT_BRIGHTNESS HW_LED_BRIGHTNESS

// TFT_eSPI配置说明 / TFT_eSPI Configuration Note
// TFT_eSPI库的配置在 lib/TFT_eSPI/User_Setups/Setup24_ST7789.h 中定义
// TFT_eSPI library configuration is defined in lib/TFT_eSPI/User_Setups/Setup24_ST7789.h
// 
// 这避免了宏重定义冲突，并确保TFT_eSPI在编译时获得正确的配置
// This avoids macro redefinition conflicts and ensures TFT_eSPI gets correct configuration at compile time

// IMU 兼容宏
#define IMU_I2C_SDA HW_IMU_SDA
#define IMU_I2C_SCL HW_IMU_SCL
#define IMU_I2C_ADDRESS HW_IMU_ADDRESS
#define IMU_INT_PIN HW_IMU_INT_PIN

// ========================================
// 编译时验证 - 使用静态断言
// ========================================

// 简单的编译时检查
#if HW_LED_COUNT <= 0
#error "LED count must be positive"
#endif

#if HW_LED_COUNT > 64
#error "LED count too high"
#endif

#if HW_LED_PIN > 48
#error "LED pin out of range for ESP32-S3"
#endif

#if HW_DISPLAY_WIDTH <= 0 || HW_DISPLAY_HEIGHT <= 0
#error "Display dimensions must be positive"
#endif

#if HW_SYSTEM_SERIAL_BAUD < 9600
#error "Serial baud rate too low"
#endif

// ========================================
// 运行时配置结构 - 仅用于调试信息
// ========================================

typedef struct {
  struct {
    uint8_t pin;
    uint8_t count;
    uint8_t brightness;
  } led;

  struct {
    uint8_t miso, mosi, sclk, cs;
    uint8_t dc, rst, bl;
    uint16_t width, height;
    uint32_t spi_freq;
  } display;

  struct {
    uint8_t sda, scl;
    uint8_t address;
    uint8_t int_pin;
  } imu;

  struct {
    uint32_t cpu_freq_mhz;
    uint32_t serial_baud;
  } system;
} hw_config_t;

// 【Linus式简化】
// 移除复杂的运行时配置函数，避免循环依赖
// 调试信息直接在各自的驱动中处理
// 
// 如果需要获取硬件配置，直接使用：
// - LED: HW_LED_* 宏
// - Display: TFT_* 宏 (来自TFT_eSPI)
// - IMU: HW_IMU_* 宏
// - System: HW_SYSTEM_* 宏

// ========================================
// 设计原则总结 - Linus风格的胜利
// ========================================

/*
 * 关键洞察：库兼容性不是问题，而是特性
 *
 * 错误的思维：
 * "hardware_config.h 和 TFT_eSPI.h 会冲突，需要复杂的依赖管理"
 *
 * 正确的思维：
 * "hardware_config.h 定义数据，TFT_eSPI.h 定义行为，完美互补"
 *
 * 实际使用：
 * #include "hardware_config.h"  // 获得 HW_DISPLAY_MOSI = 42
 * #include <TFT_eSPI.h>         // 获得 tft.drawPixel() 函数
 *
 * 结果：
 * - 零冲突
 * - 零复杂性
 * - 完美的关注点分离
 *
 * 这就是Linus所说的"好品味"：
 * "消除特殊情况，让正确的设计自然涌现"
 *
 * 教训：
 * 不要为不存在的问题创造复杂的解决方案！
 */

#endif // HARDWARE_CONFIG_H
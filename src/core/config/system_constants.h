#pragma once

//** ESP32-S3 HoloCubic 系统常量定义
//** Linus原则：消除魔数，提高代码可读性

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// 系统启动相关常量
// ========================================

//** 启动延时和超时 (原魔数已消除)
#define SYSTEM_BOOT_DELAY_MS           1000    // 系统启动稳定延时 (原值: 1000)
#define SYSTEM_SHUTDOWN_DELAY_MS       100     // 关闭外设延时 (原值: 100)
#define SYSTEM_DATA_SAVE_DELAY_MS      100     // 数据保存延时 (原值: 100)

//** I2C配置常量 (原魔数已消除)
#define I2C_CLOCK_FREQUENCY_HZ         400000  // I2C时钟频率 400kHz (原值: 400000)
#define I2C_TIMEOUT_MS                 1000    // I2C超时时间 (原值: 1000)

//** 百分比计算常量 (原魔数已消除)
#define PERCENTAGE_MULTIPLIER          100     // 百分比计算乘数 (原值: 100)

// ========================================
// 系统恐慌和错误处理常量
// ========================================

//** 恐慌处理延时
#define PANIC_GRACEFUL_DELAY_MS        100     // 优雅关闭延时
#define PANIC_RESTART_DELAY_MS         1000    // 重启前延时
#define PANIC_HALT_LOOP_DELAY_MS       1000    // 停机循环延时
#define PANIC_INPUT_CHECK_DELAY_MS     100     // 用户输入检查间隔

//** 系统健康检查阈值
#define MIN_FREE_HEAP_BYTES            10000   // 最小可用内存 (10KB)
#define PANIC_TIMEOUT_DIVISOR          1000    // 超时时间转换为秒的除数

// ========================================
// LED系统常量
// ========================================

//** LED优先级定义 (从hardware_config.h引用)
// 这些常量在hardware_config.h中定义，这里只是说明
// #define HW_LED_STARTUP_DURATION_MS  - LED启动指示持续时间

// ========================================
// 存储系统常量
// ========================================

//** SPIFFS相关常量
// 百分比计算使用 PERCENTAGE_MULTIPLIER

//** SD卡相关常量
// SD卡配置使用hardware_config.h中的引脚定义

#ifdef __cplusplus
}
#endif

//** 使用说明：
//** 1. 这个文件包含所有系统级的数值常量
//** 2. 硬件相关的常量仍在hardware_config.h中
//** 3. 应用级常量应该在各自的模块中定义
//** 4. 遵循Linus原则：简洁、清晰、无重复
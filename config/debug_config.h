#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include <Arduino.h>

/*
 * Linus Torvalds 调试配置系统
 * 
 * 设计原则：
 * 1. 编译时开关 - 零运行时开销
 * 2. 模块化调试 - 按功能分类
 * 3. 统一接口 - 简单的宏定义
 * 4. 实用主义 - 只在需要时输出
 */

// ========================================
// 调试级别定义
// ========================================

#define DEBUG_LEVEL_NONE    0
#define DEBUG_LEVEL_ERROR   1
#define DEBUG_LEVEL_WARN    2
#define DEBUG_LEVEL_INFO    3
#define DEBUG_LEVEL_DEBUG   4

// 全局调试级别 - 可以在编译时覆盖
#ifndef GLOBAL_DEBUG_LEVEL
#define GLOBAL_DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

// ========================================
// 测试代码控制 - Linus 风格统一管理
// ========================================

// 测试功能总开关 - 开发阶段启用，生产环境设为 0

#define ENABLE_TEST_CODE            1       // 开发阶段启用测试

// 具体测试模块控制
#if ENABLE_TEST_CODE
    #define ENABLE_LED_TESTS        1       // LED 测试套件
    #define ENABLE_TFT_TESTS        1       // TFT 显示测试套件
    #define ENABLE_IMU_TESTS        0       // IMU 手势识别测试套件
    #define ENABLE_SD_TESTS         1       // SD卡存储测试套件
    #define ENABLE_FLASH_TESTS      1       // Flash存储测试套件
    #define ENABLE_HARDWARE_TESTS   1       // 硬件测试
    #define ENABLE_DEBUG_COMMANDS   1       // 调试命令
    #define ENABLE_SYSTEM_INFO      1       // 系统信息打印
#else
    #define ENABLE_LED_TESTS        0       // 生产环境禁用
    #define ENABLE_TFT_TESTS        0       // 生产环境禁用
    #define ENABLE_IMU_TESTS        0       // 生产环境禁用
    #define ENABLE_SD_TESTS         0       // 生产环境禁用
    #define ENABLE_FLASH_TESTS      0       // 生产环境禁用
    #define ENABLE_HARDWARE_TESTS   0
    #define ENABLE_DEBUG_COMMANDS   0
    #define ENABLE_SYSTEM_INFO      0
#endif

// ========================================
// 模块调试开关 - 编译时配置
// ========================================

// LED驱动调试
#define DEBUG_LED_ENABLED 1

// 系统管理调试
#define DEBUG_SYSTEM_ENABLED 1

// 硬件配置调试
#define DEBUG_HARDWARE_ENABLED 1

// TFT显示测试 - 控制是否运行显示测试代码
#define DEBUG_TFT_DISPLAY_TEST 0  // 暂时禁用避免硬件冲突

// Flash存储调试
#define DEBUG_FLASH_ENABLED 1

// ========================================
// 调试宏定义 - 简单有效
// ========================================

// 基础调试宏
#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
#define DEBUG_ERROR(fmt, ...) Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_ERROR(fmt, ...)
#endif

#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_WARN
#define DEBUG_WARN(fmt, ...) Serial.printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_WARN(fmt, ...)
#endif

#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define DEBUG_INFO(fmt, ...) Serial.printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_INFO(fmt, ...)
#endif

#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DEBUG_DEBUG(fmt, ...) Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_DEBUG(fmt, ...)
#endif

// ========================================
// 模块特定调试宏
// ========================================

// LED驱动调试宏
#if DEBUG_LED_ENABLED && (GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
#define DEBUG_LOG_LED(fmt, ...) Serial.printf("[LED] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG_LED(fmt, ...)
#endif

// 系统管理调试宏
#if DEBUG_SYSTEM_ENABLED && (GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
#define DEBUG_LOG_SYSTEM(fmt, ...) Serial.printf("[SYSTEM] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG_SYSTEM(fmt, ...)
#endif

// 硬件配置调试宏
#if DEBUG_HARDWARE_ENABLED && (GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
#define DEBUG_LOG_HARDWARE(fmt, ...) Serial.printf("[HARDWARE] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG_HARDWARE(fmt, ...)
#endif

// TFT显示调试宏
#if DEBUG_TFT_DISPLAY_TEST && (GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
#define DEBUG_LOG_TFT(fmt, ...) Serial.printf("[TFT] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG_TFT(fmt, ...)
#endif

// Flash存储调试宏
#if DEBUG_FLASH_ENABLED && (GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
#define DEBUG_LOG_FLASH(fmt, ...) Serial.printf("[FLASH] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG_FLASH(fmt, ...)
#endif

// ========================================
// 便捷调试函数
// ========================================

// 打印分隔线
static inline void debug_separator(const char* title = nullptr) {
#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
    Serial.println("========================================");
    if (title) {
        Serial.printf("=== %s ===\n", title);
        Serial.println("========================================");
    }
#endif
}

// 打印内存信息
static inline void debug_memory_info() {
#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
    Serial.printf("[MEMORY] Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("[MEMORY] Min free heap: %u bytes\n", ESP.getMinFreeHeap());
    Serial.printf("[MEMORY] Heap size: %u bytes\n", ESP.getHeapSize());
    if (ESP.getPsramSize() > 0) {
        Serial.printf("[MEMORY] PSRAM size: %u bytes\n", ESP.getPsramSize());
        Serial.printf("[MEMORY] Free PSRAM: %u bytes\n", ESP.getFreePsram());
    }
#endif
}

// 打印系统信息
static inline void debug_system_info() {
#if GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
    Serial.printf("[SYSTEM] Chip: %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("[SYSTEM] CPU: %u MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("[SYSTEM] Flash: %u bytes @ %u Hz\n", 
        ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    Serial.printf("[SYSTEM] Uptime: %lu ms\n", millis());
#endif
}

// 打印Flash分区信息 - 声明，实现在使用的地方
void debug_flash_info();

#endif // DEBUG_CONFIG_H
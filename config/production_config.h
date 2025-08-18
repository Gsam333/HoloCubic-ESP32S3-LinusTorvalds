//** ESP32-S3 HoloCubic - Production Configuration
//** Linus原则：生产环境配置，禁用所有测试代码

#ifndef PRODUCTION_CONFIG_H
#define PRODUCTION_CONFIG_H

// ========================================
// 生产环境配置覆盖
// ========================================

// 强制禁用所有测试代码
#undef ENABLE_TEST_CODE
#define ENABLE_TEST_CODE            0

// 强制禁用调试功能
#undef FEATURE_SERIAL_COMMANDS
#define FEATURE_SERIAL_COMMANDS     0

// 强制禁用系统信息打印
#undef ENABLE_SYSTEM_INFO
#define ENABLE_SYSTEM_INFO          0

// 强制禁用所有测试模块
#undef ENABLE_LED_TESTS
#define ENABLE_LED_TESTS            0

#undef ENABLE_HARDWARE_TESTS
#define ENABLE_HARDWARE_TESTS       0

#undef ENABLE_DEBUG_COMMANDS
#define ENABLE_DEBUG_COMMANDS       0

// ========================================
// 生产环境优化配置
// ========================================

// 减少串口输出
#undef CORE_DEBUG_LEVEL
#define CORE_DEBUG_LEVEL            0

// 优化性能参数
#undef SYSTEM_TICK_MS
#define SYSTEM_TICK_MS              20      // 生产环境可以稍微放宽

// 电源管理更激进
#undef POWER_SAVE_TIMEOUT_MS
#define POWER_SAVE_TIMEOUT_MS       180000  // 3分钟进入省电

#undef DEEP_SLEEP_TIMEOUT_MS
#define DEEP_SLEEP_TIMEOUT_MS       900000  // 15分钟深度睡眠

// ========================================
// 编译时检查
// ========================================

#if ENABLE_TEST_CODE
#error "Production build must not include test code!"
#endif

#if FEATURE_SERIAL_COMMANDS
#error "Production build must not include serial commands!"
#endif

#pragma message("Building for PRODUCTION - all test code disabled")

#endif // PRODUCTION_CONFIG_H
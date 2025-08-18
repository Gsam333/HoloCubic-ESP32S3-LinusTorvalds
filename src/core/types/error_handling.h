//** ESP32-S3 HoloCubic - 统一错误处理系统
//** Linus原则：一致的错误处理，早期返回，清晰的错误信息

#pragma once

#include <Arduino.h>

// ========================================
// 错误处理宏 - 统一风格
// ========================================

//** 成功/失败状态码
typedef enum {
    RESULT_SUCCESS = 0,
    RESULT_ERROR_INVALID_PARAM,
    RESULT_ERROR_NOT_INITIALIZED,
    RESULT_ERROR_TIMEOUT,
    RESULT_ERROR_NO_MEMORY,
    RESULT_ERROR_HARDWARE_FAIL,
    RESULT_ERROR_NETWORK_FAIL
} result_t;

//** 统一的成功/失败日志宏
#define LOG_SUCCESS(msg) Serial.println("✓ " msg)
#define LOG_ERROR(msg)   Serial.println("✗ " msg)
#define LOG_WARNING(msg) Serial.println("⚠ " msg)
#define LOG_INFO(msg)    Serial.println("ℹ " msg)

//** 带格式化的日志宏
#define LOG_SUCCESS_F(fmt, ...) Serial.printf("✓ " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR_F(fmt, ...)   Serial.printf("✗ " fmt "\n", ##__VA_ARGS__)
#define LOG_WARNING_F(fmt, ...) Serial.printf("⚠ " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO_F(fmt, ...)    Serial.printf("ℹ " fmt "\n", ##__VA_ARGS__)

//** 早期返回宏 - Linus风格
#define RETURN_IF_NULL(ptr) \
    do { \
        if (!(ptr)) { \
            LOG_ERROR("Null pointer: " #ptr); \
            return RESULT_ERROR_INVALID_PARAM; \
        } \
    } while(0)

#define RETURN_IF_FALSE(condition) \
    do { \
        if (!(condition)) { \
            LOG_ERROR("Condition failed: " #condition); \
            return RESULT_ERROR_INVALID_PARAM; \
        } \
    } while(0)

//** 布尔返回的早期返回宏
#define RETURN_FALSE_IF_NULL(ptr) \
    do { \
        if (!(ptr)) { \
            LOG_ERROR("Null pointer: " #ptr); \
            return false; \
        } \
    } while(0)

#define RETURN_FALSE_IF(condition) \
    do { \
        if (condition) { \
            LOG_ERROR("Error condition: " #condition); \
            return false; \
        } \
    } while(0)

//** 参数验证宏
#define VALIDATE_PARAM(param, condition) \
    do { \
        if (!(condition)) { \
            LOG_ERROR_F("Invalid parameter %s: %s", #param, #condition); \
            return RESULT_ERROR_INVALID_PARAM; \
        } \
    } while(0)

// ========================================
// 错误处理辅助函数
// ========================================

//** 将结果码转换为字符串
static inline const char* result_to_string(result_t result) {
    switch (result) {
        case RESULT_SUCCESS:             return "Success";
        case RESULT_ERROR_INVALID_PARAM: return "Invalid Parameter";
        case RESULT_ERROR_NOT_INITIALIZED: return "Not Initialized";
        case RESULT_ERROR_TIMEOUT:       return "Timeout";
        case RESULT_ERROR_NO_MEMORY:     return "No Memory";
        case RESULT_ERROR_HARDWARE_FAIL: return "Hardware Failure";
        case RESULT_ERROR_NETWORK_FAIL:  return "Network Failure";
        default:                         return "Unknown Error";
    }
}

//** 检查结果并记录错误
static inline bool check_result(result_t result, const char* operation) {
    if (result == RESULT_SUCCESS) {
        LOG_SUCCESS_F("%s completed", operation);
        return true;
    } else {
        LOG_ERROR_F("%s failed: %s", operation, result_to_string(result));
        return false;
    }
}

// ========================================
// Linus风格错误处理原则
// ========================================

/*
 * 1. 早期返回 - 避免深度嵌套
 *    ❌ if (condition) { ... } else { error }
 *    ✅ if (!condition) return error; ...
 *
 * 2. 清晰的错误信息 - 便于调试
 *    ❌ return false;
 *    ✅ LOG_ERROR("WiFi connection failed"); return false;
 *
 * 3. 一致的返回值 - 统一错误处理
 *    ❌ 有些函数返回bool，有些返回int，有些返回void
 *    ✅ 统一使用result_t或bool，并有清晰的语义
 *
 * 4. 参数验证 - 信任但验证
 *    ❌ 假设参数总是正确的
 *    ✅ 验证关键参数，但不过度检查
 *
 * 5. 错误传播 - 不隐藏错误
 *    ❌ 吞掉错误，假装一切正常
 *    ✅ 记录错误并向上传播
 */
//** ESP32-S3 HoloCubic - Panic Handler Header
//** Linus原则：像 Linux 内核的 panic() 一样处理致命错误

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//** 系统 panic 类型
typedef enum {
    PANIC_INIT_FAILED = 1,      // 初始化失败
    PANIC_BOOT_FAILED,          // 启动序列失败
    PANIC_OUT_OF_MEMORY,        // 内存不足
    PANIC_HARDWARE_FAULT,       // 硬件故障
    PANIC_WATCHDOG_TIMEOUT,     // 看门狗超时
    PANIC_STACK_OVERFLOW,       // 栈溢出
    PANIC_UNKNOWN               // 未知错误
} panic_reason_t;

//** 系统 panic - 像 Linux 内核一样处理致命错误
void system_panic(panic_reason_t reason, const char* message) __attribute__((noreturn));

//** 检查系统健康状态
bool system_health_check(void);

//** 尝试系统恢复
bool system_recovery_attempt(void);

#ifdef __cplusplus
}
#endif
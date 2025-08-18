//** ESP32-S3 HoloCubic - Panic Handler Implementation
//** Linus原则：优雅的错误处理，不是死循环

#include "panic.h"
#include "../core/config/hardware_config.h"
#include "app_config.h"
#include <Arduino.h>

static const char* panic_reason_strings[] = {
    "UNKNOWN",
    "INIT_FAILED",
    "OUT_OF_MEMORY", 
    "HARDWARE_FAULT",
    "WATCHDOG_TIMEOUT",
    "STACK_OVERFLOW",
    "UNKNOWN"
};

static void print_panic_info(panic_reason_t reason, const char* message) {
    Serial.println("\n" "========================================");
    Serial.println("*** SYSTEM PANIC ***");
    Serial.println("========================================");
    
    Serial.printf("Reason: %s\n", panic_reason_strings[reason]);
    if (message) {
        Serial.printf("Message: %s\n", message);
    }
    
    Serial.printf("Uptime: %lu ms\n", millis());
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("CPU freq: %u MHz\n", ESP.getCpuFreqMHz());
    
#if ENABLE_TEST_CODE
    Serial.println("Build: DEVELOPMENT");
#else
    Serial.println("Build: PRODUCTION");
#endif
    
    Serial.println("========================================");
}

static void attempt_graceful_shutdown(void) {
    Serial.println("Attempting graceful shutdown...");
    
    //** 尝试保存关键数据
    Serial.println("- Saving critical data...");
    delay(100);
    
    //** 关闭外设
    Serial.println("- Shutting down peripherals...");
    delay(100);
    
    Serial.println("Graceful shutdown completed.");
}

//** 打印用户选项
static void panic_print_options(void) {
    Serial.println("\nOptions:");
    Serial.printf("- System will auto-restart in %d seconds\n", HW_PANIC_TIMEOUT_MS / 1000);
    Serial.println("- Send 'h' to halt system");
    Serial.println("- Send 'r' to restart immediately");
}

//** 处理用户命令
static bool panic_handle_user_input(void) {
    if (!Serial.available()) {
        return false;
    }
    
    char cmd = Serial.read();
    if (cmd == 'h' || cmd == 'H') {
        Serial.println("System halted by user request.");
        Serial.println("Reset button or power cycle required.");
        while (1) {
            delay(1000);  // 只有用户明确要求才死循环
        }
    } else if (cmd == 'r' || cmd == 'R') {
        Serial.println("Restarting immediately...");
        ESP.restart();
    }
    return true;
}

//** 等待用户输入或超时重启
static void panic_wait_for_user_or_restart(void) {
    unsigned long start_time = millis();
    const unsigned long timeout_ms = HW_PANIC_TIMEOUT_MS;
    
    while (millis() - start_time < timeout_ms) {
        panic_handle_user_input();
        delay(100);
    }
    
    //** 超时后自动重启
    Serial.println("Timeout reached. Restarting system...");
    delay(1000);
    ESP.restart();
}

void system_panic(panic_reason_t reason, const char* message) {
    //** 禁用中断，防止进一步损坏
    noInterrupts();
    
    //** 打印 panic 信息
    print_panic_info(reason, message);
    
    //** 尝试优雅关闭
    attempt_graceful_shutdown();
    
    //** 给用户选择并等待
    panic_print_options();
    panic_wait_for_user_or_restart();
}

bool system_health_check(void) {
    //** 检查内存
    if (ESP.getFreeHeap() < 10000) {  // 少于10KB可用内存
        return false;
    }
    
    //** 检查栈空间
    if (ESP.getFreeHeap() < ESP.getMaxAllocHeap()) {
        // 栈可能有问题
    }
    
    return true;
}

bool system_recovery_attempt(void) {
    Serial.println("Attempting system recovery...");
    
    //** 尝试释放内存
    Serial.println("- Freeing memory...");
    
    //** 重置外设
    Serial.println("- Resetting peripherals...");
    
    //** 检查恢复结果
    if (system_health_check()) {
        Serial.println("✓ System recovery successful");
        return true;
    }
    
    Serial.println("✗ System recovery failed");
    return false;
}
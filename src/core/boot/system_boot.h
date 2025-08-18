#pragma once

//** ESP32-S3 HoloCubic - System Boot Sequence
//** Linus原则：像Linux内核一样，清晰的启动序列和错误处理

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// 启动阶段定义
// ========================================

typedef enum {
    BOOT_STAGE_EARLY_INIT = 0,    // 早期初始化：串口、时钟
    BOOT_STAGE_HARDWARE,          // 硬件初始化：GPIO、SPI、I2C
    BOOT_STAGE_DRIVERS,           // 驱动初始化：显示、LED、IMU
    BOOT_STAGE_APPLICATION,       // 应用初始化：WiFi、服务
    BOOT_STAGE_COMPLETE          // 启动完成
} boot_stage_t;

typedef enum {
    BOOT_OK = 0,
    BOOT_EARLY_INIT_FAILED,
    BOOT_HARDWARE_FAILED,
    BOOT_DRIVERS_FAILED,
    BOOT_APPLICATION_FAILED,
    BOOT_ERROR_STORAGE          // 存储系统初始化失败
} boot_result_t;

// ========================================
// 启动序列函数声明
// ========================================

//** 阶段1：系统早期初始化
boot_result_t system_early_init(void);

//** 阶段2：打印系统信息
void system_print_banner(void);

//** 阶段3：硬件模块初始化
boot_result_t hardware_init_all(void);

//** 阶段4：应用模块初始化
boot_result_t application_init_all(void);

//** 完整启动序列 - 一键启动
boot_result_t system_boot_sequence(void);

//** 启动状态查询
boot_stage_t get_boot_stage(void);
const char* get_boot_stage_name(boot_stage_t stage);

//** 存储驱动访问接口 - 全局访问点
class FlashFS;  // 前向声明
class SdCard;   // 前向声明
FlashFS* get_flash_storage(void);
SdCard* get_sd_storage(void);

#ifdef __cplusplus
}
#endif
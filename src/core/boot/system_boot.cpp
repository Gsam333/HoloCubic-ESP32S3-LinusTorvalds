//** ESP32-S3 HoloCubic - System Boot Implementation
//** Linus原则：每个函数只做一件事，错误处理清晰

#include "debug_config.h"      // 必须首先包含，定义所有编译开关
#include "system_boot.h"        // 系统启动接口
#include "QMI8658.h"            // IMU传感器驱动
#include "app_main.h"           // 应用主程序
#include "led_manager.h"        // LED管理器
#include "display_driver.h"     // 显示驱动
#include "../config/app_constants.h"  // 应用常量

#include "imu_gesture_driver.h" // IMU手势驱动
#include "hardware_config.h"    // 硬件配置
#include "system_constants.h"   // 系统常量定义
#include <Wire.h>
#include <SPIFFS.h>  // SPIFFS文件系统
#include <FS.h>      // File系统基础类

#if ENABLE_SD_TESTS
#include "SD_MMC.h"  // SD卡存储
#endif

#if ENABLE_SYSTEM_INFO
#include "system/debug_utils.h"
#endif

#if ENABLE_TFT_TESTS
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "test/tft_display_test.h"
#endif

#if ENABLE_IMU_TESTS
#include "test/imu_gesture_test.h"
#endif

// ========================================
// 全局存储驱动实例 - Linus风格：单一真实来源
// ========================================



// ========================================
// 启动状态跟踪
// ========================================

static boot_stage_t current_boot_stage = BOOT_STAGE_EARLY_INIT;

boot_stage_t get_boot_stage(void) { return current_boot_stage; }

// ========================================
// 存储驱动访问接口 - 全局访问点
// ========================================



const char *get_boot_stage_name(boot_stage_t stage) {
  switch (stage) {
  case BOOT_STAGE_EARLY_INIT:
    return "Early Init";
  case BOOT_STAGE_HARDWARE:
    return "Hardware Init";
  case BOOT_STAGE_DRIVERS:
    return "Drivers Init";
  case BOOT_STAGE_APPLICATION:
    return "Application Init";
  case BOOT_STAGE_COMPLETE:
    return "Boot Complete";
  default:
    return "Unknown";
  }
}

// ========================================
// 启动序列实现
// ========================================

//** 阶段1：系统早期初始化 - 串口和基础设置
boot_result_t system_early_init(void) {
  current_boot_stage = BOOT_STAGE_EARLY_INIT;

  Serial.begin(HW_SYSTEM_SERIAL_BAUD);
  delay(
      HW_SYSTEM_STARTUP_DELAY_MS); //** 重要：ESP32-S3需要足够时间稳定电源和时钟

  return BOOT_OK;
}

//** 阶段2：打印系统信息和构建配置
void system_print_banner(void) {
  Serial.println("========================================");
  Serial.println("ESP32-S3 HoloCubic - Linus Style Architecture");

#if ENABLE_TEST_CODE
  Serial.println("*** DEVELOPMENT BUILD - TEST CODE ENABLED ***");
#else
  Serial.println("*** PRODUCTION BUILD ***");
#endif

  Serial.println("========================================");

  //** 编译时宏检查 - 确认测试代码状态
#if ENABLE_TEST_CODE
  Serial.println("✓ Test code is ENABLED");
#else
  Serial.println("✗ Test code is DISABLED");
#endif

#if ENABLE_TFT_TESTS
  Serial.println("✓ TFT tests are ENABLED");
#else
  Serial.println("✗ TFT tests are DISABLED");
#endif

#if ENABLE_SYSTEM_INFO
  //** 打印硬件配置 - 仅开发模式
  debug_print_hw_config();
#endif
}

//** 存储系统初始化 - Linus风格：直接执行，无冗余检查
boot_result_t storage_init_all(void) {
  Serial.println("- Storage Systems");

  //** Flash存储初始化 - SPIFFS
  Serial.println("  - Flash Storage (SPIFFS)");
  if (!SPIFFS.begin(true)) {  // true = 格式化如果挂载失败
    Serial.println("    ✗ SPIFFS mount failed");
    return BOOT_ERROR_STORAGE;
  }
  
  // 打印SPIFFS信息
  size_t total_bytes = SPIFFS.totalBytes();
  size_t used_bytes = SPIFFS.usedBytes();
  Serial.printf("    ✓ SPIFFS: %zu/%zu bytes (%.1f%% used)\n", 
                used_bytes, total_bytes, 
                (float)used_bytes / total_bytes * PERCENTAGE_MULTIPLIER);
  


#if ENABLE_SD_TESTS
  //** SD卡存储初始化 - SD_MMC
  Serial.println("  - SD Card Storage (SD_MMC)");
  
  // ESP32-S3 SD卡初始化 - 使用验证过的HoloCubic配置
  Serial.printf("    Using HoloCubic pins: CLK=%d, CMD=%d, D0=%d\n", HW_SD_CLK, HW_SD_CMD, HW_SD_D0);
  Serial.println("    Note: Using SDMMC_FREQ_DEFAULT to avoid ESP32-S3 40MHz frequency issues");
  
  // 关键解决方案：使用SDMMC_FREQ_DEFAULT避免ESP32-S3的40MHz频率问题
  SD_MMC.setPins(HW_SD_CLK, HW_SD_CMD, HW_SD_D0);
  if (SD_MMC.begin("/root", true, false, SDMMC_FREQ_DEFAULT)) {
    uint64_t cardSize = SD_MMC.cardSize() / BYTES_TO_MB; // 原魔数: (1024 * 1024)
    uint8_t cardType = SD_MMC.cardType();
    Serial.printf("    ✓ SD card initialized: %lluMB\n", cardSize);
    Serial.printf("    ✓ Card type: %s\n", 
                  cardType == CARD_MMC ? "MMC" :
                  cardType == CARD_SD ? "SDSC" :
                  cardType == CARD_SDHC ? "SDHC" : "UNKNOWN");
  } else {
    Serial.println("    ✗ SD card initialization failed with HoloCubic method");
    Serial.println("    Check: 1) SD card inserted? 2) Pin connections? 3) Card format (FAT32)?");
  }
#else
  //** SD卡存储跳过 - 测试被禁用
  Serial.println("  - SD Card Storage: Disabled");
#endif

  return BOOT_OK;
}

//** 阶段3：硬件模块初始化
boot_result_t hardware_init_all(void) {
  current_boot_stage = BOOT_STAGE_HARDWARE;

  Serial.println("Initializing hardware...");

  //** 存储系统初始化 - 优先初始化，其他模块可能需要存储
  boot_result_t storage_result = storage_init_all();
  if (storage_result != BOOT_OK) {
    Serial.println("Storage initialization failed, continuing with limited functionality");
    // 存储失败不阻止系统启动，但记录错误
  }

  //** TFT显示屏初始化
  Serial.println("- TFT Display");
  display_init();

  //** LED管理器初始化
  Serial.println("- LED Manager");
  led_manager_init();

  //** IMU初始化 - Linus风格：直接执行，调用者负责顺序
  Serial.println("- IMU System");
  
  // I2C + 传感器 + 驱动 - 一次性完成，无状态检查
  Wire.begin(HW_IMU_SDA, HW_IMU_SCL);
  Wire.setClock(I2C_CLOCK_FREQUENCY_HZ);
  Wire.setTimeout(I2C_TIMEOUT_MS);
  QMI8658_init();        // 直接初始化，无检查
  imu_gesture_init();    // 手势驱动初始化，无检查
  
  Serial.println("  ✓ IMU system initialized (Linus style - no checks)");

  //** 启动指示 - 蓝色闪烁
  led_set_solid(LED_PRIORITY_SYSTEM, 0, 0, PWM_MAX_VALUE, HW_LED_STARTUP_DURATION_MS); // 原魔数: 255

  return BOOT_OK;
}

//** 阶段4：应用模块初始化
boot_result_t application_init_all(void) {
  current_boot_stage = BOOT_STAGE_APPLICATION;

  Serial.println("- Application modules");
  app_init();

#if ENABLE_TFT_TESTS
  Serial.println("TFT display tests available - press '4' to run");
#endif

#if ENABLE_IMU_TESTS
  Serial.println(
      "IMU gesture tests enabled - UP/DOWN/LEFT/RIGHT detection active");
#endif

  current_boot_stage = BOOT_STAGE_COMPLETE;
  return BOOT_OK;
}

//** 完整启动序列 - 一键启动，带错误处理
boot_result_t system_boot_sequence(void) {
  boot_result_t result;

  // 阶段1：早期初始化
  result = system_early_init();
  if (result != BOOT_OK) {
    return result;
  }

  // 阶段2：打印横幅
  system_print_banner();

  // 阶段3：硬件初始化
  result = hardware_init_all();
  if (result != BOOT_OK) {
    Serial.printf("Hardware initialization failed at stage: %s\n",
                  get_boot_stage_name(current_boot_stage));
    return result;
  }

  // 阶段4：应用初始化
  result = application_init_all();
  if (result != BOOT_OK) {
    Serial.printf("Application initialization failed at stage: %s\n",
                  get_boot_stage_name(current_boot_stage));
    return result;
  }

  Serial.println("✓ All systems initialized successfully");
  
  // 打印存储系统状态 - 简单版本
  Serial.println("=== Storage System Status ===");
  if (SPIFFS.begin(false)) {
    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();
    Serial.printf("Flash (SPIFFS): %zu/%zu bytes (%.1f%% used)\n", 
                  used, total, (float)used / total * PERCENTAGE_MULTIPLIER);
  } else {
    Serial.println("Flash (SPIFFS): ERROR - Mount failed");
  }
  
#if ENABLE_SD_TESTS
  if (SD_MMC.cardSize() > 0) {
    uint64_t cardSize = SD_MMC.cardSize() / BYTES_TO_MB; // 原魔数: (1024 * 1024)
    Serial.printf("SD Card: %lluMB available\n", cardSize);
  } else {
    Serial.println("SD Card: Not available");
  }
#else
  Serial.println("SD Card: Tests disabled");
#endif
  Serial.println("=============================");
  
  return BOOT_OK;
}

//** 实现Flash信息打印函数
void debug_flash_info() {
#if DEBUG_FLASH_ENABLED && (GLOBAL_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
    Serial.println("========================================");
    Serial.println("=== Flash Partition Information ===");
    Serial.println("========================================");
    Serial.printf("[FLASH] Total size: %u bytes (%.2f MB)\n", 
        ESP.getFlashChipSize(), ESP.getFlashChipSize() / KB_TO_MB_DIVISOR / BYTES_TO_KB); // 原魔数: 1024.0 / 1024.0
    Serial.printf("[FLASH] Speed: %u Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("[FLASH] Mode: %u\n", ESP.getFlashChipMode());
    
    // SPIFFS信息
    if (SPIFFS.begin(false)) {
        size_t total_bytes = SPIFFS.totalBytes();
        size_t used_bytes = SPIFFS.usedBytes();
        Serial.printf("[SPIFFS] Total: %zu bytes (%.2f MB)\n", 
            total_bytes, total_bytes / KB_TO_MB_DIVISOR / BYTES_TO_KB); // 原魔数: 1024.0 / 1024.0
        Serial.printf("[SPIFFS] Used: %zu bytes (%.2f MB)\n", 
            used_bytes, used_bytes / KB_TO_MB_DIVISOR / BYTES_TO_KB); // 原魔数: 1024.0 / 1024.0
        Serial.printf("[SPIFFS] Free: %zu bytes (%.2f MB)\n", 
            total_bytes - used_bytes, (total_bytes - used_bytes) / KB_TO_MB_DIVISOR / BYTES_TO_KB); // 原魔数: 1024.0 / 1024.0
    } else {
        Serial.println("[SPIFFS] Not mounted");
    }
    Serial.println("========================================");
#endif
}
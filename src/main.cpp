//** ESP32-S3 HoloCubic - Main Entry Point
//** Linus原则：main.cpp 只做启动和调度，像 Linux 内核的 init/main.c

#include "app_main.h"        // 应用主程序
#include "debug_config.h"    // 必须首先包含，定义所有编译开关
#include "hardware_config.h" // 硬件配置
#include "panic.h"           // 系统恐慌处理
#include "system_boot.h"     // 系统启动
#include <Arduino.h>

#if ENABLE_TFT_TESTS
#include "test/tft_display_test.h"
#endif

#if ENABLE_IMU_TESTS
#include "imu_gesture_driver.h" // 使用库版本的IMU手势驱动 - Arduino自动找到lib/IMUGesture/src/
#include "test/imu_gesture_test.h" // 使用主工程的测试模块 - 测试代码属于主工程
#endif

#if ENABLE_FLASH_TESTS
#include "test/storage_test.h"
#endif

#if ENABLE_SD_TESTS
#include "test/sd_card_diagnostic.h"
#endif

//** Arduino setup() - 系统启动入口点
//** Linus原则：main() 只做调度，具体实现在独立模块中
void setup() {
  boot_result_t result = system_boot_sequence();

  if (result != BOOT_OK) {
    Serial.printf("FATAL: Boot failed with error %d at stage: %s\n", result,
                  get_boot_stage_name(get_boot_stage()));
    system_panic(PANIC_BOOT_FAILED, "System boot sequence failed");
  }

  //** 初始化成功后，执行一次性存储测试写入
  
#if ENABLE_FLASH_TESTS
  Serial.println("=== Flash Storage Write Test ===");
  
  // Flash写入测试 - 只执行一次
  if (storage_test_write_ssid()) {
    Serial.println("✓ Flash write completed in setup()");
  } else {
    Serial.println("✗ Flash write failed in setup()");
  }
  
  Serial.println("=== Flash Storage Write Complete ===");
#endif

#if ENABLE_SD_TESTS
  Serial.println("=== SD Card Storage Write Test ===");
  
  // SD卡诊断 - 帮助调试连接问题
  Serial.println("=== Running SD Card Diagnostic ===");
  sd_card_diagnostic_run();
  Serial.println("=== SD Card Diagnostic Complete ===");
  
  // SD卡写入测试 - 只执行一次
  if (storage_test_write_password_sd()) {
    Serial.println("✓ SD card write completed in setup()");
  } else {
    Serial.println("✗ SD card write failed in setup()");
  }
  
  Serial.println("=== SD Card Storage Write Complete ===");
#endif

  //** 初始化成功 - setup() 结束后 Arduino 会自动调用 loop()
}

//** Arduino loop() - 应用主循环
void loop() {
  //** 定期健康检查 - 像 Linux 内核的软看门狗
  static uint32_t last_health_check = 0;
  uint32_t now = millis();

  if (now - last_health_check > HW_SYSTEM_HEALTH_CHECK_MS) { // 系统健康检查间隔
    if (!system_health_check()) {
      system_panic(PANIC_OUT_OF_MEMORY, "System health check failed");
    }
    last_health_check = now;
  }

  //** 运行应用逻辑Wi-Fi，heart beat，command_handler等。
  app_run();

#if ENABLE_TFT_TESTS
  //** TFT显示更新 - 显示WiFi状态和存储内容
  static uint32_t last_tft_update = 0;

  if (now - last_tft_update > HW_SYSTEM_TFT_UPDATE_MS) { // TFT显示更新间隔
    Serial.println("=== Auto TFT Display Update ===");
    tft_display_test_run(); // 显示WiFi状态、Flash和SD卡内容
    last_tft_update = now;
  }
#endif

#if ENABLE_IMU_TESTS
  //** IMU手势识别测试 - 使用库版本的驱动和主工程的测试模块
  ImuGestureData *gesture_data = imu_gesture_get_data();
  imu_test_gesture_recognition(
      gesture_data);                  // 调用主工程的测试函数，内部会重置isValid
  imu_test_configuration_functions(); // 调用主工程的配置测试函数
  imu_test_display_sensor_data(gesture_data); // 显示实时传感器数据
#endif

  //** 存储数据已在setup()中写入，loop()中不需要重复验证
  //** 如果需要验证，可以通过TFT显示来确认数据是否正确显示
}
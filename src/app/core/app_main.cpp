//** ESP32-S3 HoloCubic - Application Main Controller
//** Linus原则：应用生命周期管理 - "Clear separation of concerns"
//** 职责：应用初始化、运行时调度、资源协调

#include "app_main.h"
#include "../../core/config/app_constants.h"
#include "../interface/command_handler.h"
#include "../managers/led_manager.h"
#include "../monitoring/heartbeat.h"
#include "../network/wifi_app.h"
#include <Arduino.h>

//** 简单的全局变量

uint32_t g_app_start_time = 0;

void app_init(void) {

  Serial.println("初始化应用模块...");

  //** WiFi应用初始化 - 只初始化，不连接
  Serial.println("- WiFi应用");
  wifi_app_init();

  //** 应用模块初始化
  Serial.println("- 命令处理器");
  command_handler_init();

  Serial.println("- 心跳监控");
  heartbeat_init();

  Serial.println("✓ 应用模块初始化完成");

  g_app_start_time = millis();
}

void app_run(void) {
  //** WiFi应用处理
  wifi_app_process();

  //** LED管理器处理
  led_process();

  //** WiFi状态LED指示 - 低优先级，不会干扰测试
  static uint32_t last_wifi_led_update = 0;
  uint32_t now = millis();

  if (now - last_wifi_led_update >
      WIFI_LED_UPDATE_INTERVAL_MS) { // 每2秒更新一次 (原魔数: 2000)
    const wifi_app_t *wifi_state = wifi_app_get_state();
    if (wifi_state->is_ready) {
      //** WiFi连接 - 绿色闪烁一次
      led_set_blink(LED_PRIORITY_SYSTEM, LED_COLOR_MIN_VALUE, LED_COLOR_MAX_VALUE, LED_COLOR_MIN_VALUE, LED_BLINK_ON_MS,
                    LED_BLINK_OFF_MS); // 原魔数: 0, 255, 0, 200, 200
    } else {
      //** WiFi未连接 - 红色闪烁一次
      led_set_blink(LED_PRIORITY_SYSTEM, LED_COLOR_MAX_VALUE, LED_COLOR_MIN_VALUE, LED_COLOR_MIN_VALUE, LED_BLINK_ON_MS,
                    LED_BLINK_OFF_MS); // 原魔数: 255, 0, 0, 200, 200
    }
    last_wifi_led_update = now;
  }

  //** 处理其他模块
  command_handler_process();
  heartbeat_process();

  delay(10);
}

void app_cleanup(void) {

  led_set_off(LED_PRIORITY_SYSTEM);

  g_app_start_time = 0;

  Serial.println("应用清理完成");
}
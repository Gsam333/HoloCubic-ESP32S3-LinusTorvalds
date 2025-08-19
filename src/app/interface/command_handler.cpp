//** ESP32-S3 HoloCubic - Command Handler Implementation
//** Linus原则：简单的命令分发，无复杂逻辑

#include "command_handler.h"
#include "../../config/app_config.h" // 测试代码控制
#include "../network/wifi_app.h"
#include "../../core/config/app_constants.h"

#if ENABLE_LED_TESTS
#include "../../test/led_test.h"
#endif

#if ENABLE_TFT_TESTS
#include "../../test/tft_display_test.h"
#endif

#if ENABLE_DEBUG_COMMANDS
#include "../../system/debug_utils.h"
#endif

#include "../../drivers/led/led_driver.h"
#include <Arduino.h>

void command_handler_init(void) { 
  //** 命令处理器初始化 - 无需状态跟踪
}

static void show_help(void) {
  Serial.println("\n=== Commands ===");
  Serial.println("h - Help");

#if ENABLE_DEBUG_COMMANDS
  Serial.println("c - Show config");
#endif

  //** WiFi commands
  Serial.println("w - WiFi status");

#if ENABLE_LED_TESTS
  Serial.println("1 - LED Basic test");
  Serial.println("2 - LED HSV test");
  Serial.println("3 - LED Brightness test");
#endif

#if ENABLE_TFT_TESTS
  Serial.println("4 - TFT Display test (WiFi info display)");
#endif

  Serial.println("r/g/b - Red/Green/Blue");
  Serial.println("o - Off");

#if ENABLE_TEST_CODE
  Serial.println("--- Test Commands (Development Only) ---");
#endif

  Serial.println("================\n");
}

void command_handler_process(void) {
  if (!Serial.available()) {
    return;
  }

  char cmd = Serial.read();

  //** WiFi状态查询 - 只读取，不管理
  if (cmd == 'w') {
    const wifi_app_t *wifi_state = wifi_app_get_state();
    Serial.println("\n=== WiFi Status ===");
    if (wifi_state->is_ready) {
      Serial.println("✓ Connected");
      Serial.print("Signal: ");
      Serial.print(wifi_state->rssi);
      Serial.println(" dBm");
      uint32_t uptime = (millis() - wifi_state->connect_time) / MILLISECONDS_TO_SECONDS; // 原魔数: 1000
      Serial.print("Uptime: ");
      Serial.print(uptime / SECONDS_TO_MINUTES); // 原魔数: 60
      Serial.print("m ");
      Serial.print(uptime % SECONDS_TO_MINUTES); // 原魔数: 60
      Serial.println("s");
    } else {
      Serial.println("✗ Not Connected");
      const char *state_str;
      switch (wifi_state->state) {
      case WIFI_STATE_CONNECTING:
        state_str = "Connecting...";
        break;
      case WIFI_STATE_FAILED:
        state_str = "Failed";
        break;
      default:
        state_str = "Idle";
        break;
      }
      Serial.print("State: ");
      Serial.println(state_str);
    }
    Serial.println("==================\n");
    return;
  }

  switch (cmd) {
  case 'h':
    show_help();
    break;

#if ENABLE_DEBUG_COMMANDS
  case 'c':
    debug_print_hw_config();
    break;
#endif

#if ENABLE_LED_TESTS
  case '1':
    led_test_basic();
    break;

  case '2':
    led_test_hsv();
    break;

  case '3':
    led_test_brightness();
    break;
#endif

#if ENABLE_TFT_TESTS
  case '4':
    tft_display_test_run();
    break;
#endif

  //** 基础LED控制 - 生产环境保留
  case 'r':
    led_red();
    Serial.println("LED: Red");
    break;

  case 'g':
    led_green();
    Serial.println("LED: Green");
    break;

  case 'b':
    led_blue();
    Serial.println("LED: Blue");
    break;

  case 'o':
    led_off();
    Serial.println("LED: Off");
    break;

#if ENABLE_TEST_CODE
  default:
    if (cmd >= ASCII_PRINTABLE_START && cmd <= ASCII_PRINTABLE_END) { // 可打印字符 (原魔数: 32, 126)
      Serial.printf("Unknown command: '%c' (Test mode enabled)\n", cmd);
    }
    break;
#endif
  }
}
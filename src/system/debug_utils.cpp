//** ESP32-S3 HoloCubic - Debug Utilities Implementation
//** Linus原则：调试功能集中管理

#include "debug_utils.h"
#include "core/config/hardware_config.h"
#include <Arduino.h>
#include <TFT_eSPI.h>  // 需要TFT引脚定义

void debug_print_hw_config(void) {
  Serial.println("\n=== Hardware Configuration ===");
  Serial.printf("Board: ESP32-S3 HoloCubic\n");
  Serial.printf("CPU: %u MHz\n", HW_SYSTEM_CPU_MHZ);
  Serial.printf("Serial: %u baud\n", HW_SYSTEM_SERIAL_BAUD);

  Serial.println("\nLED (WS2812):");
  Serial.printf("  Pin: %d, Count: %d, Brightness: %d\n", 
                HW_LED_PIN, HW_LED_COUNT, HW_LED_BRIGHTNESS);

  Serial.println("\nDisplay (ST7789):");
  Serial.printf("  SPI: MOSI=%d, SCLK=%d, CS=%d\n", 
                TFT_MOSI, TFT_SCLK, TFT_CS);
  Serial.printf("  Control: DC=%d, RST=%d, BL=%d\n", 
                TFT_DC, TFT_RST, TFT_BL);
  Serial.printf("  Resolution: %dx%d\n", HW_DISPLAY_WIDTH, HW_DISPLAY_HEIGHT);

  Serial.println("\nIMU (QMI8658):");
  Serial.printf("  I2C: SDA=%d, SCL=%d, Addr=0x%02X\n", 
                HW_IMU_SDA, HW_IMU_SCL, HW_IMU_ADDRESS);

  Serial.println("============================\n");
}

void debug_print_system_status(void) {
  Serial.printf("System Status:\n");
  Serial.printf("  Uptime: %lu ms\n", millis());  // %lu for unsigned long
  Serial.printf("  Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("  CPU freq: %u MHz\n", ESP.getCpuFreqMHz());
}
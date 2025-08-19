//** TFT显示驱动实现 - Linus风格 / TFT Display Driver Implementation - Linus Style
//**
//** "Bad programmers worry about the code. Good programmers worry about data structures."
//** - Linus Torvalds
//**
//** 核心设计决策 / Core Design Decisions:
//** 1. 单一静态TFT_eSPI实例 - 零动态分配 / Single static TFT_eSPI instance - zero dynamic allocation
//** 2. 无状态管理 - 调用者负责顺序 / No state management - caller responsible for order
//** 3. 直接调用 - 无包装开销 / Direct calls - no wrapper overhead
//** 4. 快速失败 - 无隐藏错误处理 / Fail fast - no hidden error handling

#include "display_driver.h"
#include "hardware_config.h"  //** 硬件配置常量 / Hardware configuration constants
#include "../../core/config/app_constants.h"  //** 应用常量 / Application constants
#include <Arduino.h>  //** 仅用于PWM函数 / Only for PWM functions

//** ========================================
//** 编译时验证 - Linus风格的"快速失败" / Compile-time Verification - Linus Style "Fail Fast"
//** ========================================

//** 验证TFT_eSPI字体配置 - 防止运行时NULL指针崩溃
//** Verify TFT_eSPI font configuration - prevent runtime NULL pointer crashes
#ifndef LOAD_GLCD
#error "LOAD_GLCD must be defined in TFT_eSPI setup - missing causes runtime crashes!"
#endif

//** 【Linus式设计改进】
//** 移除重复的引脚验证 - 现在引脚配置统一在Setup24_ST7789.h中
//** 只验证TFT_eSPI必需的配置项
#ifndef TFT_MOSI
#error "TFT_MOSI must be defined in TFT_eSPI setup"
#endif

#ifndef TFT_DC  
#error "TFT_DC must be defined in TFT_eSPI setup"
#endif

#ifndef TFT_RST
#error "TFT_RST must be defined in TFT_eSPI setup"
#endif

//** ========================================
//** 静态数据结构 - Linus的"好品味" / Static Data Structure - Linus's "Good Taste"
//** ========================================

//** 单一静态TFT显示器实例 - 直接实例化，避免指针复杂性
//** Single static TFT display instance - direct instantiation, avoid pointer complexity
static TFT_eSPI tft_display;

//** Linus原则：移除初始化标志 - 调用者负责正确的调用顺序
//** Linus principle: Remove initialization flag - caller responsible for correct order

//** ========================================
//** 核心初始化函数 / Core Initialization Functions
//** ========================================

void display_init(void) {
    display_init_with_params(HW_DISPLAY_DEFAULT_ROTATION, HW_DISPLAY_DEFAULT_BRIGHTNESS);
}

void display_init_with_params(uint8_t rotation, uint8_t backlight) {
    DISPLAY_DEBUG("Initializing display: rotation=%d, backlight=%d", rotation, backlight);

    //** 完全按照父工程的成功顺序 - 消除所有"聪明"的优化
    //** Exactly follow parent project's successful sequence - eliminate all "clever" optimizations

    //** 第1步：初始化背光PWM / Step 1: Initialize backlight PWM
    ledcSetup(HW_DISPLAY_PWM_CHANNEL, HW_DISPLAY_PWM_FREQUENCY, HW_DISPLAY_PWM_RESOLUTION);
    ledcAttachPin(TFT_BL, HW_DISPLAY_PWM_CHANNEL);  // 直接使用TFT_eSPI的定义

    //** 第2步：设置初始亮度为0 / Step 2: Set initial brightness to 0
    ledcWrite(HW_DISPLAY_PWM_CHANNEL, PWM_MAX_VALUE); // 反转逻辑：255 = 0%亮度 (原魔数: 255)

    //** 第3步：初始化TFT显示屏 / Step 3: Initialize TFT display
    tft_display.begin();

    DISPLAY_DEBUG("TFT begin() completed");
    DISPLAY_DEBUG("TFT width: %d, height: %d", tft_display.width(), tft_display.height());

    //** 第4步：清屏并启用显示 / Step 4: Clear screen and enable display
    tft_display.fillScreen(TFT_BLACK);
    tft_display.writecommand(ST7789_DISPON); // Display on

    DISPLAY_DEBUG("Display ON command sent");

    //** 第5步：设置旋转模式 / Step 5: Set rotation mode
    tft_display.setRotation(rotation);

    //** 第6步：设置最终亮度 / Step 6: Set final brightness
    display_backlight(backlight / PERCENTAGE_TO_FLOAT_DIVISOR); // 原魔数: 100.0f

    //** 第7步：最终清屏 / Step 7: Final clear
    tft_display.fillScreen(TFT_BLACK);

    DISPLAY_DEBUG("Display initialized successfully: %dx%d, rotation=%d", 
                  tft_display.width(), tft_display.height(), rotation);
}

//** ========================================
//** 基本绘图操作 - 零开销包装器 / Basic Drawing Operations - Zero Overhead Wrappers
//** ========================================

void display_clear(uint16_t color) { 
    tft_display.fillScreen(color); 
}

void display_pixel(int16_t x, int16_t y, uint16_t color) {
    tft_display.drawPixel(x, y, color);
}

void display_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    tft_display.drawLine(x0, y0, x1, y1, color);
}

void display_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft_display.fillRect(x, y, w, h, color);
}

//** ========================================
//** 配置函数 - 直接硬件控制 / Configuration Functions - Direct Hardware Control
//** ========================================

void display_backlight(float duty) {
    //** 限制范围到0.0-1.0 / Clamp range to 0.0-1.0
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    //** 反转逻辑：ESP32-S3 HoloCubic硬件特性 / Inverted logic: ESP32-S3 HoloCubic hardware characteristic
    duty = 1.0f - duty;

    //** 直接PWM控制 / Direct PWM control
    uint8_t pwm_value = (uint8_t)(duty * PWM_MAX_VALUE); // 原魔数: 255
    ledcWrite(HW_DISPLAY_PWM_CHANNEL, pwm_value);

    DISPLAY_DEBUG("Backlight set: duty=%.2f, pwm=%d", 1.0f - duty, pwm_value);
}

void display_rotation(uint8_t rotation) {
    tft_display.setRotation(rotation);
    DISPLAY_DEBUG("Rotation set: %d", rotation);
}

//** ========================================
//** 信息查询函数 - 直接TFT_eSPI访问 / Information Query Functions - Direct TFT_eSPI Access
//** ========================================

int16_t display_width(void) { 
    return tft_display.width(); 
}

int16_t display_height(void) { 
    return tft_display.height(); 
}

uint8_t display_get_rotation(void) { 
    return tft_display.getRotation(); 
}

//** ========================================
//** 直接TFT访问 - 高级用法 / Direct TFT Access - Advanced Usage
//** ========================================

TFT_eSPI* display_tft(void) { 
    return &tft_display; 
}

//** ========================================
//** 调试函数 / Debug Functions
//** ========================================

void display_debug_config(void) {
#ifdef DEBUG_DISPLAY
    Serial.println("=== Display Driver Configuration ===");
    Serial.printf("Hardware Config (from TFT_eSPI setup):\n");
    Serial.printf("  MISO=%d, MOSI=%d, SCLK=%d, CS=%d\n", 
                  TFT_MISO, TFT_MOSI, TFT_SCLK, TFT_CS);
    Serial.printf("  DC=%d, RST=%d, BL=%d\n", 
                  TFT_DC, TFT_RST, TFT_BL);
    Serial.printf("  SPI Freq=%d Hz\n", SPI_FREQUENCY);

    Serial.printf("Runtime Status:\n");
    //** Linus原则：移除初始化检查，调用者负责正确顺序
    Serial.printf("  Current Size: %dx%d\n", tft_display.width(), tft_display.height());
    Serial.printf("  Current Rotation: %d\n", tft_display.getRotation());

    Serial.printf("PWM Config:\n");
    Serial.printf("  Channel=%d, Freq=%d Hz, Resolution=%d bits\n",
                  HW_DISPLAY_PWM_CHANNEL, HW_DISPLAY_PWM_FREQUENCY, HW_DISPLAY_PWM_RESOLUTION);
    Serial.println("=====================================");
#else
    //** 在非调试模式下什么都不做 / Do nothing in non-debug mode
#endif
}
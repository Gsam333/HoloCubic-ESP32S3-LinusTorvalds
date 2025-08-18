#pragma once

//** TFT显示驱动 - Linus风格扁平接口 / TFT Display Driver - Linus Style Flat Interface
//**
//** 设计哲学 / Design Philosophy:
//** "Bad programmers worry about the code. Good programmers worry about data structures."
//**
//** 核心原则 / Core Principles:
//** 1. 零状态管理 - 调用者负责顺序 / Zero state management - caller responsible for order
//** 2. 静态内存分配 - 无动态分配 / Static memory allocation - no dynamic allocation
//** 3. 直接硬件访问 - 最小包装开销 / Direct hardware access - minimal wrapper overhead
//** 4. 快速失败 - 无隐藏错误 / Fail fast - no hidden errors
//** 5. 清洁依赖 - hardware_config.h + TFT_eSPI.h / Clean dependencies - hardware_config.h + TFT_eSPI.h

#include "hardware_config.h"
#include <TFT_eSPI.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//** ========================================
//** 核心显示函数 - 纯C接口 / Core Display Functions - Pure C Interface
//** ========================================

//** 初始化函数 / Initialization Functions
void display_init(void);
void display_init_with_params(uint8_t rotation, uint8_t backlight);

//** 基本绘图操作 / Basic Drawing Operations
void display_clear(uint16_t color);
void display_pixel(int16_t x, int16_t y, uint16_t color);
void display_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void display_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

//** 配置函数 / Configuration Functions
void display_backlight(float duty);
void display_rotation(uint8_t rotation);

//** 信息查询函数 / Information Query Functions
int16_t display_width(void);
int16_t display_height(void);
uint8_t display_get_rotation(void);

//** 直接TFT访问 - 高级用法 / Direct TFT Access - Advanced Usage
TFT_eSPI* display_tft(void);

//** 调试函数 / Debug Functions
void display_debug_config(void);

//** ========================================
//** 硬件配置常量 / Hardware Configuration Constants
//** ========================================



//** 颜色常量 / Color Constants
//** 注意：此显示屏使用BGR颜色顺序 (TFT_RGB_ORDER = TFT_BGR)
//** 因此颜色值已经调整为BGR格式以显示正确的颜色
#define DISPLAY_BLACK 0x0000    // 黑色 (BGR: 00000 000000 00000)
#define DISPLAY_WHITE 0xFFFF    // 白色 (BGR: 11111 111111 11111)
#define DISPLAY_RED 0x001F      // 红色 (BGR: 00000 000000 11111) - 蓝色通道显示红色
#define DISPLAY_GREEN 0x07E0    // 绿色 (BGR: 00000 111111 00000) - 绿色通道不变
#define DISPLAY_BLUE 0xF800     // 蓝色 (BGR: 11111 000000 00000) - 红色通道显示蓝色
#define DISPLAY_YELLOW 0x07FF   // 黄色 (BGR: 00000 111111 11111) - 绿色+蓝色通道
#define DISPLAY_CYAN 0xFFE0     // 青色 (BGR: 11111 111111 00000) - 蓝色+绿色通道
#define DISPLAY_MAGENTA 0xF81F  // 洋红 (BGR: 11111 000000 11111) - 蓝色+红色通道

//** ========================================
//** 内联便捷函数 - 零开销快捷方式 / Inline Convenience Functions - Zero Overhead Shortcuts
//** ========================================

//** 常用清屏操作 / Common Clear Operations
static inline void display_clear_black(void) { display_clear(DISPLAY_BLACK); }
static inline void display_clear_white(void) { display_clear(DISPLAY_WHITE); }

//** 背光快捷操作 / Backlight Shortcuts
static inline void display_backlight_full(void) { display_backlight(1.0f); }
static inline void display_backlight_off(void) { display_backlight(0.0f); }
static inline void display_backlight_dim(void) { display_backlight(0.1f); }
static inline void display_backlight_normal(void) { display_backlight(0.8f); }

//** 旋转模式快捷操作 / Rotation Mode Shortcuts
static inline void display_rotation_standard(void) { display_rotation(0); }
static inline void display_rotation_holographic(void) { display_rotation(4); }

//** 基本形状绘制 / Basic Shape Drawing
static inline void display_pixel_white(int16_t x, int16_t y) { display_pixel(x, y, DISPLAY_WHITE); }
static inline void display_pixel_red(int16_t x, int16_t y) { display_pixel(x, y, DISPLAY_RED); }

//** 屏幕中心计算 / Screen Center Calculation
static inline int16_t display_center_x(void) { return display_width() / 2; }
static inline int16_t display_center_y(void) { return display_height() / 2; }

//** ========================================
//** 编译时验证 / Compile-Time Validation
//** ========================================

//** 验证硬件配置的合理性 / Validate hardware configuration sanity
#if HW_DISPLAY_WIDTH <= 0 || HW_DISPLAY_HEIGHT <= 0
#error "Invalid display dimensions in hardware_config.h - must be positive"
#endif

#if HW_DISPLAY_SPI_FREQ < 1000000
#error "Display SPI frequency too low in hardware_config.h - minimum 1MHz"
#endif

#if HW_DISPLAY_SPI_FREQ > 80000000
#error "Display SPI frequency too high in hardware_config.h - maximum 80MHz for ESP32-S3"
#endif

#if DISPLAY_DEFAULT_BACKLIGHT < 0 || DISPLAY_DEFAULT_BACKLIGHT > 100
#error "Invalid default backlight value - must be 0-100"
#endif

//** 验证引脚范围 / Validate pin ranges for ESP32-S3
#if HW_DISPLAY_MOSI > 48 || HW_DISPLAY_SCLK > 48 || HW_DISPLAY_CS > 48
#error "Display SPI pins out of range for ESP32-S3 (0-48)"
#endif

#if HW_DISPLAY_DC > 48 || HW_DISPLAY_RST > 48 || HW_DISPLAY_BL > 48
#error "Display control pins out of range for ESP32-S3 (0-48)"
#endif

//** ========================================
//** 调试宏 / Debug Macros
//** ========================================

#ifdef DEBUG_DISPLAY
#define DISPLAY_DEBUG(fmt, ...) Serial.printf("[DISPLAY] " fmt "\n", ##__VA_ARGS__)
#else
#define DISPLAY_DEBUG(fmt, ...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif
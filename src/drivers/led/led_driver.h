//** ESP32-S3 HoloCubic - LED Driver (Linus Style)
//** "Good programmers worry about data structures and their relationships."

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//** LED 数据结构 - 简单直接，无状态跟踪垃圾
typedef struct {
    uint8_t r, g, b;
} led_color_t;

//** LED 接口 - 扁平化，无初始化检查垃圾
bool led_init(void);
void led_set_color(uint8_t r, uint8_t g, uint8_t b);
void led_set_hsv(uint8_t h, uint8_t s, uint8_t v);
void led_set_brightness(uint8_t brightness);
void led_off(void);

//** 便捷函数 - 直接设置，无废话
static inline void led_red(void)   { led_set_color(255, 0, 0); }
static inline void led_green(void) { led_set_color(0, 255, 0); }
static inline void led_blue(void)  { led_set_color(0, 0, 255); }

#ifdef __cplusplus
}
#endif
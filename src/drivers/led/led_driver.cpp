//** ESP32-S3 HoloCubic - LED Driver Implementation
//** "Bad programmers worry about the code. Good programmers worry about data structures."

#include "led_driver.h"
#include <FastLED.h>
#include "core/config/hardware_config.h"

//** 唯一的数据结构 - LED数组
static CRGB leds[HW_LED_COUNT];

//** 初始化 - 简单直接，无状态跟踪
bool led_init(void) {
    FastLED.addLeds<WS2812, HW_LED_PIN, GRB>(leds, HW_LED_COUNT);
    FastLED.setBrightness(HW_LED_BRIGHTNESS);
    
    //** 清除所有LED
    for (uint8_t i = 0; i < HW_LED_COUNT; i++) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
    return true;
}

//** 设置颜色 - 核心功能，无废话
void led_set_color(uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < HW_LED_COUNT; i++) {
        leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
}

//** 设置亮度 - 直接调用FastLED
void led_set_brightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
    FastLED.show();
}

//** 设置HSV颜色 - 直接使用FastLED转换
void led_set_hsv(uint8_t h, uint8_t s, uint8_t v) {
    CHSV hsv_color(h, s, v);
    for (uint8_t i = 0; i < HW_LED_COUNT; i++) {
        leds[i] = hsv_color;
    }
    FastLED.show();
}

//** 关闭LED - 就是设置为黑色
void led_off(void) {
    led_set_color(0, 0, 0);
}
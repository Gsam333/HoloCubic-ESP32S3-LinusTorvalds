//** Linus风格显示驱动使用示例 / Linus-style Display Driver Usage Example
//** 
//** "简洁胜过复杂" / "Simple is better than complex"

#include "display_driver.h"
#include "../../core/config/app_constants.h"

void example_basic_usage(void) {
    //** 基本使用 - 推荐方式 / Basic usage - recommended way
    display_init();
    display_clear_black();
    display_pixel_white(DISPLAY_CENTER_X, DISPLAY_CENTER_Y); // 原魔数: 120, 120
    display_backlight_normal();
}

void example_advanced_usage(void) {
    //** 高级使用 - 直接TFT访问 / Advanced usage - direct TFT access
    display_init();
    
    //** 类型安全，简洁明了 / Type-safe and straightforward
    TFT_eSPI* tft = display_tft();
    tft->setTextColor(TFT_WHITE);
    tft->drawString("Hello Linus!", DISPLAY_TEXT_START_X, DISPLAY_TEXT_START_Y); // 原魔数: 10, 10
    tft->drawCircle(DISPLAY_CENTER_X, DISPLAY_CENTER_Y, DISPLAY_CIRCLE_RADIUS, TFT_RED); // 原魔数: 120, 120, 50
}

//** ========================================
//** Linus风格设计的胜利 / Victory of Linus-style Design
//** ========================================
//**
//** 设计文档中验证的洞察：
//** Insight verified in design document:
//** - hardware_config.h (数据) + TFT_eSPI.h (行为) = 完美的关注点分离
//** - hardware_config.h (data) + TFT_eSPI.h (behavior) = Perfect separation of concerns
//**
//** Linus风格的简单方案：
//** Linus-style simple approach:
//** - 直接包含需要的头文件 (hardware_config.h + TFT_eSPI.h)
//** - 静态TFT_eSPI对象 (static TFT_eSPI tft_display;)
//** - 类型安全的TFT_eSPI*返回
//** - 零复杂性，零特殊情况
//**
//** "好品味就是消除特殊情况，使用清晰的数据结构"
//** "Good taste is about eliminating special cases and using clear data structures"
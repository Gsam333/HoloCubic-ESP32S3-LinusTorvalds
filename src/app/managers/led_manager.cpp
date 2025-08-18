//** ESP32-S3 HoloCubic - LED Resource Manager
//** Linus原则：单一所有权 - "One owner per resource, eliminate race conditions"
//** 职责：LED资源统一管理，优先级仲裁，消除资源竞争

#include "led_manager.h"
#include "../../core/types/error_handling.h"
#include "../../drivers/led/led_driver.h"
#include <Arduino.h>

//** 全局状态 - 单一数据源
static led_request_t g_current_request = {
    .priority = LED_PRIORITY_IDLE,
    .mode = LED_MODE_OFF,
    .red = 0, .green = 0, .blue = 0,
    .period_ms = 0,
    .duration_ms = 0,
    .start_time = 0
};



void led_manager_init(void) {
    //** 初始化LED驱动
    if (!led_init()) {
        LOG_ERROR("LED driver initialization failed");
        return;
    }
    
    //** 重置状态
    g_current_request.priority = LED_PRIORITY_IDLE;
    g_current_request.mode = LED_MODE_OFF;
    g_current_request.start_time = millis();
    
    led_off();
    
    LOG_SUCCESS("LED Manager initialized");
}

bool led_request(const led_request_t* request) {
    RETURN_FALSE_IF_NULL(request);
    
    //** 检查优先级 - 只有更高或相等优先级才能覆盖
    if (request->priority < g_current_request.priority) {
        LOG_WARNING_F("LED request rejected: priority %d < current %d", 
                     request->priority, g_current_request.priority);
        return false;
    }
    
    //** 接受请求
    g_current_request = *request;
    g_current_request.start_time = millis();
    
    return true;
}

//** 检查LED请求是否超时
static bool led_check_timeout(uint32_t now) {
    if (g_current_request.duration_ms > 0 && 
        (now - g_current_request.start_time) >= g_current_request.duration_ms) {
        //** 超时，回到空闲状态
        g_current_request.priority = LED_PRIORITY_IDLE;
        g_current_request.mode = LED_MODE_OFF;
        led_off();
        return true;
    }
    return false;
}

//** 处理闪烁模式
static void led_handle_blink(uint32_t now) {
    uint32_t elapsed = now - g_current_request.start_time;
    uint32_t cycle_pos = elapsed % g_current_request.period_ms;
    
    if (cycle_pos < (g_current_request.period_ms / 2)) {
        led_set_color(g_current_request.red, g_current_request.green, g_current_request.blue);
    } else {
        led_off();
    }
}

//** 处理呼吸灯模式
static void led_handle_pulse(uint32_t now) {
    uint32_t elapsed = now - g_current_request.start_time;
    uint32_t cycle_pos = elapsed % g_current_request.period_ms;
    float brightness = (sin(2.0 * PI * cycle_pos / g_current_request.period_ms) + 1.0) / 2.0;
    
    uint8_t r = (uint8_t)(g_current_request.red * brightness);
    uint8_t g = (uint8_t)(g_current_request.green * brightness);
    uint8_t b = (uint8_t)(g_current_request.blue * brightness);
    
    led_set_color(r, g, b);
}

void led_process(void) {
    uint32_t now = millis();
    
    //** 检查超时
    if (led_check_timeout(now)) {
        return;
    }
    
    //** 根据模式控制LED
    switch (g_current_request.mode) {
        case LED_MODE_OFF:
            led_off();
            break;
            
        case LED_MODE_SOLID:
            led_set_color(g_current_request.red, g_current_request.green, g_current_request.blue);
            break;
            
        case LED_MODE_BLINK:
            led_handle_blink(now);
            break;
            
        case LED_MODE_PULSE:
            led_handle_pulse(now);
            break;
    }
}

//** 快速接口实现
bool led_set_solid(led_priority_t priority, uint8_t r, uint8_t g, uint8_t b, uint32_t duration_ms) {
    led_request_t request = {
        .priority = priority,
        .mode = LED_MODE_SOLID,
        .red = r, .green = g, .blue = b,
        .period_ms = 0,
        .duration_ms = duration_ms,
        .start_time = 0
    };
    return led_request(&request);
}

bool led_set_blink(led_priority_t priority, uint8_t r, uint8_t g, uint8_t b, uint16_t period_ms, uint32_t duration_ms) {
    led_request_t request = {
        .priority = priority,
        .mode = LED_MODE_BLINK,
        .red = r, .green = g, .blue = b,
        .period_ms = period_ms,
        .duration_ms = duration_ms,
        .start_time = 0
    };
    return led_request(&request);
}

bool led_set_off(led_priority_t priority) {
    led_request_t request = {
        .priority = priority,
        .mode = LED_MODE_OFF,
        .red = 0, .green = 0, .blue = 0,
        .period_ms = 0,
        .duration_ms = 0,
        .start_time = 0
    };
    return led_request(&request);
}

void led_release(led_priority_t priority) {
    if (g_current_request.priority == priority) {
        g_current_request.priority = LED_PRIORITY_IDLE;
        g_current_request.mode = LED_MODE_OFF;
        led_off();
    }
}
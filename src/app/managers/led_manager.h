//** ESP32-S3 HoloCubic - LED Resource Manager
//** Linus原则：单一所有权 - "One owner per resource, eliminate race conditions"
//** 职责：LED资源统一管理，优先级仲裁，消除资源竞争

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

//** 只有LED管理器可以控制LED，其他模块只能请求

// ========================================
// LED状态优先级
// ========================================

typedef enum {
    LED_PRIORITY_IDLE = 0,      // 空闲状态
    LED_PRIORITY_SYSTEM = 1,    // 系统状态（WiFi连接等）
    LED_PRIORITY_TEST = 2,      // 测试模式
    LED_PRIORITY_PANIC = 3      // 紧急状态（最高优先级）
} led_priority_t;

typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_SOLID,     // 固定颜色
    LED_MODE_BLINK,     // 闪烁
    LED_MODE_PULSE      // 呼吸灯
} led_mode_t;

typedef struct {
    led_priority_t priority;
    led_mode_t mode;
    uint8_t red, green, blue;
    uint16_t period_ms;     // 闪烁/呼吸周期
    uint32_t duration_ms;   // 持续时间，0=永久
    uint32_t start_time;    // 开始时间
} led_request_t;

// ========================================
// LED管理器接口
// ========================================

//** 初始化LED管理器 - 调用者必须在使用其他函数前调用
void led_manager_init(void);

//** 处理LED状态 - 在主循环中调用
void led_process(void);

//** 请求LED控制
bool led_request(const led_request_t* request);

//** 快速请求接口
bool led_set_solid(led_priority_t priority, uint8_t r, uint8_t g, uint8_t b, uint32_t duration_ms);
bool led_set_blink(led_priority_t priority, uint8_t r, uint8_t g, uint8_t b, uint16_t period_ms, uint32_t duration_ms);
bool led_set_off(led_priority_t priority);

//** 释放控制权
void led_release(led_priority_t priority);

#endif // LED_MANAGER_H
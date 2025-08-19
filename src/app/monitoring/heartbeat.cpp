//** ESP32-S3 HoloCubic - Heartbeat Monitor Implementation  
//** Linus原则：非阻塞心跳，使用系统状态

#include "heartbeat.h"
#include "../../core/config/hardware_config.h"
#include "../../core/state/system_state.h"
#include "../../core/config/app_constants.h"
#include "../../drivers/led/led_driver.h"
#include <Arduino.h>

//** 心跳状态机
typedef enum {
    HEARTBEAT_IDLE,
    HEARTBEAT_LED_ON,
    HEARTBEAT_LED_OFF
} heartbeat_phase_t;

static heartbeat_phase_t heartbeat_phase = HEARTBEAT_IDLE;
static uint32_t phase_start_ms = 0;

void heartbeat_init(void) {
    HEARTBEAT_STATE()->last_beat_ms = millis();
    HEARTBEAT_STATE()->interval_ms = HEARTBEAT_DEFAULT_INTERVAL_MS; // 原魔数: 1000
    HEARTBEAT_STATE()->beat_count = 0;
    heartbeat_phase = HEARTBEAT_IDLE;
}

//** 处理心跳空闲状态
static void heartbeat_handle_idle(uint32_t now, decltype(g_system_state.heartbeat)* hb) {
    //** 检查是否到了心跳时间
    if (now - hb->last_beat_ms < hb->interval_ms) {
        return;  // 还没到心跳时间
    }
    
    //** 开始心跳 - 点亮绿色LED
    led_green();
    
    heartbeat_phase = HEARTBEAT_LED_ON;
    phase_start_ms = now;
    hb->beat_count++;
    
    //** 打印系统状态（每10次心跳打印一次）
    if (hb->beat_count % 10 == 0) {
        Serial.printf("Heartbeat #%u - Uptime: %u ms, Free heap: %u bytes\n", 
                     hb->beat_count, now, ESP.getFreeHeap());
    }
}

//** 处理LED点亮状态
static void heartbeat_handle_led_on(uint32_t now, decltype(g_system_state.heartbeat)* hb) {
    //** 检查是否到了关闭时间
    if (now - phase_start_ms < HW_LED_HEARTBEAT_ON_MS) {
        return;  // 还没到关闭时间
    }
    
    led_off();
    heartbeat_phase = HEARTBEAT_IDLE;
    hb->last_beat_ms = now;
}

void heartbeat_process(void) {
    uint32_t now = millis();
    auto* hb = HEARTBEAT_STATE();
    
    //** 使用早期返回消除嵌套
    if (heartbeat_phase == HEARTBEAT_IDLE) {
        heartbeat_handle_idle(now, hb);
        return;
    }
    
    if (heartbeat_phase == HEARTBEAT_LED_ON) {
        heartbeat_handle_led_on(now, hb);
        return;
    }
}
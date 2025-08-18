//** ESP32-S3 HoloCubic - System State (Linus Style)
//** "Show me your data structures, and I won't usually need your flowcharts"

#pragma once

#include <stdint.h>
#include <stdbool.h>

//** 系统状态 - 单一数据源，消除所有全局变量
typedef struct {
    //** 应用状态
    struct {
        bool initialized;
        uint32_t start_time_ms;
        uint32_t error_count;
    } app;
    
    //** LED状态
    struct {
        bool initialized;
        uint8_t current_r, current_g, current_b;
        uint8_t brightness;
        uint32_t last_update_ms;
    } led;
    
    //** 心跳状态
    struct {
        uint32_t last_beat_ms;
        uint32_t interval_ms;
        uint32_t beat_count;
    } heartbeat;
    
    //** 命令处理状态
    struct {
        bool initialized;
        uint32_t commands_processed;
        char last_command;
    } command;
    
    //** 系统健康状态
    struct {
        uint32_t last_check_ms;
        uint32_t free_heap_min;
        bool health_ok;
    } health;
    
} system_state_t;

//** 全局系统状态 - 唯一的全局变量
extern system_state_t g_system_state;

//** 状态访问宏 - 简化访问
#define SYS_STATE() (&g_system_state)
#define APP_STATE() (&g_system_state.app)
#define LED_STATE() (&g_system_state.led)
#define HEARTBEAT_STATE() (&g_system_state.heartbeat)
#define COMMAND_STATE() (&g_system_state.command)
#define HEALTH_STATE() (&g_system_state.health)

//** 系统状态管理函数
void system_state_init(void);
void system_state_reset(void);
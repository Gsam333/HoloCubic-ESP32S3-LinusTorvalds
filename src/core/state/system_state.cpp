//** ESP32-S3 HoloCubic - System State Implementation
//** Linus原则：单一数据源，零初始化

#include "system_state.h"
#include <string.h>

//** 全局系统状态 - 零初始化
system_state_t g_system_state = {0};

//** 系统状态初始化
void system_state_init(void) {
    //** 零初始化已经完成，只设置非零默认值
    HEARTBEAT_STATE()->interval_ms = 1000;  // 1秒心跳
    LED_STATE()->brightness = 200;          // 默认亮度
}

//** 系统状态重置
void system_state_reset(void) {
    memset(&g_system_state, 0, sizeof(g_system_state));
    system_state_init();
}
//** ESP32-S3 HoloCubic - WiFi Application Layer
//** Linus原则：数据结构优先 - "Good programmers worry about data structures"
//** 职责：WiFi连接状态管理，简化接口，单一数据源

#ifndef WIFI_APP_H
#define WIFI_APP_H

#include <stdint.h>
#include <stdbool.h>

// ========================================
// 核心数据结构 - 这是关键！
// ========================================

typedef enum {
    WIFI_STATE_IDLE = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_FAILED
} wifi_state_t;

typedef struct {
    wifi_state_t state;
    uint32_t connect_time;
    uint32_t last_check;
    int8_t rssi;
    bool is_ready;
} wifi_app_t;

// ========================================
// 简化接口 - 只要3个函数就够了
// ========================================

//** 初始化WiFi应用 - 调用者必须在使用其他函数前调用
void wifi_app_init(void);

//** 主循环处理 - 状态机核心
void wifi_app_process(void);

//** 获取状态 - 返回结构体指针
const wifi_app_t* wifi_app_get_state(void);

#endif // WIFI_APP_H
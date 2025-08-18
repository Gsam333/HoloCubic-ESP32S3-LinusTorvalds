//** ESP32-S3 HoloCubic - WiFi Application Layer
//** Linus原则：数据结构优先 - "Good programmers worry about data structures"
//** 职责：WiFi连接状态管理，简化接口，单一数据源

#include "wifi_app.h"
#include "../../core/config/hardware_config.h"
#include "../../config/secrets.h"
#include <Arduino.h>
#include <WiFi.h>

//** 全局状态 - 单一数据源
static wifi_app_t g_wifi_app = {
    .state = WIFI_STATE_IDLE,
    .connect_time = 0,
    .last_check = 0,
    .rssi = 0,
    .is_ready = false
};

void wifi_app_init(void) {
    Serial.println("WiFi App: 初始化");
    
    //** 初始化WiFi硬件
    WiFi.mode(WIFI_STA);
    
    //** 重置状态 - 但不立即连接
    g_wifi_app.state = WIFI_STATE_IDLE;
    g_wifi_app.is_ready = false;
    g_wifi_app.connect_time = 0;
    g_wifi_app.last_check = 0;
    g_wifi_app.rssi = 0;
    
    Serial.println("WiFi App: 初始化完成，等待连接命令");
}

//** 启动WiFi连接
static void wifi_start_connection(uint32_t now) {
    Serial.print("WiFi App: 开始连接到 ");
    Serial.println(WIFI_SSID_1);
    WiFi.begin(WIFI_SSID_1, WIFI_PASSWORD_1);
    g_wifi_app.state = WIFI_STATE_CONNECTING;
    g_wifi_app.connect_time = now;
    g_wifi_app.last_check = now;
}

//** 处理连接中状态
static void wifi_handle_connecting(uint32_t now) {
    if (WiFi.status() == WL_CONNECTED) {
        //** 连接成功
        g_wifi_app.state = WIFI_STATE_CONNECTED;
        g_wifi_app.is_ready = true;
        g_wifi_app.rssi = WiFi.RSSI();
        Serial.print("WiFi App: ✓ 连接成功 - IP: ");
        Serial.println(WiFi.localIP());
        return;
    }
    
    if (now - g_wifi_app.connect_time > HW_WIFI_CONNECT_TIMEOUT_MS) {
        //** 连接超时
        g_wifi_app.state = WIFI_STATE_FAILED;
        Serial.println("WiFi App: ✗ 连接超时");
    }
}

//** 处理已连接状态
static void wifi_handle_connected(uint32_t now) {
    if (WiFi.status() != WL_CONNECTED) {
        //** 连接丢失，重新连接
        g_wifi_app.state = WIFI_STATE_CONNECTING;
        g_wifi_app.is_ready = false;
        g_wifi_app.connect_time = now;
        WiFi.reconnect();
        Serial.println("WiFi App: 重新连接...");
        return;
    }
    
    //** 更新信号强度
    g_wifi_app.rssi = WiFi.RSSI();
}

//** 处理失败状态
static void wifi_handle_failed(uint32_t now) {
    //** 失败后重试
    g_wifi_app.state = WIFI_STATE_CONNECTING;
    g_wifi_app.connect_time = now;
    WiFi.begin(WIFI_SSID_1, WIFI_PASSWORD_1);
    Serial.println("WiFi App: 重试连接...");
}

void wifi_app_process(void) {
    uint32_t now = millis();
    
    //** 自动启动连接 - 只在第一次运行时
    if (g_wifi_app.state == WIFI_STATE_IDLE) {
        wifi_start_connection(now);
        return;
    }
    
    //** 定期检查状态
    if (now - g_wifi_app.last_check < HW_WIFI_STATUS_CHECK_MS) {
        return;
    }
    g_wifi_app.last_check = now;
    
    //** 状态机处理
    switch (g_wifi_app.state) {
        case WIFI_STATE_CONNECTING:
            wifi_handle_connecting(now);
            break;
            
        case WIFI_STATE_CONNECTED:
            wifi_handle_connected(now);
            break;
            
        case WIFI_STATE_FAILED:
            wifi_handle_failed(now);
            break;
            
        default:
            break;
    }
}

const wifi_app_t* wifi_app_get_state(void) {
    return &g_wifi_app;
}
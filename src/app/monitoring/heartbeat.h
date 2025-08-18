//** ESP32-S3 HoloCubic - Heartbeat Monitor Header
//** Linus原则：心跳监控独立模块

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//** 初始化心跳监控
void heartbeat_init(void);

//** 处理心跳逻辑 - 非阻塞
void heartbeat_process(void);

#ifdef __cplusplus
}
#endif
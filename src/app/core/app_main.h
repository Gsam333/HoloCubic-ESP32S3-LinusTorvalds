//** ESP32-S3 HoloCubic - Application Main Header
//** Linus原则：简洁的应用层接口

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//** Linus风格：简单的全局状态变量
extern bool g_app_initialized;
extern uint32_t g_app_start_time;

//** 应用初始化 - 只做应用层的事情
void app_init(void);

//** 应用主循环 - 处理所有应用逻辑
void app_run(void);

//** 应用清理 - 资源释放
void app_cleanup(void);

#ifdef __cplusplus
}
#endif
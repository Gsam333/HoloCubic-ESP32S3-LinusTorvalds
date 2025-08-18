//** ESP32-S3 HoloCubic - Command Handler Header
//** Linus原则：命令处理独立模块，单一职责

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//** 初始化命令处理器
void command_handler_init(void);

//** 处理串口命令 - 非阻塞
void command_handler_process(void);

#ifdef __cplusplus
}
#endif
//** ESP32-S3 HoloCubic - Debug Utilities Header
//** Linus原则：调试工具独立模块，便于条件编译

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//** 打印硬件配置信息
void debug_print_hw_config(void);

//** 打印系统状态
void debug_print_system_status(void);

#ifdef __cplusplus
}
#endif
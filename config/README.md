# Linus Torvalds 配置系统

## 设计哲学

这个配置系统体现了 Linus Torvalds 的"好品味"设计原则：

### 1. 单一职责原则
- `hardware_config.h` - 硬件配置（引脚、频率等）
- `debug_config.h` - 调试和日志配置
- `app_config.h` - 应用功能配置
- `secrets.h` - 隐私和敏感信息

### 2. 简洁性原则
- 不要复杂的条件编译嵌套
- 统一的命名约定
- 清晰的分组和注释

### 3. 安全性原则
- 敏感信息与代码分离
- 模板化的隐私配置
- .gitignore 保护

## 文件说明

### hardware_config.h
硬件相关的所有配置，包括：
- GPIO引脚定义
- SPI/I2C频率
- 第三方库兼容宏
- 编译时验证

### debug_config.h
调试系统配置，包括：
- 统一的调试级别
- 模块化的调试控制
- 性能测试宏
- 断言系统

### app_config.h
应用功能配置，包括：
- 功能开关
- 超时时间
- 刷新频率
- 默认参数

### secrets.h.template
隐私信息模板，包括：
- WiFi凭据
- API密钥
- 服务器配置
- 设备标识

## 使用方法

### 1. 设置隐私配置
```bash
# 复制模板
cp config/secrets.h.template config/secrets.h

# 编辑配置
vim config/secrets.h
```

### 2. 调整调试级别
```cpp
// 在 platformio.ini 中设置
build_flags = -DGLOBAL_DEBUG_LEVEL=3

// 或者单独设置模块
build_flags = -DNETWORK_DEBUG_LEVEL=4
```

### 3. 启用/禁用功能
```cpp
// 在 app_config.h 中修改
#define FEATURE_WEATHER_ENABLED     1  // 启用
#define FEATURE_MUSIC_ENABLED       0  // 禁用
```

### 4. 在代码中使用
```cpp
#include "config/hardware_config.h"
#include "config/debug_config.h"
#include "config/app_config.h"
#include "config/secrets.h"

// 使用硬件配置
pinMode(TFT_MOSI, OUTPUT);

// 使用调试宏
NETWORK_INFO("Connecting to %s", WIFI_SSID_PRIMARY);

// 使用应用配置
if (IS_FEATURE_ENABLED(WEATHER)) {
    // 天气功能代码
}
```

## 与原项目对比

### 原项目问题
```cpp
// 复杂的条件编译地狱
#ifdef ENABLE_TEST_MODE
  #ifdef ENABLE_DISPLAY_TEST_MODE
    #ifdef ENABLE_NETWORK_TEST_MODE
      #define DEBUG_NETWORK_OPERATIONS
    #endif
  #endif
#endif

// 重复的模式
#define ENABLE_LED_TEST_MODE
#define ENABLE_IMU_TEST_MODE
#define ENABLE_DISPLAY_TEST_MODE
```

### Linus 版本改进
```cpp
// 简单的级别控制
#define GLOBAL_DEBUG_LEVEL DEBUG_LEVEL_INFO

// 统一的调试宏
NETWORK_INFO("Connection status: %s", status);
DISPLAY_DEBUG("Refresh rate: %d fps", fps);
```

## 最佳实践

### 1. 配置分离
- 硬件配置：编译时常量
- 应用配置：功能开关和参数
- 调试配置：开发时工具
- 隐私配置：敏感信息

### 2. 安全考虑
- 永远不要提交 secrets.h
- 使用模板文件分享配置结构
- 在 .gitignore 中保护敏感文件

### 3. 维护性
- 使用清晰的命名约定
- 添加编译时验证
- 提供使用示例和文档

## 扩展指南

### 添加新的硬件模块
1. 在 `hardware_config.h` 中添加引脚定义
2. 添加编译时验证
3. 提供第三方库兼容宏
4. 更新文档

### 添加新的调试模块
1. 在 `debug_config.h` 中添加调试级别
2. 定义模块特定的调试宏
3. 在代码中使用调试宏
4. 测试不同调试级别

这个配置系统体现了真正的"好品味"：简洁、安全、可维护。

# ESP32-S3 HoloCubic - Linus Torvalds Edition
本项目基于 [@peng-zhihui](https://github.com/peng-zhihui) 的优秀开源项目 [HoloCubic](https://github.com/peng-zhihui/HoloCubic) 进行重构和改进。

**原项目特点**：
- 创新的全息显示概念
- 完整的硬件设计
- 丰富的应用功能

**本项目改进**：
- ESP32-S3 硬件适配
- Linus 风格代码重构
- 模块化架构设计
- 开发体验优化

## 设计哲学

这个项目遵循 Linus Torvalds 的软件设计原则：

### 1. "好品味" (Good Taste)
- **消除特殊情况**：重新设计数据结构，让特殊情况消失
- **统一接口**：所有驱动使用一致的初始化和访问模式
- **单一真实来源**：系统状态集中管理，避免数据不一致

### 2. 简洁性原则
- **直接硬件访问**：最小包装开销，直接操作硬件
- **编译时配置**：零运行时开销的配置系统
- **扁平化架构**：避免过度抽象和深层嵌套

### 3. 实用主义
- **解决实际问题**：专注于硬件控制和手势识别
- **性能优先**：优化内存布局和执行效率
- **错误不可能**：编译时验证，运行时无检查

### 4. 可维护性
- **清晰的模块划分**：按功能组织目录结构
- **简化的包含路径**：通过 platformio.ini 统一管理
- **完整的文档**：每个模块都有详细说明

## ⚠️ 重要配置说明

### ESP32-S3 编译参数保护

本项目使用 ESP32-S3 芯片，**必须**使用正确的编译参数：

```ini
# ✅ 正确参数
-mlongcalls

# ❌ 错误参数（会导致编译失败）
-mlong-calls
```

**如果遇到编译错误**：
```
Unknown argument '-mlong-calls'; did you mean '-mlongcalls'?
```

**解决方法**：
```bash
# 1. 检查配置
make check-config

# 2. 手动修复 platformio.ini 中的参数
# 将 -mlong-calls 改为 -mlongcalls (注意：没有连字符)

# 3. 重新编译
make build
```

**配置保护机制**：
- `platformio.ini` 中有明确的警告注释
- `scripts/check_platformio.py` 自动检查配置
- `Makefile` 提供便捷的检查命令

## 项目结构

```
ESP32-S3 HoloCubic (Linux Kernel Style)
═══════════════════════════════════════

main.cpp          ← init/main.c风格，只做启动和调度
├── app/           ← 应用层 (用户空间)
│   ├── core/      ├── managers/   ├── monitoring/
│   ├── network/   └── interface/
├── system/        ← 系统服务层
│   ├── debug_utils.*  └── panic.*
├── drivers/       ← 设备驱动层
│   ├── display/   ├── led/        └── storage/ (预留)
└── core/          ← 硬件抽象层
    ├── boot/      ├── config/     ├── state/
    └── types/

config/            ← 编译时配置
├── debug_config.h ├── app_config.h └── secrets.h

docs/              ← 统一文档系统
├── KNOWLEDGE_BASE.md     ← 核心知识库 ⭐
├── core/LINUS_RULES.md   ← 项目铁律
└── debugging/            ← 问题解决方案

lib/IMUGesture/    ← IMU手势识别库
platformio.ini     ← 构建配置 (ESP32-S3专用)
FLASH_8MB.csv      ← Flash分区表
```

## 核心特性

### 1. Linus风格的系统启动
```cpp
// 简洁的启动序列，每个函数只做一件事
boot_result_t system_boot_sequence(void) {
    system_early_init();    // 串口和基础设置
    system_print_banner();  // 系统信息
    hardware_init_all();    // 硬件模块初始化
    application_init_all(); // 应用模块初始化
    return BOOT_OK;
}
```

### 2. 统一的包含路径管理
```ini
# platformio.ini 中的包含路径配置
build_flags = 
    -I src                  # 统一src包含路径
    -I config              # 统一config包含路径
    -I src/core/boot       # 启动相关头文件
    -I src/core/config     # 配置相关头文件
    -I src/app/core        # 应用核心头文件
    -I src/drivers/display # 显示驱动头文件
```

### 3. IMU手势识别库集成
```cpp
// 使用lib/IMUGesture库进行手势识别
#include "imu_gesture_driver.h"

void setup() {
    imu_gesture_init();  // 初始化手势驱动
}

void loop() {
    ImuGestureData* gesture = imu_gesture_get_data();
    if (imu_gesture_has_gesture(gesture)) {
        // 处理手势
        gesture->isValid = false;  // 重置标志
    }
}
```

### 4. 模块化的硬件驱动
```cpp
// 显示驱动 - 直接硬件访问，最小包装
void display_init(void);
void display_clear(uint16_t color);
void display_text(const char* text, int x, int y, uint16_t color);

// LED驱动 - FastLED的薄包装
void led_init(void);
void led_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void led_show(void);
```

## 编译和使用

### 环境要求
- **PlatformIO** - 构建系统
- **ESP32-S3 DevKit** - 开发板
- **8MB Flash** - 使用 FLASH_8MB.csv 分区表
- **QMI8658** - IMU传感器
- **ST7789** - 240x240 TFT显示屏
- **WS2812** - RGB LED

### 快速开始
```bash
# 克隆项目
git clone <repository-url>
cd esp32s3-holocubic-LinusTorvalds

# 编译
pio run

# 上传固件
pio run --target upload

# 监控串口输出
pio device monitor
```

### 硬件连接

#### TFT显示屏 (ST7789 240x240)
```
MISO: GPIO 13    MOSI: GPIO 42    SCLK: GPIO 41
CS:   GPIO 10    DC:   GPIO 40    RST:  GPIO 45
BL:   GPIO 46    (背光控制)
```

#### RGB LED (WS2812)
```
DATA: GPIO 39    (2个LED)
```

#### IMU传感器 (QMI8658)
```
SDA:  GPIO 17    SCL:  GPIO 18
ADDR: 0x6B       (I2C地址)
```

#### SD卡存储 (SD_MMC模式)
```
CLK:  GPIO 2     (时钟线)
CMD:  GPIO 38    (命令线)
D0:   GPIO 1     (数据线0)
```
**注意**: 使用SD_MMC模式，支持FAT32格式的SD卡

## 功能特性

### 🎮 IMU手势识别
- **6种手势类型**：左转、右转、上推、下拉、前进、返回
- **长按检测**：连续3次相同手势触发长按
- **高性能**：执行时间 < 10μs，支持100Hz调用频率
- **零动态分配**：所有数据静态管理，内存占用 < 100字节

### 📺 TFT显示
- **240x240分辨率**：ST7789驱动芯片
- **多种旋转模式**：支持0°、90°、180°、270°旋转
- **分光棱镜模式**：HoloCubic专用显示模式
- **硬件加速**：DMA传输，高刷新率

### 💡 RGB LED控制
- **WS2812支持**：2个可编程RGB LED
- **FastLED集成**：丰富的颜色和效果
- **系统状态指示**：启动、运行、错误状态显示

### 📡 WiFi网络
- **自动连接**：启动时自动连接已配置网络
- **状态显示**：TFT屏幕显示连接状态
- **串口控制**：通过串口命令管理WiFi

### 💾 SD卡存储
- **SD_MMC模式**：高速SD卡访问，支持大容量卡
- **FAT32支持**：兼容标准文件系统
- **JSON配置**：支持从SD卡读取配置文件
- **实时显示**：TFT屏幕显示SD卡状态和内容

## 串口命令

### 基础命令
- `h` / `H` - 显示帮助信息
- `s` / `S` - 显示当前传感器数据
- `r` / `R` - 显示系统状态报告
- `i` / `I` - 运行IMU初始化测试

### 网络命令
- `w` / `W` - 显示WiFi状态
- `n` / `N` - 显示网络信息
- `p` / `P` - 扫描可用网络
- `o` / `O` - 重新连接WiFi
- `d` / `D` - 断开WiFi连接
- `a` / `A` - 自动连接WiFi

### 测试命令
- `4` - 运行TFT显示测试 (如果启用)
- IMU手势测试自动运行 (如果启用)

## 配置系统

### 调试配置 (config/debug_config.h)
```cpp
// 编译时开关，零运行时开销
#define ENABLE_TEST_CODE        1    // 启用测试代码
#define ENABLE_TFT_TESTS        1    // TFT显示测试
#define ENABLE_IMU_TESTS        1    // IMU手势测试
#define GLOBAL_DEBUG_LEVEL      3    // 调试级别
```

### 硬件配置 (src/core/config/hardware_config.h)
```cpp
// 硬件引脚定义
#define HW_IMU_SDA              17   // I2C SDA引脚
#define HW_IMU_SCL              18   // I2C SCL引脚
#define HW_RGB_LED_PIN          39   // RGB LED数据引脚
#define HW_RGB_LED_NUM          2    // LED数量

// SD卡配置 (SD_MMC模式)
#define HW_SD_CLK               2    // SD卡时钟引脚
#define HW_SD_CMD               38   // SD卡命令引脚  
#define HW_SD_D0                1    // SD卡数据引脚D0
```
关键的初始化方法：
SD_MMC.setPins(HW_SD_CLK, HW_SD_CMD, HW_SD_D0);
SD_MMC.begin("/root", true, false, SDMMC_FREQ_DEFAULT)
重要说明：使用 SDMMC_FREQ_DEFAULT 而不是默认的40MHz频率，挂载点使用 "/root" 而不是默认的 "/sdcard"

## 性能特性

### 内存使用
- **Flash使用**：~800KB (12.2% of 8MB)
- **RAM使用**：~44KB (13.6% of 320KB)
- **静态分配**：无动态内存分配，避免内存碎片
- **缓存友好**：32字节对齐的数据结构

### 实时性能
- **系统响应**：10ms主循环，实时响应
- **IMU采样**：100Hz采样率，10μs处理时间
- **显示刷新**：硬件DMA，无阻塞刷新

### 编译优化
```ini
# 编译器优化标志
-Os                     # 大小优化
-ffunction-sections     # 函数段分离
-fdata-sections         # 数据段分离
-Wl,--gc-sections      # 垃圾回收未使用段
-mlongcalls            # ESP32-S3长调用支持
```

## 扩展开发

### 添加新的硬件驱动
1. 在 `src/drivers/` 下创建新目录
2. 实现驱动的 `.h` 和 `.cpp` 文件
3. 在 `platformio.ini` 中添加包含路径
4. 在 `system_boot.cpp` 中添加初始化调用

### 添加新的应用功能
1. 在 `src/app/` 下创建功能模块
2. 在 `app_main.cpp` 中集成新功能
3. 添加相应的配置选项到 `config/` 目录

### 添加新的测试模块
1. 在 `src/test/` 下创建测试文件
2. 在 `config/debug_config.h` 中添加测试开关
3. 在 `main.cpp` 中添加测试调用

## 故障排除

### 🚨 常见问题快速修复

```bash
# ESP32-S3编译器参数错误
Unknown argument '-mlong-calls'; did you mean '-mlongcalls'?
→ 解决：make check-config

# TFT_eSPI字体崩溃
StoreProhibited (NULL指针访问)
→ 解决：在hardware_config.h添加 #define LOAD_GLCD

# SD卡无法读取
SD card mount failed
→ 解决：检查FAT32格式，引脚连接(CLK=2,CMD=38,D0=1)
```

### 📖 详细解决方案
- **完整故障排除指南**：[docs/KNOWLEDGE_BASE.md](docs/KNOWLEDGE_BASE.md#快速解决方案)
- **核心问题解决**：[docs/debugging/CORE_SOLUTIONS.md](docs/debugging/CORE_SOLUTIONS.md)
- **SD卡专项调试**：[docs/storage存储/SD_CARD_DEBUG_SOLUTION.md](docs/storage存储/SD_CARD_DEBUG_SOLUTION.md)

## 项目哲学

> *"Talk is cheap. Show me the code."* - Linus Torvalds

这个项目体现了 Linus Torvalds 的核心设计哲学：

1. **简洁胜过复杂** - 直接的硬件访问，最小的抽象层
2. **性能胜过美观** - 优化的数据结构和算法
3. **实用胜过理论** - 解决实际问题，不过度设计
4. **代码胜过文档** - 代码本身就是最好的文档

---

**现在你有了一个真正的 "Linus Style" ESP32 项目！**
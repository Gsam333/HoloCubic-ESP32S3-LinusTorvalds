#!/bin/bash
# ESP32-S3 HoloCubic 简单依赖分析脚本
# Linus风格：简洁、直接、有效

echo "🔍 ESP32-S3 HoloCubic 项目依赖分析"
echo "=================================="

# 检查项目结构
echo ""
echo "📁 项目结构:"
find src -name "*.cpp" -o -name "*.h" | head -20 | while read file; do
    echo "  $file"
done

# 分析包含关系
echo ""
echo "📚 主要库依赖:"
echo "  TFT_eSPI 引用:"
grep -r "#include.*TFT_eSPI" src/ | wc -l | xargs echo "    次数:"

echo "  FastLED 引用:"
grep -r "#include.*FastLED" src/ | wc -l | xargs echo "    次数:"

echo "  WiFi 引用:"
grep -r "#include.*WiFi" src/ | wc -l | xargs echo "    次数:"

echo "  Arduino 引用:"
grep -r "#include.*Arduino" src/ | wc -l | xargs echo "    次数:"

# 分析模块间依赖
echo ""
echo "🏗️ 模块间依赖:"
echo "  app/ 模块依赖:"
find src/app -name "*.cpp" -exec grep -l "#include.*drivers/" {} \; | wc -l | xargs echo "    -> drivers/:"

echo "  drivers/ 模块依赖:"
find src/drivers -name "*.cpp" -exec grep -l "#include.*core/" {} \; | wc -l | xargs echo "    -> core/:"

# 分析配置文件依赖
echo ""
echo "⚙️ 配置文件依赖:"
echo "  app_config.h 引用:"
grep -r "#include.*app_config" src/ | wc -l | xargs echo "    次数:"

echo "  debug_config.h 引用:"
grep -r "#include.*debug_config" src/ | wc -l | xargs echo "    次数:"

echo "  hardware_config.h 引用:"
grep -r "#include.*hardware_config" src/ | wc -l | xargs echo "    次数:"

# 生成简单的依赖树
echo ""
echo "🌳 依赖关系树:"
echo "main.cpp"
echo "├── app/"
echo "│   ├── core/app_main.h"
echo "│   ├── managers/led_manager.h"
echo "│   ├── network/wifi_app.h"
echo "│   └── interface/command_handler.h"
echo "├── drivers/"
echo "│   ├── display/display_driver.h"
echo "│   └── led/led_driver.h"
echo "├── core/"
echo "│   ├── hardware_config.h"
echo "│   └── error_handling.h"
echo "└── system/"
echo "    └── panic.h"

echo ""
echo "✅ 依赖分析完成"
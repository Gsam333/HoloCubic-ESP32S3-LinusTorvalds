#!/bin/bash
# ESP32-S3 编译环境检查脚本
# Linus风格：一次性检查所有必需环境

# 注意：不使用 set -e，因为我们需要处理命令失败的情况

echo "🔍 ESP32-S3 HoloCubic 编译环境检查"
echo "=================================="

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查结果统计
PASS_COUNT=0
FAIL_COUNT=0
WARN_COUNT=0

check_pass() {
    echo -e "✅ ${GREEN}$1${NC}"
    ((PASS_COUNT++))
}

check_fail() {
    echo -e "❌ ${RED}$1${NC}"
    ((FAIL_COUNT++))
}

check_warn() {
    echo -e "⚠️  ${YELLOW}$1${NC}"
    ((WARN_COUNT++))
}

# 1. 检查基础工具
echo ""
echo "🛠️ 基础工具检查:"

# Python
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2)
    check_pass "Python3: $PYTHON_VERSION"
else
    check_fail "Python3: 未安装"
fi

# PlatformIO
if command -v pio &> /dev/null; then
    PIO_VERSION=$(pio --version 2>&1 | head -n1)
    check_pass "PlatformIO: $PIO_VERSION"
else
    check_fail "PlatformIO: 未安装"
fi

# Git
if command -v git &> /dev/null; then
    GIT_VERSION=$(git --version | cut -d' ' -f3)
    check_pass "Git: $GIT_VERSION"
else
    check_warn "Git: 未安装 (可选)"
fi

# Make
if command -v make &> /dev/null; then
    MAKE_VERSION=$(make --version | head -n1 | cut -d' ' -f3)
    check_pass "Make: $MAKE_VERSION"
else
    check_warn "Make: 未安装 (可选)"
fi

# 2. 检查ESP32平台
echo ""
echo "🔧 ESP32平台检查:"

if pio platform list | grep -q "espressif32"; then
    ESP32_VERSION=$(pio platform show espressif32 | grep "Version" | cut -d' ' -f2)
    check_pass "ESP32平台: $ESP32_VERSION"
else
    check_fail "ESP32平台: 未安装"
fi

# 3. 检查项目配置文件
echo ""
echo "📁 项目配置检查:"

# platformio.ini
if [ -f "platformio.ini" ]; then
    check_pass "platformio.ini: 存在"
    
    # 检查关键配置
    if grep -q "esp32-s3-devkitc-1" platformio.ini; then
        check_pass "开发板配置: esp32-s3-devkitc-1"
    else
        check_fail "开发板配置: 未找到esp32-s3-devkitc-1"
    fi
    
    # 检查关键编译参数
    if grep -q "\-mlongcalls" platformio.ini; then
        check_pass "ESP32-S3参数: -mlongcalls 正确"
    elif grep -q "\-mlong-calls" platformio.ini; then
        check_fail "ESP32-S3参数: -mlong-calls 错误，应为 -mlongcalls"
    else
        check_warn "ESP32-S3参数: 未找到 -mlongcalls"
    fi
    
    # 检查分区表
    if grep -q "FLASH_8MB.csv" platformio.ini; then
        check_pass "分区表: FLASH_8MB.csv"
    else
        check_warn "分区表: 未指定或非标准配置"
    fi
else
    check_fail "platformio.ini: 不存在"
fi

# FLASH_8MB.csv
if [ -f "FLASH_8MB.csv" ]; then
    check_pass "分区表文件: FLASH_8MB.csv 存在"
else
    check_warn "分区表文件: FLASH_8MB.csv 不存在"
fi

# 4. 检查源代码结构
echo ""
echo "📂 源代码结构检查:"

REQUIRED_DIRS=("src" "config" "lib" "docs")
for dir in "${REQUIRED_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        check_pass "目录: $dir/ 存在"
    else
        check_warn "目录: $dir/ 不存在"
    fi
done

# 检查main.cpp
if [ -f "src/main.cpp" ]; then
    check_pass "主程序: src/main.cpp 存在"
else
    check_fail "主程序: src/main.cpp 不存在"
fi

# 5. 检查库依赖
echo ""
echo "📚 库依赖检查:"

# 检查lib目录下的库
if [ -d "lib" ]; then
    LIB_COUNT=$(find lib -maxdepth 1 -type d | wc -l)
    if [ $LIB_COUNT -gt 1 ]; then
        check_pass "本地库: $((LIB_COUNT-1)) 个库"
        find lib -maxdepth 1 -type d -not -path lib | while read libdir; do
            echo "    - $(basename "$libdir")"
        done
    else
        check_warn "本地库: 无本地库"
    fi
fi

# 6. 检查编译测试
echo ""
echo "🔨 编译测试:"

if command -v pio &> /dev/null && [ -f "platformio.ini" ]; then
    echo "正在进行编译测试..."
    if pio run --target checkprogsize &> /dev/null; then
        check_pass "编译测试: 通过"
        
        # 获取内存使用信息
        MEMORY_INFO=$(pio run --target checkprogsize 2>&1 | grep -E "(RAM|Flash)")
        if [ ! -z "$MEMORY_INFO" ]; then
            echo "内存使用情况:"
            echo "$MEMORY_INFO" | while read line; do
                echo "    $line"
            done
        fi
    else
        check_fail "编译测试: 失败"
    fi
else
    check_warn "编译测试: 跳过 (PlatformIO或配置文件不可用)"
fi

# 7. 总结报告
echo ""
echo "📊 检查结果总结:"
echo "=================================="
echo -e "✅ 通过: ${GREEN}$PASS_COUNT${NC} 项"
echo -e "❌ 失败: ${RED}$FAIL_COUNT${NC} 项"
echo -e "⚠️  警告: ${YELLOW}$WARN_COUNT${NC} 项"

if [ $FAIL_COUNT -eq 0 ]; then
    echo ""
    echo -e "🎉 ${GREEN}环境检查通过！可以开始开发。${NC}"
    exit 0
else
    echo ""
    echo -e "💥 ${RED}发现 $FAIL_COUNT 个问题，请修复后重新检查。${NC}"
    echo ""
    echo "修复建议:"
    echo "1. 安装缺失的工具 (Python3, PlatformIO等)"
    echo "2. 检查platformio.ini配置"
    echo "3. 确保ESP32平台已安装: pio platform install espressif32"
    echo "4. 修复编译参数: -mlong-calls → -mlongcalls"
    exit 1
fi
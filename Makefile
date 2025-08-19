# ESP32-S3 HoloCubic Makefile
# Linus风格：简单、直接、有效

.PHONY: check-config build clean upload monitor test help

# 默认目标
all: check-config build

# 检查PlatformIO配置
check-config:
	@echo "🔍 检查 ESP32-S3 配置..."
	@python3 scripts/check_platformio.py

# 构建项目
build: check-config
	@echo "🔨 构建 ESP32-S3 项目..."
	pio run

# 清理构建文件
clean:
	@echo "🧹 清理构建文件..."
	pio run --target clean

# 上传到设备
upload: check-config
	@echo "📤 上传到 ESP32-S3..."
	pio run --target upload

# 串口监控
monitor:
	@echo "📺 启动串口监控..."
	pio device monitor

# 上传并监控
upload-monitor: upload monitor

# 运行测试
test: check-config
	@echo "🧪 运行测试..."
	pio test

# 强制修复配置（仅在确认需要时使用）
fix-config:
	@echo "⚠️  强制修复 platformio.ini 配置..."
	@echo "请确认您有权限修改此配置！"
	@read -p "继续? [y/N] " confirm && [ "$$confirm" = "y" ]
	@python3 scripts/check_platformio.py --fix

# 显示帮助
help:
	@echo "ESP32-S3 HoloCubic 构建系统"
	@echo ""
	@echo "可用目标:"
	@echo "  check-config    - 检查PlatformIO配置"
	@echo "  build          - 构建项目"
	@echo "  clean          - 清理构建文件"
	@echo "  upload         - 上传到设备"
	@echo "  monitor        - 串口监控"
	@echo "  upload-monitor - 上传并监控"
	@echo "  test           - 运行测试"
	@echo "  fix-config     - 强制修复配置（需确认）"
	@echo "  help           - 显示此帮助"
	@echo ""
	@echo "⚠️  重要提醒："
	@echo "  ESP32-S3 需要 -mlongcalls 参数才能正确编译"
	@echo "  如果遇到 'Unknown argument -mlong-calls' 错误，"
	@echo "  请运行 'make check-config' 检查配置"
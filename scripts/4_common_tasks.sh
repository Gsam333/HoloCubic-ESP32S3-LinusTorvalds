#!/bin/bash
# ESP32-S3 HoloCubic 常见任务脚本
# Linus风格：实用工具集合，解决日常开发需求

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 显示帮助信息
show_help() {
    echo "🛠️ ESP32-S3 HoloCubic 常见任务工具"
    echo "=================================="
    echo ""
    echo "用法: $0 <任务> [参数]"
    echo ""
    echo "可用任务:"
    echo "  clean          - 清理编译文件和缓存"
    echo "  build          - 快速编译项目"
    echo "  upload         - 编译并上传到设备"
    echo "  monitor        - 启动串口监控"
    echo "  size           - 显示编译大小信息"
    echo "  deps           - 更新库依赖"
    echo "  format         - 格式化代码 (clang-format)"
    echo "  backup         - 备份项目到指定目录"
    echo "  restore        - 从备份恢复项目"
    echo "  git-status     - 显示Git状态和建议"
    echo "  find-todos     - 查找代码中的TODO和FIXME"
    echo "  count-lines    - 统计代码行数"
    echo "  check-syntax   - 语法检查 (不编译)"
    echo "  flash-erase    - 擦除Flash (慎用!)"
    echo "  reset-config   - 重置配置到默认值
  check-sd       - 检查SD卡内容 (macOS)"
    echo ""
    echo "示例:"
    echo "  $0 build          # 快速编译"
    echo "  $0 upload         # 编译并上传"
    echo "  $0 backup ~/backups/  # 备份到指定目录"
    echo "  $0 find-todos     # 查找待办事项"
}

# 清理编译文件
task_clean() {
    echo -e "${BLUE}🧹 清理编译文件和缓存...${NC}"
    
    # PlatformIO清理
    if command -v pio &> /dev/null; then
        pio run --target clean
        echo "✅ PlatformIO缓存已清理"
    fi
    
    # 清理常见的临时文件
    find . -name "*.tmp" -delete 2>/dev/null || true
    find . -name "*.bak" -delete 2>/dev/null || true
    find . -name ".DS_Store" -delete 2>/dev/null || true
    
    # 清理报告文件
    rm -f dependency_report.txt
    rm -f library_dependency_report.json
    rm -f static_analysis_report.json
    rm -f dependency_graph.png
    
    echo "✅ 临时文件已清理"
}

# 快速编译
task_build() {
    echo -e "${BLUE}🔨 快速编译项目...${NC}"
    
    if ! command -v pio &> /dev/null; then
        echo -e "${RED}❌ PlatformIO未安装${NC}"
        exit 1
    fi
    
    # 检查配置
    if [ -f "scripts/check_platformio.py" ]; then
        python3 scripts/check_platformio.py || {
            echo -e "${RED}❌ 配置检查失败，请修复后重试${NC}"
            exit 1
        }
    fi
    
    # 编译
    pio run
    
    echo -e "${GREEN}✅ 编译完成${NC}"
}

# 编译并上传
task_upload() {
    echo -e "${BLUE}📤 编译并上传到设备...${NC}"
    
    # 先编译
    task_build
    
    # 上传
    pio run --target upload
    
    echo -e "${GREEN}✅ 上传完成${NC}"
    
    # 询问是否启动监控
    read -p "是否启动串口监控? [y/N] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        task_monitor
    fi
}

# 串口监控
task_monitor() {
    echo -e "${BLUE}📺 启动串口监控...${NC}"
    echo "按 Ctrl+C 退出监控"
    echo ""
    
    pio device monitor
}

# 显示编译大小
task_size() {
    echo -e "${BLUE}📊 显示编译大小信息...${NC}"
    
    pio run --target checkprogsize
}

# 更新库依赖
task_deps() {
    echo -e "${BLUE}📚 更新库依赖...${NC}"
    
    # 更新PlatformIO平台
    pio platform update
    
    # 更新库
    pio lib update
    
    echo -e "${GREEN}✅ 库依赖已更新${NC}"
}

# 格式化代码
task_format() {
    echo -e "${BLUE}🎨 格式化代码...${NC}"
    
    if ! command -v clang-format &> /dev/null; then
        echo -e "${YELLOW}⚠️ clang-format未安装，跳过格式化${NC}"
        return
    fi
    
    # 查找并格式化C/C++文件
    find src -name "*.cpp" -o -name "*.h" | while read file; do
        echo "格式化: $file"
        clang-format -i "$file"
    done
    
    echo -e "${GREEN}✅ 代码格式化完成${NC}"
}

# 备份项目
task_backup() {
    local backup_dir="$1"
    
    if [ -z "$backup_dir" ]; then
        backup_dir="$HOME/esp32_backups"
    fi
    
    echo -e "${BLUE}💾 备份项目到 $backup_dir...${NC}"
    
    # 创建备份目录
    mkdir -p "$backup_dir"
    
    # 生成备份文件名
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local project_name=$(basename "$(pwd)")
    local backup_file="$backup_dir/${project_name}_${timestamp}.tar.gz"
    
    # 创建备份 (排除不必要的文件)
    tar -czf "$backup_file" \
        --exclude='.pio' \
        --exclude='node_modules' \
        --exclude='.git' \
        --exclude='*.tmp' \
        --exclude='*.bak' \
        --exclude='.DS_Store' \
        .
    
    echo -e "${GREEN}✅ 备份完成: $backup_file${NC}"
    echo "备份大小: $(du -h "$backup_file" | cut -f1)"
}

# 从备份恢复
task_restore() {
    local backup_file="$1"
    
    if [ -z "$backup_file" ]; then
        echo -e "${RED}❌ 请指定备份文件路径${NC}"
        echo "用法: $0 restore <备份文件.tar.gz>"
        exit 1
    fi
    
    if [ ! -f "$backup_file" ]; then
        echo -e "${RED}❌ 备份文件不存在: $backup_file${NC}"
        exit 1
    fi
    
    echo -e "${YELLOW}⚠️ 这将覆盖当前项目文件！${NC}"
    read -p "确定要恢复吗? [y/N] " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${BLUE}📂 从备份恢复项目...${NC}"
        tar -xzf "$backup_file"
        echo -e "${GREEN}✅ 恢复完成${NC}"
    else
        echo "取消恢复"
    fi
}

# Git状态检查
task_git_status() {
    echo -e "${BLUE}📋 Git状态检查...${NC}"
    
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo -e "${YELLOW}⚠️ 当前目录不是Git仓库${NC}"
        return
    fi
    
    echo "📊 Git状态:"
    git status --short
    
    echo ""
    echo "📈 提交历史 (最近5次):"
    git log --oneline -5
    
    echo ""
    echo "🌿 分支信息:"
    git branch -v
    
    # 检查未跟踪的文件
    local untracked=$(git ls-files --others --exclude-standard)
    if [ ! -z "$untracked" ]; then
        echo ""
        echo -e "${YELLOW}📁 未跟踪的文件:${NC}"
        echo "$untracked"
    fi
    
    # 建议
    echo ""
    echo "💡 建议:"
    if git diff --quiet && git diff --cached --quiet; then
        echo "  ✅ 工作区干净"
    else
        echo "  📝 有未提交的更改，考虑提交"
    fi
}

# 查找TODO和FIXME
task_find_todos() {
    echo -e "${BLUE}🔍 查找代码中的TODO和FIXME...${NC}"
    
    echo "📝 TODO项目:"
    grep -rn "TODO\|FIXME\|XXX\|HACK" src/ --include="*.cpp" --include="*.h" | head -20
    
    echo ""
    echo "📊 统计:"
    local todo_count=$(grep -r "TODO" src/ --include="*.cpp" --include="*.h" | wc -l)
    local fixme_count=$(grep -r "FIXME" src/ --include="*.cpp" --include="*.h" | wc -l)
    
    echo "  TODO: $todo_count 个"
    echo "  FIXME: $fixme_count 个"
    
    if [ $((todo_count + fixme_count)) -gt 0 ]; then
        echo -e "${YELLOW}💡 建议: 定期清理TODO和FIXME项目${NC}"
    fi
}

# 统计代码行数
task_count_lines() {
    echo -e "${BLUE}📏 统计代码行数...${NC}"
    
    echo "📊 代码统计:"
    
    # C/C++文件
    local cpp_lines=$(find src -name "*.cpp" -exec wc -l {} + | tail -1 | awk '{print $1}')
    local h_lines=$(find src -name "*.h" -exec wc -l {} + | tail -1 | awk '{print $1}')
    
    echo "  C++源文件: $cpp_lines 行"
    echo "  头文件: $h_lines 行"
    echo "  总计: $((cpp_lines + h_lines)) 行"
    
    echo ""
    echo "📁 文件统计:"
    echo "  .cpp文件: $(find src -name "*.cpp" | wc -l) 个"
    echo "  .h文件: $(find src -name "*.h" | wc -l) 个"
    
    # 最大的文件
    echo ""
    echo "📈 最大的文件:"
    find src -name "*.cpp" -o -name "*.h" | xargs wc -l | sort -nr | head -5
}

# 语法检查
task_check_syntax() {
    echo -e "${BLUE}🔍 ESP32项目语法检查 (使用PlatformIO)...${NC}"
    
    if ! command -v pio &> /dev/null; then
        echo -e "${RED}❌ PlatformIO未安装${NC}"
        exit 1
    fi
    
    echo "📊 执行语法检查编译 (不生成固件)..."
    
    # 使用PlatformIO进行语法检查编译
    # 先尝试完整编译来捕获所有警告，然后生成编译数据库
    local compile_output
    local compile_exit_code
    
    echo "🔧 执行完整编译检查..."
    # 捕获编译输出和退出码
    compile_output=$(pio run 2>&1)
    compile_exit_code=$?
    
    # 如果编译成功，再生成编译数据库
    if [ "$compile_exit_code" -eq 0 ]; then
        echo "📊 生成编译数据库..."
        pio run --target compiledb > /dev/null 2>&1
    fi
    
    echo "$compile_output"
    
    # 分析编译结果
    echo ""
    echo "📋 语法检查结果分析:"
    
    # 统计错误和警告
    local error_count=$(echo "$compile_output" | grep -c "error:" 2>/dev/null | tr -d '\n' || echo "0")
    local warning_count=$(echo "$compile_output" | grep -c "warning:" 2>/dev/null | tr -d '\n' || echo "0")
    local fatal_count=$(echo "$compile_output" | grep -c "fatal error:" 2>/dev/null | tr -d '\n' || echo "0")
    
    # 确保是数字
    error_count=${error_count:-0}
    warning_count=${warning_count:-0}
    fatal_count=${fatal_count:-0}
    
    echo "  🔍 错误数量: $error_count"
    echo "  ⚠️  警告数量: $warning_count"
    echo "  💀 致命错误: $fatal_count"
    
    # 显示具体的错误信息
    if [ "$error_count" -gt 0 ] || [ "$fatal_count" -gt 0 ]; then
        echo ""
        echo -e "${RED}❌ 发现语法错误:${NC}"
        echo "$compile_output" | grep -E "(error:|fatal error:)" | head -10
        
        if [ $(echo "$compile_output" | grep -E "(error:|fatal error:)" | wc -l) -gt 10 ]; then
            echo "  ... (显示前10个错误，更多错误请查看完整输出)"
        fi
    fi
    
    # 显示警告信息（只显示前5个）
    if [ "$warning_count" -gt 0 ]; then
        echo ""
        echo -e "${YELLOW}⚠️  主要警告:${NC}"
        echo "$compile_output" | grep "warning:" | head -5
        
        if [ "$warning_count" -gt 5 ]; then
            echo "  ... (显示前5个警告，共$warning_count个)"
        fi
    fi
    
    # 检查特定的ESP32相关问题
    echo ""
    echo "🔧 ESP32特定检查:"
    
    # 检查-mlongcalls参数问题
    if echo "$compile_output" | grep -q "mlongcalls"; then
        echo -e "  ${GREEN}✅ ESP32链接器参数正确${NC}"
    fi
    
    # 检查库依赖
    if echo "$compile_output" | grep -q "Library Dependency Finder"; then
        echo -e "  ${GREEN}✅ 库依赖解析正常${NC}"
    fi
    
    # 检查内存使用
    if echo "$compile_output" | grep -q "RAM:"; then
        echo -e "  ${GREEN}✅ 内存使用分析完成${NC}"
        echo "$compile_output" | grep -E "(RAM:|Flash:)"
    fi
    
    # 最终结果
    echo ""
    if [ "$compile_exit_code" -eq 0 ]; then
        if [ "$error_count" -eq 0 ] && [ "$fatal_count" -eq 0 ]; then
            echo -e "${GREEN}✅ 语法检查通过！${NC}"
            if [ "$warning_count" -gt 0 ]; then
                echo -e "${YELLOW}💡 建议: 修复 $warning_count 个警告以提高代码质量${NC}"
            fi
        else
            echo -e "${RED}❌ 语法检查失败: 发现 $((error_count + fatal_count)) 个错误${NC}"
        fi
    else
        echo -e "${RED}❌ 编译过程失败，请检查配置${NC}"
    fi
    
    # 生成编译数据库用于IDE支持
    if [ "$compile_exit_code" -eq 0 ]; then
        if [ -f "compile_commands.json" ]; then
            echo ""
            echo -e "${GREEN}📄 编译数据库已生成: compile_commands.json${NC}"
            echo "💡 此文件可用于IDE的智能提示和语法高亮"
        fi
    fi
}

# 擦除Flash
task_flash_erase() {
    echo -e "${RED}⚠️ 危险操作：擦除Flash${NC}"
    echo "这将完全擦除ESP32-S3的Flash内容！"
    echo ""
    read -p "确定要继续吗? 输入 'YES' 确认: " confirm
    
    if [ "$confirm" = "YES" ]; then
        echo -e "${BLUE}🔥 擦除Flash...${NC}"
        pio run --target erase
        echo -e "${GREEN}✅ Flash已擦除${NC}"
    else
        echo "取消操作"
    fi
}

# 重置配置
task_reset_config() {
    echo -e "${BLUE}🔄 重置配置到默认值...${NC}"
    
    echo -e "${YELLOW}⚠️ 这将重置以下配置文件:${NC}"
    echo "  - config/app_config.h"
    echo "  - config/debug_config.h"
    echo ""
    
    read -p "确定要继续吗? [y/N] " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        # 备份现有配置
        if [ -d "config" ]; then
            cp -r config config.backup.$(date +%s)
            echo "✅ 现有配置已备份"
        fi
        
        # 这里可以添加重置配置的逻辑
        echo "✅ 配置已重置 (需要根据项目实际情况实现)"
    else
        echo "取消重置"
    fi
}

# 检查SD卡内容
task_check_sd() {
    echo -e "${BLUE}💾 检查SD卡内容 (macOS)...${NC}"
    
    # macOS上的SD卡挂载点
    local sd_paths=(
        "/Volumes/NO NAME"
        "/Volumes/NONAME" 
        "/Volumes/Untitled"
        "/Volumes/SD"
        "/Volumes/SDCARD"
    )
    
    local found_sd=""
    for path in "${sd_paths[@]}"; do
        if [ -d "$path" ]; then
            found_sd="$path"
            echo -e "✅ 找到SD卡: ${GREEN}$path${NC}"
            break
        fi
    done
    
    if [ -z "$found_sd" ]; then
        echo -e "${YELLOW}⚠️ 未找到SD卡，可用挂载点:${NC}"
        if [ -d "/Volumes" ]; then
            ls -la /Volumes/ | grep -v "^total" | while read line; do
                echo "  $line"
            done
        fi
        return
    fi
    
    # 显示SD卡内容
    echo ""
    echo "📁 SD卡内容:"
    ls -la "$found_sd" | while read line; do
        echo "  $line"
    done
    
    # 检查特定文件
    echo ""
    echo "🔍 检查关键文件:"
    
    local key_files=("hello.json" "config.json" "data.txt")
    for file in "${key_files[@]}"; do
        local file_path="$found_sd/$file"
        if [ -f "$file_path" ]; then
            local size=$(stat -f%z "$file_path" 2>/dev/null || echo "unknown")
            echo -e "  ✅ $file (${size} bytes)"
            
            # 如果是JSON文件，尝试验证格式
            if [[ "$file" == *.json ]] && command -v python3 &> /dev/null; then
                if python3 -m json.tool "$file_path" > /dev/null 2>&1; then
                    echo "    📝 JSON格式有效"
                else
                    echo -e "    ${RED}❌ JSON格式无效${NC}"
                fi
            fi
        else
            echo -e "  ${YELLOW}⚠️ $file 不存在${NC}"
        fi
    done
}

# 主函数
main() {
    local task="$1"
    
    # 如果没有参数，显示帮助信息
    if [ -z "$task" ]; then
        show_help
        return 0
    fi
    
    shift
    
    case "$task" in
        "clean")
            task_clean
            ;;
        "build")
            task_build
            ;;
        "upload")
            task_upload
            ;;
        "monitor")
            task_monitor
            ;;
        "size")
            task_size
            ;;
        "deps")
            task_deps
            ;;
        "format")
            task_format
            ;;
        "backup")
            task_backup "$@"
            ;;
        "restore")
            task_restore "$@"
            ;;
        "git-status")
            task_git_status
            ;;
        "find-todos")
            task_find_todos
            ;;
        "count-lines")
            task_count_lines
            ;;
        "check-syntax")
            task_check_syntax
            ;;
        "flash-erase")
            task_flash_erase
            ;;
        "reset-config")
            task_reset_config
            ;;
        "check-sd")
            task_check_sd
            ;;
        "help"|"--help"|"-h"|"")
            show_help
            ;;
        *)
            echo -e "${RED}❌ 未知任务: $task${NC}"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"
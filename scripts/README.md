# ESP32-S3 HoloCubic Scripts 工具集

**"Talk is cheap. Show me the code."** - Linus Torvalds

## 🎯 工具概述

这是一套专为ESP32-S3 HoloCubic项目设计的开发工具脚本，遵循Linus的"简洁、实用、有效"原则。

## 📁 脚本列表 (已整理)

**Linus原则**：消除重复，单一职责，功能集中

### 1. 编译环境检查 - `1_env_check.sh`
**功能**：全面检查ESP32-S3开发环境
```bash
./scripts/1_env_check.sh
```

**检查项目**：
- ✅ 基础工具 (Python3, PlatformIO, Git, Make)
- ✅ ESP32平台安装状态
- ✅ 项目配置文件 (platformio.ini, 分区表)
- ✅ 源代码结构完整性
- ✅ 库依赖状态
- ✅ 编译测试和内存使用

**输出示例**：
```
🔍 ESP32-S3 HoloCubic 编译环境检查
==================================
✅ Python3: 3.9.7
✅ PlatformIO: 6.1.7
✅ ESP32平台: 6.4.0
✅ 编译测试: 通过
📊 检查结果总结: 8项通过, 0项失败, 2项警告
```

### 2. 库函数依赖分析 - `2_lib_deps.py`
**功能**：深度分析项目库依赖关系
```bash
python3 scripts/2_lib_deps.py
```

**分析内容**：
- 📚 PlatformIO库配置分析
- 🔍 源代码库使用统计
- 🎯 函数调用频率分析
- 🗑️ 未使用库检测
- 📁 本地库结构检查
- 💡 依赖优化建议

**输出文件**：
- `library_dependency_report.json` - 详细分析报告
- `dependency_graph.png` - 可视化依赖图 (需要graphviz)

### 3. 代码静态分析 - `3_static_analysis.py`
**功能**：分析代码质量和内存使用
```bash
python3 scripts/3_static_analysis.py
```

**分析功能**：
- 📊 全局变量和静态变量统计
- 💾 变量存储位置分析 (RAM/Flash/PSRAM)
- 🔍 Flash分区使用情况
- 🎯 代码质量问题检测
- 📈 编译内存使用报告
- 💡 内存优化建议

**检测问题**：
- 深度嵌套 (>3层)
- 长行代码 (>120字符)
- 魔数使用
- 未初始化全局变量

**输出文件**：`static_analysis_report.json`

### 4. 常见任务工具 - `4_common_tasks.sh`
**功能**：日常开发任务自动化
```bash
./scripts/4_common_tasks.sh <任务> [参数]
```

**可用任务**：
```bash
# 编译相关
./scripts/4_common_tasks.sh build          # 快速编译
./scripts/4_common_tasks.sh upload         # 编译并上传
./scripts/4_common_tasks.sh size           # 显示编译大小

# 维护相关
./scripts/4_common_tasks.sh clean          # 清理编译文件
./scripts/4_common_tasks.sh deps           # 更新库依赖
./scripts/4_common_tasks.sh format         # 格式化代码

# 项目管理
./scripts/4_common_tasks.sh backup ~/backups/    # 备份项目
./scripts/4_common_tasks.sh git-status     # Git状态检查
./scripts/4_common_tasks.sh find-todos     # 查找TODO项目
./scripts/4_common_tasks.sh count-lines    # 统计代码行数

# 调试工具
./scripts/4_common_tasks.sh monitor        # 串口监控
./scripts/4_common_tasks.sh check-syntax   # 语法检查
./scripts/4_common_tasks.sh check-sd       # 检查SD卡内容 (macOS)
./scripts/4_common_tasks.sh flash-erase    # 擦除Flash (慎用!)
```

## 🚀 快速使用

### 新环境设置
```bash
# 1. 检查开发环境
./scripts/1_env_check.sh

# 2. 如果有问题，按提示修复后重新检查
# 3. 环境OK后，进行首次编译
./scripts/4_common_tasks.sh build
```

### 日常开发流程
```bash
# 编译并上传
./scripts/4_common_tasks.sh upload

# 查看串口输出
./scripts/4_common_tasks.sh monitor

# 分析项目依赖
python3 scripts/2_lib_deps.py

# 代码质量检查
python3 scripts/3_static_analysis.py
```

### 项目维护
```bash
# 清理临时文件
./scripts/4_common_tasks.sh clean

# 更新库依赖
./scripts/4_common_tasks.sh deps

# 备份项目
./scripts/4_common_tasks.sh backup ~/my_backups/

# 查找待办事项
./scripts/4_common_tasks.sh find-todos
```

## 📊 输出报告

### 环境检查报告
- ✅/❌/⚠️ 彩色状态指示
- 详细的错误信息和修复建议
- 编译内存使用统计

### 依赖分析报告
- JSON格式详细报告
- 库使用统计和优化建议
- 未使用库检测

### 静态分析报告
- 变量存储位置分析
- 代码质量问题列表
- 内存使用优化建议

## 🛠️ 扩展开发

### 添加新的检查项
在对应脚本中添加检查函数：
```bash
# 在1_env_check.sh中添加
check_new_tool() {
    if command -v newtool &> /dev/null; then
        check_pass "NewTool: 已安装"
    else
        check_fail "NewTool: 未安装"
    fi
}
```

### 添加新的分析功能
在Python脚本中扩展分析类：
```python
def analyze_new_feature(self):
    """新的分析功能"""
    # 实现分析逻辑
    pass
```

### 添加新的常见任务
在4_common_tasks.sh中添加：
```bash
task_new_feature() {
    echo -e "${BLUE}🔧 执行新功能...${NC}"
    # 实现任务逻辑
}
```

## 🎯 设计原则

### Linus风格
- **简洁性**：每个脚本只做一件事，做好
- **实用性**：解决实际开发问题，不过度设计
- **可靠性**：错误处理完善，失败时给出明确提示

### 用户体验
- **彩色输出**：清晰的状态指示
- **详细报告**：问题定位和修复建议
- **自动化**：减少手工操作，提高效率

### 可维护性
- **模块化**：功能独立，易于扩展
- **文档化**：每个功能都有清晰说明
- **标准化**：统一的错误处理和输出格式

---

**让工具为你服务，专注于代码本身！**
## 🔄 整
理说明

### 已整合的旧脚本
以下旧脚本的功能已整合到新的工具集中：

- ❌ `check_platformio.py` → ✅ 整合到 `1_env_check.sh`
- ❌ `dependency_analyzer.py` → ✅ 整合到 `2_lib_deps.py` (增加图形化功能)
- ❌ `sd_card_check.py` → ✅ 整合到 `4_common_tasks.sh check-sd`
- ❌ `simple_deps.sh` → ✅ 整合到 `2_lib_deps.py`

### 整合优势
1. **消除重复** - 避免功能重复和维护困惑
2. **功能增强** - 新脚本功能更完整，分析更深入
3. **统一接口** - 一致的命令行接口和输出格式
4. **更好的错误处理** - 完善的异常处理和用户提示

### 迁移指南
如果您之前使用旧脚本，请按以下方式迁移：

```bash
# 旧方式 → 新方式
python3 scripts/check_platformio.py     → ./scripts/1_env_check.sh
python3 scripts/dependency_analyzer.py  → python3 scripts/2_lib_deps.py
python3 scripts/sd_card_check.py        → ./scripts/4_common_tasks.sh check-sd
./scripts/simple_deps.sh                → python3 scripts/2_lib_deps.py
```

---

**Linus原则体现**：简洁胜过复杂，统一胜过分散！
#!/usr/bin/env python3
"""
ESP32-S3 HoloCubic 项目依赖关系分析器
Linus风格：简洁、直接、有效的依赖分析工具
"""

import json
import subprocess
import sys
from pathlib import Path
import graphviz

def get_platformio_metadata():
    """获取PlatformIO项目元数据"""
    try:
        result = subprocess.run([
            'pio', 'project', 'metadata', 
            '--environment', 'esp32-s3-devkitc-1',
            '--json-output'
        ], capture_output=True, text=True, check=True)
        return json.loads(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"错误: 无法获取PlatformIO元数据: {e}")
        return None
    except json.JSONDecodeError as e:
        print(f"错误: 解析JSON失败: {e}")
        return None

def analyze_source_dependencies():
    """分析源代码依赖关系"""
    src_path = Path('src')
    dependencies = {}
    
    if not src_path.exists():
        print("错误: src目录不存在")
        return dependencies
    
    # 扫描所有C/C++文件
    for file_path in src_path.rglob('*.cpp'):
        deps = []
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()
                    if line.startswith('#include'):
                        # 提取包含的文件
                        if '"' in line:
                            # 本地包含
                            include = line.split('"')[1]
                            deps.append(('local', include))
                        elif '<' in line and '>' in line:
                            # 系统包含
                            include = line.split('<')[1].split('>')[0]
                            deps.append(('system', include))
        except Exception as e:
            print(f"警告: 无法读取文件 {file_path}: {e}")
        
        dependencies[str(file_path.relative_to('src'))] = deps
    
    return dependencies

def create_dependency_graph(metadata, source_deps):
    """创建依赖关系图"""
    dot = graphviz.Digraph(comment='ESP32-S3 HoloCubic Dependencies')
    dot.attr(rankdir='TB', size='12,8')
    dot.attr('node', shape='box', style='rounded,filled')
    
    # 添加主要模块节点
    modules = {
        'main.cpp': {'color': 'lightblue', 'label': 'Main Entry'},
        'app/': {'color': 'lightgreen', 'label': 'Application Layer'},
        'drivers/': {'color': 'lightyellow', 'label': 'Hardware Drivers'},
        'core/': {'color': 'lightcoral', 'label': 'Core System'},
        'system/': {'color': 'lightgray', 'label': 'System Utils'}
    }
    
    # 添加库依赖节点
    libraries = {
        'TFT_eSPI': {'color': 'orange', 'label': 'TFT Display'},
        'FastLED': {'color': 'red', 'label': 'LED Control'},
        'WiFi': {'color': 'cyan', 'label': 'WiFi Stack'},
        'Arduino': {'color': 'purple', 'label': 'Arduino Framework'}
    }
    
    # 添加节点
    for module, attrs in modules.items():
        dot.node(module, attrs['label'], fillcolor=attrs['color'])
    
    for lib, attrs in libraries.items():
        dot.node(lib, attrs['label'], fillcolor=attrs['color'], shape='ellipse')
    
    # 分析源代码依赖并添加边
    for source_file, deps in source_deps.items():
        source_module = get_module_category(source_file)
        
        for dep_type, dep_file in deps:
            if dep_type == 'local':
                target_module = get_module_category(dep_file)
                if source_module != target_module:
                    dot.edge(source_module, target_module)
            elif dep_type == 'system':
                lib = get_library_category(dep_file)
                if lib:
                    dot.edge(source_module, lib)
    
    return dot

def get_module_category(file_path):
    """根据文件路径确定模块类别"""
    if 'main.cpp' in file_path:
        return 'main.cpp'
    elif file_path.startswith('app/'):
        return 'app/'
    elif file_path.startswith('drivers/'):
        return 'drivers/'
    elif file_path.startswith('core/'):
        return 'core/'
    elif file_path.startswith('system/'):
        return 'system/'
    else:
        return 'core/'  # 默认分类

def get_library_category(include_file):
    """根据包含文件确定库类别"""
    if 'TFT_eSPI' in include_file or 'tft' in include_file.lower():
        return 'TFT_eSPI'
    elif 'FastLED' in include_file or 'led' in include_file.lower():
        return 'FastLED'
    elif 'WiFi' in include_file or 'wifi' in include_file.lower():
        return 'WiFi'
    elif 'Arduino' in include_file:
        return 'Arduino'
    else:
        return None

def generate_text_report(metadata, source_deps):
    """生成文本格式的依赖报告"""
    report = []
    report.append("=" * 60)
    report.append("ESP32-S3 HoloCubic 项目依赖关系报告")
    report.append("=" * 60)
    
    # 库依赖统计
    lib_stats = {}
    for source_file, deps in source_deps.items():
        for dep_type, dep_file in deps:
            if dep_type == 'system':
                lib = get_library_category(dep_file)
                if lib:
                    lib_stats[lib] = lib_stats.get(lib, 0) + 1
    
    report.append("\n📚 主要库依赖:")
    for lib, count in sorted(lib_stats.items(), key=lambda x: x[1], reverse=True):
        report.append(f"  - {lib}: {count} 次引用")
    
    # 模块依赖统计
    module_stats = {}
    for source_file, deps in source_deps.items():
        module = get_module_category(source_file)
        module_stats[module] = module_stats.get(module, 0) + len(deps)
    
    report.append("\n🏗️ 模块复杂度:")
    for module, count in sorted(module_stats.items(), key=lambda x: x[1], reverse=True):
        report.append(f"  - {module}: {count} 个依赖")
    
    # 包含路径信息
    if metadata and 'includes' in metadata:
        report.append(f"\n📁 包含路径数量:")
        for category, paths in metadata['includes'].items():
            report.append(f"  - {category}: {len(paths)} 个路径")
    
    return "\n".join(report)

def main():
    """主函数"""
    print("🔍 分析ESP32-S3 HoloCubic项目依赖关系...")
    
    # 获取PlatformIO元数据
    print("📊 获取PlatformIO元数据...")
    metadata = get_platformio_metadata()
    
    # 分析源代码依赖
    print("🔍 分析源代码依赖...")
    source_deps = analyze_source_dependencies()
    
    if not source_deps:
        print("❌ 无法分析源代码依赖")
        return 1
    
    # 生成文本报告
    print("📝 生成依赖报告...")
    report = generate_text_report(metadata, source_deps)
    
    # 保存报告
    with open('dependency_report.txt', 'w', encoding='utf-8') as f:
        f.write(report)
    print("✅ 依赖报告已保存到 dependency_report.txt")
    
    # 生成图形化依赖图
    try:
        print("🎨 生成依赖关系图...")
        graph = create_dependency_graph(metadata, source_deps)
        graph.render('dependency_graph', format='png', cleanup=True)
        print("✅ 依赖关系图已保存到 dependency_graph.png")
    except Exception as e:
        print(f"⚠️ 无法生成图形化依赖图: {e}")
        print("   请安装 graphviz: pip install graphviz")
    
    # 打印报告到控制台
    print("\n" + report)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
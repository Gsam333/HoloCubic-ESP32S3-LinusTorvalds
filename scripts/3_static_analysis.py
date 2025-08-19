#!/usr/bin/env python3
"""
ESP32-S3 HoloCubic 代码静态分析器
Linus风格：深度分析代码质量，识别全局变量、静态变量、Flash分区使用

功能：
1. 分析全局变量和静态变量
2. 检查变量存储位置（RAM/Flash/PSRAM）
3. 分析Flash分区使用情况
4. 检测代码质量问题
5. 生成内存使用报告
"""

import re
import sys
import json
import subprocess
from pathlib import Path
from collections import defaultdict
from dataclasses import dataclass
from typing import List, Dict, Set

@dataclass
class Variable:
    name: str
    type: str
    file: str
    line: int
    scope: str  # global, static, local
    storage: str  # ram, flash, psram, unknown
    size_estimate: int = 0
    initialized: bool = False

@dataclass
class Function:
    name: str
    file: str
    line: int
    complexity: int = 0
    local_vars: List[Variable] = None

class ESP32StaticAnalyzer:
    def __init__(self):
        self.project_root = Path('.')
        self.src_path = self.project_root / 'src'
        self.variables = []
        self.functions = []
        self.flash_usage = {}
        
        # ESP32-S3 内存映射
        self.memory_regions = {
            'DRAM': {'start': 0x3FC88000, 'size': 512 * 1024},  # 512KB DRAM
            'IRAM': {'start': 0x40374000, 'size': 64 * 1024},   # 64KB IRAM
            'FLASH': {'start': 0x42000000, 'size': 8 * 1024 * 1024},  # 8MB Flash
            'PSRAM': {'start': 0x3D000000, 'size': 8 * 1024 * 1024}   # 8MB PSRAM
        }
        
    def analyze_variables(self):
        """分析全局变量和静态变量"""
        print("🔍 分析变量声明...")
        
        # 匹配变量声明的正则表达式
        patterns = {
            'global_var': re.compile(r'^(?:extern\s+)?(?:const\s+)?(\w+(?:\s*\*)*)\s+(\w+)(?:\s*=\s*[^;]+)?;', re.MULTILINE),
            'static_var': re.compile(r'^static\s+(?:const\s+)?(\w+(?:\s*\*)*)\s+(\w+)(?:\s*=\s*[^;]+)?;', re.MULTILINE),
            'const_var': re.compile(r'^const\s+(\w+(?:\s*\*)*)\s+(\w+)(?:\s*=\s*[^;]+)?;', re.MULTILINE),
            'array_var': re.compile(r'^(?:static\s+)?(?:const\s+)?(\w+)\s+(\w+)\[([^\]]+)\](?:\s*=\s*[^;]+)?;', re.MULTILINE)
        }
        
        for file_path in self.src_path.rglob('*.cpp'):
            self._analyze_file_variables(file_path, patterns)
        
        for file_path in self.src_path.rglob('*.h'):
            self._analyze_file_variables(file_path, patterns)
    
    def _analyze_file_variables(self, file_path: Path, patterns: Dict):
        """分析单个文件中的变量"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            lines = content.split('\n')
            
            for pattern_name, pattern in patterns.items():
                for match in pattern.finditer(content):
                    line_num = content[:match.start()].count('\n') + 1
                    
                    if pattern_name == 'array_var':
                        var_type, var_name, array_size = match.groups()
                        size_estimate = self._estimate_array_size(var_type, array_size)
                    else:
                        var_type, var_name = match.groups()
                        size_estimate = self._estimate_type_size(var_type)
                    
                    # 确定作用域和存储位置
                    scope = self._determine_scope(match.group(0), pattern_name)
                    storage = self._determine_storage(match.group(0), var_type)
                    
                    variable = Variable(
                        name=var_name,
                        type=var_type,
                        file=str(file_path.relative_to(self.src_path)),
                        line=line_num,
                        scope=scope,
                        storage=storage,
                        size_estimate=size_estimate,
                        initialized='=' in match.group(0)
                    )
                    
                    self.variables.append(variable)
                    
        except Exception as e:
            print(f"⚠️ 无法分析文件 {file_path}: {e}")
    
    def _determine_scope(self, declaration: str, pattern_name: str) -> str:
        """确定变量作用域"""
        if 'static' in declaration:
            return 'static'
        elif pattern_name == 'global_var':
            return 'global'
        else:
            return 'unknown'
    
    def _determine_storage(self, declaration: str, var_type: str) -> str:
        """确定变量存储位置"""
        if 'const' in declaration:
            return 'flash'  # const变量通常存储在Flash中
        elif 'PROGMEM' in declaration:
            return 'flash'
        elif 'PSRAM' in declaration or 'ps_malloc' in declaration:
            return 'psram'
        elif any(t in var_type for t in ['char', 'int', 'float', 'double', 'bool']):
            return 'ram'  # 普通变量存储在RAM中
        else:
            return 'unknown'
    
    def _estimate_type_size(self, var_type: str) -> int:
        """估算变量类型大小"""
        size_map = {
            'bool': 1,
            'char': 1,
            'uint8_t': 1,
            'int8_t': 1,
            'uint16_t': 2,
            'int16_t': 2,
            'uint32_t': 4,
            'int32_t': 4,
            'int': 4,
            'float': 4,
            'double': 8,
            'long': 4,
            'size_t': 4,
        }
        
        # 处理指针
        if '*' in var_type:
            return 4  # 32位系统指针大小
        
        # 查找基础类型
        for type_name, size in size_map.items():
            if type_name in var_type:
                return size
        
        return 4  # 默认大小
    
    def _estimate_array_size(self, var_type: str, array_size: str) -> int:
        """估算数组大小"""
        base_size = self._estimate_type_size(var_type)
        
        try:
            # 尝试解析数组大小
            if array_size.isdigit():
                return base_size * int(array_size)
            else:
                # 处理宏定义等复杂情况
                return base_size * 100  # 估算值
        except:
            return base_size * 10  # 默认估算
    
    def analyze_flash_partition(self):
        """分析Flash分区使用情况"""
        print("💾 分析Flash分区使用...")
        
        partition_file = self.project_root / 'FLASH_8MB.csv'
        if not partition_file.exists():
            print("⚠️ 未找到FLASH_8MB.csv分区表文件")
            return {}
        
        partitions = {}
        try:
            with open(partition_file, 'r') as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()
                    if line and not line.startswith('#'):
                        parts = [p.strip() for p in line.split(',')]
                        if len(parts) >= 5:
                            name, type_val, subtype, offset, size = parts[:5]
                            partitions[name] = {
                                'type': type_val,
                                'subtype': subtype,
                                'offset': offset,
                                'size': size,
                                'line': line_num
                            }
        except Exception as e:
            print(f"⚠️ 无法读取分区表: {e}")
        
        return partitions
    
    def get_build_info(self):
        """获取编译信息"""
        print("🔨 获取编译信息...")
        
        try:
            # 获取编译大小信息
            result = subprocess.run([
                'pio', 'run', '--target', 'checkprogsize'
            ], capture_output=True, text=True, check=True)
            
            # 解析内存使用信息
            memory_info = {}
            for line in result.stderr.split('\n'):
                if 'RAM:' in line:
                    # 提取RAM使用信息
                    ram_match = re.search(r'RAM:\s+\[([=\s]+)\]\s+(\d+\.\d+)%\s+\(used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes\)', line)
                    if ram_match:
                        memory_info['ram'] = {
                            'used': int(ram_match.group(3)),
                            'total': int(ram_match.group(4)),
                            'percentage': float(ram_match.group(2))
                        }
                
                elif 'Flash:' in line:
                    # 提取Flash使用信息
                    flash_match = re.search(r'Flash:\s+\[([=\s]+)\]\s+(\d+\.\d+)%\s+\(used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes\)', line)
                    if flash_match:
                        memory_info['flash'] = {
                            'used': int(flash_match.group(3)),
                            'total': int(flash_match.group(4)),
                            'percentage': float(flash_match.group(2))
                        }
            
            return memory_info
            
        except subprocess.CalledProcessError as e:
            print(f"⚠️ 无法获取编译信息: {e}")
            return {}
    
    def analyze_code_quality(self):
        """分析代码质量问题"""
        print("🎯 分析代码质量...")
        
        quality_issues = []
        
        for file_path in self.src_path.rglob('*.cpp'):
            issues = self._analyze_file_quality(file_path)
            quality_issues.extend(issues)
        
        return quality_issues
    
    def _analyze_file_quality(self, file_path: Path):
        """分析单个文件的代码质量"""
        issues = []
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            for line_num, line in enumerate(lines, 1):
                line = line.strip()
                
                # 检查深度嵌套
                indent_level = (len(line) - len(line.lstrip())) // 2
                if indent_level > 3:
                    issues.append({
                        'type': 'deep_nesting',
                        'file': str(file_path.relative_to(self.src_path)),
                        'line': line_num,
                        'message': f'深度嵌套 ({indent_level} 层)',
                        'severity': 'warning'
                    })
                
                # 检查长行
                if len(line) > 120:
                    issues.append({
                        'type': 'long_line',
                        'file': str(file_path.relative_to(self.src_path)),
                        'line': line_num,
                        'message': f'行过长 ({len(line)} 字符)',
                        'severity': 'info'
                    })
                
                # 检查魔数
                magic_number_pattern = re.compile(r'\b\d{3,}\b')
                if magic_number_pattern.search(line) and not line.startswith('//'):
                    issues.append({
                        'type': 'magic_number',
                        'file': str(file_path.relative_to(self.src_path)),
                        'line': line_num,
                        'message': '可能包含魔数',
                        'severity': 'info'
                    })
                
        except Exception as e:
            print(f"⚠️ 无法分析文件质量 {file_path}: {e}")
        
        return issues
    
    def generate_report(self):
        """生成完整的静态分析报告"""
        print("🔍 ESP32-S3 HoloCubic 代码静态分析")
        print("=" * 60)
        
        # 1. 变量分析
        self.analyze_variables()
        
        # 按存储位置分组统计
        storage_stats = defaultdict(lambda: {'count': 0, 'size': 0})
        scope_stats = defaultdict(int)
        
        for var in self.variables:
            storage_stats[var.storage]['count'] += 1
            storage_stats[var.storage]['size'] += var.size_estimate
            scope_stats[var.scope] += 1
        
        print("\n📊 变量统计:")
        print(f"  总变量数: {len(self.variables)}")
        
        print("\n  按作用域分类:")
        for scope, count in sorted(scope_stats.items()):
            print(f"    {scope}: {count} 个")
        
        print("\n  按存储位置分类:")
        for storage, stats in sorted(storage_stats.items()):
            print(f"    {storage}: {stats['count']} 个变量, 约 {stats['size']} 字节")
        
        # 2. 详细变量列表
        print("\n📋 详细变量列表:")
        
        # 全局变量
        global_vars = [v for v in self.variables if v.scope == 'global']
        if global_vars:
            print(f"\n  🌐 全局变量 ({len(global_vars)} 个):")
            for var in sorted(global_vars, key=lambda x: x.size_estimate, reverse=True):
                print(f"    - {var.name} ({var.type}) - {var.file}:{var.line}")
                print(f"      存储: {var.storage}, 大小: ~{var.size_estimate} 字节")
        
        # 静态变量
        static_vars = [v for v in self.variables if v.scope == 'static']
        if static_vars:
            print(f"\n  🔒 静态变量 ({len(static_vars)} 个):")
            for var in sorted(static_vars, key=lambda x: x.size_estimate, reverse=True):
                print(f"    - {var.name} ({var.type}) - {var.file}:{var.line}")
                print(f"      存储: {var.storage}, 大小: ~{var.size_estimate} 字节")
        
        # 3. Flash分区分析
        print("\n💾 Flash分区分析:")
        partitions = self.analyze_flash_partition()
        
        if partitions:
            print("  分区表配置:")
            for name, info in partitions.items():
                print(f"    - {name}: {info['type']}/{info['subtype']}, 大小: {info['size']}")
        
        # 4. 编译信息
        print("\n🔨 编译内存使用:")
        build_info = self.get_build_info()
        
        if build_info:
            if 'ram' in build_info:
                ram = build_info['ram']
                print(f"  RAM: {ram['used']:,} / {ram['total']:,} 字节 ({ram['percentage']:.1f}%)")
            
            if 'flash' in build_info:
                flash = build_info['flash']
                print(f"  Flash: {flash['used']:,} / {flash['total']:,} 字节 ({flash['percentage']:.1f}%)")
        
        # 5. 代码质量分析
        print("\n🎯 代码质量分析:")
        quality_issues = self.analyze_code_quality()
        
        if quality_issues:
            issue_stats = defaultdict(int)
            for issue in quality_issues:
                issue_stats[issue['type']] += 1
            
            print("  发现的问题:")
            for issue_type, count in sorted(issue_stats.items()):
                print(f"    - {issue_type}: {count} 个")
            
            # 显示前10个最重要的问题
            print("\n  主要问题详情:")
            for issue in quality_issues[:10]:
                print(f"    {issue['file']}:{issue['line']} - {issue['message']}")
        else:
            print("  ✅ 未发现明显的代码质量问题")
        
        # 6. 优化建议
        print("\n💡 优化建议:")
        
        # RAM使用建议
        ram_vars = [v for v in self.variables if v.storage == 'ram']
        total_ram_usage = sum(v.size_estimate for v in ram_vars)
        if total_ram_usage > 50 * 1024:  # 50KB
            print(f"  ⚠️ RAM使用较高 (~{total_ram_usage} 字节)")
            print("     建议: 考虑将大数组移到PSRAM或Flash")
        
        # 全局变量建议
        if len(global_vars) > 10:
            print(f"  ⚠️ 全局变量较多 ({len(global_vars)} 个)")
            print("     建议: 考虑封装到类或命名空间中")
        
        # 未初始化变量
        uninit_vars = [v for v in self.variables if not v.initialized and v.scope == 'global']
        if uninit_vars:
            print(f"  ⚠️ 发现 {len(uninit_vars)} 个未初始化的全局变量")
            print("     建议: 显式初始化所有全局变量")
        
        print("\n✅ 静态分析完成")
        
        return {
            'variables': [vars(v) for v in self.variables],
            'storage_stats': dict(storage_stats),
            'scope_stats': dict(scope_stats),
            'partitions': partitions,
            'build_info': build_info,
            'quality_issues': quality_issues
        }

def main():
    """主函数"""
    analyzer = ESP32StaticAnalyzer()
    
    try:
        report_data = analyzer.generate_report()
        
        # 保存详细报告到文件
        report_file = 'static_analysis_report.json'
        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False, default=str)
        
        print(f"\n📄 详细报告已保存到: {report_file}")
        
        # 生成可视化图表
        try:
            analyzer.generate_memory_visualization(report_data)
        except ImportError:
            print("💡 提示: 安装 graphviz 可生成内存分布可视化图: pip install graphviz")
        except Exception as e:
            print(f"⚠️ 无法生成可视化图: {e}")
        
        return 0
        
    except Exception as e:
        print(f"❌ 分析过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
        return 1

def generate_memory_visualization(self, report_data):
    """生成ESP32-S3内存分布可视化图"""
    try:
        import graphviz
    except ImportError:
        raise ImportError("需要安装graphviz: pip install graphviz")
    
    print("\n🎨 生成ESP32-S3内存分布可视化图...")
    
    # 创建内存分布图
    dot = graphviz.Digraph(comment='ESP32-S3 Memory Layout', format='png')
    dot.attr(rankdir='TB', size='14,10')
    dot.attr('node', shape='box', style='filled')
    
    # ESP32-S3 内存区域
    with dot.subgraph(name='cluster_flash') as flash:
        flash.attr(label='Flash Memory (8MB)', style='filled', color='lightgray')
        
        # Flash分区
        partitions = report_data.get('partitions', {})
        for name, info in partitions.items():
            size_mb = int(info['size'], 16) / (1024 * 1024)
            color = {
                'nvs': 'lightblue',
                'otadata': 'lightgreen', 
                'app0': 'orange',
                'spiffs': 'lightyellow'
            }.get(name, 'white')
            
            flash.node(f'flash_{name}', 
                      f'{name}\\n{info["type"]}/{info["subtype"]}\\n{size_mb:.2f}MB',
                      fillcolor=color)
    
    with dot.subgraph(name='cluster_ram') as ram:
        ram.attr(label='RAM Memory (320KB)', style='filled', color='lightcyan')
        
        # RAM中的变量
        ram_vars = [v for v in report_data['variables'] if v['storage'] == 'ram']
        total_ram_size = sum(v['size_estimate'] for v in ram_vars)
        
        ram.node('ram_vars', 
                f'Variables\\n{len(ram_vars)} vars\\n{total_ram_size} bytes',
                fillcolor='lightpink')
        
        # 显示主要的RAM变量
        for var in sorted(ram_vars, key=lambda x: x['size_estimate'], reverse=True)[:5]:
            ram.node(f'var_{var["name"]}',
                    f'{var["name"]}\\n{var["type"]}\\n{var["size_estimate"]}B',
                    fillcolor='pink')
            ram.edge('ram_vars', f'var_{var["name"]}')
    
    with dot.subgraph(name='cluster_unknown') as unknown:
        unknown.attr(label='Unknown Storage', style='filled', color='lightgray')
        
        # 未知存储的变量
        unknown_vars = [v for v in report_data['variables'] if v['storage'] == 'unknown']
        total_unknown_size = sum(v['size_estimate'] for v in unknown_vars)
        
        unknown.node('unknown_vars',
                    f'Variables\\n{len(unknown_vars)} vars\\n{total_unknown_size} bytes',
                    fillcolor='lightgray')
        
        # 显示最大的未知变量
        largest_unknown = sorted(unknown_vars, key=lambda x: x['size_estimate'], reverse=True)[:3]
        for var in largest_unknown:
            unknown.node(f'unk_{var["name"]}',
                        f'{var["name"]}\\n{var["type"]}\\n{var["size_estimate"]}B',
                        fillcolor='gray')
            unknown.edge('unknown_vars', f'unk_{var["name"]}')
    
    # 添加统计信息
    dot.node('stats', 
            f'Memory Statistics\\n'
            f'Total Variables: {len(report_data["variables"])}\\n'
            f'Global: {report_data["scope_stats"]["global"]}\\n'
            f'Static: {report_data["scope_stats"]["static"]}\\n'
            f'RAM Usage: ~{total_ram_size}B\\n'
            f'Unknown: ~{total_unknown_size}B',
            fillcolor='lightyellow', shape='ellipse')
    
    # 保存图形
    try:
        dot.render('memory_layout', cleanup=True)
        print("✅ 内存分布图已保存到: memory_layout.png")
    except Exception as e:
        print(f"⚠️ 无法保存图形文件: {e}")
    
    # 生成魔数分析图
    self.generate_magic_number_chart(report_data)

def generate_magic_number_chart(self, report_data):
    """生成魔数分布分析图"""
    try:
        import graphviz
    except ImportError:
        return
    
    print("🔢 生成魔数分布分析图...")
    
    # 统计魔数分布
    magic_numbers = [issue for issue in report_data['quality_issues'] 
                    if issue['type'] == 'magic_number']
    
    # 按文件分组
    file_magic_count = {}
    for issue in magic_numbers:
        file_path = issue['file']
        # 简化文件路径
        if '/' in file_path:
            category = file_path.split('/')[0]
        else:
            category = 'root'
        
        if category not in file_magic_count:
            file_magic_count[category] = 0
        file_magic_count[category] += 1
    
    # 创建魔数分布图
    dot = graphviz.Digraph(comment='Magic Numbers Distribution', format='png')
    dot.attr(rankdir='LR', size='12,8')
    dot.attr('node', shape='box', style='filled')
    
    # 中心节点
    dot.node('magic_center', 
            f'Magic Numbers\\nTotal: {len(magic_numbers)}',
            fillcolor='red', fontcolor='white')
    
    # 按优先级分类
    priority_colors = {
        'core': 'orange',      # 高优先级
        'system': 'orange',    # 高优先级  
        'app': 'yellow',       # 中优先级
        'drivers': 'yellow',   # 中优先级
        'test': 'lightgreen'   # 低优先级
    }
    
    for category, count in sorted(file_magic_count.items(), key=lambda x: x[1], reverse=True):
        color = priority_colors.get(category, 'lightgray')
        priority = 'HIGH' if category in ['core', 'system'] else \
                  'MED' if category in ['app', 'drivers'] else 'LOW'
        
        dot.node(f'cat_{category}',
                f'{category}/\\n{count} magic numbers\\nPriority: {priority}',
                fillcolor=color)
        dot.edge('magic_center', f'cat_{category}', label=str(count))
    
    # 保存魔数分析图
    try:
        dot.render('magic_numbers_analysis', cleanup=True)
        print("✅ 魔数分析图已保存到: magic_numbers_analysis.png")
    except Exception as e:
        print(f"⚠️ 无法保存魔数分析图: {e}")

# 将方法添加到类中
ESP32StaticAnalyzer.generate_memory_visualization = generate_memory_visualization
ESP32StaticAnalyzer.generate_magic_number_chart = generate_magic_number_chart

if __name__ == '__main__':
    sys.exit(main())
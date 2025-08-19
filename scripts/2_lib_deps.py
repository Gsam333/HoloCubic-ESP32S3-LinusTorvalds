#!/usr/bin/env python3
"""
ESP32-S3 HoloCubic 库函数依赖分析器
Linus风格：深度分析库依赖关系，识别潜在问题

功能：
1. 分析PlatformIO库依赖
2. 检查库版本兼容性
3. 识别未使用的库
4. 分析库函数调用关系
5. 检测循环依赖
"""

import json
import subprocess
import sys
import re
from pathlib import Path
from collections import defaultdict, deque

class LibraryDependencyAnalyzer:
    def __init__(self):
        self.project_root = Path('.')
        self.src_path = self.project_root / 'src'
        self.lib_path = self.project_root / 'lib'
        self.dependencies = defaultdict(set)
        self.library_usage = defaultdict(int)
        self.function_calls = defaultdict(set)
        
    def get_platformio_libs(self):
        """获取PlatformIO库信息"""
        try:
            # 获取已安装的库
            result = subprocess.run([
                'pio', 'lib', 'list', '--json-output'
            ], capture_output=True, text=True, check=True)
            
            installed_libs = json.loads(result.stdout)
            
            # 获取项目依赖
            if (self.project_root / 'platformio.ini').exists():
                with open('platformio.ini', 'r') as f:
                    content = f.read()
                    
                # 提取lib_deps
                lib_deps = []
                in_lib_deps = False
                for line in content.split('\n'):
                    line = line.strip()
                    if line.startswith('lib_deps'):
                        in_lib_deps = True
                        continue
                    elif in_lib_deps:
                        if line.startswith('-') or line.startswith(';'):
                            continue
                        elif line and not line.startswith('[') and not line.startswith('#'):
                            lib_deps.append(line)
                        elif line.startswith('['):
                            break
                
                return {
                    'installed': installed_libs,
                    'declared': lib_deps
                }
        except Exception as e:
            print(f"⚠️ 无法获取PlatformIO库信息: {e}")
            return {'installed': [], 'declared': []}
    
    def analyze_source_includes(self):
        """分析源代码中的库包含"""
        include_pattern = re.compile(r'#include\s*[<"]([^>"]+)[>"]')
        library_includes = defaultdict(set)
        
        # 扫描所有源文件
        for file_path in self.src_path.rglob('*.cpp'):
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                for match in include_pattern.finditer(content):
                    include_file = match.group(1)
                    
                    # 识别库类型
                    library = self.identify_library(include_file)
                    if library:
                        library_includes[library].add(str(file_path.relative_to(self.src_path)))
                        self.library_usage[library] += 1
                        
            except Exception as e:
                print(f"⚠️ 无法读取文件 {file_path}: {e}")
        
        return library_includes
    
    def identify_library(self, include_file):
        """根据包含文件识别库"""
        library_patterns = {
            'Arduino': ['Arduino.h', 'WString.h', 'Print.h'],
            'WiFi': ['WiFi.h', 'WiFiClient.h', 'WiFiServer.h', 'WiFiUdp.h'],
            'TFT_eSPI': ['TFT_eSPI.h', 'TFT_eSPI'],
            'FastLED': ['FastLED.h', 'FastLED'],
            'ArduinoJson': ['ArduinoJson.h', 'ArduinoJson'],
            'PubSubClient': ['PubSubClient.h'],
            'AsyncTCP': ['AsyncTCP.h'],
            'ESPAsyncWebServer': ['ESPAsyncWebServer.h'],
            'SPIFFS': ['SPIFFS.h', 'FS.h'],
            'SD': ['SD.h', 'SD_MMC.h'],
            'Wire': ['Wire.h'],
            'SPI': ['SPI.h'],
            'EEPROM': ['EEPROM.h'],
            'Preferences': ['Preferences.h'],
            'esp_system': ['esp_system.h', 'esp_log.h', 'esp_err.h'],
            'FreeRTOS': ['freertos/FreeRTOS.h', 'freertos/task.h']
        }
        
        for library, patterns in library_patterns.items():
            if any(pattern in include_file for pattern in patterns):
                return library
        
        # 检查是否是本地库
        if not include_file.startswith('esp') and not include_file.endswith('.h'):
            return 'Local'
        
        return None
    
    def analyze_function_calls(self):
        """分析函数调用关系"""
        function_pattern = re.compile(r'(\w+)\s*\(')
        
        # 常见库函数模式
        library_functions = {
            'WiFi': ['WiFi.begin', 'WiFi.status', 'WiFi.localIP', 'WiFi.disconnect'],
            'TFT_eSPI': ['tft.begin', 'tft.fillScreen', 'tft.drawString', 'tft.setRotation'],
            'FastLED': ['FastLED.show', 'FastLED.setBrightness', 'FastLED.addLeds'],
            'Serial': ['Serial.begin', 'Serial.print', 'Serial.println', 'Serial.available'],
            'SPIFFS': ['SPIFFS.begin', 'SPIFFS.open', 'SPIFFS.exists'],
            'Preferences': ['preferences.begin', 'preferences.getString', 'preferences.putString']
        }
        
        function_usage = defaultdict(int)
        
        for file_path in self.src_path.rglob('*.cpp'):
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                for library, functions in library_functions.items():
                    for func in functions:
                        if func in content:
                            function_usage[f"{library}::{func}"] += content.count(func)
                            
            except Exception as e:
                print(f"⚠️ 无法分析文件 {file_path}: {e}")
        
        return function_usage
    
    def check_unused_libraries(self, platformio_libs, source_includes):
        """检查未使用的库"""
        declared_libs = set(platformio_libs.get('declared', []))
        used_libs = set(source_includes.keys())
        
        # 简化库名匹配
        def normalize_lib_name(name):
            return name.lower().replace('-', '').replace('_', '').replace(' ', '')
        
        unused_libs = []
        for declared in declared_libs:
            lib_name = declared.split('@')[0].split('/')[0]  # 提取库名
            normalized_declared = normalize_lib_name(lib_name)
            
            found = False
            for used in used_libs:
                if normalized_declared in normalize_lib_name(used):
                    found = True
                    break
            
            if not found:
                unused_libs.append(declared)
        
        return unused_libs
    
    def generate_report(self):
        """生成完整的依赖分析报告"""
        print("🔍 ESP32-S3 HoloCubic 库函数依赖分析")
        print("=" * 60)
        
        # 1. PlatformIO库信息
        print("\n📚 PlatformIO库配置:")
        platformio_libs = self.get_platformio_libs()
        
        if platformio_libs['declared']:
            print("  声明的库依赖:")
            for lib in platformio_libs['declared']:
                print(f"    - {lib}")
        else:
            print("  ⚠️ 未找到lib_deps配置")
        
        # 2. 源代码库使用分析
        print("\n🔍 源代码库使用分析:")
        source_includes = self.analyze_source_includes()
        
        if source_includes:
            for library, files in sorted(source_includes.items()):
                print(f"  📦 {library}:")
                print(f"    使用次数: {self.library_usage[library]}")
                print(f"    使用文件: {len(files)} 个")
                for file in sorted(files):
                    print(f"      - {file}")
        else:
            print("  ⚠️ 未检测到库使用")
        
        # 3. 函数调用分析
        print("\n🎯 函数调用分析:")
        function_usage = self.analyze_function_calls()
        
        if function_usage:
            for func, count in sorted(function_usage.items(), key=lambda x: x[1], reverse=True):
                if count > 0:
                    print(f"  - {func}: {count} 次调用")
        else:
            print("  ⚠️ 未检测到库函数调用")
        
        # 4. 未使用库检查
        print("\n🗑️ 未使用库检查:")
        unused_libs = self.check_unused_libraries(platformio_libs, source_includes)
        
        if unused_libs:
            print("  发现可能未使用的库:")
            for lib in unused_libs:
                print(f"    - {lib}")
            print("  💡 建议: 考虑从platformio.ini中移除未使用的库")
        else:
            print("  ✅ 所有声明的库都在使用中")
        
        # 5. 本地库分析
        print("\n📁 本地库分析:")
        if self.lib_path.exists():
            local_libs = [d for d in self.lib_path.iterdir() if d.is_dir()]
            if local_libs:
                print(f"  本地库数量: {len(local_libs)}")
                for lib_dir in local_libs:
                    print(f"    - {lib_dir.name}")
                    
                    # 检查库结构
                    if (lib_dir / 'library.properties').exists():
                        print(f"      ✅ 标准Arduino库结构")
                    elif (lib_dir / 'src').exists():
                        print(f"      ✅ 包含src目录")
                    else:
                        print(f"      ⚠️ 非标准库结构")
            else:
                print("  📦 无本地库")
        else:
            print("  📦 lib目录不存在")
        
        # 6. 依赖关系建议
        print("\n💡 优化建议:")
        
        # 检查重复功能的库
        wifi_libs = [lib for lib in source_includes.keys() if 'wifi' in lib.lower() or 'async' in lib.lower()]
        if len(wifi_libs) > 1:
            print(f"  ⚠️ 检测到多个网络库: {', '.join(wifi_libs)}")
            print("     建议: 统一使用一个网络库")
        
        # 检查显示库
        display_libs = [lib for lib in source_includes.keys() if any(x in lib.lower() for x in ['tft', 'display', 'lcd', 'oled'])]
        if len(display_libs) > 1:
            print(f"  ⚠️ 检测到多个显示库: {', '.join(display_libs)}")
            print("     建议: 统一使用一个显示库")
        
        # 内存使用建议
        heavy_libs = ['ArduinoJson', 'ESPAsyncWebServer', 'TFT_eSPI']
        used_heavy_libs = [lib for lib in heavy_libs if lib in source_includes]
        if used_heavy_libs:
            print(f"  💾 内存密集型库: {', '.join(used_heavy_libs)}")
            print("     建议: 监控内存使用，考虑使用PSRAM")
        
        print("\n✅ 库依赖分析完成")
        
        return {
            'platformio_libs': platformio_libs,
            'source_includes': source_includes,
            'function_usage': function_usage,
            'unused_libs': unused_libs
        }

def main():
    """主函数"""
    analyzer = LibraryDependencyAnalyzer()
    
    try:
        report_data = analyzer.generate_report()
        
        # 保存详细报告到文件
        report_file = 'library_dependency_report.json'
        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False, default=str)
        
        print(f"\n📄 详细报告已保存到: {report_file}")
        
        # 生成图形化依赖图 (如果graphviz可用)
        try:
            analyzer.generate_dependency_graph(report_data['source_includes'])
        except ImportError:
            print("💡 提示: 安装 graphviz 可生成可视化依赖图: pip install graphviz")
        except Exception as e:
            print(f"⚠️ 无法生成依赖图: {e}")
        
        return 0
        
    except Exception as e:
        print(f"❌ 分析过程中发生错误: {e}")
        return 1

def generate_dependency_graph(self, source_includes):
    """生成图形化依赖关系图"""
    try:
        import graphviz
    except ImportError:
        raise ImportError("需要安装graphviz: pip install graphviz")
    
    print("\n🎨 生成依赖关系图...")
    
    dot = graphviz.Digraph(comment='ESP32-S3 HoloCubic Dependencies')
    dot.attr(rankdir='TB', size='12,8')
    dot.attr('node', shape='box', style='rounded,filled')
    
    # 添加库节点
    library_colors = {
        'Arduino': 'lightblue',
        'WiFi': 'lightgreen', 
        'TFT_eSPI': 'orange',
        'FastLED': 'red',
        'ArduinoJson': 'yellow',
        'SPIFFS': 'lightgray',
        'SD': 'lightcyan',
        'Wire': 'lightpink',
        'FreeRTOS': 'lightyellow',
        'Local': 'pink'
    }
    
    for library in source_includes.keys():
        color = library_colors.get(library, 'white')
        dot.node(library, library, fillcolor=color)
    
    # 添加使用关系 (简化版)
    dot.node('Project', 'ESP32-S3 HoloCubic', fillcolor='lightcoral', shape='ellipse')
    
    for library, files in source_includes.items():
        dot.edge('Project', library, label=f"{len(files)} files")
    
    # 保存图形
    try:
        dot.render('dependency_graph', format='png', cleanup=True)
        print("✅ 依赖关系图已保存到: dependency_graph.png")
    except Exception as e:
        print(f"⚠️ 无法保存图形文件: {e}")

# 将方法添加到类中
LibraryDependencyAnalyzer.generate_dependency_graph = generate_dependency_graph

if __name__ == '__main__':
    sys.exit(main())
# Core 系统核心

**"Good programmers worry about data structures and their relationships."** - Linus Torvalds

## 模块职责

### boot/ - 系统启动
- `system_boot.*` - 启动序列管理
- `hardware_init.h` - 硬件初始化接口

### config/ - 硬件配置
- `hardware_config.h` - 集中的硬件配置定义

### state/ - 状态管理  
- `system_state.*` - 系统状态集中管理

### types/ - 类型定义
- `system_types.h` - 系统基础类型
- `error_handling.h` - 错误处理机制

## 设计原则

1. **硬件抽象层** - 为上层提供统一的硬件接口
2. **配置集中化** - 所有硬件配置在一处定义
3. **状态单一来源** - 系统状态统一管理
4. **类型安全** - 强类型定义，编译时检查

## 依赖关系

```
core/ (硬件抽象层)
├── 被依赖: drivers/, app/, system/
└── 依赖: 无 (最底层)
```

这是整个系统的基础层，遵循Linux内核的分层设计原则。
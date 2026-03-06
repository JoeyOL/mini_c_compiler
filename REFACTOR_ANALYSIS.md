# Mini C Compiler - 代码重构分析报告

## 🚨 主要问题总结

### 1. 代码结构问题

#### ❌ 头文件依赖混乱
- `scanner.h` 包含了完整的实现细节
- 循环依赖风险：`parser.h` 包含了过多具体实现
- 头文件包含了实现代码，违反了分离原则

#### ❌ 巨型文件
- `defs.h`: 664行 - 包含了所有定义、类声明和实现
- `parser.cpp`: 734行 - 过于庞大的实现文件
- `x86_64.h`: 1221行 - 代码生成器过于复杂

### 2. 设计模式问题

#### ❌ 违反单一职责原则
```cpp
// defs.h 中 - Symbol 类承担了太多职责
struct Symbol {
    std::string name;
    PrimitiveType type;
    int size;
    bool is_array;
    bool is_global;
    bool is_param = false;
    int pos_in_stack;
    std::vector<int> array_dimensions;
    
    // 这个方法不应该在这里！
    std::string getAddress() const {
        if (is_global) return name + "(%rip)";
        else {
            if (size < 4) return std::to_string(pos_in_stack + 4 - size) + "(%rbp)";
            return std::to_string(pos_in_stack) + "(%rbp)";
        }
    }
};
```

#### ❌ 紧耦合
- `SymbolTable` 直接管理代码生成细节
- AST节点包含了代码生成逻辑
- 前端和后端没有清晰分离

### 3. 代码质量问题

#### ❌ 重复代码
```cpp
// 在多个地方出现类似的类型转换
if (value.type == P_INT && other.type == P_INT) {
    return Value{P_INT, .ivalue = value.ivalue + other.ivalue};
} else if (value.type == P_FLOAT && other.type == P_FLOAT) {
    return Value{P_FLOAT, .fvalue = value.fvalue + other.fvalue};
}
// ... 这种模式在加减乘除中重复出现
```

#### ❌ 魔法数字
```cpp
// 这些数字应该定义为常量
const int MAX_IDENTIFIER_LENGTH = 1024; // 这个还好
int int_param_in_reg_count = 6;  // ❌ 魔法数字
int float_param_in_reg_count = 8; // ❌ 魔法数字
int param_on_stack_st = 16;      // ❌ 魔法数字
```

#### ❌ 错误处理不一致
```cpp
// 有些地方抛异常
throw std::runtime_error("Division by zero");

// 有些地方用assert
assert(peek().type == T_INT || peek().type == T_FLOAT);

// 有些地方直接崩溃
if (!source_file.is_open()) {
    throw std::runtime_error("Failed to open file: " + source_path);
}
```

### 4. 架构问题

#### ❌ 前端和后端耦合
```cpp
// ASTNode 包含了代码生成细节
virtual PrimitiveType getCalculateType() const {
    if (is_pointer(type) || is_array(type)) {
        return P_LONG;  // ❌ AST不应该知道目标平台的指针大小
    }
    return type;
}
```

#### ❌ 平台相关代码分散
- x86-64特定代码散落在各个文件中
- 没有抽象的中间表示
- 直接生成目标代码

## 🔧 重构建议

### 1. 架构重构

#### ✅ 分层架构
```
┌─────────────────────────────────────┐
│            前端 (Frontend)          │
├─────────────────────────────────────┤
│  1. 词法分析器 (Scanner)           │
│  2. 语法分析器 (Parser)            │
│  3. 语义分析器 (Semantic)          │
│  4. AST (抽象语法树)               │
├─────────────────────────────────────┤
│        中间表示 (Middle-end)        │
├─────────────────────────────────────┤
│  5. 中间代码生成 (IR Generator)    │
│  6. 中间表示优化 (IR Optimizer)    │
├─────────────────────────────────────┤
│          后端 (Backend)           │
├─────────────────────────────────────┤
│  7. 目标代码生成 (Code Generator)  │
│  8. 汇编输出 (Assembly Output)     │
└─────────────────────────────────────┘
```

#### ✅ 模块分离
- **defs.h** → 拆分为多个专用头文件
  - `token.h` - 词法单元定义
  - `ast_nodes.h` - AST节点定义
  - `symbol_table.h` - 符号表定义
  - `types.h` - 类型系统定义
  - `ir.h` - 中间表示定义

### 2. 具体重构方案

#### 🎯 优先级1：头文件整理
```cpp
// 当前问题：defs.h 包含了所有内容
// 建议拆分：

// token.h
enum class TokenType { /* ... */ };
struct Token { /* ... */ };

// types.h  
enum class PrimitiveType { /* ... */ };
enum class ExprType { /* ... */ };

// ast_nodes.h
class ASTNode { /* ... */ };
class ExprNode : public ASTNode { /* ... */ };
class StatementNode : public ASTNode { /* ... */ };

// symbol_table.h
class Symbol { /* ... */ };
class SymbolTable { /* ... */ };

// value.h
class Value { /* ... */ };
```

#### 🎯 优先级2：解耦AST和代码生成
```cpp
// ❌ 当前：AST节点包含代码生成逻辑
class ASTNode {
    virtual PrimitiveType getCalculateType() const {
        // 包含了平台相关逻辑
    }
};

// ✅ 建议：AST只负责表示
class ASTNode {
    virtual void accept(ASTVisitor* visitor) = 0;
};

// 代码生成通过访问者模式
class CodeGenerator : public ASTVisitor {
    void visit(BinaryExprNode* node) override {
        // 平台相关的代码生成逻辑
    }
};
```

#### 🎯 优先级3：错误处理统一
```cpp
// ✅ 建议：统一的错误处理机制
class CompilerError {
    ErrorType type;
    std::string message;
    int line;
    int column;
    std::string source_file;
};

class ErrorReporter {
    std::vector<CompilerError> errors;
    void report(ErrorType type, const std::string& msg, 
                int line, int column);
    void printErrors() const;
    bool hasErrors() const { return !errors.empty(); }
};
```

#### 🎯 优先级4：类型系统重构
```cpp
// ✅ 建议：更清晰的类型层次
class Type {
public:
    virtual ~Type() = default;
    virtual int getSize() const = 0;
    virtual std::string toString() const = 0;
};

class PrimitiveType : public Type {
    PrimitiveTypeID id;
    int getSize() const override;
};

class PointerType : public Type {
    std::shared_ptr<Type> pointee_type;
    int getSize() const override { return 8; } // 平台相关
};

class ArrayType : public Type {
    std::shared_ptr<Type> element_type;
    std::vector<int> dimensions;
    int getSize() const override;
};
```

### 3. 重构实施计划

#### 阶段1：代码整理 (1-2天)
- [ ] 拆分defs.h为多个专用头文件
- [ ] 整理#include依赖关系
- [ ] 移动实现代码到.cpp文件

#### 阶段2：架构重构 (2-3天)
- [ ] 实现访问者模式分离AST和代码生成
- [ ] 创建中间表示层
- [ ] 统一错误处理机制

#### 阶段3：代码质量提升 (1-2天)
- [ ] 消除重复代码
- [ ] 替换魔法数字为常量
- [ ] 添加单元测试

#### 阶段4：性能优化 (1天)
- [ ] 优化符号表查找
- [ ] 改进内存管理
- [ ] 添加编译时优化

## 🚀 立即可以改进的地方

### 1. 快速修复
```cpp
// 添加缺失的常量定义
namespace Constants {
    constexpr int MAX_INT_REG_PARAMS = 6;
    constexpr int MAX_FLOAT_REG_PARAMS = 8;  
    constexpr int STACK_PARAM_OFFSET = 16;
    constexpr int STACK_ALIGNMENT = 16;
}

// 统一的错误处理宏
#define COMPILER_ERROR(msg, line, col) \
    throw CompilerException(msg, line, col, __FILE__, __FUNCTION__)
```

### 2. 代码简化
```cpp
// 消除重复的类型转换代码
template<typename Op>
Value performBinaryOp(const Value& lhs, const Value& rhs, Op op) {
    // 统一的类型转换和运算逻辑
}
```

你想先从哪个方面开始重构？我可以立即开始实施这些改进！
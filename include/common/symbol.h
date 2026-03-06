#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <memory>

// 前向声明
class Symbol;
class Function;

// 符号类 - 只包含符号信息，不包含代码生成逻辑
class Symbol {
public:
    std::string name;
    PrimitiveType type;
    int size;
    bool is_array;
    bool is_global;
    bool is_param;
    int pos_in_stack; // 栈位置（由代码生成器使用）
    std::vector<int> array_dimensions; // 数组维度

    // 构造函数
    Symbol() : is_param(false), pos_in_stack(0) {}
    
    Symbol(const std::string& name, PrimitiveType type, int size, 
           bool is_array, bool is_global, int pos_in_stack = 0)
        : name(name), type(type), size(size), is_array(is_array), 
          is_global(is_global), is_param(false), pos_in_stack(pos_in_stack) {}
    
    Symbol(const std::string& name, PrimitiveType type, int size, 
           bool is_array, bool is_global, int pos_in_stack,
           const std::vector<int>& dimensions)
        : name(name), type(type), size(size), is_array(is_array), 
          is_global(is_global), is_param(false), pos_in_stack(pos_in_stack),
          array_dimensions(dimensions) {}

    // 基本操作
    bool operator==(const Symbol& other) const {
        return name == other.name;
    }
    
    bool operator!=(const Symbol& other) const {
        return !(*this == other);
    }

    // 数组相关操作
    int getArrayBaseOffset(int depth) const {
        if (!is_array || depth >= static_cast<int>(array_dimensions.size())) {
            throw std::runtime_error("Symbol::getArrayBaseOffset: Invalid depth for array dimensions");
        }
        int offset = 1;
        for (size_t i = depth + 1; i < array_dimensions.size(); ++i) {
            offset *= array_dimensions[i];
        }
        return offset;
    }

    // 调试和输出
    std::string toString() const {
        std::string result = name + ": " + std::to_string(static_cast<int>(type));
        if (is_array) {
            result += "[";
            for (size_t i = 0; i < array_dimensions.size(); ++i) {
                if (i > 0) result += ", ";
                result += std::to_string(array_dimensions[i]);
            }
            result += "]";
        }
        if (is_global) result += " (global)";
        if (is_param) result += " (param)";
        result += " size=" + std::to_string(size) + " stack=" + std::to_string(pos_in_stack);
        return result;
    }

    // Getter方法
    const std::string& getName() const { return name; }
    PrimitiveType getType() const { return type; }
    int getSize() const { return size; }
    bool isArray() const { return is_array; }
    bool isGlobal() const { return is_global; }
    bool isParam() const { return is_param; }
    int getStackPosition() const { return pos_in_stack; }
    const std::vector<int>& getArrayDimensions() const { return array_dimensions; }

    // Setter方法
    void setStackPosition(int pos) { pos_in_stack = pos; }
    void setParam(bool param) { is_param = param; }
    void setSize(int s) { size = s; }
};

// 函数符号
class Function {
public:
    std::string name;
    PrimitiveType return_type;
    std::vector<std::shared_ptr<Symbol>> params;
    bool has_return;
    int stack_size;

    Function() : return_type(P_VOID), has_return(false), stack_size(0) {}
    
    Function(const std::string& name, PrimitiveType return_type, 
             const std::vector<std::shared_ptr<Symbol>>& params)
        : name(name), return_type(return_type), params(params), 
          has_return(false), stack_size(0) {}

    std::string toString() const {
        std::string result = name + "(): " + std::to_string(static_cast<int>(return_type));
        result += " params=[";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) result += ", ";
            result += params[i]->getName() + ":" + std::to_string(static_cast<int>(params[i]->getType()));
        }
        result += "]";
        if (has_return) result += " (has_return)";
        result += " stack_size=" + std::to_string(stack_size);
        return result;
    }

    // Getter方法
    const std::string& getName() const { return name; }
    PrimitiveType getReturnType() const { return return_type; }
    const std::vector<std::shared_ptr<Symbol>>& getParams() const { return params; }
    bool hasReturn() const { return has_return; }
    int getStackSize() const { return stack_size; }

    // Setter方法
    void setHasReturn(bool ret) { has_return = ret; }
    void setStackSize(int size) { stack_size = size; }
};
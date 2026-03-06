// 简化版本的Value类实现，展示如何使用新的工具类
// 这个文件演示了如何替换原有的重复代码

#include "../include/common/value.h"
#include "../include/common/type_utils.h"
#include <stdexcept>
#include <string>

// 原有的重复代码模式（来自defs.h）：
/*
Value Value::operator+(const Value& other) const {
    if (type == P_INT && other.type == P_INT) {
        return Value{P_INT, .ivalue = ivalue + other.ivalue};
    } else if (type == P_FLOAT && other.type == P_FLOAT) {
        return Value{P_FLOAT, .fvalue = fvalue + other.fvalue};
    } else if (type == P_INT && other.type == P_FLOAT) {
        return Value{P_FLOAT, .fvalue = ivalue + other.fvalue};
    } else if (type == P_FLOAT && other.type == P_INT) {
        return Value{P_FLOAT, .fvalue = fvalue + other.ivalue};
    }
    throw std::runtime_error("Value::operator+: Incompatible types for addition");
}

Value Value::operator-(const Value& other) const {
    if (type == P_INT && other.type == P_INT) {
        return Value{P_INT, .ivalue = ivalue - other.ivalue};
    } else if (type == P_FLOAT && other.type == P_FLOAT) {
        return Value{P_FLOAT, .fvalue = fvalue - other.fvalue};
    } else if (type == P_INT && other.type == P_FLOAT) {
        return Value{P_FLOAT, .fvalue = ivalue - other.fvalue};
    } else if (type == P_FLOAT && other.type == P_INT) {
        return Value{P_FLOAT, .fvalue = fvalue - other.ivalue};
    }
    throw std::runtime_error("Value::operator-: Incompatible types for subtraction");
}

// ... 同样模式的代码在 *, /, % 中重复出现
*/

// 新的简化实现（使用TypeUtils）：

// 模板化的二元运算实现
template<typename Op>
Value Value::performBinaryOp(const Value& other, Op op, const std::string& op_name) const {
    // 检查类型兼容性
    if (!TypeUtils::areCompatibleForBinaryOp(type, other.type)) {
        throw std::runtime_error("Incompatible types for " + op_name + ": " +
                               TypeUtils::typeToString(type) + " and " + 
                               TypeUtils::typeToString(other.type));
    }
    
    // 获取结果类型
    PrimitiveType result_type = TypeUtils::getBinaryOpResultType(type, other.type);
    
    // 执行运算
    switch (result_type) {
        case P_INT: {
            int result = op(getIntValue(), other.getIntValue());
            return Value{P_INT, .ivalue = result};
        }
        case P_FLOAT: {
            double result = op(getFloatValue(), other.getFloatValue());
            return Value{P_FLOAT, .fvalue = result};
        }
        case P_LONG: {
            long result = op(getLongValue(), other.getLongValue());
            return Value{P_LONG, .lvalue = result};
        }
        default:
            throw std::runtime_error(op_name + " not supported for result type: " + 
                                   TypeUtils::typeToString(result_type));
    }
}

// 简化的运算符实现
Value Value::operator+(const Value& other) const {
    return performBinaryOp(other, 
        [](auto a, auto b) { return a + b; }, 
        "addition");
}

Value Value::operator-(const Value& other) const {
    return performBinaryOp(other, 
        [](auto a, auto b) { return a - b; }, 
        "subtraction");
}

Value Value::operator*(const Value& other) const {
    return performBinaryOp(other, 
        [](auto a, auto b) { return a * b; }, 
        "multiplication");
}

Value Value::operator/(const Value& other) const {
    return performBinaryOp(other, 
        [](auto a, auto b) { 
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b; 
        }, 
        "division");
}

Value Value::operator%(const Value& other) const {
    return performBinaryOp(other, 
        [](auto a, auto b) { 
            if (b == 0) throw std::runtime_error("Division by zero");
            return static_cast<long>(a) % static_cast<long>(b); 
        }, 
        "modulo");
}

// 比较运算
bool Value::operator==(const Value& other) const {
    if (type != other.type) return false;
    switch (type) {
        case P_INT: return ivalue == other.ivalue;
        case P_FLOAT: return fvalue == other.fvalue;
        case P_LONG: return lvalue == other.lvalue;
        case P_STRING: return strvalue == other.strvalue;
        default: return false;
    }
}

// 一元运算
Value Value::operator-() const {
    switch (type) {
        case P_INT: return Value{P_INT, .ivalue = -ivalue};
        case P_FLOAT: return Value{P_FLOAT, .fvalue = -fvalue};
        case P_LONG: return Value{P_LONG, .lvalue = -lvalue};
        default: throw std::runtime_error("Unary minus not supported for " + TypeUtils::typeToString(type));
    }
}

// 类型转换辅助函数
template<typename T>
T Value::convertTo() const {
    switch (type) {
        case P_INT:
            if constexpr (std::is_same_v<T, int>) return ivalue;
            if constexpr (std::is_same_v<T, double>) return static_cast<double>(ivalue);
            if constexpr (std::is_same_v<T, long>) return static_cast<long>(ivalue);
            if constexpr (std::is_same_v<T, char>) return static_cast<char>(ivalue);
            break;
        case P_FLOAT:
            if constexpr (std::is_same_v<T, double>) return fvalue;
            if constexpr (std::is_same_v<T, int>) return static_cast<int>(fvalue);
            if constexpr (std::is_same_v<T, long>) return static_cast<long>(fvalue);
            if constexpr (std::is_same_v<T, char>) return static_cast<char>(fvalue);
            break;
        case P_LONG:
            if constexpr (std::is_same_v<T, long>) return lvalue;
            if constexpr (std::is_same_v<T, int>) return static_cast<int>(lvalue);
            if constexpr (std::is_same_v<T, double>) return static_cast<double>(lvalue);
            if constexpr (std::is_same_v<T, char>) return static_cast<char>(lvalue);
            break;
        default:
            throw std::runtime_error("Cannot convert " + TypeUtils::typeToString(type) + 
                                     " to target type");
    }
    throw std::runtime_error("Unsupported type conversion");
}

// 使用示例：展示如何简化原有的复杂逻辑
void demonstrateSimplification() {
    // 原有的复杂类型检查代码：
    /*
    if ((lhs_type == P_INT || lhs_type == P_INTARR) && 
        (rhs_type == P_INT || rhs_type == P_INTARR)) {
        // ... 复杂的类型处理逻辑
    }
    */
    
    // 新的简化版本：
    if (TypeUtils::areCompatibleForBinaryOp(lhs_type, rhs_type)) {
        PrimitiveType result_type = TypeUtils::getBinaryOpResultType(lhs_type, rhs_type);
        // 统一的处理逻辑
    }
}

// 展示错误处理的改进
void demonstrateErrorHandling() {
    try {
        Value int_val;
        int_val.setIntValue(10);
        
        Value float_val;  
        float_val.setFloatValue(3.14);
        
        // 这会正常工作，因为int和float兼容
        Value result = int_val + float_val;
        
    } catch (const std::exception& e) {
        // 统一的错误处理
        ErrorReporter::getInstance().reportTypeError(
            e.what(), current_line, current_column);
    }
}
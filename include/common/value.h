#pragma once

#include "types.h"
#include "type_utils.h"
#include <string>
#include <stdexcept>
#include <limits>

class Value {
public:
    PrimitiveType type;
    union {
        int ivalue;    // For integer literals
        long lvalue;   // For long integer literals
        double fvalue; // For floating-point literals
    };
    std::string strvalue; // For string literals

    Value() : type(P_NONE), ivalue(0) {}
    
    Value(PrimitiveType t) : type(t), ivalue(0) {}
    
    // 构造函数
    void setIntValue(int value) {
        ivalue = value;
        type = P_INT;
    }
    
    void setFloatValue(double value) {
        fvalue = value;
        type = P_FLOAT;
    }
    
    void setStringValue(const std::string& value) {
        strvalue = value;
        type = P_STRING;
    }
    
    void setLongValue(long value) {
        lvalue = value;
        type = P_LONG;
    }

    // 获取值的方法 - 带类型检查
    int getIntValue() const {
        if (type == P_INT) return ivalue;
        if (type == P_FLOAT) return static_cast<int>(fvalue);
        if (type == P_LONG) return static_cast<int>(lvalue);
        throw std::runtime_error("Cannot convert " + TypeUtils::typeToString(type) + " to int");
    }
    
    double getFloatValue() const {
        if (type == P_FLOAT) return fvalue;
        if (type == P_INT) return static_cast<double>(ivalue);
        if (type == P_LONG) return static_cast<double>(lvalue);
        throw std::runtime_error("Cannot convert " + TypeUtils::typeToString(type) + " to float");
    }
    
    long getLongValue() const {
        if (type == P_LONG) return lvalue;
        if (type == P_INT) return static_cast<long>(ivalue);
        if (type == P_FLOAT) return static_cast<long>(fvalue);
        throw std::runtime_error("Cannot convert " + TypeUtils::typeToString(type) + " to long");
    }
    
    char getCharValue() const {
        if (type == P_INT) return static_cast<char>(ivalue);
        if (type == P_FLOAT) return static_cast<char>(fvalue);
        if (type == P_LONG) return static_cast<char>(lvalue);
        throw std::runtime_error("Cannot convert " + TypeUtils::typeToString(type) + " to char");
    }
    
    const char* getStringValue() const {
        if (type == P_STRING) return strvalue.c_str();
        throw std::runtime_error("Value is not a string type");
    }

    // 统一的二元运算实现
    Value operator+(const Value& other) const {
        return performBinaryOp(other, [](auto a, auto b) { return a + b; }, "addition");
    }
    
    Value operator-(const Value& other) const {
        return performBinaryOp(other, [](auto a, auto b) { return a - b; }, "subtraction");
    }
    
    Value operator*(const Value& other) const {
        return performBinaryOp(other, [](auto a, auto b) { return a * b; }, "multiplication");
    }
    
    Value operator/(const Value& other) const {
        return performBinaryOp(other, [](auto a, auto b) { 
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b; 
        }, "division");
    }
    
    Value operator%(const Value& other) const {
        return performBinaryOp(other, [](auto a, auto b) { 
            if (b == 0) throw std::runtime_error("Division by zero");
            return static_cast<long>(a) % static_cast<long>(b); 
        }, "modulo");
    }

    // 比较运算
    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        switch (type) {
            case P_INT: return ivalue == other.ivalue;
            case P_FLOAT: return fvalue == other.fvalue;
            case P_LONG: return lvalue == other.lvalue;
            case P_STRING: return strvalue == other.strvalue;
            default: return false;
        }
    }
    
    bool operator!=(const Value& other) const {
        return !(*this == other);
    }

    // 一元运算
    Value operator-() const {
        switch (type) {
            case P_INT: return Value{P_INT, .ivalue = -ivalue};
            case P_FLOAT: return Value{P_FLOAT, .fvalue = -fvalue};
            case P_LONG: return Value{P_LONG, .lvalue = -lvalue};
            default: throw std::runtime_error("Unary minus not supported for " + TypeUtils::typeToString(type));
        }
    }

    // 工具方法
    std::string toString() const {
        switch (type) {
            case P_INT: return std::to_string(ivalue);
            case P_FLOAT: return std::to_string(fvalue);
            case P_LONG: return std::to_string(lvalue);
            case P_STRING: return strvalue;
            default: return "Unknown Value Type";
        }
    }

    PrimitiveType getType() const { return type; }
    
    bool isNumeric() const {
        return TypeUtils::isNumericType(type);
    }
    
    bool isInteger() const {
        return TypeUtils::isIntegerType(type);
    }
    
    bool isFloat() const {
        return TypeUtils::isFloatType(type);
    }

private:
    // 模板化的二元运算实现，消除重复代码
    template<typename Op>
    Value performBinaryOp(const Value& other, Op op, const std::string& op_name) const {
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
};
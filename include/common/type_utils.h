#pragma once

#include "types.h"
#include <string>
#include <stdexcept>

class TypeUtils {
public:
    // 获取类型大小
    static int getTypeSize(PrimitiveType type) {
        switch (type) {
            case P_INT: case P_INTARR: return 4;
            case P_FLOAT: case P_FLOATARR: return 8;
            case P_CHAR: case P_CHARARR: return 1;
            case P_LONG: case P_LONGARR: return 8;
            case P_VOIDPTR: case P_INTPTR: case P_FLOATPTR: case P_CHARPTR: case P_LONGPTR:
                return 8; // 64位架构上的指针大小
            case P_VOID: return 0;
            default: 
                throw std::runtime_error("Unknown primitive type: " + std::to_string(static_cast<int>(type)));
        }
    }

    // 检查类型是否兼容用于二元运算
    static bool areCompatibleForBinaryOp(PrimitiveType lhs, PrimitiveType rhs) {
        // 相同类型总是兼容的
        if (lhs == rhs) return true;
        
        // 数值类型之间的兼容性
        if (isNumericType(lhs) && isNumericType(rhs)) {
            return true;
        }
        
        // 指针类型需要更严格的检查
        if (is_pointer(lhs) || is_pointer(rhs)) {
            return lhs == rhs; // 指针类型必须完全相同
        }
        
        return false;
    }

    // 获取二元运算的结果类型
    static PrimitiveType getBinaryOpResultType(PrimitiveType lhs, PrimitiveType rhs) {
        if (!areCompatibleForBinaryOp(lhs, rhs)) {
            throw std::runtime_error("Incompatible types for binary operation");
        }
        
        // 如果类型相同，返回该类型
        if (lhs == rhs) return lhs;
        
        // 数值类型提升规则
        if (lhs == P_FLOAT || rhs == P_FLOAT) return P_FLOAT;
        if (lhs == P_LONG || rhs == P_LONG) return P_LONG;
        
        return P_INT; // 默认返回int
    }

    // 检查是否为数值类型
    static bool isNumericType(PrimitiveType type) {
        return type == P_INT || type == P_FLOAT || type == P_LONG;
    }

    // 检查是否为整数类型
    static bool isIntegerType(PrimitiveType type) {
        return type == P_INT || type == P_LONG || type == P_CHAR;
    }

    // 检查是否为浮点类型
    static bool isFloatType(PrimitiveType type) {
        return type == P_FLOAT;
    }

    // 类型转换为字符串（用于调试和错误信息）
    static std::string typeToString(PrimitiveType type) {
        switch (type) {
            case P_INT: return "int";
            case P_FLOAT: return "float";
            case P_CHAR: return "char";
            case P_LONG: return "long";
            case P_VOID: return "void";
            case P_STRING: return "string";
            case P_INTPTR: return "int*";
            case P_FLOATPTR: return "float*";
            case P_CHARPTR: return "char*";
            case P_LONGPTR: return "long*";
            case P_VOIDPTR: return "void*";
            case P_INTARR: return "int[]";
            case P_FLOATARR: return "float[]";
            case P_CHARARR: return "char[]";
            case P_LONGARR: return "long[]";
            case P_NONE: return "none";
            default: return "unknown_type_" + std::to_string(static_cast<int>(type));
        }
    }

    // 检查类型是否可以隐式转换
    static bool canImplicitlyConvert(PrimitiveType from, PrimitiveType to) {
        // 相同类型总是可以
        if (from == to) return true;
        
        // 数值类型之间的转换
        if (isNumericType(from) && isNumericType(to)) {
            // int -> long -> float 是允许的
            if (from == P_INT && (to == P_LONG || to == P_FLOAT)) return true;
            if (from == P_LONG && to == P_FLOAT) return true;
            return false;
        }
        
        // 指针类型转换（只允许相同类型的指针）
        if (is_pointer(from) && is_pointer(to)) {
            return from == to;
        }
        
        return false;
    }

    // 获取类型对齐要求
    static int getTypeAlignment(PrimitiveType type) {
        int size = getTypeSize(type);
        // 简单的对齐规则：取类型大小和平台对齐要求的最小值
        return size < 8 ? size : 8; // 64位平台上最大对齐为8字节
    }

    // 计算数组的总大小
    static int getArraySize(PrimitiveType element_type, const std::vector<int>& dimensions) {
        if (dimensions.empty()) {
            throw std::runtime_error("Array dimensions cannot be empty");
        }
        
        int element_size = getTypeSize(element_type);
        int total_size = element_size;
        
        for (int dim : dimensions) {
            if (dim <= 0) {
                throw std::runtime_error("Array dimension must be positive");
            }
            total_size *= dim;
        }
        
        return total_size;
    }

    // 检查两个类型是否等价（考虑const等修饰符，这里简化处理）
    static bool areEquivalent(PrimitiveType lhs, PrimitiveType rhs) {
        return lhs == rhs;
    }

    // 获取类型的
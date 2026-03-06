#pragma once

namespace Constants {
    // 架构相关常量
    constexpr int POINTER_SIZE = 8;  // 64位架构
    constexpr int STACK_ALIGNMENT = 16;
    constexpr int STACK_PARAM_OFFSET = 16;
    
    // 寄存器参数限制
    constexpr int MAX_INT_REG_PARAMS = 6;
    constexpr int MAX_FLOAT_REG_PARAMS = 8;
    
    // 基本类型大小
    constexpr int INT_SIZE = 4;
    constexpr int FLOAT_SIZE = 8;
    constexpr int CHAR_SIZE = 1;
    constexpr int LONG_SIZE = 8;
    
    // 标识符限制
    constexpr int MAX_IDENTIFIER_LENGTH = 1024;
    
    // 数组维度限制
    constexpr int MAX_ARRAY_DIMENSIONS = 8;
}

namespace LabelType {
    enum Type {
        IF_LABEL,
        BLOCK_LABEL,
        WHILE_LABEL,
        FOR_LABEL,
        FUNCT_LABEL,
        FLOAT_CONSTANT_LABEL,
        STRING_CONSTANT_LABEL
    };
}
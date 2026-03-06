#pragma once

#include <string>
#include <vector>
#include <map>

enum PrimitiveType {
    P_NONE,
    P_INT, P_FLOAT, P_VOID, P_STRING, P_CHAR, P_LONG,
    P_INTPTR, P_FLOATPTR, P_CHARPTR, P_LONGPTR, P_VOIDPTR,
    P_INTARR, P_FLOATARR, P_CHARARR, P_LONGARR
};

enum ExprType {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_XOR, A_LSHIFT, A_RSHIFT,
    A_EQ, A_NE, A_LT, A_LE, A_GT, A_GE, A_AND, A_OR, A_ASSIGN, A_MOD
};

enum UnaryOp {
    U_PLUS, U_MINUS, U_NOT, U_INVERT, U_PREINC, U_PREDEC, U_POSTINC, U_POSTDEC,
    U_ADDR, U_DEREF, U_TRANSFORM, U_SCALE
};

enum StmtType {
    S_PRINT, S_ASSIGN, S_IF, S_WHILE, S_RETURN, S_BLOCK, S_EXPR, S_VARDEF, S_FOR, S_FUNCTDEF,
    S_BREAK, S_CONTINUE
};

// 类型工具函数
inline bool is_pointer(PrimitiveType type) {
    return type == P_CHARPTR || type == P_FLOATPTR || type == P_INTPTR || type == P_LONGPTR || type == P_VOIDPTR;
}

inline bool is_array(PrimitiveType type) {
    return type == P_INTARR || type == P_FLOATARR || type == P_CHARARR || type == P_LONGARR;
}

inline PrimitiveType pointTo(const PrimitiveType &type) {
    if (type == P_INT || type == P_INTARR) {
        return P_INTPTR;
    } else if (type == P_FLOAT || type == P_FLOATARR) {
        return P_FLOATPTR;
    } else if (type == P_CHAR || type == P_CHARARR) {
        return P_CHARPTR;
    } else if (type == P_LONG || type == P_LONGARR) {
        return P_LONGPTR;
    } else if (type == P_VOID) {
        return P_VOIDPTR;
    }
    throw std::runtime_error("Unknown type for pointer declaration");
}

inline PrimitiveType valueAt(const PrimitiveType &type) {
    switch (type) {
        case P_INTPTR: case P_INTARR: return P_INT;
        case P_FLOATPTR: case P_FLOATARR : return P_FLOAT;
        case P_CHARPTR: case P_CHARARR: return P_CHAR;
        case P_LONGPTR: case P_LONGARR : return P_LONG;
        case P_VOIDPTR: return P_VOID;
        default: throw std::runtime_error("Unknown type for value at pointer declaration");
    }
}

inline PrimitiveType arrayTo(PrimitiveType type) {
    switch (type) {
        case P_INT: return P_INTARR;
        case P_FLOAT: return P_FLOATARR;
        case P_CHAR: return P_CHARARR;
        case P_LONG: return P_LONGARR;
        default: throw std::runtime_error("Not an array type");
    }
}
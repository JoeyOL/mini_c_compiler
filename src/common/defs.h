#include <map>
#pragma once

enum {
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT
};

struct Token {
    int type;
    int value; // For T_INTLIT
    int line_no; // Line number in the source file
    int column_no; // Column number in the source file
};

enum ExprType {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_INTLIT
};

enum UnaryType {
    U_INT
};

class ASTNode {
    public:
        ASTNode() = default;
        ~ASTNode() = default;
        virtual void walk() = 0; // Pure virtual function for walking the AST
        virtual int getValue() const { return value; } // Default implementation for ASTNode
    protected:
        int value; // For A_INTLIT
};

static const std::map<ExprType, int> precedence = {
    {A_ADD, 1},
    {A_SUBTRACT, 1},
    {A_MULTIPLY, 2},
    {A_DIVIDE, 2},
    {A_INTLIT, 0} // Integer literals have the lowest precedence
};
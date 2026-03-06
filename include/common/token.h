#pragma once

#include <string>

enum TokenType {
    T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH,  T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE, T_LBRACKET, T_RBRACKET, T_MOD,
    T_IDENTIFIER, T_PRINT, T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
    T_SEMI, T_NUMBER, T_INT, T_ASSIGN, T_COMMA, T_VOID, T_CHAR, T_FLOAT, T_LONG,
    T_LT, T_GT, T_LE, T_GE, T_NE, T_EQ, T_NOT, T_LOGAND, T_LOGOR, T_AMPER, T_OR, T_INVERT, T_INC, T_DEC, T_XOR, T_LSHIFT, T_RSHIFT,
    T_STRING, T_BREAK, T_CONTINUE
};

struct Token {
    TokenType type;
    Value value; // Value of the token, if applicable
    int line_no; // Line number in the source file
    int column_no; // Column number in the source file
};
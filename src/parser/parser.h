#include "common/defs.h"
#include "parser/BinaryExpNode.h"
#include "parser/UnaryExpNode.h"
#include "scanner/scanner.h"
#include <memory>
#include <vector>
#pragma once

class Parser {
public:
    Parser(std::vector<Token> &toks) : toks(toks) {}
    std::shared_ptr<ASTNode> parseBinaryExpression();

    // 上述方法并不能正确解析优先级，下面提供两种可以正确解析的方法
    // 1. 优先级
    /*
    expression: number
        | expression '*' expression
        | expression '/' expression
        | expression '+' expression
        | expression '-' expression
        ;

    number:  T_INTLIT
            ;
    */
    std::shared_ptr<ASTNode> parseBinaryExpressionWithPrecedence(int prev_precedence);
    // 2. 使用加法和乘法的两个BNF
    /*
        expression: additive_expression
        ;

    additive_expression:
        multiplicative_expression
        | additive_expression '+' multiplicative_expression
        | additive_expression '-' multiplicative_expression
        ;

    multiplicative_expression:
        number
        | number '*' multiplicative_expression
        | number '/' multiplicative_expression
        ;

    number:  T_INTLIT
            ;
     */
    std::shared_ptr<ASTNode> parseAdditiveExpression();
    std::shared_ptr<ASTNode> parseMultiplicativeExpression();
private:
    std::vector<Token> &toks;
    size_t current = 0;
    std::shared_ptr<ASTNode> parimary();
    ExprType arithop(const Token &tok);
    // Additional private methods for parsing would go here.
    // For example, methods to parse terms, factors, etc.
};
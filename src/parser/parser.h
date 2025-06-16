#include "common/defs.h"
#include "parser/ExprNode.h"
#include "parser/StatementNode.h"
#include "scanner/scanner.h"
#include <memory>
#include <vector>
#include <iostream>
#include <assert.h>
#pragma once

class Parser {
public:
    Parser(std::vector<Token> &toks) : toks(toks) {}
    std::shared_ptr<ExprNode> parseBinaryExpression();

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

    std::shared_ptr<ExprNode> parseExpressionWithPrecedence(int prev_precedence);
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
    std::shared_ptr<ExprNode> parseAdditiveExpression();
    std::shared_ptr<ExprNode> parseMultiplicativeExpression();

    // print 语句的解析
    /* block: : '{' '}'          // empty, i.e. no statement
      |      '{' statement '}'
      |      '{' statement statements '}'
      ;

    statement: 'print' expression ';'
     ;
     */
    std::shared_ptr<BlockNode> parseBlock();
    std::shared_ptr<PrintStatementNode> parsePrintStatement();

    // 有关变量
    /*
    statements: statement
        |      statement statements
        ;

    statement: 'print' expression ';'
        |     'int'   identifier ';'
        |     identifier '=' expression ';'
        ;

    identifier: T_IDENT
        ;
    */

    std::shared_ptr<VariableDeclareNode> parseVariableDeclare();
    std::shared_ptr<AssignmentNode> parseAssignment();

    // TODO 目前必须要有{}
    // if_else 
    /*
    if_statement: if_head
    |        if_head 'else' block
    ;

    if_head: 'if' '(' expression ')' block  ;
    */
   std::shared_ptr<IfStatementNode> parseIfStatement();

   // while: while_statement: 'while' '(' true_false_expression ')' compound_statement  ;
   std::shared_ptr<WhileStatementNode> parseWhileStatement();

   // for: 
   /*
    for_statement: 'for' '(' preop_statement ';'
                          true_false_expression ';'
                          postop_statement ')' compound_statement  ;

    preop_statement:  statement  ;        (for now)
    postop_statement: statement  ;        (for now)
 */
    std::shared_ptr<StatementNode> parseSingleStatement();
    std::shared_ptr<ForStatementNode> parseForStatement();

private:
    std::vector<Token> &toks;
    size_t current = 0;
    std::shared_ptr<ExprNode> parimary();
    ExprType arithop(const Token &tok);
    Token consume() {
        return toks[current++];
    }
    Token peek() const {
        if (current < toks.size()) {
            return toks[current];
        }
        return Token{T_EOF, Value{}, -1, -1}; // Return EOF token if out of bounds
    }
    // Additional private methods for parsing would go here.
    // For example, methods to parse terms, factors, etc.
};
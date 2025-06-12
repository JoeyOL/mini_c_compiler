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
    std::unique_ptr<ASTNode> parseBinaryExpression();
private:
    std::vector<Token> &toks;
    size_t current = 0;
    std::unique_ptr<ASTNode> parimary();
    ExprType arithop(const Token &tok);
    // Additional private methods for parsing would go here.
    // For example, methods to parse terms, factors, etc.
};
#include "parser/parser.h"


std::shared_ptr<ExprNode> Parser::parseBinaryExpression() {
    std::shared_ptr<ExprNode> left = parimary();
    if (current >= toks.size()) {
        return left; // If no token is available, return the left node
    }
    ExprType type = arithop(toks[current++]);
    std::shared_ptr<ExprNode> right = parseBinaryExpression();
    return std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
}

std::shared_ptr<ExprNode> Parser::parimary() {
    const Token &tok = toks[current++];
    if (tok.type == T_PLUS || tok.type == T_MINUS) {
        return std::make_shared<UnaryExpNode>(tok.type == T_PLUS ? U_PLUS : U_MINUS, parimary());
    } else if (tok.type == T_LPAREN) {
        auto ret = parseBinaryExpressionWithPrecedence(0);
        if (current >= toks.size() || toks[current].type != T_RPAREN) {
            throw std::runtime_error("Parser::parimary: Expected ')' at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
        }
        current++; // Skip the closing parenthesis
        return ret;
    } else if (tok.type == T_NUMBER) {
        return std::make_shared<ValueNode>(tok.value);
    } else {
        throw std::runtime_error("Parser::parimary: Unexpected token type" + 
            std::to_string(tok.type) + " at line " + 
            std::to_string(tok.line_no) + ", column " + 
            std::to_string(tok.column_no));
    }
}

ExprType Parser::arithop(const Token &tok) {
    switch (tok.type) {
        case T_PLUS: return A_ADD;
        case T_MINUS: return A_SUBTRACT;
        case T_STAR: return A_MULTIPLY;
        case T_SLASH: return A_DIVIDE;
        default:
            throw std::runtime_error("Parser::arithop: Unexpected token type " + 
                std::to_string(tok.type) + " at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
    }
}

std::shared_ptr<ExprNode> Parser::parseBinaryExpressionWithPrecedence(int prev_precedence) {
    std::shared_ptr<ExprNode> left = parimary();
    if (current >= toks.size() || toks[current].type == T_RPAREN || toks[current].type == T_SEMI) {
        return left; // If no token is available, return the left node
    }
    while (precedence.at(arithop(toks[current])) > prev_precedence) {
        ExprType type = arithop(toks[current++]);
        std::shared_ptr<ExprNode> right = parseBinaryExpressionWithPrecedence(precedence.at(type));
        left = std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
        if (current >= toks.size() || toks[current].type == T_RPAREN || toks[current].type == T_SEMI) {
            return left; // If no token is available, return the left node
        }
    }
    return left;
}

std::shared_ptr<ExprNode> Parser::parseAdditiveExpression() {
    std::shared_ptr<ExprNode> left = parseMultiplicativeExpression();
    while (current < toks.size()) {
        if (toks[current].type != T_PLUS && toks[current].type != T_MINUS) {
            throw std::runtime_error("Parser::parseAdditiveExpression: Expected '+' or '-' operator at line " + 
                std::to_string(toks[current].line_no) + ", column " + 
                std::to_string(toks[current].column_no));
        }
        ExprType type = arithop(toks[current++]);
        std::shared_ptr<ExprNode> right = parseMultiplicativeExpression();
        left = std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
    }
    
    return left;
}

std::shared_ptr<ExprNode> Parser::parseMultiplicativeExpression() {
    std::shared_ptr<ExprNode> left = parimary();
    while (current < toks.size() && (toks[current].type == T_STAR || toks[current].type == T_SLASH)) {
        ExprType type = arithop(toks[current++]);
        std::shared_ptr<ExprNode> right = parimary();
        left = std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
    }
    
    return left;
}

std::shared_ptr<PrintStatementNode> Parser::parsePrintStatement() { 
    assert(toks[current++].type == T_PRINT);
    std::shared_ptr<ExprNode> expr = parseBinaryExpressionWithPrecedence(0);
    assert(toks[current++].type == T_SEMI);
    return std::make_shared<PrintStatementNode>(std::move(expr));
}

std::shared_ptr<StatementsNode> Parser::parseStatements() { 
    std::shared_ptr<StatementsNode> stmts = std::make_shared<StatementsNode>();
    while (current < toks.size()) {
        std::shared_ptr<StatementNode> stmt;
        if (toks[current].type == T_PRINT) {
            std::shared_ptr<StatementNode> stmt = parsePrintStatement();
            stmts->addStatement(stmt);
        } else {
            throw std::runtime_error("Parser::parseStatement: Expected statement at line " + 
                std::to_string(toks[current].line_no) + ", column " + 
                std::to_string(toks[current].column_no));
        }
    }
    return stmts;
}
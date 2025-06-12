#include "parser/parser.h"


std::unique_ptr<ASTNode> Parser::parseBinaryExpression() {
    std::unique_ptr<ASTNode> left = parimary();
    if (current >= toks.size()) {
        return left; // If no token is available, return the left node
    }
    ExprType type = arithop(toks[current++]);
    std::unique_ptr<ASTNode> right = parseBinaryExpression();
    return std::make_unique<BinaryExpNode>(type, std::move(left), std::move(right));
}

std::unique_ptr<ASTNode> Parser::parimary() {
    const Token &tok = toks[current++];
    if (tok.type == T_INTLIT) {
        return std::make_unique<UnaryExpNode>(UnaryType::U_INT, tok.value); // Assuming ASTNode can be constructed with an integer value
    }
    else {
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


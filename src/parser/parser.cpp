#include "parser/parser.h"


std::shared_ptr<ExprNode> Parser::parseBinaryExpression() {
    std::shared_ptr<ExprNode> left = parimary();
    if (current >= toks.size()) {
        return left; // If no token is available, return the left node
    }
    ExprType type = arithop(consume());
    std::shared_ptr<ExprNode> right = parseBinaryExpression();
    return std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
}

std::shared_ptr<ExprNode> Parser::parimary() {
    const Token &tok = consume();
    if (tok.type == T_PLUS || tok.type == T_MINUS || tok.type == T_NOT) {
        return std::make_shared<UnaryExpNode>(tok.type, parimary());
    } else if (tok.type == T_LPAREN) {
        auto ret = parseBinaryExpressionWithPrecedence(0);
        if (current >= toks.size() || peek().type != T_RPAREN) {
            throw std::runtime_error("Parser::parimary: Expected ')' at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
        }
        consume();
        return ret;
    } else if (tok.type == T_NUMBER) {
        return std::make_shared<ValueNode>(tok.value);
    } else if (tok.type == T_IDENTIFIER) {
        Symbol sym = symbol_table.getSymbol(tok.value.strvalue);
        return std::make_shared<LValueNode>(sym);
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
        case T_EQ: return A_EQ;
        case T_NE: return A_NE;
        case T_LT: return A_LT;
        case T_LE: return A_LE;
        case T_GT: return A_GT;
        case T_GE: return A_GE;
        default:
            throw std::runtime_error("Parser::arithop: Unexpected token type " + 
                std::to_string(tok.type) + " at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
    }
}

std::shared_ptr<ExprNode> Parser::parseBinaryExpressionWithPrecedence(int prev_precedence) {
    std::shared_ptr<ExprNode> left = parimary();
    if (current >= toks.size() || peek().type == T_RPAREN || peek().type == T_SEMI) {
        return left; // If no token is available, return the left node
    }
    while (precedence.at(arithop(peek())) > prev_precedence) {
        ExprType type = arithop(consume());
        std::shared_ptr<ExprNode> right = parseBinaryExpressionWithPrecedence(precedence.at(type));
        left = std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
        if (current >= toks.size() || peek().type == T_RPAREN || peek().type == T_SEMI) {
            return left; // If no token is available, return the left node
        }
    }
    return left;
}

std::shared_ptr<ExprNode> Parser::parseAdditiveExpression() {
    std::shared_ptr<ExprNode> left = parseMultiplicativeExpression();
    while (current < toks.size()) {
        if (peek().type != T_PLUS && peek().type != T_MINUS) {
            throw std::runtime_error("Parser::parseAdditiveExpression: Expected '+' or '-' operator at line " + 
                std::to_string(peek().line_no) + ", column " + 
                std::to_string(peek().column_no));
        }
        ExprType type = arithop(consume());
        std::shared_ptr<ExprNode> right = parseMultiplicativeExpression();
        left = std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
    }
    
    return left;
}

std::shared_ptr<ExprNode> Parser::parseMultiplicativeExpression() {
    std::shared_ptr<ExprNode> left = parimary();
    while (current < toks.size() && (peek().type == T_STAR || peek().type == T_SLASH)) {
        ExprType type = arithop(consume());
        std::shared_ptr<ExprNode> right = parimary();
        left = std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
    }
    
    return left;
}

std::shared_ptr<PrintStatementNode> Parser::parsePrintStatement() { 
    assert(consume().type == T_PRINT);
    std::shared_ptr<ExprNode> expr = parseBinaryExpressionWithPrecedence(0);
    assert(consume().type == T_SEMI);
    return std::make_shared<PrintStatementNode>(std::move(expr));
}

std::shared_ptr<BlockNode> Parser::parseBlock() { 
    std::shared_ptr<BlockNode> stmts = std::make_shared<BlockNode>();
    assert(consume().type == T_LBRACE);
    while (current < toks.size() && peek().type != T_RBRACE) {
        if (peek().type == T_PRINT) {
            std::shared_ptr<StatementNode> stmt = parsePrintStatement();
            stmts->addStatement(stmt);
        } else if (peek().type == T_INT) {
            std::shared_ptr<VariableDeclareNode> var_decl = parseVariableDeclare();
            stmts->addStatement(var_decl);
        } else if (peek().type == T_IDENTIFIER) {
            std::shared_ptr<AssignmentNode> assign_stmt = parseAssignment();
            stmts->addStatement(assign_stmt);
        } else if (peek().type == T_LBRACE) {
            std::shared_ptr<BlockNode> block = parseBlock();
            stmts->addStatement(block);
        } else if (peek().type == T_IF) { 
            std::shared_ptr<IfStatementNode> if_stmt = parseIfStatement();
            stmts->addStatement(if_stmt);
        } else {
            throw std::runtime_error("Parser::parseStatement: Expected statement at line " + 
                std::to_string(peek().line_no) + ", column " + 
                std::to_string(peek().column_no));
        }
    }
    assert(consume().type == T_RBRACE);
    return stmts;
}

std::shared_ptr<VariableDeclareNode> Parser::parseVariableDeclare() {
    auto var_decl = std::make_shared<VariableDeclareNode>(PrimitiveType::P_INT);
    assert(consume().type == T_INT);
    do {
        if (peek().type == T_COMMA) {
            consume();
        }
        if (current >= toks.size() || peek().type != T_IDENTIFIER) {
            throw std::runtime_error("Parser::parseVariableDeclare: Expected identifier at line " + 
                std::to_string(peek().line_no) + ", column " + 
                std::to_string(peek().column_no));
        }
        std::string var_name = consume().value.strvalue;
        if (peek().type == T_ASSIGN) {
            consume();
            std::shared_ptr<ExprNode> initializer = parseBinaryExpressionWithPrecedence(0);
            var_decl->addIdentifier(var_name, std::move(initializer));
        } else {
            var_decl->addIdentifier(var_name);
        }
        symbol_table.addSymbol(var_name, P_INT); // Assuming all variables are of type int for simplicity
    } while (current < toks.size() && peek().type == T_COMMA);
    assert(consume().type == T_SEMI);
    return var_decl;
}

std::shared_ptr<AssignmentNode> Parser::parseAssignment() { 
    assert(peek().type == T_IDENTIFIER);
    std::string identifier = consume().value.strvalue;
    if (current >= toks.size() || peek().type != T_ASSIGN) {
        throw std::runtime_error("Parser::parseAssignment: Expected '=' after identifier at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    Symbol sym = symbol_table.getSymbol(identifier); // Check if the identifier exists in the symbol table
    assert(consume().type == T_ASSIGN); // Skip the '=' token
    std::shared_ptr<ExprNode> expr = parseBinaryExpressionWithPrecedence(0);
    assert(consume().type == T_SEMI);
    return std::shared_ptr<AssignmentNode>(new AssignmentNode(sym, std::move(expr)));
}

std::shared_ptr<IfStatementNode> Parser::parseIfStatement() {
    assert(consume().type == T_IF);
    assert(consume().type == T_LPAREN);
    // 注意到condition可能是一算术表达式，而不是比较
    std::shared_ptr<ExprNode> condition = parseBinaryExpressionWithPrecedence(0);
    assert(consume().type == T_RPAREN);
    std::shared_ptr<BlockNode> then_stmt = parseBlock();
    if (peek().type == T_ELSE) {
        consume();
        std::shared_ptr<BlockNode> else_stmt = parseBlock();
        return std::make_shared<IfStatementNode>(condition, then_stmt, else_stmt);
    } else {
        return std::make_shared<IfStatementNode>(condition, then_stmt);
    }
}
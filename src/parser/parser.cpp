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
        auto ret = parseExpressionWithPrecedence(0);
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
        if (peek().type == T_ASSIGN) {
            putback(); // Put back the identifier token
            return parseAssignment();
        }
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
        case T_AND: return A_AND;
        case T_OR: return A_OR;
        default:
            throw std::runtime_error("Parser::arithop: Unexpected token type " + 
                std::to_string(tok.type) + " at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
    }
}

std::shared_ptr<ExprNode> Parser::parseExpressionWithPrecedence(int prev_precedence) {
    std::shared_ptr<ExprNode> left = parimary();
    if (current >= toks.size() || peek().type == T_RPAREN || peek().type == T_SEMI || peek().type == T_COMMA) {
        return left; // If no token is available, return the left node
    }
    while (precedence.at(arithop(peek())) > prev_precedence) {
        ExprType type = arithop(consume());
        std::shared_ptr<ExprNode> right = parseExpressionWithPrecedence(precedence.at(type));
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
    std::shared_ptr<ExprNode> expr = parseExpressionWithPrecedence(0);
    return std::make_shared<PrintStatementNode>(std::move(expr));
}

std::shared_ptr<BlockNode> Parser::parseBlock() { 
    std::shared_ptr<BlockNode> stmts = std::make_shared<BlockNode>();
    assert(consume().type == T_LBRACE);
    while (current < toks.size() && peek().type != T_RBRACE) {
        if (peek().type == T_PRINT) {
            std::shared_ptr<StatementNode> stmt = parsePrintStatement();
            assert(consume().type == T_SEMI);
            stmts->addStatement(stmt);
        } else if (peek().type == T_INT || peek().type == T_CHAR 
                || peek().type == T_FLOAT || peek().type == T_LONG) {
            std::shared_ptr<VariableDeclareNode> var_decl = parseVariableDeclare();
            assert(consume().type == T_SEMI);
            stmts->addStatement(var_decl);
        } else if (peek().type == T_IDENTIFIER) {
            std::shared_ptr<AssignmentNode> assign_stmt = parseAssignment();
            assert(consume().type == T_SEMI);
            stmts->addStatement(assign_stmt);
        } else if (peek().type == T_LBRACE) {
            std::shared_ptr<BlockNode> block = parseBlock();
            stmts->addStatement(block);
        } else if (peek().type == T_IF) { 
            std::shared_ptr<IfStatementNode> if_stmt = parseIfStatement();
            stmts->addStatement(if_stmt);
        } else if (peek().type == T_WHILE) {
            std::shared_ptr<WhileStatementNode> while_stmt = parseWhileStatement();
            stmts->addStatement(while_stmt);
        } else if (peek().type == T_SEMI) {
            consume(); // Skip empty statement
        } else if (peek().type == T_FOR) {
            std::shared_ptr<ForStatementNode> for_stmt = parseForStatement();
            stmts->addStatement(for_stmt);
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
    assert(peek().type == T_INT || peek().type == T_CHAR || peek().type == T_FLOAT || peek().type == T_LONG);
    auto var_decl = std::make_shared<VariableDeclareNode>(consume().type);
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
            std::shared_ptr<ExprNode> initializer = parseExpressionWithPrecedence(0);
            var_decl->addIdentifier(var_name, std::move(initializer));
        } else {
            var_decl->addIdentifier(var_name);
        }
        symbol_table.addSymbol(var_name, var_decl->getVariableType()); // Assuming all variables are of type int for simplicity
    } while (current < toks.size() && peek().type == T_COMMA);
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
    std::shared_ptr<ExprNode> expr = parseExpressionWithPrecedence(0);
    return std::shared_ptr<AssignmentNode>(new AssignmentNode(sym, std::move(expr)));
}

std::shared_ptr<IfStatementNode> Parser::parseIfStatement() {
    assert(consume().type == T_IF);
    assert(consume().type == T_LPAREN);
    // 注意到condition可能是一算术表达式，而不是比较
    std::shared_ptr<ExprNode> condition = parseExpressionWithPrecedence(0);
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

std::shared_ptr<WhileStatementNode> Parser::parseWhileStatement() {
    assert(consume().type == T_WHILE);
    assert(consume().type == T_LPAREN);
    std::shared_ptr<ExprNode> condition = parseExpressionWithPrecedence(0);
    assert(consume().type == T_RPAREN);
    std::shared_ptr<BlockNode> body = parseBlock();
    return std::make_shared<WhileStatementNode>(std::move(condition), std::move(body));
}

std::shared_ptr<StatementNode> Parser::parseSingleStatement() {
    if (peek().type == T_PRINT) {
        return parsePrintStatement();
    } else if (peek().type == T_INT) {
        return parseVariableDeclare();
    } else if (peek().type == T_IDENTIFIER) {
        return parseAssignment();
    } else if (peek().type == T_IF) {
        return parseIfStatement();
    } else if (peek().type == T_WHILE) {
        return parseWhileStatement();
    } else {
        return nullptr;
    }
}


std::shared_ptr<ForStatementNode> Parser::parseForStatement() {
    assert(consume().type == T_FOR);
    assert(consume().type == T_LPAREN);
    std::shared_ptr<StatementNode> preop_stmt = parseSingleStatement();
    assert(consume().type == T_SEMI);
    std::shared_ptr<ExprNode> condition = parseExpressionWithPrecedence(0);
    assert(consume().type == T_SEMI);
    std::shared_ptr<StatementNode> postop_stmt = parseSingleStatement();
    assert(consume().type == T_RPAREN);
    std::shared_ptr<BlockNode> body = parseBlock();
    return std::make_shared<ForStatementNode>(std::move(preop_stmt), std::move(condition), std::move(body), std::move(postop_stmt));
}


// TODO: 目前只支持void类型的无参数函数
std::shared_ptr<FunctionDeclareNode> Parser::parseFunctionDeclare() {
    assert(consume().type == T_VOID);
    if (current >= toks.size() || peek().type != T_IDENTIFIER) {
        throw std::runtime_error("Parser::parseFunctionDeclare: Expected function name at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    std::string func_name = consume().value.strvalue;
    assert(consume().type == T_LPAREN);
    assert(consume().type == T_RPAREN);
    std::shared_ptr<BlockNode> body = parseBlock();
    return std::make_shared<FunctionDeclareNode>(func_name, P_VOID, std::move(body));
}
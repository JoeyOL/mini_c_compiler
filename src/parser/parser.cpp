#include "parser/parser.h"

// 假设只能声明全局变量，不能在函数体外更改全局变量值
std::shared_ptr<Pragram> Parser::parsePragram() {
    auto ret = std::make_shared<Pragram>();
    while (!token_end()) {
        assert(peek().type == T_INT || peek().type == T_FLOAT || 
               peek().type == T_CHAR || peek().type == T_LONG || 
               peek().type == T_VOID);
        if (peek().type == T_VOID) {
            auto func = parseFunctionDeclare();
            ret->addFunction(func);
        } else if (peek(2).type != T_LPAREN) {
            auto var = parseVariableDeclare();
            assert(consume().type == T_SEMI); // Expect a semicolon after variable declaration
            ret->addGlobalVariable(var);
        } else {
            auto func = parseFunctionDeclare();
            ret->addFunction(func);
        }
    }
    return ret;
}

std::shared_ptr<ExprNode> Parser::parseBinaryExpression() {
    std::shared_ptr<ExprNode> left = parimary();
    if (current >= toks.size()) {
        return left; // If no token is available, return the left node
    }
    ExprType type = arithop(consume());
    std::shared_ptr<ExprNode> right = parseBinaryExpression();
    return std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
}


std::shared_ptr<ExprNode> Parser::prefixExpr() {
    const Token &tok = peek();
    if (tok.type == T_AMPER)  {
        consume(); // Consume the '&' token
        int amper_count = 1;
        while (peek().type == T_AMPER) {
            amper_count++;
            consume(); // Consume all consecutive '&' tokens
        }

        auto primary_node = parimary();
        if (auto x = std::dynamic_pointer_cast<LValueNode>(primary_node)) {
            // 获取指针类型
            PrimitiveType type = x->getPrimitiveType();
            while (amper_count-- > 0) {
                type = pointTo(type);
            }
            std::shared_ptr<UnaryExpNode> unary_node = std::make_shared<UnaryExpNode>(U_ADDR, primary_node, type);
            return unary_node;
        } else {
            throw std::runtime_error("Parser::prefixExpr: Expected lvalue after '&' at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
        }

    } else if (tok.type == T_STAR) {
        consume();
        int star_count = 1;
        while (peek().type == T_STAR) {
            star_count++;
            consume(); // Consume all consecutive '*' tokens
        }
        // 解引用符号 '*'后面必须接一个变量
        auto primary_node = parimary();
        if (auto x = std::dynamic_pointer_cast<LValueNode>(primary_node)) {
            // 获取指针类型
            PrimitiveType type = x->getPrimitiveType();
            while (star_count-- > 0) {
                type = valueAt(type);
            }
            std::shared_ptr<UnaryExpNode> unary_node = std::make_shared<UnaryExpNode>(U_DEREF, primary_node, type);
            return unary_node;
        } else {
            throw std::runtime_error("Parser::prefixExpr: Expected lvalue after '&' at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
        }

    } else {
        return parimary(); // If no prefix operator, just parse the primary expression
    }
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
        } else if (peek().type == T_LPAREN) {
            putback(); // Put back the identifier token
            return parseFunctionCall();
        } else {
            Symbol sym = symbol_table.getSymbol(tok.value.strvalue);
            return std::make_shared<LValueNode>(sym);
        }
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
    std::shared_ptr<ExprNode> left = prefixExpr();
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
            std::shared_ptr<ExprNode> expr = parseExpressionWithPrecedence(0);
            assert(consume().type == T_SEMI);
            stmts->addStatement(expr);

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
        } else if (peek().type == T_RETURN) {
            std::shared_ptr<ReturnStatementNode> return_stmt = parseReturnStatement();
            assert(consume().type == T_SEMI);
            stmts->addStatement(return_stmt);
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

        // 检查是不是指针类型
        // 这里实际上支持了 int ******a这种操作，但是会导致pointTo操作失败，后续实现
        PrimitiveType type = var_decl->getVariableType();
        while (peek().type == T_STAR) {
            consume();
            type = pointTo(type); // Update the type to pointer type
        }
        var_decl->setVariableType(type); // Set the variable type to pointer type


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
    } while (current < toks.size() && consume().type == T_COMMA);
    putback(); // Put back the last token, which should be a semicolon or end of statement
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

// 要么是声明，要么是表达式
std::shared_ptr<StatementNode> Parser::parseSingleStatement() {
    if (peek().type == T_INT || peek().type == T_CHAR ||
        peek().type == T_FLOAT || peek().type == T_LONG) {
        return parseVariableDeclare();
    } else if (peek().type == T_IDENTIFIER) {
        return parseExpressionWithPrecedence(0);
    } else if (peek().type == T_SEMI) return nullptr;

    throw std::runtime_error("Parser::parseSingleStatement: Expected single statement at line " + 
        std::to_string(peek().line_no) + ", column " + 
        std::to_string(peek().column_no));
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
    assert(peek().type == T_VOID || peek().type == T_CHAR || peek().type == T_FLOAT || peek().type == T_LONG || peek().type == T_INT);
    TokenType return_type = consume().type; // Get the return type of the function
    if (current >= toks.size() || peek().type != T_IDENTIFIER) {
        throw std::runtime_error("Parser::parseFunctionDeclare: Expected function name at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    std::string func_name = consume().value.strvalue;
    assert(consume().type == T_LPAREN);
    assert(consume().type == T_RPAREN);
    std::shared_ptr<BlockNode> body = parseBlock();
    auto ret = std::make_shared<FunctionDeclareNode>(func_name, return_type, std::move(body));
    symbol_table.addFunction(func_name, ret->getReturnType()); // Add the function to the symbol table
    return ret;
}

std::shared_ptr<FunctionCallNode> Parser::parseFunctionCall() {
    if (current >= toks.size() || peek().type != T_IDENTIFIER) {
        throw std::runtime_error("Parser::parseFunctionCall: Expected function name at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    std::string func_name = consume().value.strvalue;
    if (peek().type != T_LPAREN) {
        throw std::runtime_error("Parser::parseFunctionCall: Expected '(' after function name at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    consume(); // Skip '('
    Function func = symbol_table.getFunction(func_name); // Check if the function exists in the symbol table
    std::vector<std::shared_ptr<ExprNode>> args;
    if (peek().type != T_RPAREN) { // If there are arguments
        do {
            std::shared_ptr<ExprNode> arg = parseExpressionWithPrecedence(0);
            args.push_back(std::move(arg));
            if (peek().type == T_COMMA) {
                consume(); // Skip ','
            } else if (peek().type != T_RPAREN) {
                throw std::runtime_error("Parser::parseFunctionCall: Expected ',' or ')' at line " + 
                    std::to_string(peek().line_no) + ", column " + 
                    std::to_string(peek().column_no));
            }
        } while (peek().type != T_RPAREN);
    }
    assert(consume().type == T_RPAREN); // Skip ')'
    return std::make_shared<FunctionCallNode>(func_name, std::move(args), func.return_type);
}

std::shared_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    assert(consume().type == T_RETURN);
    std::shared_ptr<ExprNode> return_value = nullptr;
    if (peek().type != T_SEMI) { // If there is a return value
        return_value = parseExpressionWithPrecedence(0);
    }
    return std::make_shared<ReturnStatementNode>(std::move(return_value));
}
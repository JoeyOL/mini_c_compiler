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
        } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(primary_node)) {
            // 不会出现对指针先解引用在取地址的情况，因此出现了解引用的话一定是数组
            assert(x->getOp() == U_DEREF); // Expect a dereference operation
            auto y = std::dynamic_pointer_cast<LValueNode>(x->getExpr());
            assert(y != nullptr); // Expect the expression to be an lvalue
            assert(y->isArray());
            std::shared_ptr<UnaryExpNode> unary_node = std::make_shared<UnaryExpNode>(U_ADDR, y, pointTo(y->getPrimitiveType()));
            return unary_node; // Return the unary node with address operation
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
        // 解引用符号 '*'后面必须接一个变量或者指针表达式
        std::shared_ptr<ExprNode> primary_node;
        if (peek().type == T_LPAREN) {
            consume(); // Consume the '(' token
            primary_node = parseExpressionWithPrecedence(0);
            assert(consume().type == T_RPAREN); // Expect a closing parenthesis
        } else {
            primary_node = parimary();
        }
        // 获取指针类型
        PrimitiveType type = primary_node->getPrimitiveType();
        while (star_count-- > 0) {
            type = valueAt(type);
        }
        std::shared_ptr<UnaryExpNode> unary_node = std::make_shared<UnaryExpNode>(U_DEREF, primary_node, type);
        return unary_node;
    } else if (tok.type == T_PLUS || tok.type == T_MINUS || tok.type == T_NOT || tok.type == T_INVERT) {
        consume(); // Consume the prefix operator token
        return std::make_shared<UnaryExpNode>(tok.type, prefixExpr());
    } else if (tok.type == T_INC || tok.type == T_DEC) {
        consume();
        auto expr_node = prefixExpr();
        if (auto x = std::dynamic_pointer_cast<LValueNode>(expr_node)) {
            // 如果是自增自减操作，必须是一个左值
            return std::make_shared<UnaryExpNode>(tok.type == T_INC ? U_PREINC : U_PREDEC, x, x->getPrimitiveType());
        } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(expr_node)) {
            // 如果是自增自减操作，必须是一个左值
            if (x->getOp() == U_DEREF) {
                return std::make_shared<UnaryExpNode>(tok.type == T_INC ? U_PREINC : U_PREDEC, x, x->getPrimitiveType());
            } else {
                throw std::runtime_error("Parser::prefixExpr: Expected lvalue after prefix operator at line " + 
                    std::to_string(tok.line_no) + ", column " + 
                    std::to_string(tok.column_no));
            }
        } else {
            throw std::runtime_error("Parser::prefixExpr: Expected lvalue after prefix operator at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
        }
    } else {
        return parimary(); // If no prefix operator, just parse the primary expression
    }
}

std::shared_ptr<UnaryExpNode> Parser::parseArrayAccess() {
    const Token &tok = consume();
    if (tok.type != T_IDENTIFIER) {
        throw std::runtime_error("Parser::parseArrayAccess: Expected identifier at line " +
            std::to_string(tok.line_no) + ", column " + 
            std::to_string(tok.column_no));
    }
    std::shared_ptr<Symbol> sym = symbol_table.getSymbol(tok.value.strvalue);
    int depth = 0, offset = -1;
    std::vector<std::shared_ptr<ExprNode>> indices;
    while (peek().type == T_LBRACKET) {
        consume(); // Consume the '[' token
        std::shared_ptr<ExprNode> index = parseExpressionWithPrecedence(0);
        offset = sym->getArrayBaseOffset(depth);
        // offset /= symbol_table.typeToSize(valueAt(sym->type)); // Calculate the base offset for the array element
        if (offset != 1) {
            index = std::make_shared<BinaryExpNode>(A_MULTIPLY, index, std::make_shared<ValueNode>(Value{P_INT, .ivalue = offset}));
            indices.push_back(index); // If offset is not 1, scale the index by the base offset
        } else {
            indices.push_back(index); // If offset is 1, just use the index directly
        }
        assert(consume().type == T_RBRACKET); // Expect a closing bracket
        depth++;
    }
    std::shared_ptr<ExprNode> left = indices.empty() ? nullptr : indices[0];
    if (!indices.empty()) indices.erase(indices.begin());
    while (indices.size() > 0) {
        std::shared_ptr<ExprNode> right = indices[0];
        indices.erase(indices.begin());
        left = std::make_shared<BinaryExpNode>(A_ADD, left, right);
    }
    if (left != nullptr) {
        left = std::make_shared<UnaryExpNode>(U_SCALE, left, P_LONG); // Scale the index by the size of the array element
        left->setOffset(symbol_table.typeToSize(valueAt(sym->type))); // Set the offset for the array element
    }
    std::shared_ptr<LValueNode> lvaule = std::make_shared<LValueNode>(sym, left);
    lvaule->setIndexLen(depth); // Set the index length for the lvalue
    if (depth == (int)sym->array_dimensions.size()) {
        std::shared_ptr<UnaryExpNode> ret = std::make_shared<UnaryExpNode>(U_DEREF, lvaule, valueAt(sym->type));
        return ret;
    } else {
        // if (depth != sym->array_dimensions.size() - 1) {
        //     throw std::runtime_error("Parser::parseArrayAccess: Array access depth mismatch at line " + 
        //         std::to_string(tok.line_no) + ", column " + 
        //         std::to_string(tok.column_no));
        // }
        PrimitiveType point_type = pointTo(sym->type);
        // 只允许 *(p+1) = expr 这种形式，如a[5][5], *(a[5] + 1) = expr
        std::shared_ptr<UnaryExpNode> ret = std::make_shared<UnaryExpNode>(U_ADDR, lvaule, point_type);
        return ret;
    }
}


std::shared_ptr<ExprNode> Parser::parimary() {
    const Token &tok = consume();
    if (tok.type == T_LPAREN) {
        auto ret = parseExpressionWithPrecedence(0);
        if (current >= toks.size() || peek().type != T_RPAREN) {
            throw std::runtime_error("Parser::parimary: Expected ')' at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
        }
        consume();
        return ret;
    } else if (tok.type == T_NUMBER || tok.type == T_STRING) {
        return std::make_shared<ValueNode>(tok.value);
    } else if (tok.type == T_IDENTIFIER) {
        std::shared_ptr<ExprNode> ret;
        if (peek().type == T_LPAREN) {
            putback(); // Put back the identifier token
            ret = parseFunctionCall();
        } else {
            std::shared_ptr<Symbol> sym = symbol_table.getSymbol(tok.value.strvalue);
            if (peek().type == T_LBRACKET || sym->is_array) {
                putback();
                ret = parseArrayAccess();
            }
            else ret = std::make_shared<LValueNode>(sym);
        }

        // 解析后缀
        if (peek().type == T_INC || peek().type == T_DEC) {
            Token inc_dec_tok = consume(); // Consume the increment/decrement token
            if (auto x = std::dynamic_pointer_cast<LValueNode>(ret)) {
                // 如果是自增自减操作，必须是一个左值
                return std::make_shared<UnaryExpNode>(inc_dec_tok.type == T_INC ? U_POSTINC : U_POSTDEC, x, x->getPrimitiveType());
            } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(ret)) {
                // 如果是自增自减操作，必须是一个左值
                if (x->getOp() == U_DEREF) {
                    return std::make_shared<UnaryExpNode>(inc_dec_tok.type == T_INC ? U_POSTINC : U_POSTDEC, x, x->getPrimitiveType());
                } else {
                    throw std::runtime_error("Parser::parimary: Expected lvalue after increment/decrement at line " + 
                        std::to_string(tok.line_no) + ", column " + 
                        std::to_string(tok.column_no));
                }
            } else {
                throw std::runtime_error("Parser::parimary: Expected lvalue after increment/decrement at line " + 
                    std::to_string(tok.line_no) + ", column " + 
                    std::to_string(tok.column_no));
            }
        }
        return ret; // Return the lvalue or function call node
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
        case T_OR: return A_OR;
        case T_AMPER : return A_AND; // '&' is treated as bitwise AND
        case T_XOR: return A_XOR; // '^' is treated as bitwise XOR
        case T_LSHIFT: return A_LSHIFT; // '<<' is treated as left
        case T_RSHIFT: return A_RSHIFT; // '>>' is treated as right
        case T_MOD: return A_MOD; // '%' is treated as modulo
        default:
            throw std::runtime_error("Parser::arithop: Unexpected token type " + 
                std::to_string(tok.type) + " at line " + 
                std::to_string(tok.line_no) + ", column " + 
                std::to_string(tok.column_no));
    }
}

std::shared_ptr<ExprNode> Parser::parseExpressionWithPrecedence(int prev_precedence) {
    std::shared_ptr<ExprNode> left = prefixExpr();
    if (current >= toks.size() || peek().type == T_RPAREN || peek().type == T_SEMI || peek().type == T_COMMA || peek().type == T_RBRACKET) {
        return left; // If no token is available, return the left node
    }
    while (peek().type == T_ASSIGN || precedence.at(arithop(peek())) > prev_precedence) {
        if (peek().type == T_ASSIGN) {
            consume();
            std::shared_ptr<ExprNode> right = parseExpressionWithPrecedence(0);
            std::shared_ptr<AssignmentNode> ret;
            if (auto lvalue_node = std::dynamic_pointer_cast<LValueNode>(left)) {
                // If left is an lvalue, create an assignment node
                ret = std::make_shared<AssignmentNode>(lvalue_node, std::move(right));
            } else if (auto unary_node = std::dynamic_pointer_cast<UnaryExpNode>(left)) {
                // If left is a unary expression, it should be an lvalue
                // 只支持 *(p+1) = expr 这种形式
                assert(unary_node->getOp() == U_DEREF);
                ret = std::make_shared<AssignmentNode>(unary_node, std::move(right));
                ret->setType(unary_node->getPrimitiveType()); // Set the type of the assignment node
            }
            return ret; // Return the assignment node
        } else {
            ExprType type = arithop(consume());
            std::shared_ptr<ExprNode> right = parseExpressionWithPrecedence(precedence.at(type));
            auto ret= std::make_shared<BinaryExpNode>(type, std::move(left), std::move(right));
            ret->updateTypeAfterCal();
            if (current >= toks.size() || peek().type == T_RPAREN || peek().type == T_SEMI || peek().type == T_COMMA || peek().type == T_RBRACKET) {
                return ret; // If no token is available, return the left node
            }
            left = std::move(ret); // Update left to the new binary expression node
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
    symbol_table.enterScope(); // Enter a new scope for the block
    std::shared_ptr<BlockNode> stmts = std::make_shared<BlockNode>();
    bool stmt_limit = false;
    if (peek().type == T_LBRACE) {
        consume();
    } else {
        stmt_limit = true; // If the next token is not '{', we limit the statements to one
    }
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
        } else if (peek().type == T_IDENTIFIER || peek().type == T_STAR) {
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
        } else if (peek().type == T_BREAK) {
            consume();
            if (loop_end_labels.empty()) {
                throw std::runtime_error("Parser::parseBlock: 'break' statement not inside a loop at line " + 
                    std::to_string(peek().line_no) + ", column " + 
                    std::to_string(peek().column_no));
            }
            std::shared_ptr<BreakStatementNode> break_stmt = std::make_shared<BreakStatementNode>(loop_end_labels.back());
            assert(consume().type == T_SEMI); // Expect a semicolon after break statement
            stmts->addStatement(break_stmt);
        } else if (peek().type == T_CONTINUE) {
            consume(); // Consume the 'continue' token
            if (loop_st_labels.empty()) {
                throw std::runtime_error("Parser::parseBlock: 'continue' statement not inside a loop at line " + 
                    std::to_string(peek().line_no) + ", column " + 
                    std::to_string(peek().column_no));
            }
            std::shared_ptr<ContinueStatementNode> continue_stmt = std::make_shared<ContinueStatementNode>(loop_st_labels.back());
            assert(consume().type == T_SEMI); // Expect a semicolon after continue statement
            stmts->addStatement(continue_stmt);
        } else {
            throw std::runtime_error("Parser::parseStatement: Expected statement at line " + 
                std::to_string(peek().line_no) + ", column " + 
                std::to_string(peek().column_no));
        }
        if (stmt_limit) {
            break; // If we are limiting to one statement, break after the first statement
        }
    }
    if (!stmt_limit) {
        assert(consume().type == T_RBRACE);
    }
    symbol_table.exitScope(); // Exit the scope after parsing the block
    return stmts;
}

std::shared_ptr<ArrayInitializer> Parser::parseArrayInitializer(std::shared_ptr<Symbol>sym, std::vector<int> &dimensions, int depth) {
    if (depth >= (int)dimensions.size()) {
        throw std::runtime_error("Parser::parseArrayInitializer: Depth exceeds dimensions size at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    assert(consume().type == T_LBRACE);
    std::shared_ptr<ArrayInitializer> array_init = std::make_shared<ArrayInitializer>(sym, dimensions, depth);
    while(peek().type != T_RBRACE) {
        if (peek().type == T_NUMBER) {
            auto value = std::make_shared<ValueNode>(consume().value);
            array_init->addInitializer(value);
        } else if (peek().type == T_LBRACE) {
            if (!array_init->canAcceptNestedInitializer()) {
                throw std::runtime_error("Parser::parseArrayInitializer: Can not accpet a nested initializer " + 
                    std::to_string(peek().line_no) + ", column " + 
                    std::to_string(peek().column_no));
            }
            // 递归解析嵌套的数组初始化
            std::shared_ptr<ArrayInitializer> nested_init = parseArrayInitializer(sym, dimensions, depth + 1);
            array_init->addInitializer(nested_init);
        } else {
            throw std::runtime_error("Parser::parseArrayInitializer: Expected number or '{' at line " + 
                std::to_string(peek().line_no) + ", column " + 
                std::to_string(peek().column_no));
        }
        if (peek().type != T_RBRACE) assert(consume().type == T_COMMA);
    }
    assert(consume().type == T_RBRACE); // Expect a closing brace
    return array_init;
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

        if (peek().type == T_LBRACKET) {
            while (peek().type == T_LBRACKET) {
                consume(); // Consume the '[' token
                if (peek().type != T_NUMBER) {
                    throw std::runtime_error("Parser::parseVariableDeclare: Expected number after '[' at line " + 
                        std::to_string(peek().line_no) + ", column " + 
                        std::to_string(peek().column_no));
                }
                int array_size = consume().value.ivalue;
                assert(consume().type == T_RBRACKET); // Expect a closing bracket
                var_decl->addDimension(array_size); // Add the array dimension to the variable declaration
            }
            var_decl->setArray(true); // Set the variable as an array
        }

        if (!var_decl->isArray()) symbol_table.addSymbol(var_name, var_decl->getVariableType()); // Assuming all variables are of type int for simplicity
        else symbol_table.addSymbol(var_name, var_decl->getVariableType(), var_decl->getDimensions()); // Add the variable to the symbol table with its dimensions

        std::shared_ptr<Symbol> sym = symbol_table.getSymbol(var_name); // Get the symbol from the symbol table

        if (peek().type == T_ASSIGN) {
            consume();
            std::shared_ptr<ExprNode> initializer;
            if (!var_decl->isArray()) initializer = parseExpressionWithPrecedence(0);
            else {
                auto dims = var_decl->getDimensions();
                initializer = parseArrayInitializer(sym, dims, 0);
                initializer->setPrimitiveType(valueAt(var_decl->getVariableType())); // Set the primitive type of the initializer
                auto y = std::dynamic_pointer_cast<ArrayInitializer>(initializer);
            }
            var_decl->addIdentifier(sym, std::move(initializer));
        } else {
            var_decl->addIdentifier(sym);
        }

        // 添加到符号表

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
    std::shared_ptr<Symbol> sym = symbol_table.getSymbol(identifier); // Check if the identifier exists in the symbol table
    assert(consume().type == T_ASSIGN); // Skip the '=' token
    std::shared_ptr<ExprNode> expr = parseExpressionWithPrecedence(0);
    return std::shared_ptr<AssignmentNode>(new AssignmentNode(std::make_shared<LValueNode>(sym), std::move(expr)));
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
    std::string while_label_no = labelAllocator.getLabel(LableType::WHILE_LABEL);
    std::string while_st = "WHILE_START_" + while_label_no;
    std::string while_end = "WHILE_END_" + while_label_no;
    loop_st_labels.push_back(while_st); // Push the start label for the loop
    loop_end_labels.push_back(while_end); // Push the end label for the loop
    std::shared_ptr<BlockNode> body = parseBlock();
    auto ret = std::make_shared<WhileStatementNode>(std::move(condition), std::move(body));
    ret->setLabels(while_st, while_end); // Set the labels for the while statement
    loop_st_labels.pop_back(); // Pop the start label for the loop
    loop_end_labels.pop_back(); // Pop the end label for the loop
    return ret;
}

// 要么是声明，要么是表达式
std::shared_ptr<StatementNode> Parser::parseSingleStatement() {
    if (peek().type == T_INT || peek().type == T_CHAR ||
        peek().type == T_FLOAT || peek().type == T_LONG) {
        return parseVariableDeclare();
    } else if (peek().type == T_SEMI) { 
        return nullptr;
    } else {
        return parseExpressionWithPrecedence(0); // Parse as an expression
    }


    throw std::runtime_error("Parser::parseSingleStatement: Expected single statement at line " + 
        std::to_string(peek().line_no) + ", column " + 
        std::to_string(peek().column_no));
}


std::shared_ptr<ForStatementNode> Parser::parseForStatement() {
    symbol_table.enterScope();
    assert(consume().type == T_FOR);
    assert(consume().type == T_LPAREN);
    std::shared_ptr<StatementNode> preop_stmt = parseSingleStatement();
    assert(consume().type == T_SEMI);
    std::shared_ptr<ExprNode> condition = parseExpressionWithPrecedence(0);
    assert(consume().type == T_SEMI);
    std::shared_ptr<StatementNode> postop_stmt = parseSingleStatement();
    assert(consume().type == T_RPAREN);
    std::string for_label_no = labelAllocator.getLabel(LableType::FOR_LABEL);
    std::string for_st = "FOR_START_" + for_label_no;
    std::string for_end = "FOR_END_" + for_label_no;
    loop_st_labels.push_back(for_st); // Push the start label for the loop
    loop_end_labels.push_back(for_end); // Push the end label for the loop
    std::shared_ptr<BlockNode> body = parseBlock();
    symbol_table.exitScope(); // Exit the scope after parsing the for statement
    auto ret = std::make_shared<ForStatementNode>(std::move(preop_stmt), std::move(condition), std::move(body), std::move(postop_stmt));
    ret->setLabels(for_st, for_end); // Set the labels for the for statement
    loop_st_labels.pop_back(); // Pop the start label for the loop
    loop_end_labels.pop_back(); // Pop the end label for the loop
    return ret;
}

PrimitiveType tokenTypeToPrimitiveType(TokenType type) {
    switch (type) {
        case T_INT: return P_INT;
        case T_FLOAT: return P_FLOAT;
        case T_CHAR: return P_CHAR;
        case T_LONG: return P_LONG;
        case T_VOID: return P_VOID; // 虽然void类型没有实际意义，但为了兼容性保留
        default:
            throw std::runtime_error("Parser::tokenTypeToPrimitiveType: Unexpected token type " + 
                std::to_string(type));
    }
}


std::shared_ptr<FunctionParamNode> Parser::parseFunctionParam() {
    auto ret = std::make_shared<FunctionParamNode>();
    if (peek().type == T_RPAREN) {
        return ret; // Return an empty function parameter node
    }
    do {
        assert(peek().type == T_INT || peek().type == T_CHAR || peek().type == T_FLOAT || peek().type == T_LONG);
        std::shared_ptr<Symbol> sym = std::make_shared<Symbol>();

        // 检查是不是指针类型
        // 这里实际上支持了 int ******a这种操作，但是会导致pointTo操作失败，后续实现
        PrimitiveType type = tokenTypeToPrimitiveType(consume().type);
        while (peek().type == T_STAR) {
            consume();
            type = pointTo(type); // Update the type to pointer type
        }
        sym->type = type; // Set the variable type to pointer type


        if (current >= toks.size() || peek().type != T_IDENTIFIER) {
            throw std::runtime_error("Parser::parseVariableDeclare: Expected identifier at line " + 
                std::to_string(peek().line_no) + ", column " + 
                std::to_string(peek().column_no));
        }
        sym->name = consume().value.strvalue;


        // 解析数组，第一维长度可以缺省
        bool first_dimension = true;
        std::vector<int> dimensions;
        if (peek().type == T_LBRACKET) {
            while (peek().type == T_LBRACKET) {
                consume(); // Consume the '[' token
                if (peek().type != T_NUMBER) {
                    if (first_dimension) {
                        first_dimension = false; // 第一维可以缺省
                        int array_size = -1;
                        dimensions.push_back(array_size); // Add a placeholder for the first dimension
                    } else {
                        throw std::runtime_error("Parser::parseVariableDeclare: Expected number after '[' at line " + 
                            std::to_string(peek().line_no) + ", column " + 
                            std::to_string(peek().column_no));
                    }
                } else {
                    first_dimension = false; // 第一维可以缺省
                    int array_size = consume().value.ivalue;
                    dimensions.push_back(array_size); // Add the array dimension to the variable declaration
                }
                assert(consume().type == T_RBRACKET); // Expect a closing bracket
            }
            sym->is_array = true; // Set the variable as an array
            sym->array_dimensions = dimensions; // Set the array dimensions
            sym->type = arrayTo(sym->type);
        }
        // 添加到符号表
        ret->addParam(sym); // Add the parameter to the function parameter node

    } while (current < toks.size() && consume().type == T_COMMA);
    putback(); // Put back the last token, which should be a semicolon or end of statement
    return ret;
}


// TODO: 目前只支持void类型的无参数函数
std::shared_ptr<FunctionDeclareNode> Parser::parseFunctionDeclare() {
    assert(peek().type == T_VOID || peek().type == T_CHAR || peek().type == T_FLOAT || peek().type == T_LONG || peek().type == T_INT);
    PrimitiveType return_type = tokenTypeToPrimitiveType(consume().type); // Get the return type of the function

    while (peek().type == T_STAR) {
        consume(); // Consume the '*' token for pointer type
        return_type = pointTo(return_type); // Update the return type to pointer type
    }

    if (current >= toks.size() || peek().type != T_IDENTIFIER) {
        throw std::runtime_error("Parser::parseFunctionDeclare: Expected function name at line " + 
            std::to_string(peek().line_no) + ", column " + 
            std::to_string(peek().column_no));
    }
    std::string func_name = consume().value.strvalue;
    assert(consume().type == T_LPAREN);
    std::shared_ptr<FunctionParamNode> param = parseFunctionParam(); // Parse the function parameters, if any
    assert(consume().type == T_RPAREN);
    // 处理符号表
    symbol_table.addFunction(func_name, return_type, param->getParams()); // Add the function to the symbol table
    symbol_table.enterFunction(param->getParams()); // Enter the function scope with the parameters

    std::shared_ptr<BlockNode> body = parseBlock();
    auto ret = std::make_shared<FunctionDeclareNode>(func_name, return_type, std::move(body), std::move(param));
    symbol_table.exitFuction(); // Exit the function scope after parsing the function declaration
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
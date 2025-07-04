#include "semantic/semantic.h"

void Semantic::check() {
    for (auto & var : ast->getGlobalVariables()) {
        checkVariableDeclare(var);
    }
    for (auto & func : ast->getFunctions()) {
        checkFunctionDeclare(func);
    }
}

bool Semantic::checkLvalueValid(std::shared_ptr<ExprNode> lvalue) {
    if (auto lvalue_type = std::dynamic_pointer_cast<LValueNode>(lvalue)) return true;
    else if (auto unary_node = std::dynamic_pointer_cast<UnaryExpNode>(lvalue)) {
        if (unary_node->getOp() == U_DEREF) {
            // Dereference operation, check if the inner expression is a valid lvalue
            return true;
        }
        else {
            // Other unary operations are not valid lvalues
            throw std::runtime_error("Semantic::checkLvalueValid: Invalid lvalue for assignment");
        }
    } else {
        // If it's not an LValueNode or a valid unary expression, it's not a valid lvalue
        throw std::runtime_error("Semantic::checkLvalueValid: Invalid lvalue for assignment");
    }
}

void Semantic::checkAssignment(std::shared_ptr<AssignmentNode> node) {
    std::shared_ptr<ExprNode> lvalue = node->getLvalue();
    assert(checkLvalueValid(lvalue)); // Ensure lvalue is valid for assignment
    checkExpression(lvalue);
    checkExpression(node->getExpr());
    if (!assignCompatible(lvalue->getPrimitiveType(), node->getExpr()->getPrimitiveType(), node->isNeedTransform())) {
        throw std::runtime_error("Semantic::checkAssignment: Type mismatch for assignment");
    }
    if (node->isNeedTransform()) {
        node->setExpr(std::make_shared<UnaryExpNode>(U_TRANSFORM, node->getExpr(), node->getPrimitiveType()));
    }
}

void Semantic::checkVariableDeclare(std::shared_ptr<VariableDeclareNode> node) { 
    if (node->getIdentifiers().empty()) {
        throw std::runtime_error("Semantic::checkVariableDeclare: No identifiers found in variable declaration");
    }
    PrimitiveType var_type = node->getVariableType();
    for (const auto& identifier : node->getIdentifiers()) {
        if (node->getInitializer(identifier) != nullptr) {
            // Check initializer type
            auto initializer = node->getInitializer(identifier);

            if (is_array(var_type)) {
                checkExpression(initializer);
                continue;
            }

            checkExpression(initializer);
            if (!assignCompatible(var_type, initializer->getPrimitiveType(), initializer->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkVariableDeclare: Type mismatch for initializer of " + identifier.name);
            }
            if (initializer->isNeedTransform()) {
                initializer = std::make_shared<UnaryExpNode>(U_TRANSFORM, initializer, var_type);
                node->setInitializer(identifier, initializer);
            }
        }
    }
}

bool Semantic::assignCompatible(PrimitiveType type1, PrimitiveType type2, bool& need_transform) {
    // Check if two types are compatible for assignment
    if (type1 == P_VOID || type2 == P_VOID || type1 == P_NONE || type2 == P_NONE) {
        return false; // Void type is not compatible with any other type
    }

    if (is_pointer(type1)) {
        if (type1 != type2) {
            throw std::runtime_error("Semantic::assignCompatible: Pointer types do not match for assignment");
        }
        need_transform = false; // Pointer types are compatible as long as they match
        return true;
    }

    return typeCompatible(type1, type2, need_transform); // Use type compatibility check for non-pointer types
}

bool Semantic::typeCompatible(PrimitiveType type1, PrimitiveType type2, bool& need_transform) {
    // Check if two types are compatible
    if (type1 == P_VOID || type2 == P_VOID || type1 == P_NONE || type2 == P_NONE) {
        need_transform = false; // Void type is not compatible with any other type
        return false;
    }

    if (is_pointer(type1)) type1 = P_LONG;
    if (is_pointer(type2)) type2 = P_LONG;
    if (type1 == type2) {
        need_transform = false;
    } else {
        need_transform = true; // Different types may require transformation
    }
    return true; // Same type is always compatible
}

void Semantic::checkPrint(std::shared_ptr<PrintStatementNode> node) {
    auto expr = node->getExpression();
    checkExpression(expr);
    if (expr == nullptr) {
        throw std::runtime_error("Semantic::checkPrint: No expression found in print statement");
    }
    // Check if the expression type is compatible with print
    if (expr->getCalculateType() == P_INT || expr->getCalculateType() == P_CHAR || expr->getCalculateType() == P_LONG) {
        if (!typeCompatible(expr->getCalculateType(), P_LONG, node->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkPrint: Type mismatch in print statement, expected int");
        }
        if (node->isNeedTransform()) {
            node->setExpression(std::make_shared<UnaryExpNode>(U_TRANSFORM, expr, P_LONG));
        }
    } else {
        if (!typeCompatible(expr->getCalculateType(), P_FLOAT, node->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkPrint: Type mismatch in print statement, expected string");
        }
        if (node->isNeedTransform()) {
            node->setExpression(std::make_shared<UnaryExpNode>(U_TRANSFORM, expr, P_FLOAT));
        }
    }
}

void Semantic::checkBlock(std::shared_ptr<BlockNode> node) { 
    for (auto& stmt : node->getStatements()) {
        checkStatement(stmt);
    }
}

// TODO: func call 参数匹配
void Semantic::checkStatement(std::shared_ptr<StatementNode> stmt) { 
    switch (stmt->getStmtType()) {
        case S_VARDEF:
            checkVariableDeclare(std::dynamic_pointer_cast<VariableDeclareNode>(stmt));
            break;
        case S_EXPR:
            checkExpression(std::dynamic_pointer_cast<ExprNode>(stmt));
            break;
        case S_ASSIGN:
            checkAssignment(std::dynamic_pointer_cast<AssignmentNode>(stmt));
            break;
        case S_PRINT:
            checkPrint(std::dynamic_pointer_cast<PrintStatementNode>(stmt));
            break;
        case S_IF:
            checkIfStatement(std::dynamic_pointer_cast<IfStatementNode>(stmt));
            break;
        case S_BLOCK:
            checkBlock(std::dynamic_pointer_cast<BlockNode>(stmt));
            break;
        case S_WHILE:
            checkWhileStatement(std::dynamic_pointer_cast<WhileStatementNode>(stmt));
            break;
        case S_FOR:
            checkForStatement(std::dynamic_pointer_cast<ForStatementNode>(stmt));
            break;
        case S_RETURN:
            checkReturnStatement(std::dynamic_pointer_cast<ReturnStatementNode>(stmt));
            break;
    }
}

void Semantic::checkReturnStatement(std::shared_ptr<ReturnStatementNode> node) {
    if (node->getExpression() != nullptr) {
        checkExpression(node->getExpression());
        PrimitiveType return_type = symbol_table.getCurrentFunction().return_type;
        if (is_pointer(return_type)) {
            if (node->getExpression()->getPrimitiveType() != return_type) {
                throw std::runtime_error("Semantic::checkReturnStatement: Pointer type mismatch in return statement");
            }
        } else {
            if (!typeCompatible(return_type, node->getExpression()->getPrimitiveType(), node->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkReturnStatement: Type mismatch in return statement");
            }
        }

        if (node->isNeedTransform()) {
            node->setExpression(std::make_shared<UnaryExpNode>(U_TRANSFORM, node->getExpression(), return_type));
        }

    } else {
        assert(symbol_table.getCurrentFunction().return_type == P_VOID);
    }
    node->setFunction(symbol_table.getCurrentFunction());
    symbol_table.setCurrentFunctionReturn(true); // Mark that the current function has a return statement
}

void Semantic::checkIfStatement(std::shared_ptr<IfStatementNode> node) { 
    checkExpression(node->getCondition());
    checkStatement(node->getThenStatement());
    if (node->getElseStatement()) {
        checkStatement(node->getElseStatement());
    }
}

void Semantic::checkForStatement(std::shared_ptr<ForStatementNode> node) { 
    if (node->getPreopStatement()) {
        checkStatement(node->getPreopStatement());
    }
    checkExpression(node->getCondition());
    checkStatement(node->getBody());
    if (node->getPostopStatement()) {
        checkStatement(node->getPostopStatement());
    }
}

void Semantic::checkWhileStatement(std::shared_ptr<WhileStatementNode> node) { 
    checkExpression(node->getCondition());
    checkStatement(node->getBody());
}

void Semantic::checkExpression(std::shared_ptr<ExprNode> node) {
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(node)) {
        checkExpression(x->getLeft());
        checkExpression(x->getRight());
        if (x->getLeft()->getPrimitiveType() == P_VOID || x->getRight()->getPrimitiveType() == P_VOID || 
            x->getLeft()->getPrimitiveType() == P_NONE || x->getRight()->getPrimitiveType() == P_NONE) {
            throw std::runtime_error("Semantic::checkExpression: Void type in binary expression");
        }

        // 判断是不是两个指针做运算，两个指针只能做比较
        if (is_pointer(x->getLeft()->getPrimitiveType()) && is_pointer(x->getRight()->getPrimitiveType()) && 
            (x->getOp() != A_EQ && x->getOp() != A_NE) && (x->getOp() != A_LT && x->getOp() != A_LE && x->getOp() != A_GT && x->getOp() != A_GE)) {
            throw std::runtime_error("Semantic::checkExpression: Binary expression cannot have two pointer types");
        }

        // 指针只能进行加减法
        if (is_pointer(x->getLeft()->getPrimitiveType()) || is_pointer(x->getRight()->getPrimitiveType())) {
            if (x->getOp() != A_ADD && x->getOp() != A_SUBTRACT) {
                throw std::runtime_error("Semantic::checkExpression: Pointer types can only be used with addition or subtraction");
            }
        }

        if (!typeCompatible(x->getLeft()->getCalculateType(), x->getRight()->getCalculateType(), x->getLeft()->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkExpression: Type mismatch in binary expression");
        }

        // 获取计算类型
        x->updateCalType();

        if (x->getOp() == A_AND || x->getOp() == A_OR || x->getOp() == A_XOR || x->getOp() == A_MOD) {
            if (x->getLeft()->getPrimitiveType() == P_FLOAT || x->getRight()->getPrimitiveType() == P_FLOAT) {
                throw std::runtime_error("Semantic::checkExpression: Bitwise operators can only be applied to int");
            }
        }

        // 对于左移和右移操作，确保将左值转化为long， 右值转换为char
        if (x->getOp() == A_LSHIFT || x->getOp() == A_RSHIFT) {
            if (!typeCompatible(x->getLeft()->getPrimitiveType(), P_LONG, x->getLeft()->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkExpression: Left operand of shift must be long");
            }
            if (!typeCompatible(x->getRight()->getPrimitiveType(), P_CHAR, x->getRight()->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkExpression: Right operand of shift must be char");
            }
            if (x->getRight()->isNeedTransform() || x->getLeft()->isNeedTransform()) {
                if (x->getRight()->isNeedTransform()) {
                    x->setRight(std::make_shared<UnaryExpNode>(U_TRANSFORM, x->getRight(), P_CHAR));
                }
                if (x->getLeft()->isNeedTransform()) {
                    x->setLeft(std::make_shared<UnaryExpNode>(U_TRANSFORM, x->getLeft(), P_LONG));
                }
            }
        } else {
            typeCompatible(x->getCalType(), x->getRight()->getCalculateType(), x->getRight()->isNeedTransform());
            typeCompatible(x->getCalType(), x->getLeft()->getCalculateType(), x->getLeft()->isNeedTransform());
            // 如果需要转换，创建一个unary表达式节点
            if (x->getRight()->isNeedTransform() || x->getLeft()->isNeedTransform()) {
                if (x->getRight()->isNeedTransform()) {
                    x->setRight(std::make_shared<UnaryExpNode>(U_TRANSFORM, x->getRight(), x->getCalType()));
                }
                if (x->getLeft()->isNeedTransform()) {
                    x->setLeft(std::make_shared<UnaryExpNode>(U_TRANSFORM, x->getLeft(), x->getCalType()));
                }
            }
        }

        // 如果左右表达式有一个的offset不等于1，需要创建一个unary表达式节点
        if (x->getLeft()->getOffset() != 1 || x->getRight()->getOffset() != 1) {
            if (x->getLeft()->getOffset() != 1 && x->getRight()->getOffset() != 1) {
                // do nothing
            } else if (x->getRight()->getOffset() != 1) {
                auto y = std::make_shared<UnaryExpNode>(U_SCALE, x->getLeft(), x->getLeft()->getPrimitiveType());
                y->setOffset(x->getRight()->getOffset());
                x->setLeft(y);
            } else if (x->getLeft()->getOffset() != 1) {
                auto y = std::make_shared<UnaryExpNode>(U_SCALE, x->getRight(), x->getRight()->getPrimitiveType());
                y->setOffset(x->getLeft()->getOffset());
                x->setRight(y);
            }
        }

        x->updateTypeAfterCal();

    } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(node)) {
        // Value nodes are already checked during parsing
        checkExpression(x->getExpr()); // Recursively check the operand of the unary expression
        if (x->getOp() == U_INVERT && x->getExpr()->getPrimitiveType() == P_FLOAT) {
            throw std::runtime_error("Semantic::checkExpression: Bitwise NOT operator can only be applied to int");
        }
        x->updateType();
    } else if (auto x = std::dynamic_pointer_cast<AssignmentNode>(node)) {
        checkAssignment(x);
    } else if (auto x = std::dynamic_pointer_cast<FunctionCallNode>(node)) {
        checkFunctionCall(x);
    } else if (auto x = std::dynamic_pointer_cast<ArrayInitializer>(node)) {
        for (auto& elem : x->getElements()) {
            if (auto y = std::dynamic_pointer_cast<ArrayInitializer>(elem)) {
                y->setPrimitiveType(x->getPrimitiveType()); // Set the type of the nested array initializer
                checkExpression(y);
            } 
        }
    } else if (auto x = std::dynamic_pointer_cast<LValueNode>(node)) {
        if (!x->isArray()) return;
        std::shared_ptr<ExprNode> index = x->getIndex();
        if (index != nullptr) {
            checkExpression(index);
            if (!typeCompatible(index->getPrimitiveType(), P_INT, index->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkExpression: Index type must be int for array access");
            }
            if (index->isNeedTransform()) {
                index = std::make_shared<UnaryExpNode>(U_TRANSFORM, index, P_INT);
                x->setIndex(index);
            }
        }
    }
}


void Semantic::checkFunctionCall(std::shared_ptr<FunctionCallNode> node) {
    Function func = symbol_table.getFunction(node->getIdentifier());
    std::vector<std::shared_ptr<ExprNode>> args = node->getArguments();
    if (func.params.size() != args.size()) {
        throw std::runtime_error("Semantic::checkFunctionCall: Function call argument count mismatch for " + node->getIdentifier());
    }
    for (size_t i = 0; i < args.size(); ++i) {
        checkExpression(args[i]);
        if (!func.params[i]->is_array) {
            if (!assignCompatible(func.params[i]->type, args[i]->getPrimitiveType(), args[i]->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkFunctionCall: Type mismatch in function call argument " + std::to_string(i) + " for " + node->getIdentifier());
            }
            if (args[i]->isNeedTransform()) {
                args[i] = std::make_shared<UnaryExpNode>(U_TRANSFORM, args[i], func.params[i]->type);
            }
        } else {
            auto x = std::dynamic_pointer_cast<UnaryExpNode> (args[i]);
            assert(x != nullptr && x->getOp() == U_ADDR);
            auto lvalue = std::dynamic_pointer_cast<LValueNode>(x->getExpr());
            assert(lvalue != nullptr && is_array(lvalue->getPrimitiveType()));
            // 匹配维度
        
            if (lvalue->getPrimitiveType() != func.params[i]->type) {
                throw std::runtime_error("Semantic::checkFunctionCall: Type mismatch in function call argument " + std::to_string(i) + " for " + node->getIdentifier());
            }

            if (lvalue->getIndex() != nullptr) {
                checkExpression(lvalue->getIndex());
                if (!typeCompatible(lvalue->getIndex()->getPrimitiveType(), P_INT, lvalue->getIndex()->isNeedTransform())) {
                    throw std::runtime_error("Semantic::checkFunctionCall: Index type must be int for array access in function call");
                }
                if (lvalue->getIndex()->isNeedTransform()) {
                    lvalue->setIndex(std::make_shared<UnaryExpNode>(U_TRANSFORM, lvalue->getIndex(), P_INT));
                }
            }

            int index_len = lvalue->getIndexLen();
            auto lvalue_sym = lvalue->getIdentifier();
            if (lvalue_sym.array_dimensions.size() - index_len != func.params[i]->array_dimensions.size()) {
                throw std::runtime_error("Semantic::checkFunctionCall: Array dimensions mismatch in function call argument " + std::to_string(i) + " for " + node->getIdentifier());
            }

            for (size_t j = 0; j < func.params[i]->array_dimensions.size(); ++j) {
                if (lvalue_sym.array_dimensions[j + index_len] != func.params[i]->array_dimensions[j] && func.params[i]->array_dimensions[j] != -1) {
                    throw std::runtime_error("Semantic::checkFunctionCall: Array dimensions mismatch in function call argument " + std::to_string(i) + " for " + node->getIdentifier());
                }
            }

        }

    }
    node->setArguments(args);
    node->updateParamCount();
}

void Semantic::checkFunctionDeclare(std::shared_ptr<FunctionDeclareNode> node) { 
    symbol_table.setCurrentFunction(symbol_table.getFunction(node->getIdentifier()));
    checkBlock(node->getBody());
    assert(symbol_table.getCurrentFunction().has_return || node->getReturnType() == P_VOID);
}
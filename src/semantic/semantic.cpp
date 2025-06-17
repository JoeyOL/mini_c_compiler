#include "semantic/semantic.h"

void Semantic::check() {
    for (auto & var : ast->getGlobalVariables()) {
        checkVariableDeclare(var);
    }
    for (auto & func : ast->getFunctions()) {
        checkFunctionDeclare(func);
    }
}

void Semantic::checkAssignment(std::shared_ptr<AssignmentNode> node) {
    Symbol identifier = node->getIdentifier();
    checkExpression(node->getExpr());
    if (!typeCompatible(identifier.type, node->getExpr()->getType(), node->isNeedTransform())) {
        throw std::runtime_error("Semantic::checkAssignment: Type mismatch for assignment to " + identifier.name);
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
            checkExpression(initializer);
            if (!typeCompatible(var_type, initializer->getType(), initializer->isNeedTransform())) {
                throw std::runtime_error("Semantic::checkVariableDeclare: Type mismatch for initializer of " + identifier);
            }
        }
    }
}

bool Semantic::typeCompatible(PrimitiveType type1, PrimitiveType type2, bool& need_transform) {
    // Check if two types are compatible
    if (type1 == type2) {
        need_transform = false;
        return true; // Same type is always compatible
    }
    if ((type1 == P_INT && type2 == P_FLOAT) || (type1 == P_FLOAT && type2 == P_INT) 
        || (type1 == P_CHAR && type2 == P_INT) || (type1 == P_INT && type2 == P_CHAR)
        || (type1 == P_LONG && type2 == P_INT) || (type1 == P_INT && type2 == P_LONG)
        || (type1 == P_LONG && type2 == P_FLOAT) || (type1 == P_FLOAT && type2 == P_LONG)
        || (type1 == P_LONG && type2 == P_CHAR) || (type1 == P_CHAR && type2 == P_LONG)
        || (type1 == P_CHAR && type2 == P_FLOAT) || (type1 == P_FLOAT && type2 == P_CHAR)) {
        need_transform = true; // Indicate that a transformation is needed
        return true; // int and float are compatible
    }
    return false; // Other types are not compatible
}

void Semantic::checkPrint(std::shared_ptr<PrintStatementNode> node) {
    auto expr = node->getExpression();
    checkExpression(expr);
    if (expr == nullptr) {
        throw std::runtime_error("Semantic::checkPrint: No expression found in print statement");
    }
    // Check if the expression type is compatible with print
    if (expr->getType() == P_INT || expr->getType() == P_CHAR || expr->getType() == P_LONG) {
        if (!typeCompatible(expr->getType(), P_LONG, node->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkPrint: Type mismatch in print statement, expected int");
        }
        node->setType(P_LONG);
    } else {
        if (!typeCompatible(expr->getType(), P_FLOAT, node->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkPrint: Type mismatch in print statement, expected string");
        }
        node->setType(P_FLOAT);
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
        if (!typeCompatible(return_type, node->getExpression()->getType(), node->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkReturnStatement: Type mismatch in return statement");
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
        if (x->getLeft()->getType() == P_VOID || x->getRight()->getType() == P_VOID) {
            throw std::runtime_error("Semantic::checkExpression: Void type in binary expression");
        }
        if (!typeCompatible(x->getLeft()->getType(), x->getRight()->getType(), x->getLeft()->isNeedTransform())) {
            throw std::runtime_error("Semantic::checkExpression: Type mismatch in binary expression");
        }
        x->updateCalType();
        typeCompatible(x->getCalType(), x->getRight()->getType(), x->getRight()->isNeedTransform());
        typeCompatible(x->getCalType(), x->getLeft()->getType(), x->getLeft()->isNeedTransform());
        x->updateTypeAfterCal();

    } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(node)) {
        // Value nodes are already checked during parsing
        checkExpression(x->getExpr()); // Recursively check the operand of the unary expression
        x->updateType();
    } else if (auto x = std::dynamic_pointer_cast<AssignmentNode>(node)) {
        checkAssignment(x);
    } else if (auto x = std::dynamic_pointer_cast<FunctionCallNode>(node)) {
        auto args = x->getArguments();
        for (auto arg : args) {
            checkExpression(arg);
        }
    }
}

void Semantic::checkFunctionDeclare(std::shared_ptr<FunctionDeclareNode> node) { 
    symbol_table.setCurrentFunction(symbol_table.getFunction(node->getIdentifier()));
    checkBlock(node->getBody());
    assert(symbol_table.getCurrentFunction().has_return || node->getReturnType() == P_VOID);
}
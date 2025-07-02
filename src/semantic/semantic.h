#pragma once
#include "common/defs.h"
#include "parser/parser.h"

class Semantic {
public:
    Semantic(std::shared_ptr<Pragram> ast) : ast(ast) {}
    void check();
    void checkAssignment(std::shared_ptr<AssignmentNode> node);
    void checkBlock(std::shared_ptr<BlockNode> node);
    void checkExpression(std::shared_ptr<ExprNode> node);
    void checkVariableDeclare(std::shared_ptr<VariableDeclareNode> node);
    void checkFunctionDeclare(std::shared_ptr<FunctionDeclareNode> node);
    void checkFunctionCall(std::shared_ptr<FunctionCallNode> node);
    void checkPrint(std::shared_ptr<PrintStatementNode> node);
    void checkIfStatement(std::shared_ptr<IfStatementNode> node);
    void checkWhileStatement(std::shared_ptr<WhileStatementNode> node);
    void checkForStatement(std::shared_ptr<ForStatementNode> node);
    void checkStatement(std::shared_ptr<StatementNode> node);
    void checkReturnStatement(std::shared_ptr<ReturnStatementNode> node);
private:
    std::shared_ptr<Pragram> ast;
    bool typeCompatible(PrimitiveType type1, PrimitiveType type2, bool &need_transform);
    bool assignCompatible(PrimitiveType type1, PrimitiveType type2, bool &need_transform);
    bool checkLvalueValid(std::shared_ptr<ExprNode> lvalue);
};
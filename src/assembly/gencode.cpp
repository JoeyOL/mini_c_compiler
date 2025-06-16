#include "assembly/gencode.h"

int GenCode::walkExpr(const std::shared_ptr<ExprNode>& ast) { 
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(ast)) {
        // TODO
        // if (x->getOp() == A_AND) {
        //     return walkAndExpr(x);
        // } else if (x->getOp() == A_OR) {
        //     return walkOrExpr(x);
        // }
        // Handle binary expression node
        int reg1 = walkExpr(x->getLeft());
        int reg2 = walkExpr(x->getRight());
        switch (x->getOp()) {
            case A_ADD: return cgadd(reg1, reg2); // Add the two registers and return the result
            case A_SUBTRACT: return cgsub(reg1, reg2); // Subtract the two registers and return the result
            case A_MULTIPLY: return cgmul(reg1, reg2); // Multiply the two registers and return the result
            case A_DIVIDE: return cgdiv(reg1, reg2); // Divide the two registers and return the result
            case A_EQ: return cgequal(reg1, reg2);
            case A_NE: return cgnotequal(reg1, reg2);
            case A_LT: return cglessthan(reg1, reg2);
            case A_LE: return cglessequal(reg1, reg2);
            case A_GT: return cggreaterthan(reg1, reg2);
            case A_GE: return cggreaterequal(reg1, reg2);
            default:
                throw std::runtime_error("GenCode::generate: Unknown binary expression type");
        }
    } else if (auto x = std::dynamic_pointer_cast<UnaryExpNode>(ast)) {
        int reg = walkExpr(x->getExpr()); // Walk the expression in the unary node
        // Handle unary expression node
        if (x->getOp() == U_MINUS) {
            return cgneg(reg); // Load the integer value into a register
        } else if (x->getOp() == U_PLUS) {
            return reg;
        } else if (x->getOp() == U_NOT) {
            return cgnot(reg);
        } else  {
            throw std::runtime_error("GenCode::generate: Unknown unary expression type");
        }
    } else if (auto x = std::dynamic_pointer_cast<ValueNode>(ast)) {
        // Handle value node
        return cgload(x->getValue()); // Load the value into a register
    } else if (auto x = std::dynamic_pointer_cast<LValueNode>(ast)) {
        return cgloadglob(x->getIdentifier().name.c_str()); // Load the global variable into a register
    } else {
        throw std::runtime_error("GenCode::generate: Unknown expression node type");
    }
}

void GenCode::walkCondition(const std::shared_ptr<ExprNode>& ast, std::string false_label) {
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(ast)) {
        int reg1 = walkExpr(x->getLeft()); // Walk the left expression
        int reg2 = walkExpr(x->getRight()); // Walk the right expression
        switch (x->getOp()) {
            case A_EQ: return cgnotequaljump(reg1, reg2, false_label.c_str());
            case A_NE: return cgequaljump(reg1, reg2, false_label.c_str());
            case A_LT: return cggreaterequaljump(reg1, reg2, false_label.c_str());
            case A_LE: return cggreaterthanjump(reg1, reg2, false_label.c_str());
            case A_GT: return cglessequaljump(reg1, reg2, false_label.c_str());
            case A_GE: return cglessthanjump(reg1, reg2, false_label.c_str());
            case A_ADD: reg1 = cgadd(reg1, reg2); break;// Add the two registers and return the result
            case A_SUBTRACT: reg1 = cgsub(reg1, reg2); break;// Subtract the two registers and return the result
            case A_MULTIPLY: reg1 = cgmul(reg1, reg2); break;// Multiply the two registers and return the result
            case A_DIVIDE: reg1 = cgdiv(reg1, reg2); break;// Divide the two registers and return the result
        }
        reg2 = cgload(Value{ .type = P_INT, .ivalue = 0 }); // Load zero into a register
        return cgequaljump(reg1, reg2, false_label.c_str()); // Compare the result with zero and jump if equal
    } else {
        int reg1 = walkExpr(ast); // Walk the expression in the condition
        int reg2 = cgload(Value{ .type = P_INT, .ivalue = 0 }); // Load zero into a register
        return cgequaljump(reg1, reg2, false_label.c_str()); // Compare the result with zero and jump if equal
    }
}

int GenCode::walkStatement(const std::shared_ptr<StatementNode>& ast) {
    if (auto x = std::dynamic_pointer_cast<BlockNode>(ast)) {
        std::string block_label = labelAllocator.getLabel(LableType::BLOCK_LABEL);
        cglabel(block_label.c_str()); // Generate a label for the block
        for (const auto& stmt : x->getStatements()) {
            int reg = walkStatement(stmt); // Walk each statement in the statements node
            if (stmt->getStmtType() == S_ASSIGN) {
                freereg(reg);
            }
        }
        return 0;
    } else if (auto x  = std::dynamic_pointer_cast<PrintStatementNode>(ast)) {
        int reg = walkExpr(x->getExpression());
        cgprintint(reg);
        return 0;
    } else if (auto x = std::dynamic_pointer_cast<VariableDeclareNode>(ast)) {
        for (const auto& identifier: x->getIdentifiers()) {
            cgglobsym(identifier.c_str());
            auto initializer = x->getInitializer(identifier);
            if (initializer != nullptr) {
                int reg = walkExpr(initializer);
                cgstorglob(reg, identifier.c_str());
                regManager->freeRegister(reg);
            }
        }
        return 0;
    } else if (auto x = std::dynamic_pointer_cast<AssignmentNode>(ast)) {
        int reg = walkExpr(x->getExpr());
        cgstorglob(reg, x->getIdentifier().name.c_str());
        return reg;
    } else if (auto x = std::dynamic_pointer_cast<IfStatementNode>(ast)) {
        // 目前只考虑都是Block
        std::string if_label_no = labelAllocator.getLabel(LableType::IF_LABEL);
        std::string if_true = "IF_TRUE_" + if_label_no;
        std::string if_false = "IF_FALSE_" + if_label_no;
        std::string if_end = "IF_END_" + if_label_no;
        walkCondition(x->getCondition(), if_false); // Walk the condition and generate code for the jump
        cglabel(if_true.c_str());
        walkStatement(x->getThenStatement()); // Walk the then statement
        cgjump(if_end.c_str());
        cglabel(if_false.c_str());
        if (x->getElseStatement() != nullptr) {
            walkStatement(x->getElseStatement()); // Walk the else statement
        }
        cglabel(if_end.c_str()); // Generate the end label for the if statement
        return 0;
    } else if (auto x = std::dynamic_pointer_cast<WhileStatementNode>(ast)) {
        std::string while_label_no = labelAllocator.getLabel(LableType::WHILE_LABEL);
        std::string while_start = "WHILE_START_" + while_label_no;
        std::string while_end = "WHILE_END_" + while_label_no;
        cglabel(while_start.c_str()); // Generate the start label for the while loop
        walkCondition(x->getCondition(), while_end); // Walk the condition and generate code for the jump
        walkStatement(x->getBody()); // Walk the body of the while loop
        cgjump(while_start.c_str()); // Jump back to the start of the loop
        cglabel(while_end.c_str()); // Generate the end label for the while loop
        return 0;
    } else if (auto x = std::dynamic_pointer_cast<ForStatementNode>(ast)) {
        std::string for_label_no = labelAllocator.getLabel(LableType::FOR_LABEL);
        std::string for_start = "FOR_START_" + for_label_no;
        std::string for_end = "FOR_END_" + for_label_no;
        if (x->getPreopStatement() != nullptr) {
            walkStatement(x->getPreopStatement()); // Walk the pre-operation statement
        }
        cglabel(for_start.c_str()); // Generate the start label for the for loop
        walkCondition(x->getCondition(), for_end); // Walk the condition and generate code for the jump
        walkStatement(x->getBody()); // Walk the body of the for loop
        if (x->getPostopStatement() != nullptr) {
            walkStatement(x->getPostopStatement()); // Walk the post-operation statement
        }
        cgjump(for_start.c_str()); // Jump back to the start of the loop
        cglabel(for_end.c_str()); // Generate the end label for the for loop
        return 0;
    } else {
        throw std::runtime_error("GenCode::generate: Unknown statement node type");
    }
}

void GenCode::walkFunction(const std::shared_ptr<FunctionDeclareNode>& ast) {
    std::string func_name = ast->getIdentifier();
    cgfuncpreamble(func_name.c_str()); // Generate function preamble code
    walkStatement(ast->getBody()); // Walk the function body to generate code
    cgfuncpostamble();

}

int GenCode::walkAST(const std::shared_ptr<ASTNode>& ast) {
    if (auto x = std::dynamic_pointer_cast<VariableDeclareNode>(ast)) {
        return walkStatement(x);
    } if (auto x = std::dynamic_pointer_cast<FunctionDeclareNode>(ast)) {
        walkFunction(x);
        return 0;
    } else {
        throw std::runtime_error("GenCode::generate: Unknown AST node type");
    }
}

void GenCode::generate(const std::shared_ptr<ASTNode>& ast) {
    cgpreamble(); // Generate preamble code
    assert(walkAST(ast) == 0); // Walk the AST to generate code
    // cgpostamble(); // Generate postamble code
}
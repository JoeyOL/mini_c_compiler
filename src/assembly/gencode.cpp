#include "assembly/gencode.h"

int GenCode::walkExpr(const std::shared_ptr<ExprNode>& ast) { 
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(ast)) {
        // Handle binary expression node
        int reg1 = walkAST(x->getLeft());
        int reg2 = walkAST(x->getRight());
        switch (x->getType()) {
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
        int reg = walkAST(x->getExpr()); // Walk the expression in the unary node
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

int GenCode::walkStatement(const std::shared_ptr<StatementNode>& ast) {
    if (auto x = std::dynamic_pointer_cast<StatementsNode>(ast)) {
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
    } else {
        throw std::runtime_error("GenCode::generate: Unknown statement node type");
    }
}

int GenCode::walkAST(const std::shared_ptr<ASTNode>& ast) {
    if (auto x = std::dynamic_pointer_cast<ExprNode>(ast)) {
        return walkExpr(x);
    } else if (auto x  = std::dynamic_pointer_cast<StatementNode>(ast)) {
        return walkStatement(x);
    } else {
        throw std::runtime_error("GenCode::generate: Unknown AST node type");
    }
}

void GenCode::generate(const std::shared_ptr<ASTNode>& ast) {
    cgpreamble(); // Generate preamble code
    assert(walkAST(ast) == 0); // Walk the AST to generate code
    cgpostamble(); // Generate postamble code
}
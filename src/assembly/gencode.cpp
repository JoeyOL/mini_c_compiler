#include "assembly/gencode.h"

int GenCode::walkAST(const std::shared_ptr<ASTNode>& ast) {
    if (auto x = std::dynamic_pointer_cast<BinaryExpNode>(ast)) {
        // Handle binary expression node
        int reg1 = walkAST(x->getLeft());
        int reg2 = walkAST(x->getRight());
        switch (x->getType()) {
            case A_ADD: return cgadd(reg1, reg2); // Add the two registers and return the result
            case A_SUBTRACT: return cgsub(reg1, reg2); // Subtract the two registers and return the result
            case A_MULTIPLY: return cgmul(reg1, reg2); // Multiply the two registers and return the result
            case A_DIVIDE: return cgdiv(reg1, reg2); // Divide the two registers and return the result
            default:
                throw std::runtime_error("GenCode::generate: Unknown binary expression type");
        }
    } else if (auto y = std::dynamic_pointer_cast<UnaryExpNode>(ast)) {
        int reg = walkAST(y->getExpr()); // Walk the expression in the unary node
        // Handle unary expression node
        if (y->getOp() == U_MINUS) {
            return cgneg(reg); // Load the integer value into a register
        } else if (y->getOp() == U_PLUS) {
            return reg;
        } else {
            throw std::runtime_error("GenCode::generate: Unknown unary expression type");
        }
    } else if (auto z = std::dynamic_pointer_cast<ValueNode>(ast)) {
        // Handle value node
        return cgload(z->getValue()); // Load the value into a register
    } else {
        throw std::runtime_error("GenCode::generate: Unknown AST node type");
    }
}

void GenCode::generate(const std::shared_ptr<ASTNode>& ast) {
    cgpreamble(); // Generate preamble code
    int reg = walkAST(ast); // Walk the AST to generate code
    cgprintint(reg); // Print the integer value in the register
    cgpostamble(); // Generate postamble code
}
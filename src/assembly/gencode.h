#include "parser/parser.h"
#include <iostream>
#include <vector>
#include <memory>
#pragma once

class RegisterManager {
    public:
        RegisterManager() {
            allocated.resize(registers.size(), false); // Initialize all registers as free
        };
        ~RegisterManager() = default;

        // Allocate a register
        int allocateRegister() {
            for (int i = 0; i < registers.size(); ++i) {
                if (!allocated[i]) {
                    allocated[i] = true; // Mark the register as allocated
                    return i; // Return the index of the allocated register
                }
            }
            throw std::runtime_error("No free registers available");
        }

        // Free a register
        void freeRegister(int reg) {
            if (reg < 0 || reg >= registers.size()) {
                throw std::out_of_range("Register index out of range");
            }
            if (!allocated[reg]) {
                throw std::runtime_error("Register is not allocated");
            }
            allocated[reg] = false; // Mark the register as free
        }

        // Get the current register count
        void freeAllRegister() {
            for (int i = 0; i < allocated.size(); ++i) {
                allocated[i] = false; // Mark all registers as free
            }
        }

        std::string getRegister(int reg) const {
            if (reg < 0 || reg >= registers.size()) {
                throw std::out_of_range("Register index out of range");
            }
            return registers[reg];
        }

        std::string getRegisterLower8bit(int reg) const {
            if (reg < 0 || reg >= registers.size()) {
                throw std::out_of_range("Register index out of range");
            }
            return registers_lower8bit[reg];
        }

    private:
        const std::vector<std::string> registers = { "%r8", "%r9", "%r10", "%r11" };
        const std::vector<std::string> registers_lower8bit = { "%r8b", "%r9b", "%r10b", "%r11b" };
        std::vector<bool> allocated; // Track allocated registers
};

class GenCode {
    public:
        GenCode(std::string outputFileName)
            : regManager(std::make_unique<RegisterManager>()), outputFile(outputFileName, std::ios::out) {
            if (!outputFile.is_open()) {
                throw std::runtime_error("Could not open output file: " + outputFileName);
            }
        }
        ~GenCode() = default;

        // Generate code for the given AST node
        void generate(const std::shared_ptr<ASTNode>& ast);
    private:
        std::unique_ptr<RegisterManager> regManager; // Register manager to handle register allocation
        std::fstream outputFile; // Output file stream for generated code
        
        int cgload(Value value) {
            // Load the value into a register and return the register index
            int reg = regManager->allocateRegister();
            if (value.type == P_INT) {
                outputFile << "\tmovq\t$" << value.ivalue << ", " << regManager->getRegister(reg) << "\n";
            } else {
                throw std::runtime_error("GenCode::cgload: Only Support int value type for loading into register");
            }
            return reg;
        }

        int cgadd(int reg1, int reg2) {
            // Add the value in reg2 to reg1 and return the result in reg1
            outputFile << "\taddq\t" << regManager->getRegister(reg1) << ", " << regManager->getRegister(reg2) << "\n";
            regManager->freeRegister(reg1);
            return reg2; // Return the register containing the result
        }

        int cgsub(int reg1, int reg2) {
            // Subtract the value in reg2 from reg1 and return the result in reg1
            outputFile << "\tsubq\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
            regManager->freeRegister(reg2);
            return reg1; // Return the register containing the result
        }

        int cgneg(int reg) {
            // Negate the value in the specified register
            outputFile << "\tnegq\t" << regManager->getRegister(reg) << "\n";
            return reg; // Return the register containing the negated value
        }

        int cgmul(int reg1, int reg2) {
            // Multiply the value in reg1 by the value in reg2 and return the result in reg1
            outputFile << "\timulq\t" << regManager->getRegister(reg1) << ", " << regManager->getRegister(reg2) << "\n";
            regManager->freeRegister(reg1);
            return reg2; // Return the register containing the result
        }

        int cgdiv(int reg1, int reg2) {
            outputFile << "\tmovq\t" << regManager->getRegister(reg1) << ", %rax\n"; // Move reg1 to rax
            outputFile << "\tcqto\n"; // Sign-extend rax to rdx:rax
            outputFile << "\tidivq\t" << regManager->getRegister(reg2) << "\n"; // Divide rdx:rax by reg2
            outputFile << "\tmovq\t%rax, " << regManager->getRegister(reg1) << "\n"; // Move result back to reg1
            regManager->freeRegister(reg2);
            return reg1; // Return the register containing the result
        }

        void cgprintint(int reg) {
            // Print the integer value in the specified register
            outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", %rdi\n"; // Move value to rdi
            outputFile << "\tcall\tprintint\n"; 
            regManager->freeRegister(reg); // Free the register after use
        }

        void cgpreamble() {
            outputFile << 
              "\t.text\n" 
              ".LC0:\n"
              "\t.string\t\"%d\\n\"\n"
              "printint:\n"
              "\tpushq\t%rbp\n"
              "\tmovq\t%rsp, %rbp\n"
              "\tsubq\t$16, %rsp\n"
              "\tmovl\t%edi, -4(%rbp)\n"
              "\tmovl\t-4(%rbp), %eax\n"
              "\tmovl\t%eax, %esi\n"
              "\tleaq	.LC0(%rip), %rdi\n"
              "\tmovl	$0, %eax\n"
              "\tcall	printf@PLT\n"
              "\tnop\n"
              "\tleave\n"
              "\tret\n"
              "\n"
              "\t.globl\tmain\n"
              "\t.type\tmain, @function\n"
              "main:\n"
              "\tpushq\t%rbp\n"
              "\tmovq	%rsp, %rbp\n";
        }

        void cgpostamble() {
            outputFile << 
              "\tmovl\t$0, %eax\n" // Return 0
	          "\tpopq %rbp\n"
              "\tret\n";
            regManager->freeAllRegister(); // Free all registers at the end
        }

        int cgloadglob(const char *identifier) {
            // Load the value of the global variable into a register
            int reg = regManager->allocateRegister();
            outputFile <<
                "\tmovq\t" << identifier << "(%rip), " << regManager->getRegister(reg) << "\n";
            return reg; // Return the register containing the loaded value
        }

        int cgstorglob(int r, const char *identifier) {
            outputFile <<
                "\tmovq\t" << regManager->getRegister(r) << ", " << identifier << "(%rip)\n";
            return r;
        }

        void cgglobsym(const char *sym) {
            outputFile <<
                "\t.comm\t" << sym << ",8,8\n"; // Declare a global symbol
        }

        void freereg(int reg) {
            regManager->freeRegister(reg); // Free the specified register
        }

        int cgcompare(int r1, int r2, const char *op) {
            outputFile <<
                "\tcmpq\t" << regManager->getRegister(r2) << "," << regManager->getRegister(r1) << "\n"
                "\t" << op << "\t" << regManager->getRegisterLower8bit(r2) << "\n"
                "\tmovzbq\t" << regManager->getRegisterLower8bit(r2) << "," << regManager->getRegister(r2) << "\n"; 
            regManager->freeRegister(r1);
            return r2;
        }

        int cgequal(int r1, int r2) {
            return cgcompare(r1, r2, "sete");
        }

        int cgnotequal(int r1, int r2) {
            return cgcompare(r1, r2, "setne");
        }

        int cggreaterthan(int r1, int r2) {
            return cgcompare(r1, r2, "setg");
        }

        int cglessthan(int r1, int r2) {
            return cgcompare(r1, r2, "setl");
        }

        int cggreaterequal(int r1, int r2) {
            return cgcompare(r1, r2, "setge");
        }

        int cglessequal(int r1, int r2) {
            return cgcompare(r1, r2, "setle");
        }

        int cgnot(int r1) {
            int r2 = cgload(Value{ .type = P_INT, .ivalue = 0}); // Allocate a new register for the result
            return cgcompare(r1, r2, "sete");
        }

        void cglabel(const char *label) {
            outputFile << label << ":\n"; // Generate a label in the output file
        }

        void cgjump(const char* label) {
            outputFile << "\tjmp\t" << label << "\n"; // Generate an unconditional jump to the specified label
        }

        void cgequaljump(int r1, int r2, const char *label) {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                       << "\tje\t" << label << "\n"; // Generate a conditional jump if equal
            regManager->freeRegister(r1);
            regManager->freeRegister(r2);
        }
        void cgnotequaljump(int r1, int r2, const char *label) {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                       << "\tjne\t" << label << "\n"; // Generate a conditional jump if not equal
            regManager->freeRegister(r1);
            regManager->freeRegister(r2);
        }
        void cggreaterequaljump(int r1, int r2, const char *label) {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                       << "\tjge\t" << label << "\n"; // Generate a conditional jump if greater than or equal
            regManager->freeRegister(r1);
            regManager->freeRegister(r2);
        }
        void cglessequaljump(int r1, int r2, const char *label) {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                       << "\tjle\t" << label << "\n"; // Generate a conditional jump if less than or equal
            regManager->freeRegister(r1);
            regManager->freeRegister(r2);
        }
        void cglessthanjump(int r1, int r2, const char *label) {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                       << "\tjl\t" << label << "\n"; // Generate a conditional jump if less than
            regManager->freeRegister(r1);
            regManager->freeRegister(r2);
        }
        void cggreaterthanjump(int r1, int r2, const char *label) {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                       << "\tjg\t" << label << "\n"; // Generate a conditional jump if greater than
            regManager->freeRegister(r1);
            regManager->freeRegister(r2);
        }

        int walkAST(const std::shared_ptr<ASTNode>& ast);
        int walkStatement(const std::shared_ptr<StatementNode>& ast);
        int walkExpr(const std::shared_ptr<ExprNode>& ast);
        void walkCondition(const std::shared_ptr<ExprNode>& ast, std::string false_label);
};
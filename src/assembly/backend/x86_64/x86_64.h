#include "common/defs.h"
#include "assembly/backend/backend.h"
#pragma once
class X86RegisterManager: public RegisterManager {
    public:
        X86RegisterManager() {
            std::vector<bool> allocated_int(registers_int.size(), false);
            std::vector<bool> allocated_float(registers_float.size(), false);
            type_to_register_status[P_INT] = allocated_int;
            type_to_register_status[P_CHAR] = allocated_int; // Using int registers for char
            type_to_register_status[P_FLOAT] = allocated_float;
            type_to_register_status[P_LONG] = allocated_int; // Using int registers for long
            type_to_registers[P_INT] = registers_int;
            type_to_registers[P_CHAR] = registers_char;
            type_to_registers[P_FLOAT] = registers_float;
            type_to_registers[P_LONG] = registers_long;
        };
        ~X86RegisterManager() = default;

        // Allocate a register
        Reg allocateRegister(PrimitiveType type) override {
            auto& allocated = type_to_register_status.at(type);
            int idx = -1;
            for (int i = 0; i < allocated.size(); ++i) {
                if (!allocated[i]) { // Find a free register
                    allocated[i] = true; // Mark the register as allocated
                    idx = i;
                    break;
                }
            }
            if (type == P_INT || type == P_LONG || type == P_CHAR) {
                std::vector<PrimitiveType> valid_types = {P_INT, P_CHAR, P_LONG};
                for (const auto& valid_type : valid_types) {
                    auto& allocated_other = type_to_register_status.at(valid_type);
                    allocated_other[idx] = true;
                }
                return Reg{type, idx}; // Return the allocated register
            } else if (type == P_FLOAT) {
                allocated[idx] = true; // Mark the register as allocated
                return Reg{type, idx}; // Return the allocated register
            }
            throw std::runtime_error("No free registers available");
        }

        // Free a register
        void freeRegister(Reg reg) override {
            if (reg.type == P_INT || reg.type == P_LONG || reg.type == P_CHAR) {
                std::vector<PrimitiveType> valid_types = {P_INT, P_CHAR, P_LONG};
                for (const auto& valid_type : valid_types) {
                    auto& allocated = type_to_register_status.at(valid_type);
                    if (reg.idx < 0 || reg.idx >= allocated.size()) {
                        throw std::out_of_range("Register index out of range");
                    }
                    if (!allocated[reg.idx]) {
                        throw std::runtime_error("Register is not allocated");
                    }
                    allocated[reg.idx] = false; // Mark the register as free
                }
            } else if (reg.type == P_FLOAT) {
                auto& allocated = type_to_register_status.at(P_FLOAT);
                if (reg.idx < 0 || reg.idx >= allocated.size()) {
                    throw std::out_of_range("Register index out of range");
                }
                if (!allocated[reg.idx]) {
                    throw std::runtime_error("Register is not allocated");
                }
                allocated[reg.idx] = false; // Mark the register as free
            } else {
                throw std::runtime_error("Unsupported register type for freeing");
            }

        }

        // Get the current register count
        void freeAllRegister() override {
            for (auto& [type, allocated] : type_to_register_status) {
                for (int i = 0; i < allocated.size(); ++i) {
                    allocated[i] = false; // Mark all registers as free
                }
            }
        }

        std::string getRegister(Reg reg) const override {
            auto registers = type_to_registers.at(reg.type);
            auto& allocated = type_to_register_status.at(reg.type);
            if (reg.idx < 0 || reg.idx >= registers.size()) {
                throw std::out_of_range("Register index out of range");
            }
            if (!allocated[reg.idx]) {
                throw std::runtime_error("Register is not allocated");
            }
            return registers[reg.idx];
        }

    private:
        std::map<PrimitiveType, std::vector<std::string>> type_to_registers;
        std::map<PrimitiveType, std::vector<bool>> type_to_register_status;
        const std::vector<std::string> registers_int = { "%r8d", "%r9d", "%r10d", "%r11d" };
        const std::vector<std::string> registers_char = { "%r8b", "%r9b", "%r10b", "%r11b" };
        const std::vector<std::string> registers_long = { "%r8", "%r9", "%r10", "%r11" }; // Long registers for 64-bit operations
        const std::vector<std::string> registers_float = { "%xmm0", "%xmm1", "%xmm2", "%xmm3" }; // Floating-point registers
};


class X86AssemblyCode : public AssemblyCode {
public:
    X86AssemblyCode(const std::string &outputFileName): outputFile(outputFileName, std::ios::out),
      regManager(std::make_unique<X86RegisterManager>()) {
        if (!outputFile.is_open()) {
            throw std::runtime_error("Could not open output file: " + outputFileName);
        }
      }

    Reg cgload(Value value) override {
        // Load the value into a register and return the register index
        Reg reg = regManager->allocateRegister(value.type);
        if (value.type == P_INT || value.type == P_CHAR || value.type == P_LONG) {
            reg.type = P_LONG;
            outputFile << "\tmovq\t$" << value.ivalue << ", " << regManager->getRegister(reg) << "\n";
            reg.type = value.type; // Restore the original type
        } else if (value.type == P_FLOAT) {
            outputFile << "\tmovsd\t$" << value.fvalue << ", " << regManager->getRegister(reg) << "\n";
        } else {
            throw std::runtime_error("GenCode::cgload: Only Support int value type for loading into register");
        }
        return reg;
    }

    Reg cgadd(Reg reg1, Reg reg2) override {
        // Add the value in reg2 to reg1 and return the result in reg1
        if (reg1.type != reg2.type) {
            throw std::runtime_error("GenCode::cgadd: Registers must be of the same type");
        }
        PrimitiveType type = reg1.type;

        switch (reg1.type) {
            case P_INT:
            case P_CHAR:
            case P_LONG:
                reg1.type = P_LONG; reg2.type = P_LONG; 
                outputFile << "\taddq\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                reg1.type = type;
                break;
            case P_FLOAT:
                outputFile << "\taddsd\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            default:
                throw std::runtime_error("GenCode::cgadd: Unsupported register type for addition");
        }
        return reg1; // Return the register containing the result
    }

    Reg cgsub(Reg reg1, Reg reg2) override {
        if (reg1.type != reg2.type) {
            throw std::runtime_error("GenCode::cgadd: Registers must be of the same type");
        }
        PrimitiveType type = reg1.type;
        // Subtract the value in reg2 from reg1 and return the result in reg1
        switch (reg1.type) {
            case P_INT:
            case P_CHAR:
            case P_LONG:
                reg1.type = P_LONG; reg2.type = P_LONG; 
                outputFile << "\tsubq\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                reg1.type = type;
                break;
            case P_FLOAT:
                outputFile << "\tsubsd\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            default:
                throw std::runtime_error("GenCode::cgsub: Unsupported register type for subtraction");
        }
        return reg1; // Return the register containing the result
    }

    Reg cgneg(Reg reg) override {
        // Negate the value in the specified register
        PrimitiveType type = reg.type;

        switch (reg.type) {
            case P_INT:
            case P_LONG:
            case P_CHAR:
                reg.type = P_LONG; 
                outputFile << "\tnegq\t" << regManager->getRegister(reg) << "\n";
                reg.type = type;
                break;
            case P_FLOAT:
                outputFile << "\tnegsd\t" << regManager->getRegister(reg) << "\n";
            default:
                throw std::runtime_error("GenCode::cgneg: Unsupported register type for negation");
        }
       
        return reg; // Return the register containing the negated value
    }

    Reg cgmul(Reg reg1, Reg reg2) override {
        if (reg1.type != reg2.type) {
            throw std::runtime_error("GenCode::cgadd: Registers must be of the same type");
        }
        PrimitiveType type = reg1.type;

        switch (reg1.type) {
            case P_INT:
            case P_CHAR:
            case P_LONG:
                reg1.type = P_LONG; reg2.type = P_LONG; 
                outputFile << "\timulq\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                reg1.type = type;
                break;
            case P_FLOAT:
                outputFile << "\tmulsd\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            default:
                throw std::runtime_error("GenCode::cgmul: Unsupported register type for multiplication");
        }

        return reg2; // Return the register containing the result
    }

    Reg cgdiv(Reg reg1, Reg reg2) override {
        if (reg1.type != reg2.type) {
            throw std::runtime_error("GenCode::cgadd: Registers must be of the same type");
        }
        PrimitiveType type = reg1.type;
        switch (reg1.type) {
            case P_INT:
            case P_CHAR:
            case P_LONG:
                reg1.type = P_LONG; reg2.type = P_LONG;
                outputFile << "\tmovq\t" << regManager->getRegister(reg1) << ", %rax\n"; // Move reg1 to rax
                outputFile << "\tcqto\n"; // Sign-extend rax to rdx:rax
                outputFile << "\tidivq\t" << regManager->getRegister(reg2) << "\n"; // Divide rdx:rax by reg2
                outputFile << "\tmovq\t%rax, " << regManager->getRegister(reg1) << "\n"; // Move result back to reg1
                regManager->freeRegister(reg2);
                reg1.type = type; // Restore the original type
                break;
            case P_FLOAT:
                outputFile << "\tmovsd\t" << regManager->getRegister(reg1) << ", %xmm4\n"; // Move reg1 to xmm0
                outputFile << "\tmovsd\t" << regManager->getRegister(reg2) << ", %xmm5\n"; // Move reg2 to xmm1
                outputFile << "\tdivsd\t%xmm4, %xmm5\n"; // Divide xmm0 by xmm1
                outputFile << "\tmovsd\t%xmm4, " << regManager->getRegister(reg1) << "\n"; // Move result back to reg1
                regManager->freeRegister(reg2);
                break;
        }

        return reg1; // Return the register containing the result
    }

    void cgprintlong(Reg reg) override {
        // Print the integer value in the specified register
        outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", %rdi\n"; // Move value to rdi
        outputFile << "\tcall\tprintint\n"; 
        regManager->freeRegister(reg); // Free the register after use
    }

    void cgprintfloat(Reg reg) override {
        // Print the floating-point value in the specified register
        outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", %xmm4\n"; // Move value to xmm0
        outputFile << "\tcall\tprintfloat\n";
        regManager->freeRegister(reg); // Free the register after use
    }

    void cgpreamble() override {
        regManager->freeAllRegister(); // Free all registers at the start
        outputFile<< "\t.text\n"
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
          "\tcall	printf@PLT\n" "\tnop\n" "\tleave\n" "\tret\n" "\n"
          ".LC1:\n"
          "\t.string\t\"%f\\n\"\n"
          "printfloat:\n"
          "\tpushq\t%rbp\n"
          "\tmovq\t%rsp, %rbp\n"
          "\tsubq\t$16, %rsp\n"
          "\tmovsd\t%xmm4, -8(%rbp)\n"
          "\tmovsd\t-8(%rbp), %xmm4\n"
          "\tleaq\t.LC1(%rip), %rdi\n"
          "\tmovl\t$1, %eax\n"
          "\tcall\tprintf@PLT\n"
          "\tnop\n"
          "\tleave\n"
          "\tret\n";
      }

    void cgpostamble() override {
        outputFile << 
          "\tmovl\t$0, %eax\n" // Return 0
          "\tpopq %rbp\n"
          "\tret\n";
        regManager->freeAllRegister(); // Free all registers at the end
    }

    Reg cgloadglob(const char *identifier, PrimitiveType type) override {
        // Load the value of the global variable into a register
        Reg reg = regManager->allocateRegister(type);

        if (type == P_INT) {
            outputFile <<
                "\tmovl\t" << identifier << "(%rip), " << regManager->getRegister(reg) << "\n";
            reg.type = P_INT; // Set the type of the register to int
            return reg; // Return the register containing the loaded value
        } else if (type == P_CHAR) {
            reg.type = P_LONG;
            outputFile <<
                "\tmovzbq\t" << identifier << "(%rip), " << regManager->getRegister(reg) << "\n";
            reg.type = P_CHAR;
            return reg; // Return the register containing the loaded value
        } else if (type == P_FLOAT) {
            outputFile << 
                "\tmovsd\t" << identifier << "(%rip), " << regManager->getRegister(reg) << "\n";
            return reg; // Return the register containing the loaded value
        } else if (type == P_LONG) {
            outputFile << 
                "\tmovq\t" << identifier << "(%rip), " << regManager->getRegister(reg) << "\n";
            reg.type = P_LONG;
            return reg;
        } 
    }

    Reg cgstorglob(Reg r, const char *identifier, PrimitiveType type) override {
        if (type == P_INT) {
            outputFile <<
            "\tmovl\t" << regManager->getRegister(r) << ", " << identifier << "(%rip)\n";
            return r;
        } else if (type == P_CHAR) {
            outputFile <<
            "\tmovb\t" << regManager->getRegister(r) << ", " << identifier << "(%rip)\n";
            return r;
        } else if (type == P_FLOAT) {
            outputFile <<
            "\tmovsd\t" << regManager->getRegister(r) << ", " << identifier << "(%rip)\n";
            return r;
        } else if (type == P_LONG) {
            outputFile <<
            "\tmovq\t" << regManager->getRegister(r) << ", " << identifier << "(%rip)\n";
            return r; // Return the register containing the stored value
        } else {
            throw std::runtime_error("GenCode::cgstorglob: Unsupported type for storing global variable");
        }

    }

    void cgglobsym(Symbol sym) override {
        outputFile <<
            "\t.comm\t" << sym.name << "," << sym.size << "," << sym.size << "\n"; // Declare a global symbol
    }

    void freereg(Reg reg) override {
        regManager->freeRegister(reg); // Free the specified register
    }

    Reg cgcompare(Reg r1, Reg r2, const char *op) override {
        if (r1.type != r2.type) {
            throw std::runtime_error("GenCode::cgcompare: Registers must be of the same type for comparison");
        }
        Reg r3;
        switch (r1.type) {
            case P_LONG:

                outputFile <<
                    "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    "\t" << op << "\t" << regManager->getRegister(r2) << "\n"
                    "\tmovzbq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r2) << "\n"; 
                regManager->freeRegister(r1);
                return r2; // Return the register containing the result

            case P_CHAR:
            case P_INT:
                r3 = regManager->allocateRegister(P_LONG); // Allocate a new register for the result
                outputFile <<
                    "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    "\t" << op << "\t" << regManager->getRegister(r3) << "\n"
                    "\tmovzbq\t" << regManager->getRegister(r3) << ", " << regManager->getRegister(r3) << "\n"; 
                regManager->freeRegister(r1);
                regManager->freeRegister(r2);
                return r3; // Return the register containing the result
            case P_FLOAT:
                // 用整型寄存器存储结果
                r3 = regManager->allocateRegister(P_LONG);
                outputFile << 
                    "\tucomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    "\t" << op << "\t%al" << "\n"
                    << "\tmovzbq\t%al, " << r3.idx << "\n";
                regManager->freeRegister(r1);
                regManager->freeRegister(r2);
                return r3; // Return the register containing the result
            default:
                throw std::runtime_error("GenCode::cgcompare: Unsupported type for comparison");
        }
        return Reg{.type = P_NONE, .idx = 0}; // Return an invalid register if comparison fails
    }

    Reg cgequal(Reg r1, Reg r2) override {
        return cgcompare(r1, r2, "sete");
    }

    Reg cgnotequal(Reg r1, Reg r2) override {
        return cgcompare(r1, r2, "setne");
    }

    Reg cggreaterthan(Reg r1, Reg r2) override {
        return cgcompare(r1, r2, "setg");
    }

    Reg cglessthan(Reg r1, Reg r2) override {
        return cgcompare(r1, r2, "setl");
    }

    Reg cggreaterequal(Reg r1, Reg r2) override {
        return cgcompare(r1, r2, "setge");
    }

    Reg cglessequal(Reg r1, Reg r2) override {
        return cgcompare(r1, r2, "setle");
    }

    Reg cgnot(Reg r1) override {
        if (r1.type == P_INT) {
            r1 = cgfloat2int(r1);
        }
        Reg r2 = cgload(Value{ .type = P_INT, .ivalue = 0}); // Allocate a new register for the result
        return cgcompare(r1, r2, "sete");
    }

    void cglabel(const char *label) override {
        outputFile << label << ":\n"; // Generate a label in the output file
    }

    void cgjump(const char* label) override {
        outputFile << "\tjmp\t" << label << "\n"; // Generate an unconditional jump to the specified label
    }

    void cgequaljump(Reg r1, Reg r2, const char *label) override {
        if (r1.type == P_FLOAT) {
            outputFile << "\tcomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                      << "\tje\t" << label << "\n";
        } else {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    << "\tje\t" << label << "\n"; // Generate a conditional jump if equal
        }

        regManager->freeRegister(r1);
        regManager->freeRegister(r2);
    }
    void cgnotequaljump(Reg r1, Reg r2, const char *label) override {
        if (r1.type == P_FLOAT) {
            outputFile << "\tcomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                      << "\tjne\t" << label << "\n";
        } else {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    << "\tjne\t" << label << "\n"; // Generate a conditional jump if not equal
        }

        regManager->freeRegister(r1);
        regManager->freeRegister(r2);
    }
    void cggreaterequaljump(Reg r1, Reg r2, const char *label) override {
        if (r1.type == P_FLOAT) {
            outputFile << "\tcomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                      << "\tjae\t" << label << "\n";
        } else {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    << "\tjge\t" << label << "\n"; // Generate a conditional jump if greater than or equal
        }

        regManager->freeRegister(r1);
        regManager->freeRegister(r2);
    }
    void cglessequaljump(Reg r1, Reg r2, const char *label) override {
        if (r1.type == P_FLOAT) {
            outputFile << "\tcomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                      << "\tjbe\t" << label << "\n";
        } else {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                     << "\tjle\t" << label << "\n"; // Generate a conditional jump if less than or equal
        }

        regManager->freeRegister(r1);
        regManager->freeRegister(r2);
    }
    void cglessthanjump(Reg r1, Reg r2, const char *label) override {
        if (r1.type == P_FLOAT) {
            outputFile << "\tcomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                      << "\tjb\t" << label << "\n";
        } else {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    << "\tjl\t" << label << "\n"; // Generate a conditional jump if less than
        }

        regManager->freeRegister(r1);
        regManager->freeRegister(r2);
    }
    void cggreaterthanjump(Reg r1, Reg r2, const char *label) override {
        if (r1.type == P_FLOAT) {
            outputFile << "\tcomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                      << "\tja\t" << label << "\n";
        } else {
            outputFile << "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    << "\tjg\t" << label << "\n"; // Generate a conditional jump if greater than
        }

        regManager->freeRegister(r1);
        regManager->freeRegister(r2);
    }

    void cgfuncpreamble(const char *name) override {
        outputFile << 
            "\t.text\n"
            "\t.globl\t" << name << "\n"
            "\t.type\t" << name << ", @function\n"
            << name << ":\n"
            "\tpushq\t%rbp\n"
            "\tmovq\t%rsp, %rbp\n";
    }

    void cgfuncpostamble() override {
        outputFile << 
            "\tmovl\t$0, %eax\n" // Return 0
            "\tpopq\t%rbp\n"
            "\tret\n";
        // regManager->freeAllRegister(); // Free all registers at the end of the function
    }

    Reg cgint2char(Reg reg) override {
        reg.type = P_CHAR; // Update the register type to character
        return reg; // Return the register containing the character
    }

    Reg cgchar2int(Reg reg) override {
        // Convert the character in the specified register to an integer
        outputFile << "\tmovzbl\t" << regManager->getRegister(reg) << ", " ;
        reg.type = P_INT;
        outputFile << regManager->getRegister(reg) << "\n"; // Zero-extend to int
        return reg; // Return the register containing the integer
    }

    Reg cgfloat2int(Reg reg) override {
        // Convert the float in the specified register to an integer
        Reg r2 = regManager->allocateRegister(P_INT); // Allocate a new register for the result
        outputFile << "\tcvttsd2si\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(r2) << "\n"; // Convert float to int
        regManager->freeRegister(reg); // Free the original float register
        return r2;
    }

    Reg cgint2float(Reg reg) override { 
        Reg r2 = regManager->allocateRegister(P_FLOAT);
        outputFile << "\tcvtsi2sd\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(r2) << "\n";
        regManager->freeRegister(reg);
        return r2;
    }

    Reg cgfloat2char(Reg reg) override {
        reg = cgfloat2int(reg); // Convert float to int first
        reg = cgint2char(reg); // Then convert int to char
        return reg; // Return the register containing the character
    }

    Reg cgchar2float(Reg reg) override { 
        reg = cgchar2int(reg); // Convert char to int first
        reg = cgint2float(reg); // Then convert int to float
        return reg; // Return the register containing the float
    }

    Reg cgint2long(Reg reg) override {
        // Convert the integer in the specified register to a long
        outputFile << "\tmovslq\t" << regManager->getRegister(reg) << ", " ;
        reg.type = P_LONG; // Update the register type to long
        outputFile << regManager->getRegister(reg) << "\n"; // Move int to long
        return reg; // Return the register containing the long
    }

    Reg cglong2int(Reg reg) override {
        reg.type = P_INT; // Update the register type to integer
        return reg; // Return the register containing the integer
    }

    Reg cgfloat2long(Reg reg) override {
        // Convert the float in the specified register to a long
        Reg r2 = regManager->allocateRegister(P_LONG); // Allocate a new register for the result
        outputFile << "\tcvttsd2si\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(r2) << "\n"; // Convert float to long
        regManager->freeRegister(reg); // Free the original float register
        return r2;
    }

    Reg cglong2float(Reg reg) override {
        // Convert the long in the specified register to a float
        Reg r2 = regManager->allocateRegister(P_FLOAT); // Allocate a new register for the result
        outputFile << "\tcvtsi2sd\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(r2) << "\n"; // Convert long to float
        regManager->freeRegister(reg); // Free the original long register
        return r2;
    }

    Reg cglong2char(Reg reg) override {
        reg.type = P_CHAR; // Update the register type to character
        return reg; // Return the register containing the character
    }

    Reg cgchar2long(Reg reg) override {
        // Convert the character in the specified register to a long
        outputFile << "\tmovzbq\t" << regManager->getRegister(reg) << ", " ;
        reg.type = P_LONG; // Update the register type to long
        outputFile << regManager->getRegister(reg) << "\n"; // Zero-extend to long
        return reg; // Return the register containing the long
    }

private:
    std::unique_ptr<X86RegisterManager> regManager; // Register manager for handling register allocation
    std::ofstream outputFile; // Output file stream for writing assembly code
};
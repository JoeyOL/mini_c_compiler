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
        
        std::string getRegisterLower8bit(Reg reg) const {
            reg.type = P_CHAR; // Treat as char for lower 8-bit register
            return getRegister(reg);
        }

    private:
        std::map<PrimitiveType, std::vector<std::string>> type_to_registers;
        std::map<PrimitiveType, std::vector<bool>> type_to_register_status;
        const std::vector<std::string> registers_int = { "%r8d", "%r9d", "%r10d", "%r11d" };
        const std::vector<std::string> registers_char = { "%r8b", "%r9b", "%r10b", "%r11b" };
        const std::vector<std::string> registers_long = { "%r8", "%r9", "%r10", "%r11" }; // Long registers for 64-bit operations
        const std::vector<std::string> registers_float = { "%xmm1", "%xmm2", "%xmm3", "%xmm4" }; // Floating-point registers
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
        PrimitiveType type = value.type == P_STRING ? P_LONG : value.type; // Use P_CHARPTR for string values
        Reg reg = regManager->allocateRegister(type);
        if (value.type == P_INT || value.type == P_CHAR) {
            reg.type = P_INT;
            outputFile << "\tmovl\t$" << value.ivalue << ", " << regManager->getRegister(reg) << "\n";
            reg.type = value.type; // Restore the original type
        } else if (value.type == P_FLOAT) {
            outputFile << "\tmovsd\t" << float_constants[value.fvalue] << "(%rip)" << ", " << regManager->getRegister(reg) << "\n";
        } else if (value.type == P_LONG) {
            reg.type = P_LONG;
            outputFile << "\tmovq\t$" << value.lvalue << ", " << regManager->getRegister(reg) << "\n";
            reg.type = value.type; // Restore the original type
        } else if (value.type == P_STRING) {
            std::string label = string_constants[value.strvalue];
            outputFile << "\tleaq\t" << label << "(%rip), " << regManager->getRegister(reg) << "\n";
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

        return reg1; // Return the register containing the result
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
                outputFile << "\tdivsd\t" << regManager->getRegister(reg2) << "," << regManager->getRegister(reg1) << "\n"; // Divide xmm0 by xmm1
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
        outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", %xmm0\n"; // Move value to xmm0
        outputFile << "\tcall\tprintfloat\n";
        regManager->freeRegister(reg); // Free the register after use
    }

    void cgfloatconst() {
        outputFile << ".section\t.data\n";
        for (auto & [value, name] : float_constants) {
            outputFile << name << ":\n";
            outputFile << "\t.double\t" << value << "\n"; // Store float as int
        }
    }
    void cgstringconst() {
        outputFile << ".section\t.data\n";
        for (auto & [value, name] : string_constants) {
            outputFile << name << ":\n";
            outputFile << "\t.string\t\"" << value << "\"\n"; // Store string as a null-terminated string
        }
    }
    void cgpreamble() override {
        cgfloatconst(); // Generate float constants section
        cgstringconst(); // Generate string constants section
        regManager->freeAllRegister(); // Free all registers at the start
        // outputFile<< "\t.text\n"
        //     ".LC0:\n"
        //     "\t.string\t\"%d\\n\"\n"
        //     "printint:\n"
        //     "\tpushq\t%rbp\n"
        //     "\tmovq\t%rsp, %rbp\n"
        //     "\tsubq\t$16, %rsp\n"
        //     "\tmovl\t%edi, -4(%rbp)\n"
        //     "\tmovl\t-4(%rbp), %eax\n"
        //     "\tmovl\t%eax, %esi\n"
        //     "\tleaq	.LC0(%rip), %rdi\n"
        //     "\tmovl	$0, %eax\n"
        //     "\tcall	printf@PLT\n" "\tnop\n" "\tleave\n" "\tret\n" "\n"
        //     ".LC1:\n"
        //     "\t.string\t\"%f\\n\"\n"
        //     "printfloat:\n"
        //     "\tpushq\t%rbp\n"
        //     "\tmovq\t%rsp, %rbp\n"
        //     "\tsubq\t$16, %rsp\n"
        //     "\tmovsd\t%xmm0, -8(%rbp)\n"
        //     "\tmovsd\t-8(%rbp), %xmm0\n"
        //     "\tleaq\t.LC1(%rip), %rdi\n"
        //     "\tmovl\t$1, %eax\n"
        //     "\tcall\tprintf@PLT\n"
        //     "\tnop\n"
        //     "\tleave\n"
        //     "\tret\n"
        //     ".LC2:\n"
        //     "\t.string\t\"%ld\\n\"\n"
        //     "printlong:\n"
        //     "\tpushq\t%rbp\n"
        //     "\tmovq\t%rsp, %rbp\n"
        //     "\tsubq\t$16, %rsp\n"
        //     "\tmovq\t%rdi, -8(%rbp)\n"
        //     "\tmovq\t-8(%rbp), %rax\n"
        //     "\tmovq\t%rax, %xmm0\n"
        //     "\tmovl\t$2, %eax\n"
        //     "\tmovq\t.LC2(%rip), %rsi\n"
        //     "\tmovq\t%rax, %rdi\n"
        //     "\tcall\tprintf@PLT\n"
        //     "\tnop\n"
        //     "\tleave\n"
        //     "\tret\n";
      }

    void cgpostamble() override {
        outputFile << 
          "\tmovl\t$0, %eax\n" // Return 0
          "\tpopq %rbp\n"
          "\tret\n";
        regManager->freeAllRegister(); // Free all registers at the end
    }

    Reg cgloadsym(Symbol identifier, PrimitiveType type) override {
        if (is_pointer(type)) type = P_LONG; // Treat pointers as long for loading
        // Load the value of the global variable into a register
        Reg reg = regManager->allocateRegister(type);
        std::string addr = identifier.getAddress(); // Get the address of the global variable
        if (type == P_INT) {
            outputFile <<
                "\tmovl\t" << addr << ", " << regManager->getRegister(reg) << "\n";
            reg.type = P_INT; // Set the type of the register to int
            return reg; // Return the register containing the loaded value
        } else if (type == P_CHAR) {
            reg.type = P_LONG;
            outputFile <<
                "\tmovzbq\t" << addr << ", " << regManager->getRegister(reg) << "\n";
            reg.type = P_CHAR;
            return reg; // Return the register containing the loaded value
        } else if (type == P_FLOAT) {
            outputFile << 
                "\tmovsd\t" << addr << ", " << regManager->getRegister(reg) << "\n";
            return reg; // Return the register containing the loaded value
        } else if (type == P_LONG) {
            outputFile << 
                "\tmovq\t" << addr << ", " << regManager->getRegister(reg) << "\n";
            reg.type = P_LONG;
            return reg;
        } 
    }

    Reg cgstorsym(Reg r, Symbol identifer, PrimitiveType type) override {
        type = is_pointer(type) ? P_LONG : type; // Treat pointers as long for storing
        std::string addr = identifer.getAddress();
        if (type == P_INT) {
            outputFile <<
            "\tmovl\t" << regManager->getRegister(r) << ", " << addr << "\n";
            return r;
        } else if (type == P_CHAR) {
            outputFile <<
            "\tmovb\t" << regManager->getRegister(r) << ", " << addr << "\n";
            return r;
        } else if (type == P_FLOAT) {
            outputFile <<
            "\tmovsd\t" << regManager->getRegister(r) << ", " << addr << "\n";
            return r;
        } else if (type == P_LONG) {
            outputFile <<
            "\tmovq\t" << regManager->getRegister(r) << ", " << addr << "\n";
            return r; // Return the register containing the stored value
        } else {
            throw std::runtime_error("GenCode::cgstorglob: Unsupported type for storing global variable");
        }

    }

    void cglocalsym(Symbol sym) override {
        // outputFile << 
        //     "\taddq\t$" << sym.size << ", %rsp\n" // Adjust stack pointer for local variable
        return;
    }

    void cgglobsym(Symbol sym, std::shared_ptr<ArrayInitializer> init = nullptr) override {
        outputFile <<
            "\t.data\n\t.globl\t" <<sym.name << "\n" << sym.name << ":\t" ; // Declare a global symbol
        switch (sym.type) {
            case P_INT:
                outputFile << "\t.long\t0\n"; // Initialize int global variable to 0
                break;
            case P_CHAR:
                outputFile << "\t.byte\t0\n"; // Initialize char global variable to 0
                break;
            case P_FLOAT:
                outputFile << "\t.double\t0.0\n"; // Initialize float global variable to 0.0
                break;
            case P_LONG:
                outputFile << "\t.quad\t0\n"; // Initialize long global variable to 0
                break;
            case P_INTPTR : case P_FLOATPTR : case P_CHARPTR : case P_LONGPTR : case P_VOIDPTR:
                outputFile << "\t.quad\t0\n"; // Initialize pointer global variable to 0
                break;
            case P_INTARR : case P_FLOATARR : case P_CHARARR : case P_LONGARR:
                if (init != nullptr) cgglobarray(sym, init);
                else {
                    outputFile << "\t.zero\t" << sym.size << "\n"; // Initialize array global variable to 0
                }
                break;
            default:
                throw std::runtime_error("GenCode::cgglobsym: Unsupported type for global symbol");
        }
       
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
                    "\t" << op << "\t" << regManager->getRegisterLower8bit(r2) << "\n"
                    "\tmovzbq\t" << regManager->getRegisterLower8bit(r2) << ", " << regManager->getRegister(r2) << "\n"; 
                regManager->freeRegister(r1);
                return r2; // Return the register containing the result

            case P_CHAR:
            case P_INT:
                r3 = regManager->allocateRegister(P_LONG); // Allocate a new register for the result
                outputFile <<
                    "\tcmpq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    "\t" << op << "\t" << regManager->getRegisterLower8bit(r3) << "\n"
                    "\tmovzbq\t" << regManager->getRegisterLower8bit(r3) << ", " << regManager->getRegister(r3) << "\n"; 
                regManager->freeRegister(r1);
                regManager->freeRegister(r2);
                return r3; // Return the register containing the result
            case P_FLOAT:
                // 用整型寄存器存储结果
                r3 = regManager->allocateRegister(P_LONG);
                outputFile << 
                    "\tucomisd\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"
                    "\t" << op << "\t" << regManager->getRegisterLower8bit(r3)  << "\n"
                    << "\tmovzbq "<< "\t" << regManager->getRegisterLower8bit(r3) << ", " << regManager->getRegister(r3) << "\n";
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
        if (r1.type != P_FLOAT) return cgcompare(r1, r2, "setg");
        else return cgcompare(r1, r2, "seta");
    }

    Reg cglessthan(Reg r1, Reg r2) override {
        if (r1.type != P_FLOAT) return cgcompare(r1, r2, "setl");
        else return cgcompare(r1, r2, "setb");
    }

    Reg cggreaterequal(Reg r1, Reg r2) override {
        if (r1.type != P_FLOAT) return cgcompare(r1, r2, "setge");
        else return cgcompare(r1, r2, "setae");
    }

    Reg cglessequal(Reg r1, Reg r2) override {
        if (r1.type != P_FLOAT) return cgcompare(r1, r2, "setle");
        else return cgcompare(r1, r2, "setbe");
    }

    Reg cgnot(Reg r1) override {
        if (r1.type == P_FLOAT) {
            r1 = cgfloat2long(r1);
        } else r1.type = P_LONG; // Ensure the register is treated as long for comparison
        Reg r2 = cgload(Value{ .type = P_LONG, .ivalue = 0}); // Allocate a new register for the result
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

    void cgfuncpreamble(Function func) override {
        outputFile << 
            "\t.text\n"
            "\t.globl\t" << func.name << "\n"
            "\t.type\t" << func.name << ", @function\n"
            << func.name << ":\n"
            "\tpushq\t%rbp\n"
            "\tmovq\t%rsp, %rbp\n"
            "\tsubq\t$" << func.stack_size << ", %rsp\n"; // Adjust stack pointer for local variables
        
    }

    void cgfuncpostamble(Function func, const char *label) override {
        cglabel(label);
        outputFile << 
            "\taddq\t$" << func.stack_size << ", %rsp\n"; // Restore stack pointer
        outputFile << 
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

    Reg cgcall(const char *name, const Reg reg) override {
        if (reg.idx != -1) {
            if (reg.type != P_FLOAT) {
                Reg r1 = Reg{.type = P_LONG, .idx = reg.idx}; // Create a temporary register for the function call
                outputFile << "\tmovq\t" << regManager->getRegister(r1) << ", %rdi\n"; // Move the register value to rdi for the function call
            } else {
                outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", %xmm0\n"; // Move the float value to xmm0 for the function call
            }
        }
        Reg out = regManager->allocateRegister(reg.type);
        Reg r1 = Reg{.type = P_LONG, .idx = out.idx}; // Create a temporary register for the function call
        outputFile << "\tcall\t" << name << "\n"; // Call the specified function
        if (reg.type == P_FLOAT) {
            outputFile << "\tmovsd\t%xmm0, " << regManager->getRegister(out) << "\n";

        }
        else outputFile << "\tmovq\t%rax, " << regManager->getRegister(r1) << "\n";
        if (reg.idx != -1) regManager->freeRegister(reg); // Free the original register after the call
        return out;
    }

    void cgreturn(const Reg reg, const char *end_label) override {
        switch (reg.type) {
            case P_CHAR:
                outputFile << "\tmovb\t" << regManager->getRegister(reg) << ", %eax\n";
                break;
            case P_INT:
                outputFile << "\tmovl\t" << regManager->getRegister(reg) << ", %eax\n";
                break;
            case P_LONG:
                outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", %rax\n";
                break;
            case P_FLOAT:
                outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", %xmm0\n";
                break;
        }
        regManager->freeRegister(reg); // Free the register after use
        cgjump(end_label);
    }

    Reg cgaddress(Symbol identifier) override {
        Reg reg = regManager->allocateRegister(P_LONG); // Allocate a register for the address
        outputFile << "\tleaq\t" << identifier.getAddress() << ", " << regManager->getRegister(reg) << "\n"; // Load the address into the register
        return reg; // Return the register containing the address
    }

    Reg cgderef(Reg reg, PrimitiveType type) override {
        Reg float_reg;
        switch (type) {
            case P_INTPTR: case P_INTARR :
                outputFile << "\tmovq\t(" << regManager->getRegister(reg) << ")" << ", " << regManager->getRegister(reg) << "\n"; // Move the value at the address in reg to eax
                reg.type = P_INT; // Update the register type to int
                break;
            case P_CHARPTR: case P_CHARARR:
                outputFile << "\tmovzbq\t(" << regManager->getRegister(reg) << ")" << ", " << regManager->getRegister(reg) << "\n"; // Move the byte at the address in reg to al
                reg.type = P_CHAR; // Update the register type to char
                break;
            case P_FLOATPTR: case P_FLOATARR:
                float_reg = regManager->allocateRegister(P_FLOAT); // Allocate a new register for the float value
                outputFile << "\tmovsd\t(" << regManager->getRegister(reg) << ")" << ", " << regManager->getRegister(float_reg) << "\n";; // Move the double at the address in reg to xmm0
                regManager->freeRegister(reg); // Free the original register
                reg = float_reg;// Update the register type to float
                break;
            case P_LONGPTR: case P_LONGARR:
                outputFile << "\tmovq\t(" << regManager->getRegister(reg) << ")" << ", " << regManager->getRegister(reg) << "\n"; // Move the value at the address in reg to rax
                reg.type = P_LONG; // Update the register type to long
                break;
            default:
                throw std::runtime_error("GenCode::cgderef: Unsupported type for dereferencing");
        }
        return reg; // Return the register containing the dereferenced value
    }

    Reg cgshlconst(Reg reg, int value) override {
        // Shift the value in the specified register left by the given constant
        if (reg.type == P_INT || reg.type == P_LONG || reg.type == P_CHAR) {
            outputFile << "\tsalq\t$" << value << ", " << regManager->getRegister(reg) << "\n"; // Shift left
        } else {
            throw std::runtime_error("GenCode::cgshlconst: Unsupported register type for shift left");
        }
        return reg; // Return the register containing the shifted value
    }
    

    Reg cgstorderef(Reg reg, Reg addr, PrimitiveType type) override {
        // Store the value in the specified register to the address pointed by identifier
        if (type == P_INT) {
            outputFile << "\tmovl\t" << regManager->getRegister(reg) << ", (" << regManager->getRegister(addr) << ")\n"; // Store int value
        } else if (type == P_CHAR) {
            outputFile << "\tmovb\t" << regManager->getRegister(reg) << ", (" << regManager->getRegister(addr) << ")\n"; // Store char value
        } else if (type == P_FLOAT) {
            outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", (" << regManager->getRegister(addr) << ")\n"; // Store float value
        } else if (type == P_LONG) {
            outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", (" << regManager->getRegister(addr) << ")\n"; // Store long value
        } else {
            throw std::runtime_error("GenCode::cgstorderef: Unsupported type for storing dereferenced value");
        }
        regManager->freeRegister(addr); // Free the address register after use
        return reg; // Return the register containing the stored value
    }

    void cgglobarray(Symbol sym, std::shared_ptr<ArrayInitializer> init) {
        PrimitiveType type = init->getPrimitiveType();
        for (auto &elem: init->getElements()) {
            if (auto x = std::dynamic_pointer_cast<ValueNode>(elem)) {
                if (type == P_INT) {
                    outputFile << "\t.long\t" << x->getIntValue() << "\n"; // Store int value
                } else if (type == P_CHAR) {
                    outputFile << "\t.byte\t" << x->getCharValue() << "\n"; // Store char value
                } else if (type== P_FLOAT) {
                    outputFile << "\t.double\t" << x->getFloatValue() << "\n"; // Store float value
                } else if (type == P_LONG) {
                    outputFile << "\t.quad\t" << x->getLongValue() << "\n"; // Store long value
                } else {
                    throw std::runtime_error("GenCode::cgglobarray: Unsupported type for global array element");
                }
            } else if (auto x = std::dynamic_pointer_cast<ArrayInitializer>(elem)) {
                cgglobarray(sym, x); // Recursively handle nested array initializers
            } else {
                throw std::runtime_error("GenCode::cgglobarray: Unsupported element type in array initializer");
            }
        }
        if (init->getLeftSize()) {
            outputFile << "\t.zero\t" << init->getLeftSize() * symbol_table.typeToSize(type) << "\n"; // Allocate space for the array if specified
        }
    }

    void cginc(Symbol identifier, PrimitiveType type) override {
        // Increment the value of the global variable by 1
        if (type == P_INT) {
            outputFile << "\tincb\t" << identifier.getAddress() << "\n"; // Increment int value
        } else if (type == P_CHAR) {
            outputFile << "\tincl\t" << identifier.getAddress() << "\n"; // Increment char value
        } else if (type == P_LONG || type == P_CHARPTR) {
            outputFile << "\tincq\t" << identifier.getAddress() << "\n"; // Increment long value
        } else if (type == P_FLOAT) {
            Reg r1 = cgload(Value{ .type = P_FLOAT, .fvalue = 1.0f }); // Load float constant 1.0
            Reg r2 = regManager->allocateRegister(P_FLOAT); // Allocate a register for the float value
            outputFile << "\tmovsd\t" << identifier.getAddress() << ", " << regManager->getRegister(r2) << "\n"; // Load the float value from the global variable
            outputFile << "\taddsd\t" << regManager->getRegister(r1) << ", " << regManager->getRegister(r2) << "\n"; // Add float value
            outputFile << "\tmovsd\t" << regManager->getRegister(r2) << ", " << identifier.getAddress() << "\n"; // Store the incremented value back to the global variable
            regManager->freeRegister(r1); // Free the register used for the float constant
            regManager->freeRegister(r2); // Free the register used for the float value
        } else if (type == P_INTPTR) {
            outputFile << "\taddq\t$4, " << identifier.getAddress() << "\n"; // Increment pointer by 4 bytes
        } else if (type == P_FLOATPTR || type == P_LONGPTR) {
            outputFile << "\taddq\t$8, " << identifier.getAddress() << "\n";
        } else {
            throw std::runtime_error("GenCode::cginc: Unsupported type for incrementing global variable");
        }
    }

    void cgdec(Symbol identifier, PrimitiveType type) override {
        // Decrement the value of the global variable by 1
        if (type == P_INT) {
            outputFile << "\tdecb\t" << identifier.getAddress() << "\n"; // Decrement int value
        } else if (type == P_CHAR) {
            outputFile << "\tdecl\t" << identifier.getAddress() << "\n"; // Decrement char value
        } else if (type == P_LONG || type == P_CHARPTR) {
            outputFile << "\tdecq\t" << identifier.getAddress() << "\n"; // Decrement long value
        } else if (type == P_FLOAT) {
            Reg r1 = cgload(Value{ .type = P_FLOAT, .fvalue = 1.0f }); // Load float constant 1.0
            Reg r2 = regManager->allocateRegister(P_FLOAT); // Allocate a register for the float value
            outputFile << "\tmovsd\t" << identifier.getAddress() << ", " << regManager->getRegister(r2) << "\n"; // Load the float value from the global variable
            outputFile << "\tsubsd\t" << regManager->getRegister(r1) << ", " << regManager->getRegister(r2) << "\n"; // Subtract float value
            outputFile << "\tmovsd\t" << regManager->getRegister(r2) << ", " << identifier.getAddress() << "\n"; // Store the decremented value back to the global variable
            regManager->freeRegister(r1); // Free the register used for the float constant
            regManager->freeRegister(r2); // Free the register used for the float value
        } else if (type == P_INTPTR) {
            outputFile << "\tsubq\t$4, " << identifier.getAddress() << "\n"; // Decrement pointer by 4 bytes
        } else if (type == P_FLOATPTR || type == P_LONGPTR) {
            outputFile << "\tsubq\t$8, " << identifier.getAddress() << "\n";
        } else {
            throw std::runtime_error("GenCode::cgdec: Unsupported type for decrementing global variable");
        }
    }

    void cginc(Reg addr, PrimitiveType type) override {
        // Increment the value at the address pointed by the register by 1
        if (type == P_INT) {
            outputFile << "\tincb\t(" << regManager->getRegister(addr) << ")\n"; // Increment int value
        } else if (type == P_CHAR) {
            outputFile << "\tincl\t(" << regManager->getRegister(addr) << ")\n"; // Increment char value
        } else if (type == P_LONG || type == P_CHARPTR) {
            outputFile << "\tincq\t(" << regManager->getRegister(addr) << ")\n"; // Increment long value
        } else if (type == P_FLOAT) {
            Reg r1 = cgload(Value{ .type = P_FLOAT, .fvalue = 1.0f }); // Load float constant 1.0
            Reg r2 = regManager->allocateRegister(P_FLOAT); // Allocate a register for the float value
            outputFile << "\tmovsd\t(" << regManager->getRegister(addr) << "), " << regManager->getRegister(r2) << "\n"; // Load the float value from the address in addr
            outputFile << "\taddsd\t" << regManager->getRegister(r1) << ", " << regManager->getRegister(r2) << "\n"; // Add float value
            outputFile << "\tmovsd\t" << regManager->getRegister(r2) << ", (" << regManager->getRegister(addr) << ")\n"; // Store the incremented value back to the address in addr
            regManager->freeRegister(r1); // Free the register used for the float constant
            regManager->freeRegister(r2); // Free the register used for the float value
        } else if (type == P_INTPTR) {
            outputFile << "\taddq\t$4, (" << regManager->getRegister(addr) << ")\n"; // Increment pointer by 4 bytes
        } else if (type == P_FLOATPTR || type == P_LONGPTR) {
            outputFile << "\taddq\t$8, (" << regManager->getRegister(addr) << ")\n";
        } else {
            throw std::runtime_error("GenCode::cginc: Unsupported type for incrementing dereferenced value");
        }
        regManager->freeRegister(addr); // Free the address register after use
    }

    void cgdec(Reg addr, PrimitiveType type) override {
        // Decrement the value at the address pointed by the register by 1
        if (type == P_INT) {
            outputFile << "\tdecb\t(" << regManager->getRegister(addr) << ")\n"; // Decrement int value
        } else if (type == P_CHAR) {
            outputFile << "\tdecl\t(" << regManager->getRegister(addr) << ")\n"; // Decrement char value
        } else if (type == P_LONG || type == P_CHARPTR) {
            outputFile << "\tdecq\t(" << regManager->getRegister(addr) << ")\n"; // Decrement long value
        } else if (type == P_FLOAT) {
            Reg r1 = cgload(Value{ .type = P_FLOAT, .fvalue = 1.0f }); // Load float constant 1.0
            Reg r2 = regManager->allocateRegister(P_FLOAT); // Allocate a register for the float value
            outputFile << "\tmovsd\t(" << regManager->getRegister(addr) << "), " << regManager->getRegister(r2) << "\n"; // Load the float value from the address in addr
            outputFile << "\tsubsd\t" << regManager->getRegister(r1) << ", " << regManager->getRegister(r2) << "\n"; // Subtract float value
            outputFile << "\tmovsd\t" << regManager->getRegister(r2) << ", (" << regManager->getRegister(addr) << ")\n"; // Store the decremented value back to the address in addr
            regManager->freeRegister(r1); // Free the register used for the float constant
            regManager->freeRegister(r2); // Free the register used for the float value
        } else if (type == P_INTPTR) {
            outputFile << "\tsubq\t$4, (" << regManager->getRegister(addr) << ")\n"; // Decrement pointer by 4 bytes
        } else if (type == P_FLOATPTR || type == P_LONGPTR) {
            outputFile << "\tsubq\t$8, (" << regManager->getRegister(addr) << ")\n";
        } else {
            throw std::runtime_error("GenCode::cgdec: Unsupported type for decrementing dereferenced value");
        }
    }

    Reg cginvert(Reg reg) override {
        if (reg.type == P_INT) {
            outputFile << "\tnotl\t" << regManager->getRegister(reg) << "\n"; // Negate int value
        } else if (reg.type == P_CHAR) {
            outputFile << "\tnotb\t" << regManager->getRegister(reg) << "\n"; // Negate char value
        } else if (reg.type == P_LONG) {
            outputFile << "\tnotq\t" << regManager->getRegister(reg) << "\n"; // Negate long value
        } else {
            throw std::runtime_error("GenCode::cginvert: Unsupported type for negation");
        }
        return reg; // Return the register containing the negated value
    }

    Reg cgor(Reg r1, Reg r2) override {
        if (r1.type != r2.type) {
            throw std::runtime_error("GenCode::cgor: Registers must be of the same type for bitwise OR");
        }
        if (r1.type == P_INT) {
            outputFile << "\torl\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise OR for int
        } else if (r1.type == P_CHAR) {
            outputFile << "\torb\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise OR for char
        } else if (r1.type == P_LONG) {
            outputFile << "\torq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise OR for long
        } else {
            throw std::runtime_error("GenCode::cgor: Unsupported type for bitwise OR");
        }
        regManager->freeRegister(r2); // Free the second register after use
        return r1; // Return the register containing the result of the bitwise OR operation
    }

    Reg cgand(Reg r1, Reg r2) override {
        if (r1.type != r2.type) {
            throw std::runtime_error("GenCode::cgand: Registers must be of the same type for bitwise AND");
        }
        if (r1.type == P_INT) {
            outputFile << "\tandl\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise AND for int
        } else if (r1.type == P_CHAR) {
            outputFile << "\tandb\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise AND for char
        } else if (r1.type == P_LONG) {
            outputFile << "\tandq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise AND for long
        } else {
            throw std::runtime_error("GenCode::cgand: Unsupported type for bitwise AND");
        }
        regManager->freeRegister(r2); // Free the second register after use
        return r1; // Return the register containing the result of the bitwise AND operation
    }

    Reg cgxor(Reg r1, Reg r2) override {
        if (r1.type != r2.type) {
            throw std::runtime_error("GenCode::cgxor: Registers must be of the same type for bitwise XOR");
        }
        if (r1.type == P_INT) {
            outputFile << "\txorl\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise XOR for int
        } else if (r1.type == P_CHAR) {
            outputFile << "\txorb\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise XOR for char
        } else if (r1.type == P_LONG) {
            outputFile << "\txorq\t" << regManager->getRegister(r2) << ", " << regManager->getRegister(r1) << "\n"; // Perform bitwise XOR for long
        } else {
            throw std::runtime_error("GenCode::cgxor: Unsupported type for bitwise XOR");
        }
        regManager->freeRegister(r2); // Free the second register after use
        return r1; // Return the register containing the result of the bitwise XOR operation
    }

    Reg cgshl(Reg r1, Reg r2) override {
        assert(r2.type == P_CHAR && r1.type == P_LONG);
        outputFile << "\tmovb\t" << regManager->getRegisterLower8bit(r2) << ", %cl\n"; // Move the lower 8 bits of r2 to cl
        outputFile << "\tshlq\t" << "%cl, " << regManager->getRegister(r1) << "\n"; // Shift left
        regManager->freeRegister(r2); // Free the second register after use
        return r1; // Return the register containing the shifted value
    }

    Reg cgshr(Reg r1, Reg r2) override {
        assert(r2.type == P_CHAR && r1.type == P_LONG);
        outputFile << "\tmovb\t" << regManager->getRegisterLower8bit(r2) << ", %cl\n"; // Move the lower 8 bits of r2 to cl
        outputFile << "\tshrq\t" << "%cl, " << regManager->getRegister(r1) << "\n"; // Shift right
        regManager->freeRegister(r2); // Free the second register after use
        return r1; // Return the register containing the shifted value
    }

private:
    std::unique_ptr<X86RegisterManager> regManager; // Register manager for handling register allocation
    std::ofstream outputFile; // Output file stream for writing assembly code
};
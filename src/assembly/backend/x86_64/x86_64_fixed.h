// 修复版本的x86_64代码生成器
// 主要修复了movsd指令使用错误的问题

#include "common/defs.h"
#include "assembly/backend/backend.h"
#pragma once

class X86RegisterManagerFixed: public RegisterManager {
    public:
        X86RegisterManagerFixed() {
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
        ~X86RegisterManagerFixed() = default;

        // Allocate a register
        Reg allocateRegister(PrimitiveType type) override {
            auto& allocated = type_to_register_status.at(type);
            int idx = -1;
            for (size_t i = 0; i < allocated.size(); ++i) {
                if (!allocated[i]) { // Find a free register
                    allocated[i] = true; // Mark the register as allocated
                    idx = i;
                    break;
                }
            }
            if (idx == -1) {
                throw std::runtime_error("No free registers available");
            }
            if (type == P_INT || type == P_LONG || type == P_CHAR) {
                std::vector<PrimitiveType> valid_types = {P_INT, P_CHAR, P_LONG};
                for (const auto& valid_type : valid_types) {
                    auto& allocated_other = type_to_register_status.at(valid_type);
                    allocated_other[idx] = true;
                }
                return Reg{type, false, idx}; // Return the allocated register
            } else if (type == P_FLOAT) {
                allocated[idx] = true; // Mark the register as allocated
                return Reg{type, false, idx}; // Return the allocated register
            }
            return Reg{P_NONE, false, -1}; // Return an invalid register if type is unsupported
        }

        // Free a register
        void freeRegister(Reg reg) override {
            if (reg.type == P_INT || reg.type == P_LONG || reg.type == P_CHAR) {
                std::vector<PrimitiveType> valid_types = {P_INT, P_CHAR, P_LONG};
                for (const auto& valid_type : valid_types) {
                    auto& allocated = type_to_register_status.at(valid_type);
                    if (reg.idx >= 0 && reg.idx < (int)allocated.size()) {
                        allocated[reg.idx] = false; // Mark the register as free
                    }
                }
            } else if (reg.type == P_FLOAT) {
                auto& allocated = type_to_register_status.at(reg.type);
                if (reg.idx >= 0 && reg.idx < (int)allocated.size()) {
                    allocated[reg.idx] = false; // Mark the register as free
                }
            }
        }

        void reset() override {
            for (auto& [type, allocated] : type_to_register_status) {
                for (bool& status : allocated) {
                    status = false; // Mark all registers as free
                }
            }
        }

        std::string getRegister(Reg reg) const override {
            auto registers = type_to_registers.at(reg.type);
            auto& allocated = type_to_register_status.at(reg.type);
            if (reg.idx < 0 || reg.idx >= (int)registers.size()) {
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

        Reg allocateParamRegister(PrimitiveType type) {
            if (type == P_FLOAT) {
                Reg ret;
                if (float_param_count >= max_float_param_count) {
                    ret.type = P_NONE; // No more float registers available
                } else {
                    ret.type = P_FLOAT;
                    ret.idx = float_param_count; // Use the next available float register
                    ret.is_param = true;
                    float_param_count++;
                }
                return ret;
            } else {
                Reg ret;
                if (int_param_count >= max_int_param_count) {
                    ret.type = P_NONE; // No more int registers available
                } else {
                    ret.type = type;
                    ret.idx = int_param_count; // Use the next available int register
                    ret.is_param = true;
                    int_param_count++;
                }
                return ret;
            }
        }

        void resetParamCount() {
            int_param_count = 0; // Reset the integer parameter count
            float_param_count = 0; // Reset the floating-point parameter count
        }

        std::string getParamRegister(Reg reg) const {
            if (reg.type == P_FLOAT) {
                if (reg.idx < 0 || reg.idx >= (int)registers_float_param.size()) {
                    throw std::out_of_range("Float register index out of range");
                }
                return registers_float_param[reg.idx];
            } else if (reg.type == P_INT) {
                if (reg.idx < 0 || reg.idx >= (int)registers_int_param.size()) {
                    throw std::out_of_range("Int register index out of range");
                }
                return registers_int_param[reg.idx];
            } else if (reg.type == P_CHAR) {
                if (reg.idx < 0 || reg.idx >= (int)registers_char_param.size()) {
                    throw std::out_of_range("Char register index out of range");
                }
                return registers_char_param[reg.idx];
            } else if (reg.type == P_LONG) {
                if (reg.idx < 0 || reg.idx >= (int)registers_long_param.size()) {
                    throw std::out_of_range("Long register index out of range");
                }
                return registers_long_param[reg.idx];
            }
            throw std::runtime_error("Unsupported register type for parameter");
        }

    private:
        std::map<PrimitiveType, std::vector<std::string>> type_to_registers;
        std::map<PrimitiveType, std::vector<bool>> type_to_register_status;
        const std::vector<std::string> registers_int = { "%r10d", "%r11d", "%r12d", "%r13d" };
        const std::vector<std::string> registers_char = { "%r10b", "%r11b", "%r12b", "%r13b" };
        const std::vector<std::string> registers_long = { "%r10", "%r11", "%r12", "%r13" }; // Long registers for 64-bit operations
        const std::vector<std::string> registers_float = { "%xmm8", "%xmm9", "%xmm10", "%xmm11" }; // Floating-point registers

        const std::vector<std::string> registers_float_param = { "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7" }; // Floating-point registers for function parameters
        const std::vector<std::string> registers_int_param = { "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d" }; // Integer registers for function parameters
        const std::vector<std::string> registers_char_param = { "%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"  }; // Char registers for function parameters
        const std::vector<std::string> registers_long_param = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9" }; // Long registers for function parameters                                                                    

        int int_param_count = 0; // Count of integer parameters
        int max_int_param_count = 6; // Maximum count of integer parameters
        int float_param_count = 0; // Count of floating-point parameters
        int max_float_param_count = 8; // Maximum count of floating-point parameters
};

class X86CodeGenFixed: public CodeGen {
    public:
        X86CodeGenFixed(const std::string& filename) : outputFile(filename), regManager(std::make_unique<X86RegisterManagerFixed>()) {
            if (!outputFile.is_open()) {
                throw std::runtime_error("Cannot open output file: " + filename);
            }
        }
        ~X86CodeGenFixed() {
            if (outputFile.is_open()) {
                outputFile.close();
            }
        }

    // 修复的核心函数：正确处理整数到浮点的转换
    void cgprintfloat(Reg reg) override {
        // Print the floating-point value in the specified register
        if (reg.type == P_INT || reg.type == P_LONG) {
            // 如果是整数类型，需要先转换为浮点
            Reg float_reg = cgint2float(reg);
            outputFile << "\tmovsd\t" << regManager->getRegister(float_reg) << ", %xmm0\n"; // Move float value to xmm0
            regManager->freeRegister(float_reg);
        } else if (reg.type == P_FLOAT) {
            outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", %xmm0\n"; // Move float value to xmm0
            regManager->freeRegister(reg); // Free the register after use
        } else {
            throw std::runtime_error("X86CodeGenFixed::cgprintfloat: Unsupported register type");
        }
        outputFile << "\tcall\tprintfloat\n";
    }

    void cgprintlong(Reg reg) override {
        // Print the integer value in the specified register
        if (reg.type == P_INT) {
            // 将int转换为long，然后打印
            Reg long_reg = cgint2long(reg);
            outputFile << "\tmovq\t" << regManager->getRegister(long_reg) << ", %rdi\n"; // Move value to rdi
            regManager->freeRegister(long_reg);
        } else if (reg.type == P_LONG) {
            outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", %rdi\n"; // Move value to rdi
            regManager->freeRegister(reg); // Free the register after use
        } else {
            throw std::runtime_error("X86CodeGenFixed::cgprintlong: Unsupported register type");
        }
        outputFile << "\tcall\tprintint\n"; 
    }

    // 修复类型转换函数
    Reg cgint2float(Reg reg) override { 
        Reg r2 = regManager->allocateRegister(P_FLOAT);
        if (reg.type == P_INT) {
            outputFile << "\tcvtsi2sd\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(r2) << "\n";
        } else if (reg.type == P_LONG) {
            outputFile << "\tcvtsi2sd\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(r2) << "\n";
        } else {
            regManager->freeRegister(r2);
            throw std::runtime_error("X86CodeGenFixed::cgint2float: Unsupported input type");
        }
        regManager->freeRegister(reg);
        return r2;
    }

    Reg cgint2long(Reg reg) override {
        // Convert the integer in the specified register to a long
        if (reg.type == P_INT) {
            outputFile << "\tmovslq\t" << regManager->getRegister(reg) << ", " << regManager->getRegister(reg) << "\n";
            reg.type = P_LONG; // Update the register type to long
        }
        return reg; // Return the register containing the long
    }

    // 其他函数保持不变，但修复寄存器使用问题
    Reg cgload(Value value) override {
        // Load the value into a register and return the register index
        PrimitiveType type = value.type == P_STRING ? P_LONG : value.type; // Use P_CHARPTR for string values
        Reg reg = regManager->allocateRegister(type);
        if (value.type == P_INT || value.type == P_CHAR) {
            reg.type = P_INT;
            outputFile << "\tmovl\t$" << value.ivalue << ", " << regManager->getRegister(reg) << "\n";
            reg.type = value.type; // Restore the original type
        } else if (value.type == P_FLOAT) {
            // 浮点常量需要特殊处理
            std::string const_label = float_constants[value.fvalue];
            outputFile << "\tmovsd\t" << const_label << "(%rip), " << regManager->getRegister(reg) << "\n";
        } else if (value.type == P_LONG) {
            reg.type = P_LONG;
            outputFile << "\tmovq\t$" << value.lvalue << ", " << regManager->getRegister(reg) << "\n";
            reg.type = value.type; // Restore the original type
        } else if (value.type == P_STRING) {
            std::string label = string_constants[value.strvalue];
            outputFile << "\tleaq\t" << label << "(%rip), " << regManager->getRegister(reg) << "\n";
        } else {
            throw std::runtime_error("X86CodeGenFixed::cgload: Only Support int value type for loading into register");
        }
        return reg;
    }

    // 修复函数调用参数传递
    void cgloadparamtoreg(Reg reg, int idx) override {
        // Load the parameter value from the register to the parameter register
        Reg reg_param =  {.type = reg.type, .is_param = true, .idx = idx}; // Allocate a register for the parameter
        
        if (reg.type == P_FLOAT) {
            outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", " << regManager->getParamRegister(reg_param) << "\n"; // Move float value to parameter register
        } else if (reg.type == P_INT) {
            outputFile << "\tmovl\t" << regManager->getRegister(reg) << ", " << regManager->getParamRegister(reg_param) << "\n"; // Move value to parameter register
        } else if (reg.type == P_LONG) {
            outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", " << regManager->getParamRegister(reg_param) << "\n"; // Move value to parameter register
        } else if (reg.type == P_CHAR) {
            outputFile << "\tmovb\t" << regManager->getRegister(reg) << ", " << regManager->getParamRegister(reg_param) << "\n"; // Move char value to parameter register
        } else {
            throw std::runtime_error("X86CodeGenFixed::cgloadparamtoreg: Unsupported type for loading parameter to register");
        }
        regManager->freeRegister(reg); // Free the original register after loading to parameter register
    }

    // 修复函数返回值处理
    Reg cgcall(const char *name, PrimitiveType ret_type) override {
        outputFile << "\tcall\t" << name << "\n"; // Call the specified function
        if (ret_type != P_VOID) {
            Reg out = regManager->allocateRegister(ret_type);
            if (ret_type == P_FLOAT) {
                // 浮点返回值已经在%xmm0中，需要移动到目标寄存器
                outputFile << "\tmovsd\t%xmm0, " << regManager->getRegister(out) << "\n";
            } else {
                // 整数返回值在%rax中，需要移动到目标寄存器
                out.type = P_LONG;
                outputFile << "\tmovq\t%rax, " << regManager->getRegister(out) << "\n";
                out.type = ret_type; // Set the return type of the register
            }
            return out;
        }
        return Reg{.type = P_NONE, .idx = 0}; // Return an invalid register if the function returns void
    }

    // 修复函数返回处理
    void cgreturn(const Reg reg, const char *end_label) override {
        switch (reg.type) {
            case P_CHAR:
                outputFile << "\tmovb\t" << regManager->getRegister(reg) << ", %al\n";
                break;
            case P_INT:
                outputFile << "\tmovl\t" << regManager->getRegister(reg) << ", %eax\n";
                break;
            case P_LONG:
                outputFile << "\tmovq\t" << regManager->getRegister(reg) << ", %rax\n";
                break;
            case P_FLOAT:
                // 浮点返回值需要放在%xmm0中
                if (reg.idx != 0) { // 如果不在%xmm0中，需要移动
                    outputFile << "\tmovsd\t" << regManager->getRegister(reg) << ", %xmm0\n";
                }
                break;
            default:
                throw std::runtime_error("X86CodeGenFixed::cgreturn: Unsupported register type for return");
        }
        if (reg.type != P_VOID) regManager->freeRegister(reg); // Free the register after use
        cgjump(end_label);
    }

    // 其余函数保持不变...
    void cgadd(Reg reg1, Reg reg2) override {
        if (reg1.type != reg2.type) {
            throw std::runtime_error("X86CodeGenFixed::cgadd: Registers must be of the same type");
        }

        switch (reg1.type) {
            case P_INT:
                outputFile << "\taddl\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            case P_CHAR:
                outputFile << "\taddb\t" << regManager->getRegisterLower8bit(reg2) << ", " << regManager->getRegisterLower8bit(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            case P_LONG:
                outputFile << "\taddq\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            case P_FLOAT:
                outputFile << "\taddsd\t" << regManager->getRegister(reg2) << ", " << regManager->getRegister(reg1) << "\n";
                regManager->freeRegister(reg2);
                break;
            default:
                throw std::runtime_error("X86CodeGenFixed::cgadd: Unsupported register type for addition");
        }
    }

    // 其他运算函数类似...
    // 为了简洁，这里只展示关键修复部分

    private:
        std::ofstream outputFile; // Output file stream for writing assembly code
        std::unique_ptr<X86RegisterManagerFixed> regManager; // Register manager for handling register allocation
};
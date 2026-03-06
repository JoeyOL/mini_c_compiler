// 重构版本的scanner.cpp - 展示如何使用新的错误处理系统
// 这个文件演示了如何替换原有的错误处理

#include "scanner/scanner.h"
#include "../include/common/error.h"
#include "../include/common/constants.h"
#include <iostream>
#include <filesystem>
#include <cmath>
#include <limits>

// 原有的错误处理方式：
/*
void Scanner::scanNumeric(Token& token, char c) {
    if ((c < '0' || c > '9') && c != '.') {
        throw std::runtime_error("Invalid numeric character: " + std::string(1, c) 
                                 + " at line " + std::to_string(line_no) 
                                 + ", column " + std::to_string(column_no));
    }
    // ... 其余代码
}
*/

// 新的错误处理方式：
void Scanner::scanNumeric(Token& token, char c) {
    if ((c < '0' || c > '9') && c != '.') {
        LEXICAL_ERROR("Invalid numeric character: '" + std::string(1, c) + "'", 
                     line_no, column_no);
        return; // 错误已经报告，返回一个默认的token
    }
    
    token.type = T_NUMBER;
    long integer_part = 0;
    
    if (c != '.') {
        integer_part = scanint(c);
        if (integer_part > std::numeric_limits<int>::max()) {
            // 使用新的错误处理
            LEXICAL_ERROR("Integer literal too large: " + std::to_string(integer_part), 
                         line_no, column_no);
            token.value.setLongValue(0); // 设置为默认值
        } else {
            token.value.setIntValue(static_cast<int>(integer_part));
        }
        c = next();
    }

    if (c == '.') {
        c = next();
        if (c < '0' || c > '9') {
            LEXICAL_ERROR("Invalid fractional part: '" + std::string(1, c) + "'", 
                         line_no, column_no);
            token.value.setFloatValue(0.0);
            return;
        }
        double fractional_part = scanint(c);
        double value = integer_part + fractional_part / std::pow(10, std::to_string(static_cast<int>(fractional_part)).length());
        token.value.setFloatValue(value);
        
    } else if (c == 'e' || c == 'E') {
        // Handle scientific notation with error checking
        int exponent = 0;
        c = next();
        bool negativeExponent = false;
        
        if (c == '-') {
            negativeExponent = true;
            c = next();
        } else if (c == '+') {
            c = next();
        }
        
        if (c < '0' || c > '9') {
            LEXICAL_ERROR("Invalid exponent: '" + std::string(1, c) + "'", 
                         line_no, column_no);
            token.value.setFloatValue(0.0);
            return;
        }
        
        exponent = scanint(c);
        if (negativeExponent) {
            exponent = -exponent;
        }
        
        double base = token.value.getFloatValue();
        double result = base * std::pow(10, exponent);
        token.value.setFloatValue(result);
    } else {
        putback(c);
    }
}

// 改进的标识符扫描，带有关键字检查
void Scanner::scanIdentifier(Token& token, char c) {
    std::string identifier;
    
    while (isalnum(c) || c == '_') {
        identifier += c;
        c = next();
    }
    putback(c);
    
    // 检查标识符长度
    if (identifier.length() > Constants::MAX_IDENTIFIER_LENGTH) {
        LEXICAL_ERROR("Identifier too long: " + identifier.substr(0, 20) + "...", 
                     line_no, column_no);
        identifier = identifier.substr(0, Constants::MAX_IDENTIFIER_LENGTH);
    }
    
    // 检查是否为关键字
    if (matchKeyword(identifier, token)) {
        return; // 是关键字，token已经设置
    }
    
    // 普通标识符
    token.type = T_IDENTIFIER;
    
    // 这里可以添加符号表查找逻辑
    // 检查标识符是否已经在符号表中定义等
}

// 改进的字符串扫描
void Scanner::scanString(Token& token, char c) {
    std::string str_value;
    c = next(); // 跳过开始的引号
    
    while (c != '"' && c != 0) {
        if (c == '\\') {
            // 处理转义字符
            c = next();
            switch (c) {
                case 'n': str_value += '\n'; break;
                case 't': str_value += '\t'; break;
                case 'r': str_value += '\r'; break;
                case '\\': str_value += '\\'; break;
                case '"': str_value += '"'; break;
                case '0': str_value += '\0'; break;
                default:
                    LEXICAL_ERROR("Invalid escape sequence: \\" + std::string(1, c), 
                                 line_no, column_no);
                    str_value += c; // 继续处理
                    break;
            }
        } else {
            str_value += c;
        }
        c = next();
    }
    
    if (c == 0) {
        LEXICAL_ERROR("Unterminated string literal", line_no, column_no);
        token.value.setStringValue("");
        return;
    }
    
    token.type = T_STRING;
    token.value.setStringValue(str_value);
}

// 改进的主扫描函数，带错误恢复
bool Scanner::scan(Token& token) {
    try {
        skip();
        
        char c = next();
        if (c == 0) {
            token.type = T_EOF;
            return false;
        }
        
        token.line_no = line_no;
        token.column_no = column_no;
        
        // 根据字符类型分发处理
        if (c >= '0' && c <= '9') {
            scanNumeric(token, c);
        } else if (isalpha(c) || c == '_') {
            scanIdentifier(token, c);
        } else if (c == '"') {
            scanString(token, c);
        } else if (c == '\'') {
            scanChar(token, c);
        } else {
            // 处理运算符和分隔符
            switch (c) {
                case '+': 
                    if (peek() == '+') { next(); token.type = T_INC; }
                    else if (peek() == '=') { next(); token.type = T_ADD_ASSIGN; } // 新增 +=
                    else token.type = T_PLUS; 
                    break;
                case '-': 
                    if (peek() == '-') { next(); token.type = T_DEC; }
                    else if (peek() == '=') { next(); token.type = T_SUB_ASSIGN; } // 新增 -=
                    else if (peek() == '>') { next(); token.type = T_ARROW; } // 新增 ->
                    else token.type = T_MINUS; 
                    break;
                case '*': 
                    if (peek() == '=') { next(); token.type = T_MUL_ASSIGN; } // 新增 *=
                    else token.type = T_STAR; 
                    break;
                case '/': 
                    if (peek() == '=') { next(); token.type = T_DIV_ASSIGN; } // 新增 /=
                    else if (peek() == '/') { 
                        // 单行注释
                        while (next() != '\n' && peek() != 0) {}
                        return scan(token); // 递归扫描下一个token
                    }
                    else if (peek() == '*') { 
                        // 多行注释
                        next(); // 跳过 *
                        char prev = 0;
                        char curr = next();
                        while (!(prev == '*' && curr == '/') && curr != 0) {
                            prev = curr;
                            curr = next();
                        }
                        if (curr == 0) {
                            LEXICAL_ERROR("Unterminated comment", line_no, column_no);
                            return false;
                        }
                        return scan(token); // 递归扫描下一个token
                    }
                    else token.type = T_SLASH; 
                    break;
                // ... 其他运算符处理
                default:
                    LEXICAL_ERROR("Unexpected character: '" + std::string(1, c) + "'", 
                                 line_no, column_no);
                    return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        // 捕获所有异常并转换为编译器错误
        LEXICAL_ERROR("Exception during scanning: " + std::string(e.what()), 
                     line_no, column_no);
        return false;
    }
}

// 错误恢复机制
void Scanner::recoverFromError() {
    // 跳过当前行，尝试恢复扫描
    char c;
    do {
        c = next();
    } while (c != '\n' && c != 0);
    
    if (c == '\n') {
        line_no++;
        column_no = 0;
    }
}

// 统计和报告函数
void Scanner::printStatistics() const {
    std::cout << "Scanner Statistics:" << std::endl;
    std::cout << "  Lines processed: " << line_no << std::endl;
    std::cout << "  Errors reported: " << ErrorReporter::getInstance().getErrorCount() << std::endl;
    
    if (ErrorReporter::getInstance().hasErrors()) {
        std::cout << "  First error at line: " << std::endl;
        ErrorReporter::getInstance().printAllErrors();
    }
}
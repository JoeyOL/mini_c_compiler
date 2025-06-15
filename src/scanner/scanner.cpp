#include "scanner/scanner.h"
#include <iostream>
#include <filesystem>
#include <cmath>

void Scanner::skip() {
    char c = next();
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = next();
    }
    putback(c);
}

void Scanner::putback(char c) {
    putback_char = c;
}

char Scanner::next() {
    if (putback_char != 0) {
        char c = putback_char;
        putback_char = 0;
        return c;
    }
    char c;
    source_file.get(c);
    if (source_file.eof()) {
        return 0; // EOF
    }
    if (c == '\n') {
        line_no++;
        column_no = 0;
    }
    column_no++;
    return c;
}

char Scanner::peek() {
    char c = next();
    putback(c);
    return c;
}

int Scanner::scanint(int c) {
    int value = 0;
    while (c >= '0' && c <= '9') {
        value = value * 10 + (c - '0');
        c = next();
    }
    putback(c);
    return value;
}

void Scanner::scanNumeric(Token& token, char c) {
    if ((c < '0' || c > '9') && c != '.') {
        throw std::runtime_error("Invalid numeric character: " + std::string(1, c) 
                                 + " at line " + std::to_string(line_no) 
                                 + ", column " + std::to_string(column_no));
    }
    token.type = T_NUMBER;
    int integer_part = 0;
    if (c != '.') {
        integer_part = scanint(c);
        token.value.setIntValue(integer_part);
        c = next();
    }

    if (c == '.') {
        c = next();
        if (c < '0' || c > '9') {
            throw std::runtime_error("Invalid fractional part: " + std::string(1, c) 
                                     + " at line " + std::to_string(line_no) 
                                     + ", column " + std::to_string(column_no));
        }
        double fractional_part = scanint(c);
        double value = integer_part + fractional_part / std::pow(10, std::to_string((int)fractional_part).length());
        token.value.setFloatValue(value);
    } else if (c == 'e' || c == 'E') {
        // Handle scientific notation
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
            throw std::runtime_error("Invalid exponent: " + std::string(1, c) 
                                     + " at line " + std::to_string(line_no) 
                                     + ", column " + std::to_string(column_no));
        }
        exponent = scanint(c);
        if (negativeExponent) {
            exponent = -exponent;
        }
        double value = integer_part * std::pow(10, exponent);
        token.value.setFloatValue(value);
    } else {
        putback(c);
    }
}

Scanner::Scanner(const std::string& source_path)
    : line_no(1), column_no(0), putback_char(0), source_path(source_path) {

    source_file.open(source_path, std::ios::in);
    if (!source_file.is_open()) {
        throw std::runtime_error("Could not open source file: " + source_path);
    }
}

void Scanner::scanIdentifier(Token& token, char c) { 
    // Handle identifiers or keywords
    std::string identifier;
    identifier += c;
    c = next();
    while (isalnum(c) || c == '_') {
        identifier += c;
        c = next();
    }
    if (identifier.size() >= MAX_IDENTIFIER_LENGTH) {
        throw std::runtime_error("Identifier too long: " + identifier + 
                                 " at line " + std::to_string(line_no) 
                                 + ", column " + std::to_string(column_no));
    }
    putback(c);
    if (!matchKeyword(identifier, token)) {
        token.type = T_IDENTIFIER; // Assuming T_IDENTIFIER is defined in TokenType
        token.value.setStringValue(identifier); // Assuming setStringValue is defined
    }
}

bool Scanner::scan(Token& token) {
    skip();
    char c = next();
    if (c == 0) {
        token.type = T_EOF; // EOF
        return false;
    }
    
    if (c == '+') {
        token.type = T_PLUS;
    } else if (c == '-') {
        token.type = T_MINUS;
    } else if (c == '*') {
        token.type = T_STAR;
    } else if (c == '/') {
        token.type = T_SLASH;
    } else if ((c >= '0' && c <= '9') || c == '.') {
        scanNumeric(token, c);
    } else if (c == '(') {
        token.type = T_LPAREN;
    } else if (c == ')') {
        token.type = T_RPAREN;
    } else if (isalpha(c) || c == '_') {
        scanIdentifier(token, c);
    } else if (c == ';') {
        token.type = T_SEMI;
    } else if (c == '=') { 
        if (peek() == '=') {
            next();
            token.type = T_EQ;
        }
        else token.type = T_ASSIGN;
    } else if (c == ',') {
        token.type = T_COMMA;
    } else if (c == '<') {
        if (peek() == '=') {
            next();
            token.type = T_LE;
        }
        else token.type = T_LT;
    } else if (c == '>') {
        if (peek() == '=') {
            next();
            token.type = T_GE;
        }
        else token.type = T_GT;
    } else if (c == '!') {
        if (peek() == '=') {
            next();
            token.type = T_NE;
        }
        else token.type = T_NOT;
    } else {
Error:
        throw std::runtime_error("Unexpected character: " + std::string(1, c) 
                                 + " at line " + std::to_string(line_no) 
                                 + ", column " + std::to_string(column_no));
    }
    token.line_no = line_no;
    token.column_no = column_no;
    return true;
}
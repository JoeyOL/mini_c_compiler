#include "scanner/scanner.h"
#include <iostream>
#include <filesystem>

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

int Scanner::scanint(int c) {
    int value = 0;
    while (c >= '0' && c <= '9') {
        value = value * 10 + (c - '0');
        c = next();
    }
    putback(c);
    return value;
}

Scanner::Scanner(const std::string& source_path)
    : line_no(1), column_no(0), putback_char(0), source_path(source_path) {

    source_file.open(source_path, std::ios::in);
    if (!source_file.is_open()) {
        throw std::runtime_error("Could not open source file: " + source_path);
    }
}

bool Scanner::scan(Token& token) {
    skip();
    char c = next();
    if (c == 0) {
        token.type = -1; // EOF
        return false;
    }
    
    switch (c) {
    case '+':
        token.type = T_PLUS;
        token.value = 0; // No value for operators
        break;
    case '-':
        token.type = T_MINUS;
        token.value = 0; // No value for operators
        break;
    case '*':
        token.type = T_STAR;
        token.value = 0; // No value for operators
        break;
    case '/':
        token.type = T_SLASH;
        token.value = 0; // No value for operators
        break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9':
        token.type = T_INTLIT;
        token.value = scanint(c);
        break;
    default:
        throw std::runtime_error("Unexpected character: " + std::string(1, c) 
                                 + " at line " + std::to_string(line_no) 
                                 + ", column " + std::to_string(column_no));
    }
    return true;
}
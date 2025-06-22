#include <string>
#include <fstream>
#include "common/defs.h"
#pragma once

class Scanner {
    public:
        // Constructor
        Scanner(const std::string& source_path);
        // Destructor
        ~Scanner() = default;
        bool scan(Token& token);
    private:
        std::map<std::string, TokenType> keywords = {
            {"print", T_PRINT},
            {"int", T_INT},
            {"if", T_IF},
            {"else", T_ELSE},
            {"while", T_WHILE},
            {"for", T_FOR},
            {"void", T_VOID},
            {"char", T_CHAR},
            {"long", T_LONG},
            {"float", T_FLOAT},
            {"return", T_RETURN},
        };
        int line_no;
        int column_no;
        char putback_char;
        std::string source_path;
        std::fstream source_file;

        void skip();
        void putback(char c);
        long scanint(int c);
        char next();
        char peek();
        void scanNumeric(Token& token, char c);
        void scanIdentifier(Token& token, char c);
        bool matchKeyword(const std::string& iden, Token& token) {
            auto it = keywords.find(iden);
            if (it != keywords.end()) {
                token.type = it->second;
                return true; // Keyword matched
            }
            return false; // Not a keyword
        }
};
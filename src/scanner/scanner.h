#include <string>
#include <fstream>
#include "common/defs.h"
#include "../mio/single_include/mio/mio.hpp"
#pragma once

class Scanner {
    public:
        // Constructor
        Scanner(const std::string& source_path){
            source_file = mio::mmap_source(source_path);
            if (!source_file.is_open()) {
                throw std::runtime_error("Failed to open file: " + source_path);
            }
            read_index = 0;
        }
        // Destructor
        ~Scanner() = default;
        bool scan(Token& token);
        void release() {
            source_file.unmap();
        }
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
            {"break", T_BREAK},
            {"continue", T_CONTINUE},
        };
        int line_no;
        int column_no;
        char putback_char;
        std::string source_path;
        mio::mmap_source source_file;
        int read_index;

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
        void scanString(Token& token, char c);
        void scanChar(Token& token, char c);
};
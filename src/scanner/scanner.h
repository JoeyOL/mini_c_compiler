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
        int line_no;
        int column_no;
        char putback_char;
        std::string source_path;
        std::fstream source_file;

        void skip();
        void putback(char c);
        int scanint(int c);
        char next();
};
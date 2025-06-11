#include "common/defs.h"
#include "scanner/scanner.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    try {
        Scanner scanner = Scanner(argv[1]);
        Token token;
        while (scanner.scan(token)) {
            switch (token.type) {
            case T_INTLIT:
                std::cout << "Integer Literal: " << token.value << std::endl;
                break;
            case T_PLUS:
                std::cout << "Plus Operator" << std::endl;
                break;
            case T_MINUS:
                std::cout << "Minus Operator" << std::endl;
                break;
            case T_STAR:
                std::cout << "Star Operator" << std::endl;
                break;
            case T_SLASH:
                std::cout << "Slash Operator" << std::endl;
                break;
            }
        }
        std::cout << "End of file reached." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
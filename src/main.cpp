#include "common/defs.h"
#include "scanner/scanner.h"
#include "parser/parser.h"
#include "assembly/gencode.h"
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    try {
        Scanner scanner = Scanner(argv[1]);
        Token token;
        std::vector<Token> tokens;
        while (scanner.scan(token)) {
            switch (token.type) {
            case T_INTLIT:
                std::cout << "Integer Literal: " << token.value.ivalue << std::endl;
                break;
            case T_FLOATLIT:
                std::cout << "Float Literal: " << token.value.fvalue << std::endl;
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
            tokens.push_back(token);
        }
        std::cout << "End of file reached." << std::endl;

        Parser parser = Parser(tokens);
        // std::shared_ptr<ASTNode> ast = parser.parseBinaryExpression();
        std::shared_ptr<ASTNode> ast = parser.parseBinaryExpressionWithPrecedence(0);
        // std::shared_ptr<ASTNode> ast = parser.parseAdditiveExpression();
        std::cout << "Parsed AST successfully." << std::endl;
        ast->walk(); // Walk the AST to print the structure and values
        std::cout << "AST walk completed." << std::endl;

        // Code generation would go here, e.g., generating assembly code from the AST
        GenCode genCode("output.s");
        genCode.generate(ast);
        std::cout << "Code generation completed. Output written to output.s." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
#include "common/defs.h"
#include "scanner/scanner.h"
#include "parser/parser.h"
#include "assembly/gencode.h"
#include "semantic/semantic.h"
#include <iostream>
#include <vector>
#include <filesystem>
SymbolTable symbol_table;
std::map<double, std::string> float_constants; // Map to store float literals
std::map<std::string, std::string> string_constants; // Map to store string literals
LabelAllocator labelAllocator; // Static label allocator for generating unique labels

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    std::string output_file = "output.s";
    if (argc > 2) {
        output_file += argv[2]; // Allow user to specify output file
    }
    try {
        float_constants[1.0] = labelAllocator.getLabel(FLOAT_CONSTANT_LABEL);
        Scanner scanner = Scanner(argv[1]);
        Token token;
        std::vector<Token> tokens;
        while (scanner.scan(token)) {
            switch (token.type) {
            case T_NUMBER:
                if (token.value.type == P_FLOAT) 
                    std::cout << "Float Literal: " << token.value.fvalue << std::endl;
                else 
                    std::cout << "Integer Literal: " << token.value.ivalue << std::endl;
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
        std::shared_ptr<Pragram> ast = parser.parsePragram();
        // std::shared_ptr<ASTNode> ast = parser.parseAdditiveExpression();
        std::cout << "Parsed AST successfully." << std::endl;
        ast->walk(""); // Walk the AST to print the structure and values
        std::cout << "AST walk completed." << std::endl;

        std::cout << "Semantic check." << std::endl;
        Semantic semantic(ast);
        semantic.check();
        ast->walk(""); // Walk the AST again after semantic check
        std::cout << "Semantic check completed." << std::endl;

        // Code generation would go here, e.g., generating assembly code from the AST
        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        GenCode genCode(output_file);
        genCode.generate(ast);
        std::cout << "Code generation completed. Output written to output.s." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
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
    bool enable_log = true; // Flag to enable or disable logging
    if (argc > 2) {
        if (std::string(argv[2]) == "-disable_log") {
            enable_log = false;
        }
    }
    try {
        float_constants[1.0] = labelAllocator.getLabel(FLOAT_CONSTANT_LABEL);
        Scanner scanner = Scanner(argv[1]);
        Token token;
        std::vector<Token> tokens;
        while (scanner.scan(token)) {
            tokens.push_back(token);
        }
        scanner.release();
        if (enable_log) std::cout << "End of file reached." << std::endl;

        Parser parser = Parser(tokens);
        // std::shared_ptr<ASTNode> ast = parser.parseBinaryExpression();
        std::shared_ptr<Pragram> ast = parser.parsePragram();
        // std::shared_ptr<ASTNode> ast = parser.parseAdditiveExpression();
        if (enable_log) {
            std::cout << "Parsed AST successfully." << std::endl;
            ast->walk(""); // Walk the AST to print the structure and values
            std::cout << "AST walk completed." << std::endl;
        }

        if (enable_log) std::cout << "Semantic check." << std::endl;
        Semantic semantic(ast);
        semantic.check();
        if (enable_log) {
            ast->walk(""); // Walk the AST again after semantic check
            std::cout << "Semantic check completed." << std::endl;
        }

        // Code generation would go here, e.g., generating assembly code from the AST
        if (enable_log) std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        GenCode genCode(output_file);
        genCode.generate(ast);
        if (enable_log) std::cout << "Code generation completed. Output written to output.s." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
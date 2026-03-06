#pragma once

#include <string>
#include <vector>
#include <iostream>

enum class ErrorType {
    LEXICAL_ERROR,      // 词法错误
    SYNTAX_ERROR,       // 语法错误
    SEMANTIC_ERROR,     // 语义错误
    TYPE_ERROR,         // 类型错误
    CODEGEN_ERROR,      // 代码生成错误
    INTERNAL_ERROR      // 内部错误
};

class CompilerError {
public:
    CompilerError(ErrorType type, const std::string& message, 
                   int line, int column, const std::string& file = "")
        : type_(type), message_(message), line_(line), column_(column), file_(file) {}

    ErrorType getType() const { return type_; }
    const std::string& getMessage() const { return message_; }
    int getLine() const { return line_; }
    int getColumn() const { return column_; }
    const std::string& getFile() const { return file_; }

    void print() const {
        std::cerr << getErrorTypeString() << ": " << message_;
        if (!file_.empty()) {
            std::cerr << " in " << file_;
        }
        if (line_ > 0) {
            std::cerr << " at line " << line_ << ", column " << column_;
        }
        std::cerr << std::endl;
    }

private:
    std::string getErrorTypeString() const {
        switch (type_) {
            case ErrorType::LEXICAL_ERROR: return "Lexical Error";
            case ErrorType::SYNTAX_ERROR: return "Syntax Error";
            case ErrorType::SEMANTIC_ERROR: return "Semantic Error";
            case ErrorType::TYPE_ERROR: return "Type Error";
            case ErrorType::CODEGEN_ERROR: return "Code Generation Error";
            case ErrorType::INTERNAL_ERROR: return "Internal Error";
            default: return "Unknown Error";
        }
    }

    ErrorType type_;
    std::string message_;
    int line_;
    int column_;
    std::string file_;
};

class ErrorReporter {
public:
    static ErrorReporter& getInstance() {
        static ErrorReporter instance;
        return instance;
    }

    void report(ErrorType type, const std::string& message, 
                int line = 0, int column = 0, const std::string& file = "") {
        errors_.emplace_back(type, message, line, column, file);
    }

    void reportLexicalError(const std::string& message, int line, int column, const std::string& file = "") {
        report(ErrorType::LEXICAL_ERROR, message, line, column, file);
    }

    void reportSyntaxError(const std::string& message, int line, int column, const std::string& file = "") {
        report(ErrorType::SYNTAX_ERROR, message, line, column, file);
    }

    void reportSemanticError(const std::string& message, int line, int column, const std::string& file = "") {
        report(ErrorType::SEMANTIC_ERROR, message, line, column, file);
    }

    void reportTypeError(const std::string& message, int line, int column, const std::string& file = "") {
        report(ErrorType::TYPE_ERROR, message, line, column, file);
    }

    void reportCodeGenError(const std::string& message, int line, int column, const std::string& file = "") {
        report(ErrorType::CODEGEN_ERROR, message, line, column, file);
    }

    void reportInternalError(const std::string& message, int line, int column, const std::string& file = "") {
        report(ErrorType::INTERNAL_ERROR, message, line, column, file);
    }

    void printAllErrors() const {
        if (errors_.empty()) {
            return;
        }

        std::cerr << "Compilation failed with " << errors_.size() << " error(s):" << std::endl;
        for (const auto& error : errors_) {
            error.print();
        }
    }

    bool hasErrors() const {
        return !errors_.empty();
    }

    size_t getErrorCount() const {
        return errors_.size();
    }

    void clear() {
        errors_.clear();
    }

private:
    ErrorReporter() = default;
    ErrorReporter(const ErrorReporter&) = delete;
    ErrorReporter& operator=(const ErrorReporter&) = delete;

    std::vector<CompilerError> errors_;
};

// 便利宏定义
#define COMPILER_ERROR(type, msg) \
    ErrorReporter::getInstance().report(type, msg, __LINE__, 0, __FILE__)

#define LEXICAL_ERROR(msg, line, col) \
    ErrorReporter::getInstance().reportLexicalError(msg, line, col)

#define SYNTAX_ERROR(msg, line, col) \
    ErrorReporter::getInstance().reportSyntaxError(msg, line, col)

#define SEMANTIC_ERROR(msg, line, col) \
    ErrorReporter::getInstance().reportSemanticError(msg, line, col)

#define TYPE_ERROR(msg, line, col) \
    ErrorReporter::getInstance().reportTypeError(msg, line, col)

#define CODEGEN_ERROR(msg, line, col) \
    ErrorReporter::getInstance().reportCodeGenError(msg, line, col)

#define INTERNAL_ERROR(msg, line, col) \
    ErrorReporter::getInstance().reportInternalError(msg, line, col)
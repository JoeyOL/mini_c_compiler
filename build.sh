#!/bin/bash

# 完整的编译器构建脚本
echo "Building mini C compiler..."

# 编译所有的源文件
g++ -std=c++17 -Wall -O0 -g -ggdb3 \
    -o comp \
    src/main.cpp \
    src/scanner/scanner.cpp \
    src/semantic/semantic.cpp \
    src/parser/parser.cpp \
    src/assembly/gencode.cpp \
    -I./src

if [ $? -eq 0 ]; then
    echo "Build successful! Executable: comp"
    echo "Usage: ./comp <source_file> [-disable_log]"
else
    echo "Build failed!"
    exit 1
fi
cmake_minimum_required(VERSION 3.10)

project(Compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -O0 -g -ggdb3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

add_executable(comp
    src/main.cpp
    src/scanner/scanner.cpp
    src/parser/parser.cpp
    src/parser/ExprNode.h
    src/parser/StatementNode.h
    src/parser/parser.h
    src/assembly/gencode.cpp
    src/assembly/gencode.h
)

target_include_directories(comp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

enable_testing()

add_test(NAME while COMMAND bash -c "cd /home/joe/compiler; chmod +x test/09_while_statement/runtests; ./test/09_while_statement/runtests")
add_test(NAME for_loop COMMAND bash -c "cd /home/joe/compiler; chmod +x test/10_for_loops/runtests; ./test/10_for_loops/runtests")

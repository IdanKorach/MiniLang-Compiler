#!/bin/bash

# run_program.sh - Automated build and test script for MiniLang Compiler

echo "=== Building MiniLang Compiler ==="

# Generate lexer
echo "Generating lexer..."
lex ast.l
if [ $? -ne 0 ]; then
    echo "ERROR: Lexer generation failed!"
    exit 1
fi

# Generate parser
echo "Generating parser..."
bison -y ast.y
if [ $? -ne 0 ]; then
    echo "ERROR: Parser generation failed!"
    exit 1
fi

# Compile semantic analysis module
echo "Compiling semantic analysis..."
cc -c semantic_analysis.c -o semantic_analysis.o
if [ $? -ne 0 ]; then
    echo "ERROR: Semantic analysis compilation failed!"
    exit 1
fi

# Compile code generation module
echo "Compiling code generation..."
cc -c codegen.c -o codegen.o
if [ $? -ne 0 ]; then
    echo "ERROR: Code generation compilation failed!"
    exit 1
fi

# Link everything together
echo "Linking..."
cc -o ast y.tab.c semantic_analysis.o codegen.o -ll -Ly
if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed!"
    exit 1
fi

echo "=== Build Successful! ==="
echo ""

# Run the program with the test file
echo "=== Running Test ==="
if [ -f "ast.t" ]; then
    ./ast < ast.t
else
    echo "WARNING: ast.t not found. Please create a test file or run manually:"
    echo "  ./ast < your_test_file.txt"
fi

echo ""
echo "=== Done ==="
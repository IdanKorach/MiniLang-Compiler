# MiniLang Compiler

A yacc/lex-based compiler frontend that parses source code, builds an Abstract Syntax Tree (AST), and performs semantic analysis with type checking and symbol table management.

## Features

- **Lexical Analysis**: Tokenizes source code using flex/lex with robust string literal handling
- **Syntax Analysis**: Parses tokens into an AST using yacc/bison
- **AST Generation**: Creates a structured tree representation of the code
- **Semantic Analysis**: 
  - Variable declaration tracking with scope management
  - Function parameter validation with default values support
  - Symbol table construction with scope isolation
  - Type checking for declarations, assignments, and initializations
  - Variable usage validation (checks if variables are declared before use)
  - Redeclaration error detection
  - Comprehensive string literal handling with escape sequences
- **Error Handling**: Detailed error reporting for syntax and semantic issues

## Prerequisites

- `flex` (or `lex`)
- `bison` (or `yacc`)
- `gcc` or any C compiler

## Installation

1. Clone the repository:
```bash
git clone <repository-url>
cd MiniLang-Compiler
```

## Compilation

Run these commands in sequence:

```bash
# Generate lexer
lex ast.l

# Generate parser
bison -y ast.y

# Compile semantic analysis module
cc -c semantic_analysis.c -o semantic_analysis.o

# Link everything together
cc -o ast y.tab.c semantic_analysis.o -ll -Ly
```

## Usage

### Basic Usage

Run the compiler with an input file:
```bash
./ast < input_file.txt
```

### Input Format

The compiler expects source code with the following syntax:

#### Functions
```python
def function_name(param_list) -> return_type: {
    // statements
}

# Function with parameters and default values
def test1(int p1; string p20; float p2: 2.718; bool p3: true) -> bool: {
    bool temp = false;
    return temp;
}

# Main function (required)
def __main__(): {
    int x = 5;
}
```

#### Variable Declarations
```python
int x;
float y = 3.14;
string name = "example";
bool flag = true;
```

#### String Literals
```python
string simple = "hello world";
string empty = "";
string with_escape = "Say \"Hello\"";
string with_newline = "Line 1\nLine 2";
string single_quoted = 'also supported';
```

#### Parameter Types
- Parameters are separated by semicolons (`;`)
- Parameters can have default values: `float p2: 2.718`
- All four data types supported: `int`, `float`, `string`, `bool`

#### Control Structures
```python
if (condition) : {
    // statements
}

while (condition) : {
    // statements
}
```

## Output

The compiler generates two main outputs:

### 1. AST Visualization
A hierarchical tree representation of the parsed code:
```
(function
    test1
    (params
        (int
            p1
        )
        (string
            p20
        )
        (float
            p2
            (2.718)
        )
        (bool
            p3
        )
    )
    (return_type
        bool
    )
    (init
        (declare
            bool
            temp
        )
        false
    )
    // ... more AST nodes
)
```

### 2. Semantic Analysis Results
```
=== Starting semantic analysis ===
Entering function scope: test1
Processing parameters...
Found parameter: int p1
  Added variable 'p1' of type 'int' to scope test1
Found parameter: string p20
  Added variable 'p20' of type 'string' to scope test1
Found parameter: float p2
  Added variable 'p2' of type 'float' to scope test1
Found parameter: bool p3
  Added variable 'p3' of type 'bool' to scope test1
Found declaration: bool temp
  Added variable 'temp' of type 'bool' to scope test1
Entering function scope: __main__
=== Semantic analysis completed successfully ===
```

## Current Semantic Analysis Features

### ✅ **Fully Implemented**
- **Function scope creation**: Each function gets its own scope
- **Parameter tracking**: Function parameters (including default values) are added to function scope
- **Variable declarations**: Local variables are tracked in their scope
- **Variable usage validation**: Ensures variables are declared before use
- **Assignment type checking**: Validates type compatibility in assignments and initializations
- **Redeclaration checking**: Prevents declaring the same variable twice in same scope
- **Type validation**: Ensures all declared types are valid (int, string, bool, float)
- **Scope isolation**: Variables in different functions don't conflict
- **String literal handling**: Comprehensive support for double/single quotes, escape sequences, and special characters
- **Expression type inference**: Determines types of literals, variables, and expressions
- **Clean error reporting**: Provides clear, non-redundant error messages

### 🚧 **Future Enhancements**
- Block scopes for if/while statements
- Function call validation and parameter checking
- Return type checking
- More complex expression evaluation
- Control flow analysis

## Grammar Features

### Supported Data Types
- `int`: Integer values
- `float`: Floating-point numbers (e.g., `3.14`)
- `string`: String literals with double (`"`) or single (`'`) quotes
- `bool`: Boolean values (`true`/`false`)

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`, `**`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `and`, `or`, `not`

### Language Constructs
- Function definitions with typed parameters and default values
- Variable declarations and assignments
- Control flow (`if`/`elif`/`else`, `while`)
- Function calls
- Return statements
- Comments (using `#`)

## Error Handling

The compiler provides detailed error messages for:

### Syntax Errors
- Missing semicolons
- Unmatched parentheses
- Invalid expressions

### Semantic Errors
- **Variable usage**: `Variable 'x' used before declaration`
- **Type mismatches**: `Type mismatch in initialization of 'y'. Expected: int, Got: string`
- **Redeclaration**: `Variable 'x' already declared in this scope`
- **Invalid types**: `Unknown type 'invalidtype'`
- **Missing main**: `Error: No '__main__' function found.`

## File Structure

```
├── ast.y                    # Yacc grammar file
├── ast.l                    # Lex lexical analyzer with string literal support
├── semantic_analysis.h      # Header for semantic analysis
├── semantic_analysis.c      # Semantic analysis implementation
├── .gitignore              # Git ignore file for generated/compiled files
├── README.md               # This file
└── test_files/             # Sample input files
```

## Testing

The compiler has been thoroughly tested with:
- Various string literal formats (empty, quoted, escaped)
- All data types and their combinations
- Function parameters with and without defaults
- Variable usage validation and type checking
- Scope isolation between functions
- Error detection and reporting

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with various input files
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Built using Flex/Lex for lexical analysis
- Parser generated with Bison/Yacc
- Semantic analysis using custom symbol table implementation
- Comprehensive testing for robust string literal and type checking
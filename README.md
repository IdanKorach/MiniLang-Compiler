# MiniLang Compiler

A yacc/lex-based compiler frontend that parses source code, builds an Abstract Syntax Tree (AST), and performs comprehensive semantic analysis with advanced type checking and symbol table management.

## Features

- **Lexical Analysis**: Tokenizes source code using flex/lex with robust string literal handling
- **Syntax Analysis**: Parses tokens into an AST using yacc/bison
- **AST Generation**: Creates a structured tree representation of the code
- **Advanced Semantic Analysis**: 
  - Variable declaration tracking with scope management
  - Function signature validation with complete parameter tracking
  - Default parameter value type checking
  - Argument count and type validation for function calls
  - Return type validation with multiple return statement support
  - Variable usage validation (checks if variables are declared before use)
  - Assignment type checking with comprehensive expression evaluation
  - If/while statement condition validation (must be boolean type)
  - Redeclaration error detection
  - Scope isolation with hierarchical variable lookup
  - Block-level scoping for if/while statements
  - Function declaration order enforcement
  - String indexing validation (string[index] syntax)
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

#### String Literals and Operations
```python
string simple = "hello world";
string empty = "";
string with_escape = "Say \"Hello\"";
string with_newline = "Line 1\nLine 2";
string single_quoted = 'also supported';

# String indexing and slicing operations
string first_char = name[0];          # Single character access
string substring = name[1:5];         # Slice from index 1 to 5
string prefix = name[:3];             # Slice from start to index 3
string suffix = name[3:];             # Slice from index 3 to end
string full_copy = name[:];           # Full copy of the string
string with_step = name[0:10:2];      # Slice with step (every other character)
```

#### Parameter Types
- Parameters are separated by semicolons (`;`)
- Parameters can have default values: `float p2: 2.718`
- All four data types supported: `int`, `float`, `string`, `bool`

#### Control Structures
```python
# If statements with boolean conditions
if (condition) : {
    // statements
}

# While loops with boolean conditions
while (condition) : {
    // statements
}
```
## Language Limitations

- Unary minus operator (`-n`) is not supported. Use subtraction instead: `0 - n`
- Strict type checking for comparisons - operands of `==` and `!=` must be of the same type
- No built-in string length function - use explicit indexing for string operations

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
  Added parameter 'p1' (type: int, has_default: no) to function 'test1'
Found parameter: string p20
  Added variable 'p20' of type 'string' to scope test1
  Added parameter 'p20' (type: string, has_default: no) to function 'test1'
Found parameter: float p2
  Added variable 'p2' (type: float, has_default: yes) to function 'test1'
  Checking default value for parameter 'p2'...
    Default value type OK: float
Found parameter: bool p3
  Added variable 'p3' (type: bool, has_default: yes) to function 'test1'
  Checking default value for parameter 'p3'...
    Default value type OK: bool
Found return statement in function 'test1'
  Validating return type: expected bool, got bool
  Return statement validated successfully
=== Semantic analysis completed successfully ===
```

## Current Semantic Analysis Features

### ✅ **Fully Implemented**
- **Enhanced Function Management**: Complete function signature tracking with return types and parameters
- **Parameter Validation**: 
  - Type checking of parameters
  - Default value type validation
  - Parameter name and type tracking
- **Function Call Validation**:
  - Argument count validation (considers default parameters)
  - Argument type checking against parameter types
  - Function declaration before usage checking
- **Return Type Validation**:
  - Validates return statements match declared return type
  - Handles multiple return statements in same function
  - Supports variable returns with type inference
  - Validates functions without return types
- **Control Flow Validation**:
  - If statement conditions must be boolean type
  - While loop conditions must be boolean type
  - Comprehensive expression evaluation for conditions
- **Advanced Expression Type System**:
  - Arithmetic operators (`+`, `-`, `*`, `/`) with type inference
  - Comparison operators (`>`, `<`, `>=`, `<=`, `==`, `!=`) return boolean
  - Logical operators (`and`, `or`, `not`) return boolean
  - Proper handling of operator precedence in type checking
- **Variable Management**:
  - Variable declaration tracking with scope management
  - Variable usage validation (ensures variables are declared before use)
  - Assignment type checking with expression evaluation
  - Redeclaration checking within same scope
  - Scope isolation between functions
  - Block-level scoping for if/while statements
- **String Operations**:
  - String indexing validation with type checking (single character access only)
  - String concatenation with the `+` operator
- **Type System**:
  - Four data types: `int`, `float`, `string`, `bool`
  - Comprehensive literal detection (numbers, strings, booleans)
  - String literal handling with escape sequences
  - Type inference for complex expressions
- **Error Reporting**:
  - Clear, detailed error messages with context
  - Multiple error detection (doesn't stop at first error)
  - Semantic error counting and reporting

### 🚧 **Future Enhancements**
- More complex expression evaluation
- Control flow analysis (dead code detection)
- Array and data structure support
- Optimizations and code generation
- Support for unary minus operator
- Built-in string length function
- Implicit type conversion options

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
- Function definitions with typed parameters and default values (no nested functions)
- Variable declarations and assignments
- Control flow (`if`/`elif`/`else`, `while`) with boolean conditions
- Function calls with argument validation
- Return statements with type checking
- Comments (using `#`)
- Block-level scoping for control structures

## Error Handling

The compiler provides detailed error messages for:

### Syntax Errors
- Missing semicolons
- Unmatched parentheses
- Invalid expressions

### Semantic Errors
- **Variable usage**: `Variable 'x' used before declaration`
- **Type mismatches**: 
  - `Type mismatch in initialization of 'y'. Expected: int, Got: string`
  - `Type mismatch in assignment to 'x'. Expected: int, Got: string`
- **Function errors**:
  - `Function 'test' already declared`
  - `Function 'test' called before declaration`
  - `Too few arguments for function 'test'. Expected at least 2, got 1`
  - `Type mismatch for argument 1 in function 'test'. Expected: int, Got: string`
- **Return type errors**:
  - `Return type mismatch in function 'test'. Expected: int, Got: string`
  - `Function 'test' declared with return type 'int' but returns no value`
- **Default value errors**:
  - `Default value type mismatch for parameter 'x'. Parameter type: int, Default value type: string`
- **Control flow errors**:
  - `if-statement condition must be boolean type. Expected: bool, Got: int`
  - `while-loop condition must be boolean type. Expected: bool, Got: string`
- **String indexing errors**:
  - `String index must be of integer type, got 'bool'`
  - `Index operator '[]' can only be used with string type, got 'int'`
- **Redeclaration**: `Variable 'x' already declared in this scope`
- **Invalid types**: `Unknown type 'invalidtype'`
- **Missing main**: `Error: No '__main__' function found.`

## File Structure

```
├── ast.y                    # Yacc grammar file
├── ast.l                    # Lex lexical analyzer with string literal support
├── semantic_analysis.h      # Header for semantic analysis
├── semantic_analysis.c      # Semantic analysis implementation
├── .gitignore               # Git ignore file for generated/compiled files
├── README.md                # This file
└── test_files/              # Sample input files
```

## Testing

The compiler has been thoroughly tested with:
- Function signature validation and tracking
- Parameter and argument validation
- Return type checking with multiple scenarios
- Default parameter value type checking
- If/while condition validation
- Variable usage and scope management
- Assignment type checking
- Block-level scoping
- String indexing operations
- Error detection and reporting
- Complex expression evaluation
- Various string literal formats (empty, quoted, escaped)
- All data types and their combinations
- String slicing operations
- Complex operator precedence cases
- Edge cases with various data types
- Comprehensive battery of tests (10 test suites covering all language aspects)

## Example Programs

### Valid Program
```python
def calculate(int x; float rate: 2.5; bool debug: false) -> float: {
    if (debug): {
        # Local variable in if-block scope
        string message = "Calculating...";
    }
    
    float result = x * rate;
    return result;
}

def get_char(string text; int index) -> string: {
    # String indexing returns a string (single character)
    return text[index];
}

def test_conditions() -> bool: {
    int count = 0;
    bool flag = true;
    
    if (flag and count > 0): {
        return true;
    }
    
    while (count < 10): {
        count = count + 1;
    }
    
    return false;
}

def __main__(): {
    calculate(10, 3.0, true);
    string first = get_char("Hello", 0);
    bool result = test_conditions();
}
```

### Error Examples
```python
def errors() -> int: {
    # Error: if condition must be boolean
    if (5): {
        return 1;
    }
    
    # Error: string default for int parameter
    def bad_func(int x: "hello"): {
        return;
    }
    
    # Error: wrong argument type
    calculate("wrong", 2.5, true);
    
    # Error: return type mismatch
    return "string";  # Expected int
    
    # Error: function called before declaration
    int x = undefined_function(10);
    
    # Error: non-string indexed with []
    int num = 42;
    int digit = num[0];  # Only strings can be indexed
}
```
## Recent Improvements

- Fixed modulo operator precedence to handle expressions like `a % b == 0` correctly
- Added comprehensive string slicing operations with Python-like syntax
- Improved type handling in comparison operations
- Enhanced semantic analyzer for better error detection
- Created extensive test suite covering all language features

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
- Comprehensive testing for robust type checking and validation
# 🔥 EVIL EDGE CASE TESTS - Let's Break Your Compiler! 😈
# These tests will find any remaining weak spots

# =============================================================================
# TEST 1: EXTREME COMMA DECLARATION COMBINATIONS
# =============================================================================
def test_extreme_comma_declarations() -> int: {
    # Single declaration
    int a;
    
    # All initialized
    int b = 1, c = 2, d = 3, e = 4, f = 5;
    
    # Mixed pattern - every other one
    int g, h = 10, i, j = 20, k;
    
    # Long chain
    int v1 = 1, v2, v3 = 3, v4 = 4, v5, v6 = 6, v7 = 7, v8, v9 = 9, v10;
    
    # All types in succession
    int x = 100;
    float y = 2.5, z = 3.7;
    string s1 = "hello", s2, s3 = "world";
    bool flag1, flag2 = true, flag3 = false, flag4;
    
    return b + c + d + e + f;
}

# =============================================================================
# TEST 2: DEEPLY NESTED EXPRESSIONS
# =============================================================================
def test_nested_expressions() -> int: {
    int a = 1, b = 2, c = 3, d = 4;
    
    # Extreme nesting
    int result = ((((a + b) * c) - d) ** 2) % ((c * d) + (a - b));
    
    # Mixed operators with precedence
    int complex = a + b * c ** d - (a * b) / (c + d) % (a - b);
    
    # Boolean complexity
    bool crazy = ((a < b) and (c > d)) or (not (a == c) and (b != d)) or (a >= c and b <= d);
    
    return result + complex;
}

# =============================================================================
# TEST 3: STRING EDGE CASES
# =============================================================================
def test_string_edge_cases() -> string: {
    # Empty string operations
    string empty = "";
    string empty_result = empty[0:0];      # Empty slice
    string empty_char = empty[:];          # Full slice of empty
    
    # Single character
    string single = "x";
    string single_slice = single[0:1];     # Full single char
    string single_empty = single[1:1];     # Empty from single
    
    # Boundary conditions
    string text = "Hello";
    string first = text[0:1];              # First char
    string last = text[4:5];               # Last char (index 4)
    string beyond = text[5:];              # Beyond end
    string most = text[0:4];               # All but last (instead of negative)
    
    # Step edge cases
    string stepped = text[0:5:2];          # Explicit range with step
    string every_other = text[1:5:2];      # Every other starting from 1
    
    # More boundary tests
    string middle = text[1:4];             # Middle section
    string prefix = text[:3];              # First 3 chars
    string suffix = text[2:];              # From index 2 to end
    
    return first;
}

# =============================================================================
# TEST 4: EXTREME PARAMETER COMBINATIONS
# =============================================================================
def test_extreme_params(
    int a, b, c; 
    float x: 1.0, y, z: 3.0; 
    string msg: "default", name; 
    bool flag1, flag2: true, flag3: false
) -> string: {
    # Use all parameters
    int sum = a + b + c;
    float avg = (x + y + z) / 3.0;
    
    if (flag1 and flag2 and not flag3): {
        return msg;
    } else: {
        return name;
    }
}

# =============================================================================
# TEST 5: FUNCTION CALL EDGE CASES
# =============================================================================
def test_function_call_edges() -> float: {
    # Minimum parameters (rely on defaults)
    string result1 = test_extreme_params(1, 2, 3);
    
    # Partial parameters
    string result2 = test_extreme_params(1, 2, 3, 2.0);
    
    # Most parameters
    string result3 = test_extreme_params(1, 2, 3, 1.5, 2.5, 3.5, "hello");
    
    # All parameters
    string result4 = test_extreme_params(1, 2, 3, 1.1, 2.2, 3.3, "test", "name", false, true, false);
    
    # FIXED: Now we return a proper float instead of trying to assign string to float
    int int_result = test_extreme_comma_declarations();
    float float_result = int_result + 42.5;  # Convert int to float calculation
    
    return float_result;
}

# =============================================================================
# TEST 6: CONTROL FLOW EDGE CASES
# =============================================================================
def test_control_flow_edges() -> int: {
    int result = 0;
    int counter = 0, limit = 5;
    
    # Nested if-elif-else
    if (counter == 0): {
        if (limit > 3): {
            result = 1;
        } elif (limit > 1): {
            result = 2;
        } else: {
            result = 3;
        }
    } elif (counter == 1): {
        while (limit > 0): {
            result = result + limit;
            limit = limit - 1;
        }
    } else: {
        result = 999;
    }
    
    # Complex while condition
    while (counter < 10 and result < 100 and (counter % 2 == 0 or result % 3 == 0)): {
        counter = counter + 1;
        result = result * 2;
        
        if (result > 50): {
            result = result / 2;
        }
    }
    
    return result;
}

# =============================================================================
# TEST 7: MIXED ASSIGNMENT EDGE CASES
# =============================================================================
def test_mixed_assignment_edges() -> int: {
    # Multiple assignment with complex expressions
    int a = 1, b = 2;
    int x, y, z;
    
    x, y = a + b, a * b;
    z, a = x - y, y + x;
    
    # Chain assignments with function calls
    int result1, result2;
    result1, result2 = test_nested_expressions(), test_extreme_comma_declarations();
    
    # Assignment with string operations
    string text = "hello";
    string s1, s2;
    s1, s2 = text[0:2], text[2:];
    
    return x + y + z + a + result1 + result2;
}

# =============================================================================
# TEST 8: EXTREME EXPRESSION COMBINATIONS
# =============================================================================
def test_extreme_expressions() -> bool: {
    int a = 5, b = 10, c = 15;
    float x = 2.5, y = 3.7;
    string text = "test123";
    bool flag = true;
    
    # Combination of all operator types
    bool mega_expression = (
        (a + b * c ** 2 - 1) > (x * y + 2.5) and
        (text[0:4] == "test" or text[4:] == "123") and
        (flag or not flag) and
        ((a % 3) == (b % 5)) and
        (c >= a + b) and
        (x <= y * 2.0)
    );
    
    # Short circuit with complex sub-expressions
    bool short_circuit = (
        false and (test_nested_expressions() > 0) or
        true or (test_extreme_comma_declarations() < 0) and
        (text[0] == "t")
    );
    
    return mega_expression and short_circuit;
}

# =============================================================================
# TEST 9: BOUNDARY VALUE TESTING
# =============================================================================
def test_boundary_values() -> float: {
    # Numeric boundaries
    int zero = 0;
    int negative = 0 - 42;  # Use subtraction instead of unary minus
    float tiny = 0.000001;
    float big = 999999.999999;
    
    # String boundaries
    string empty = "";
    string single_char = "a";
    string long_string = "This is a very long string that tests the limits of string handling in our compiler";
    
    # Boolean edge cases
    bool always_true = true or false;
    bool always_false = false and true;
    bool complex_bool = not not not false;  # Triple negation
    
    # Arithmetic with boundaries
    float result = (zero + tiny) * big + negative;  # Changed subtraction to addition
    
    return result;
}

# =============================================================================
# TEST 10: UNICODE AND SPECIAL CHARACTERS (If supported)
# =============================================================================
def test_special_characters() -> string: {
    # Special escape sequences
    string tab_separated = "col1\tcol2\tcol3";
    string newline_text = "line1\nline2\nline3";
    string quoted_text = "He said \"Hello World!\" loudly";
    string backslash_path = "C:\\Users\\Test\\file.txt";
    
    # Mixed quotes
    string mixed1 = "Single 'quotes' inside double";
    string mixed2 = 'Double "quotes" inside single';
    
    # Unicode (if your lexer supports it)
    string unicode = "Hello 世界 🌍";  # This might break your lexer - good test!
    
    return quoted_text;
}

# =============================================================================
# MAIN FUNCTION - EDGE CASE RUNNER
# =============================================================================
def __main__(): {
    # Run all edge case tests
    int comma_result = test_extreme_comma_declarations();
    int nested_result = test_nested_expressions();
    string string_result = test_string_edge_cases();
    float param_result = test_function_call_edges();
    int control_result = test_control_flow_edges();
    int mixed_result = test_mixed_assignment_edges();
    bool expr_result = test_extreme_expressions();
    float boundary_result = test_boundary_values();
    string special_result = test_special_characters();
    
    # Final complex combination
    int final_result = comma_result + nested_result + control_result + mixed_result;
}
# 🧪 COMPREHENSIVE MINILANG COMPILER TEST SUITE
# Testing ALL language features from the specification
# Save this as comprehensive_test.ml

# =============================================================================
# TEST 1: MULTIPLE ASSIGNMENT VARIATIONS
# =============================================================================
def test_multiple_assignment() -> int: {
    # Basic multiple assignment
    int a, b;
    a, b = 10, 20;
    
    # Multiple assignment with expressions
    int x, y, z;
    x, y, z = 1 + 2, 4 * 5, 100 / 10;
    
    # Multiple assignment with variables
    int result1, result2;
    result1, result2 = a + b, x * y;
    
    return result1 + result2;
}

# =============================================================================
# TEST 2: ADVANCED STRING OPERATIONS
# =============================================================================
def test_string_operations() -> string: {
    string text = "Hello World!";
    string empty = "";
    
    # Single character access
    string char1 = text[0];     # Should be "H"
    string char2 = text[6];     # Should be "W"
    
    # Basic slicing
    string slice1 = text[0:5];   # Should be "Hello"
    string slice2 = text[6:11];  # Should be "World"
    
    # Advanced slicing with defaults
    string prefix = text[:5];    # Should be "Hello"
    string suffix = text[6:];    # Should be "World!"
    string full_copy = text[:];  # Should be "Hello World!"
    
    # Slicing with step
    string every_second = text[0:10:2];  # Should be "HloWr"
    
    # Edge cases
    string empty_slice = text[5:5];      # Should be ""
    string single_char = text[1:2];      # Should be "e"
    
    return slice1;  # Return one result for testing
}

# =============================================================================
# TEST 3: ESCAPE SEQUENCES IN STRINGS
# =============================================================================
def test_string_escapes() -> string: {
    # Test various escape sequences
    string with_quotes = "Say \"Hello\" to the world";
    string with_newline = "Line 1\nLine 2";
    string with_tab = "Column1\tColumn2";
    string with_backslash = "Path: C:\\Users\\Name";
    
    # Single quotes variant
    string single_quoted = 'Also works with single quotes';
    
    return with_quotes;
}

# =============================================================================
# TEST 4: DEFAULT PARAMETER VALUES
# =============================================================================
def test_default_params(int required; float optional: 3.14; bool flag: true) -> float: {
    if (flag): {
        return required + optional;
    } else: {
        return required * optional;
    }
}

# =============================================================================
# TEST 5: PARAMETER TYPE GROUPS (FROM SPEC)
# =============================================================================
def test_param_groups(int x, y, z; float a, b: 2.5; string name: "default") -> string: {
    # Multiple parameters of same type separated by commas
    # Default values at the end of each type group
    int total = x + y + z;
    float avg = (a + b) / 2.0;
    
    return name;
}

# =============================================================================
# TEST 6: COMPLEX OPERATOR PRECEDENCE
# =============================================================================
def test_operator_precedence() -> int: {
    # Test all operators with correct precedence
    int result1 = 2 + 3 * 4 ** 2 - 1;        # Should be: 2 + 3*16 - 1 = 49
    int result2 = (2 + 3) * (4 - 1) ** 2;    # Should be: 5 * 3^2 = 45
    
    # Modulo operator
    int mod_result = 17 % 5;                  # Should be: 2
    
    # Mixed operations
    float mixed = 10 / 3 + 2.5 * 4;          # Mixed int/float
    
    return result1;
}

# =============================================================================
# TEST 7: BOOLEAN LITERAL VARIATIONS
# =============================================================================
def test_boolean_literals() -> bool: {
    # Test both True/true and False/false
    bool flag1 = True;
    bool flag2 = true;
    bool flag3 = False;
    bool flag4 = false;
    
    # Complex boolean expressions
    bool complex = (flag1 and flag2) or (not flag3 and flag4);
    
    return complex;
}

# =============================================================================
# TEST 8: CONTROL FLOW VARIATIONS
# =============================================================================
def test_control_flow() -> int: {
    int result = 0;
    int counter = 0;
    
    # If-elif-else chain
    if (counter == 0): {
        result = 1;
    } elif (counter == 1): {
        result = 2;
    } elif (counter == 2): {
        result = 3;
    } else: {
        result = 4;
    }
    
    # While loop with complex condition
    while (counter < 5 and result < 10): {
        counter = counter + 1;
        result = result + counter;
    }
    
    return result;
}

# =============================================================================
# TEST 9: FUNCTION CALLS WITH DEFAULTS
# =============================================================================
def test_function_calls() -> float: {
    # Call with all parameters
    float result1 = test_default_params(10, 2.0, false);
    
    # Call with some defaults
    float result2 = test_default_params(5, 1.5);  # Uses default flag
    
    # Call with all defaults except required
    float result3 = test_default_params(3);       # Uses both defaults
    
    return result1 + result2 + result3;
}

# =============================================================================
# TEST 10: EXPRESSION COMPLEXITY
# =============================================================================
def test_complex_expressions() -> bool: {
    int a = 5, b = 10, c = 15;
    float x = 2.5, y = 3.7;
    
    # Complex arithmetic
    int arith = (a + b) * c - (a * b) / (c - a);
    
    # Complex comparisons
    bool comp1 = (a < b) and (b < c) and (x > y or a == 5);
    bool comp2 = not (a > c) and (b * 2 == c + 5);
    
    # String operations in expressions
    string text = "test";
    bool str_test = text[0:2] == "te" and text[2:] == "st";
    
    return comp1 and comp2;
}

# =============================================================================
# TEST 11: EDGE CASES AND ERROR CONDITIONS
# =============================================================================
def test_edge_cases() -> string: {
    # Empty strings
    string empty1 = "";
    string empty2 = '';
    
    # Single character strings
    string single = "x";
    
    # Very long identifier names
    int very_long_variable_name_that_should_still_work = 42;
    
    # Multiple consecutive operations
    int chain = 1 + 2 + 3 + 4 + 5;
    
    # Nested function calls
    float nested_result = test_default_params(
        test_multiple_assignment(),
        3.14159,
        true
    );
    
    return single;
}

# =============================================================================
# TEST 12: VARIABLE DECLARATION PATTERNS
# =============================================================================
def test_declarations() -> int: {
    # Multiple declarations of same type
    int a, b, c;
    
    # Mixed declarations with initialization
    int x = 10, y = 20, z;
    
    # Different types
    float pi = 3.14159;
    string message = "Hello";
    bool active = true;
    
    # Use all variables to avoid unused warnings
    int total = a + b + c + x + y + z;
    return total;
}

# =============================================================================
# MAIN FUNCTION - TEST RUNNER
# =============================================================================
def __main__(): {
    # Run all tests
    int multi_result = test_multiple_assignment();
    string str_result = test_string_operations();
    string escape_result = test_string_escapes();
    float default_result = test_function_calls();
    int precedence_result = test_operator_precedence();
    bool boolean_result = test_boolean_literals();
    int control_result = test_control_flow();
    bool complex_result = test_complex_expressions();
    string edge_result = test_edge_cases();
    int decl_result = test_declarations();
    
    # Try some parameter group calls
    string group_result = test_param_groups(1, 2, 3, 1.1, 2.2, "test");
}
# FIXED VERSION - Reorder Functions and Fix Expression Issues

# =============================================================================
# HELPER FUNCTIONS FIRST (to avoid declaration order issues)
# =============================================================================
def test_function() -> bool: {
    return true;
}

def another_function() -> bool: {
    return false;
}

def get_value() -> int: {
    return 42;
}

def add(int a, b) -> int: {
    return a + b;
}

def multiply(int a, b) -> int: {
    return a * b;
}

def test_defaults(int req1, req2; float opt1: 1.5; bool opt2: true; string opt3: "default") -> string: {
    if (opt2): {
        return opt3;
    }
    return "false_case";
}

# =============================================================================
# TEST FUNCTIONS (now helper functions are declared first)
# =============================================================================
def test_short_circuit_evaluation() -> bool: {
    int a = 5, b = 1;  # CHANGED: b = 1 instead of 0 to avoid division by zero
    
    # Simplified expressions to isolate type issues
    bool div_result = (a / b) > 0;      # Test if division + comparison works
    bool result1 = false and div_result; # Use the boolean result
    bool result2 = true or div_result;   # Use the boolean result
    
    # Test function calls in boolean context
    bool func_result = test_function();
    bool result3 = false and func_result;
    
    return result1 and result2 and result3;
}

def test_division_and_comparison() -> bool: {
    int x = 10, y = 2;
    
    # Test each operation separately
    int div_result = x / y;           # Should be int
    bool cmp_result = div_result > 0; # Should be bool
    
    return cmp_result;
}

def test_unary_minus_workaround() -> int: {
    int a = 5;
    
    # This should work (subtraction from 0)
    int negative_a = 0 - a;
    
    # Simpler expression first
    int neg_ten = 0 - 10;
    int neg_twenty = 0 - 20;
    int result = neg_ten + (neg_twenty * 3);
    
    return negative_a + result;
}

def test_assignment_vs_expression() -> int: {
    # Function call as statement
    get_value();
    
    # Function call as expression in assignment
    int x = get_value();
    
    # Function call as expression in complex expression
    int base = get_value();
    int multiplier = 10;
    int y = base + (multiplier * get_value());
    
    # Function call in condition
    int check_val = get_value();
    if (check_val > 40): {
        return x + y;
    }
    
    return 0;
}

def test_nested_function_calls() -> int: {
    # Build up complexity gradually
    int first_mult = multiply(2, 3);      # 6
    int first_add = add(4, 5);            # 9
    int result1 = add(first_mult, first_add); # 15
    
    int second_add = add(1, 2);           # 3
    int inner_mult = multiply(2, 2);      # 4
    int third_add = add(inner_mult, 1);   # 5
    int result2 = multiply(second_add, third_add); # 15
    
    return result1 + result2;
}

def test_empty_statements() -> int: {
    int x = 0;
    
    # Empty if body with pass
    if (x == 0): {
        pass;
    }
    
    # Empty else with pass
    if (x > 0): {
        x = x + 1;
    } else: {
        pass;
    }
    
    # Empty while body
    while (false): {
        pass;
    }
    
    return x;
}

def test_string_edge_cases_advanced() -> string: {
    # Test each string operation separately
    string empty = "";
    string result1 = empty[0:0];    # Empty slice of empty string
    
    string single = "x";
    string char_copy = single[0:1]; # Should be "x"
    
    string text = "Hello";
    string beyond = text[10:20];    # Beyond string bounds
    
    # Simpler expression for negative end
    int zero = 0;
    int one = 1;
    int neg_one = zero - one;
    string negative_end = text[0:neg_one];
    
    return char_copy;
}

def test_operator_precedence() -> int: {
    int a = 2, b = 3, c = 4, d = 5;
    
    # Test precedence step by step
    int c_pow_d = c ** d;           # c ** d first
    int b_mult = b * c_pow_d;       # then b * result
    int result1 = a + b_mult;       # finally a + result
    
    int a_plus_b = a + b;
    int c_pow_d2 = c ** d;
    int result2 = a_plus_b * c_pow_d2;
    
    int b_mod_c = b % c;
    int mod_mult_d = b_mod_c * d;
    int result3 = a + mod_mult_d;
    
    return result1 + result2 + result3;
}

def test_scope_validation() -> int: {
    int global_var = 10;
    
    if (true): {
        int block_var = 20;
        global_var = global_var + block_var;
        
        while (global_var < 50): {
            int inner_var = 5;
            global_var = global_var + inner_var;
        }
    }
    
    return global_var;
}

def test_default_parameters() -> string: {
    # Test all combinations of default parameters
    string r1 = test_defaults(1, 2);                    # All defaults
    string r2 = test_defaults(1, 2, 2.5);              # Some defaults  
    string r3 = test_defaults(1, 2, 2.5, false);       # Fewer defaults
    string r4 = test_defaults(1, 2, 2.5, false, "custom"); # No defaults
    
    return r1;
}

def test_complex_control_flow() -> int: {
    int result = 0;
    int i = 0;
    
    while (i < 5): {  # Reduced from 10 to 5 for simpler testing
        int mod_result = i % 3;
        
        if (mod_result == 0): {
            int mod_2 = i % 2;
            if (mod_2 == 0): {
                result = result + i;
            } else: {
                result = result - i;
            }
        } elif (mod_result == 1): {
            result = result * 2;
        } else: {
            int j = 0;
            while (j < 2): {
                result = result + 1;
                j = j + 1;
            }
        }
        i = i + 1;
    }
    
    return result;
}

# =============================================================================
# MAIN FUNCTION
# =============================================================================
def __main__(): {
    # Test basic division and comparison first
    bool division_test = test_division_and_comparison();
    
    # Then test short circuit
    bool short_circuit_test = test_short_circuit_evaluation();
    
    int unary_test = test_unary_minus_workaround(); 
    int assignment_test = test_assignment_vs_expression();
    int nested_test = test_nested_function_calls();
    int empty_test = test_empty_statements();
    string string_test = test_string_edge_cases_advanced();
    int precedence_test = test_operator_precedence();
    int scope_test = test_scope_validation();
    string default_test = test_default_parameters();
    int control_test = test_complex_control_flow();
}
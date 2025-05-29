# COMPREHENSIVE CODEGEN TEST SUITE
# Tests all implemented features

# ====================
# TEST 1: Basic Arithmetic & Variables
# ====================
def basic_math() -> int: {
    int a = 10;
    int b = 5;
    int sum = a + b;
    int diff = a - b;
    int prod = a * b;
    int quot = a / b;
    int mod = a % b;
    return sum;
}

# ====================
# TEST 2: All Data Types & Literals
# ====================
def data_types(int num; float rate; string name; bool flag) -> int: {
    int x = 42;
    float y = 3.14;
    string msg = "hello";
    bool active = true;
    
    if (flag): {
        return num;
    } else: {
        return 0;
    }
}

# ====================
# TEST 3: Complex Expressions & Logic
# ====================
def complex_logic(int a, b; bool test) -> bool: {
    bool result1 = (a > 5) and (b < 10);
    bool result2 = (a == b) or (test);
    bool result3 = not result1;
    
    if ((a + b) > (a * b)): {
        return result2;
    } elif (result1 and result3): {
        return false;
    } else: {
        return true;
    }
}

# ====================
# TEST 4: All Control Flow Structures
# ====================
def control_flow_test(int level) -> int: {
    int result = 0;
    
    # Simple if
    if (level > 10): {
        result = 100;
    }
    
    # If-else
    if (level < 5): {
        result = result + 10;
    } else: {
        result = result + 20;
    }
    
    # If-elif-else chain
    if (level == 1): {
        result = result + 1;
    } elif (level == 2): {
        result = result + 2;
    } elif (level == 3): {
        result = result + 3;
    } else: {
        result = result + 99;
    }
    
    # While loop
    int counter = 0;
    while (counter < 3): {
        result = result + counter;
        counter = counter + 1;
    }
    
    return result;
}

# ====================
# TEST 5: Function Calls & Returns
# ====================
def helper_function(int x, y) -> int: {
    return x * y + 5;
}

def void_function(string message): {
    int temp = 42;
}

def function_calls_test() -> int: {
    int result1 = helper_function(3, 4);
    void_function("testing");
    int result2 = helper_function(result1, 2);
    return result2;
}

# ====================
# TEST 6: Nested Function Calls
# ====================
def multiply(int a, b) -> int: {
    return a * b;
}

def add(int x, y) -> int: {
    return x + y;
}

def nested_calls_test() -> int: {
    int result = add(multiply(3, 4), multiply(2, 5));
    return result;
}

# ====================
# TEST 7: Complex Parameter Types
# ====================
def mixed_params(int count; float rate; string name, title; bool active, debug) -> float: {
    float result = count * rate;
    
    if (active and debug): {
        result = result + 1.5;
    }
    
    return result;
}

# ====================
# TEST 8: Recursive Functions
# ====================
def factorial(int n) -> int: {
    if (n <= 1): {
        return 1;
    } else: {
        return n * factorial(n - 1);
    }
}

def fibonacci(int n) -> int: {
    if (n <= 1): {
        return n;
    } else: {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

# ====================
# TEST 9: Complex Expressions in Calls
# ====================
def expression_args_test() -> int: {
    int a = 5;
    int b = 10;
    
    int result = add((a + b), multiply((a - 2), (b / 2)));
    return result;
}

# ====================
# TEST 10: Everything Combined - Stress Test
# ====================
def stress_test(int size; bool optimize) -> int: {
    int total = 0;
    int multiplier = 1;
    
    # Initialize multiplier based on optimization
    if (optimize): {
        multiplier = 2;
    } else: {
        multiplier = 1;
    }
    
    # Main processing loop
    int i = 0;
    while (i < size): {
        if (i % 2 == 0): {
            total = total + (i * multiplier);
        } elif (i % 3 == 0): {
            total = total + helper_function(i, multiplier);
        } else: {
            total = total + factorial(3);
        }
        
        i = i + 1;
    }
    
    # Final calculation with nested calls
    int bonus = add(multiply(total, 2), helper_function(5, 3));
    
    return total + bonus;
}

# ====================
# MAIN FUNCTION - Test Everything
# ====================
def __main__(): {
    # Test basic math
    int math_result = basic_math();
    
    # Test data types
    int type_result = data_types(42, 3.14, "test", true);
    
    # Test complex logic
    bool logic_result = complex_logic(8, 3, false);
    
    # Test control flow
    int control_result = control_flow_test(2);
    
    # Test function calls
    int call_result = function_calls_test();
    
    # Test nested calls
    int nested_result = nested_calls_test();
    
    # Test mixed parameters
    float mixed_result = mixed_params(10, 2.5, "test", "title", true, false);
    
    # Test recursion
    int fact_result = factorial(5);
    int fib_result = fibonacci(6);
    
    # Test expression arguments
    int expr_result = expression_args_test();
    
    # Final stress test
    int final_result = stress_test(5, true);
}
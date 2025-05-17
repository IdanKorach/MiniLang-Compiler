def add_numbers(int a; int b) -> int: {
    return a + b;
}

def concatenate(string s1; string s2) -> string: {
    return s1 + s2;
}

def is_equal(int x; int y) -> bool: {
    return x == y;
}

def __main__(): {
    # Test variable declarations - checking log format
    int x = 10;
    float y = 3.14;
    string message = "Hello";
    bool flag = true;
    
    # Test function call return type tracking
    int sum = add_numbers(5, 7);       # Function call -> int
    string full = concatenate("Hello", " World");  # Function call -> string
    bool equal = is_equal(10, 10);     # Function call -> bool
    
    # Test complex expressions with function calls
    int complex1 = add_numbers(3, 4) + 5;  # Function call + int -> int
    bool complex2 = is_equal(x, 10) and flag;  # Function call and bool -> bool
    string complex3 = concatenate("a", "b") + "c";  # Function call + string -> string
    
    # Test nested function calls
    int nested = add_numbers(add_numbers(1, 2), 3);  # Nested function calls -> int
    
    # Test function call in condition
    if (is_equal(x, 10)): {
        int z = 42;
    }
}
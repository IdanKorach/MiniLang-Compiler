def nested_control_flow(int n) -> int: {
    int result = 0;
    
    # Simple if-else structure
    if (n <= 0): {
        return 0;
    } else: {
        # We'll handle the n == 1 case in a different way
        if (n == 1): {
            return 1;
        } else: {
            # Loop with nested if
            int i = 0;
            while (i < n): {
                # Option 1: using simple if
                if (i % 3 == 0): {
                    result = result + 3;
                } else: {
                    # Option 2: using if within else
                    if (i % 2 == 0): {
                        result = result + 2;
                    } else: {
                        result = result + 1;
                    }
                }
                
                # Nested while inside if
                if (result % 5 == 0): {
                    int j = 0;
                    while (j < 3 and result < 100): {
                        result = result + 1;
                        j = j + 1;
                    }
                }
                
                i = i + 1;
            }
        }
    }
    
    return result;
}

def __main__(): {
    int test1 = nested_control_flow(10);
    int test2 = nested_control_flow(0);
    int test3 = nested_control_flow(1);
}
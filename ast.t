def func1(): {
    int x = 1;  # Valid
}

def func2(): {
    int x = 2;  # Valid - different scope
}

def __main__(): {
    int x = 3;  # Valid - different scope
}
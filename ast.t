def func1(): {
    int x = 1;
}

def func2(): {
    func1();  # Valid - func1 declared above
}

def __main__(): {
    func1();  # Valid
    func2();  # Valid
}
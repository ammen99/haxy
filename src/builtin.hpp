/* This file contains the *standard library* for haxy */
#include <iostream>
#include "val.hpp"

void print() {
    std::cout << std::endl;
}

template<class... Args> void print(Value v, Args ... args) {
    std::cout << v << " "; 
    print(args...);
}

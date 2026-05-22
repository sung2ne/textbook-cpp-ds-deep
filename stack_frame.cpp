// filename: stack_frame.cpp
// g++ -std=c++17 -O0 -fno-omit-frame-pointer -o stack_frame stack_frame.cpp

#include <iostream>
#include <cstdint>

void bar() {
    int local_bar = 300;
    std::cout << "bar::local_bar 주소: " << &local_bar << "\n";
}

void foo() {
    int local_foo = 100;
    int array[4] = {1, 2, 3, 4};

    std::cout << "foo::local_foo 주소: " << &local_foo << "\n";
    std::cout << "foo::array[0] 주소: " << &array[0] << "\n";
    std::cout << "foo::array[3] 주소: " << &array[3] << "\n";
    bar();  // bar의 스택 프레임이 추가됨
}

int main() {
    int main_var = 42;
    std::cout << "main::main_var 주소: " << &main_var << "\n";
    foo();
    return 0;
}

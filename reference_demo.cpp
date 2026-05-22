// filename: reference_demo.cpp
// g++ -std=c++17 -O2 -o reference_demo reference_demo.cpp

#include <iostream>

int main() {
    int x = 42;

    // lvalue reference — 기존 변수의 별명
    int& ref = x;
    ref = 100;  // x도 100이 됨
    std::cout << "x = " << x << "\n";  // 100

    // const reference — 임시 객체나 rvalue도 받을 수 있음
    const int& cref = 200;  // 임시 객체를 수명 연장해서 저장
    // cref = 300;  // ❌ 불가

    // 참조는 초기화 후 재바인딩 불가
    int y = 50;
    int& ref2 = y;
    ref2 = x;  // ref2가 x를 가리키는 게 아니라, y에 x 값을 대입함

    std::cout << "y = " << y << "\n";  // 100

    return 0;
}

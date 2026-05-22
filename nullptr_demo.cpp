// filename: nullptr_demo.cpp
// g++ -std=c++17 -O2 -o nullptr_demo nullptr_demo.cpp

#include <iostream>

void process(int* p)  { std::cout << "int* 버전\n"; }
void process(int n)   { std::cout << "int 버전\n"; }

int main() {
    process(nullptr);  // int* 버전 호출 — 명확
    process(NULL);     // int 버전 호출 — NULL은 정수 0이므로 모호
    process(0);        // int 버전 호출

    // nullptr의 타입은 std::nullptr_t
    // 어떤 포인터 타입과도 암묵적으로 변환됨
    int* p = nullptr;
    if (!p) std::cout << "포인터가 null입니다\n";

    return 0;
}

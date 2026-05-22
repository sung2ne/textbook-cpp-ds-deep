// filename: const_pointer.cpp
// g++ -std=c++17 -O2 -Wall -o const_pointer const_pointer.cpp

#include <iostream>

int main() {
    int a = 10;
    int b = 20;

    // 1) int* — 포인터도 가변, 가리키는 값도 가변
    int* p1 = &a;
    *p1 = 11;   // 값 변경 OK
    p1 = &b;    // 포인터 자체 변경 OK

    // 2) const int* — 가리키는 값은 불변, 포인터 자체는 가변
    //    (pointer to const int)
    const int* p2 = &a;
    // *p2 = 12;  // ❌ 컴파일 에러
    p2 = &b;    // 포인터 자체 변경 OK

    // 3) int* const — 포인터 자체는 불변, 가리키는 값은 가변
    //    (const pointer to int)
    int* const p3 = &a;
    *p3 = 13;   // 값 변경 OK
    // p3 = &b;  // ❌ 컴파일 에러

    // 4) const int* const — 포인터도 불변, 가리키는 값도 불변
    //    (const pointer to const int)
    const int* const p4 = &a;
    // *p4 = 14;  // ❌ 컴파일 에러
    // p4 = &b;   // ❌ 컴파일 에러

    std::cout << "p1=" << *p1 << " p3=" << *p3 << "\n";

    return 0;
}

// filename: value_category.cpp
// g++ -std=c++17 -O2 -o value_category value_category.cpp

#include <iostream>
#include <type_traits>
#include <string>

// value category를 출력하는 헬퍼 (decltype 활용)
#define PRINT_CATEGORY(expr) \
    do { \
        using T = decltype((expr)); \
        if constexpr (std::is_lvalue_reference_v<T>) \
            std::cout << #expr << " → lvalue\n"; \
        else if constexpr (std::is_rvalue_reference_v<T>) \
            std::cout << #expr << " → xvalue\n"; \
        else \
            std::cout << #expr << " → prvalue\n"; \
    } while(0)

int global_x = 10;

int getValue()      { return 42; }      // prvalue 반환
int& getRef()       { return global_x; }// lvalue 반환
int&& getRvalueRef(){ return std::move(global_x); }  // xvalue 반환

int main() {
    int x = 5;
    int arr[3] = {1, 2, 3};
    std::string s = "hello";

    std::cout << "=== lvalue 예시 ===\n";
    PRINT_CATEGORY(x);           // lvalue — 이름 있는 변수
    PRINT_CATEGORY(global_x);   // lvalue — 전역 변수
    PRINT_CATEGORY(arr[0]);     // lvalue — 배열 원소
    PRINT_CATEGORY(*(&x));      // lvalue — 포인터 역참조
    PRINT_CATEGORY(getRef());   // lvalue — lvalue 참조 반환 함수

    std::cout << "\n=== prvalue 예시 ===\n";
    PRINT_CATEGORY(42);         // prvalue — 정수 리터럴
    PRINT_CATEGORY(x + 1);     // prvalue — 산술 연산 결과
    PRINT_CATEGORY(getValue()); // prvalue — 값 반환 함수
    PRINT_CATEGORY(std::string("world"));  // prvalue — 임시 객체

    std::cout << "\n=== xvalue 예시 ===\n";
    PRINT_CATEGORY(std::move(x));           // xvalue — std::move 결과
    PRINT_CATEGORY(static_cast<int&&>(x));  // xvalue — rvalue ref 캐스트
    PRINT_CATEGORY(getRvalueRef());         // xvalue — rvalue ref 반환

    // lvalue는 주소를 취할 수 있음
    int* px = &x;         // OK
    // int* p2 = &42;     // ❌ prvalue는 주소 없음
    // int* p3 = &std::move(x);  // ❌ xvalue도 주소 취하기 불가

    return 0;
}

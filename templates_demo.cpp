// filename: templates_demo.cpp
// g++ -std=c++17 -O2 -o templates_demo templates_demo.cpp

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

// 함수 템플릿: 타입 T에 대해 최댓값 반환
template<typename T>
const T& max_val(const T& a, const T& b) {
    return (a < b) ? b : a;
}

// 명시적 특수화 — const char*는 별도 처리
template<>
const char* const& max_val<const char*>(const char* const& a, const char* const& b) {
    return (std::strcmp(a, b) < 0) ? b : a;
}

// 다중 타입 파라미터
template<typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    return a + b;
}

// C++17: auto 반환 타입 추론
template<typename T, typename U>
auto multiply(T a, U b) {
    return a * b;
}

int main() {
    std::cout << "=== 함수 템플릿 ===\n";
    std::cout << max_val(3, 7) << "\n";          // int 버전
    std::cout << max_val(3.14, 2.71) << "\n";    // double 버전
    std::cout << max_val(std::string("abc"), std::string("xyz")) << "\n";

    // 명시적 타입 지정
    std::cout << max_val<double>(3, 7.5) << "\n";  // int→double 변환

    std::cout << add(3, 4.5) << "\n";       // int + double = double
    std::cout << multiply(3, 4.5f) << "\n"; // int * float = float
}

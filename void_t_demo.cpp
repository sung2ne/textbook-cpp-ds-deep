// filename: void_t_demo.cpp
// g++ -std=c++17 -O2 -o void_t_demo void_t_demo.cpp

#include <iostream>
#include <type_traits>

// std::void_t: 모든 인자를 void로 변환, 유효하지 않으면 치환 실패
// 기본 템플릿 — has_begin 없음
template<typename T, typename = void>
struct has_begin : std::false_type {};

// 특수화 — T::begin()이 있을 때만 유효
template<typename T>
struct has_begin<T, std::void_t<decltype(std::declval<T>().begin())>>
    : std::true_type {};

template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

struct MyContainer {
    int* begin() { return nullptr; }
    int* end()   { return nullptr; }
    std::size_t size() { return 0; }
};

struct NoBegin {
    int value;
};

int main() {
    std::cout << "MyContainer has begin: " << has_begin<MyContainer>::value << "\n";  // 1
    std::cout << "NoBegin has begin: "     << has_begin<NoBegin>::value   << "\n";    // 0
    std::cout << "MyContainer has size: "  << has_size<MyContainer>::value  << "\n";  // 1
    std::cout << "int has size: "          << has_size<int>::value          << "\n";  // 0

    // 범위 기반 for가 가능한지 컴파일 타임 확인
    static_assert(has_begin<MyContainer>::value, "MyContainer는 반복 가능해야 함");
    static_assert(!has_begin<NoBegin>::value,    "NoBegin은 반복 불가");

    return 0;
}

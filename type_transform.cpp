// filename: type_transform.cpp
// g++ -std=c++17 -O2 -o type_transform type_transform.cpp

#include <iostream>
#include <type_traits>

template<typename T>
void show_type_transforms() {
    // 상수성 제거/추가
    using NoConst = std::remove_const_t<T>;
    using WithConst = std::add_const_t<T>;

    // 참조 제거/추가
    using NoRef = std::remove_reference_t<T>;
    using AsLRef = std::add_lvalue_reference_t<T>;
    using AsRRef = std::add_rvalue_reference_t<T>;

    // 포인터 제거/추가
    using NoPtr = std::remove_pointer_t<T>;
    using AsPtr = std::add_pointer_t<T>;

    // cv (const volatile) 제거
    using NoCv = std::remove_cv_t<T>;

    // decay: 배열→포인터, 함수→함수포인터, const/ref 제거
    using Decayed = std::decay_t<T>;
}

// std::remove_reference_t 실용 예
template<typename T>
T& as_lvalue(T&& x) {
    // T&&에서 참조 제거 후 lvalue ref 반환
    static_assert(!std::is_reference_v<T> || std::is_lvalue_reference_v<T&&>);
    return static_cast<std::remove_reference_t<T>&>(x);
}

// conditional — 조건에 따라 타입 선택
template<typename T>
using StorageType = std::conditional_t<
    sizeof(T) <= sizeof(void*),
    T,        // 작은 타입: 값으로 저장
    T*        // 큰 타입: 포인터로 저장
>;

int main() {
    static_assert(std::is_same_v<std::remove_const_t<const int>, int>);
    static_assert(std::is_same_v<std::remove_reference_t<int&&>, int>);
    static_assert(std::is_same_v<std::remove_pointer_t<int*>, int>);
    static_assert(std::is_same_v<std::decay_t<const int&>, int>);
    static_assert(std::is_same_v<std::decay_t<int[5]>, int*>);

    // conditional 예시
    StorageType<int>          si;     // int (4 <= 8)
    StorageType<long double>  sld;   // long double* (16 > 8)

    std::cout << "StorageType<int> size:  "
              << sizeof(StorageType<int>) << "\n";          // 4
    std::cout << "StorageType<long double> size: "
              << sizeof(StorageType<long double>) << "\n";  // 8 (포인터 크기)

    return 0;
}

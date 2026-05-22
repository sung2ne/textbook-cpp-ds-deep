// filename: custom_trait.cpp
// g++ -std=c++17 -O2 -o custom_trait custom_trait.cpp

#include <iostream>
#include <type_traits>

// 특정 타입이 스마트 포인터인지 검사
template<typename T>
struct is_smart_ptr : std::false_type {};

template<typename T>
struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};

template<typename T>
struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};

template<typename T>
struct is_smart_ptr<std::weak_ptr<T>> : std::true_type {};

// 헬퍼 변수 템플릿 (C++14 스타일)
template<typename T>
inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;

// pair의 두 타입이 같은지 검사
template<typename T>
struct is_homogeneous_pair : std::false_type {};

template<typename T>
struct is_homogeneous_pair<std::pair<T, T>> : std::true_type {};

int main() {
    static_assert(is_smart_ptr_v<std::unique_ptr<int>>);
    static_assert(is_smart_ptr_v<std::shared_ptr<double>>);
    static_assert(!is_smart_ptr_v<int*>);
    static_assert(!is_smart_ptr_v<int>);

    static_assert(is_homogeneous_pair<std::pair<int, int>>::value);
    static_assert(!is_homogeneous_pair<std::pair<int, double>>::value);

    std::cout << "is_smart_ptr<unique_ptr<int>>: "
              << is_smart_ptr_v<std::unique_ptr<int>> << "\n";
    std::cout << "is_smart_ptr<int*>: "
              << is_smart_ptr_v<int*> << "\n";

    return 0;
}

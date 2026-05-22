// filename: enable_if_practical.cpp
// g++ -std=c++17 -O2 -o enable_if_practical enable_if_practical.cpp

#include <iostream>
#include <type_traits>
#include <iterator>
#include <vector>
#include <list>

// 정수형이면 비트 이동, 그 외엔 일반 나눗셈
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
halve(T x) {
    return x >> 1;  // 비트 이동 (정수형 최적화)
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T>
halve(T x) {
    return x * T(0.5);  // 부동소수점 곱셈
}

// 컨테이너 크기 반환 — random access iterator가 있으면 O(1), 없으면 O(n)
template<typename Container>
std::enable_if_t<
    std::is_same_v<
        typename std::iterator_traits<typename Container::iterator>::iterator_category,
        std::random_access_iterator_tag
    >,
    std::size_t>
fast_size(const Container& c) {
    std::cout << "[O(1)] ";
    return c.size();
}

template<typename Container>
std::enable_if_t<
    !std::is_same_v<
        typename std::iterator_traits<typename Container::iterator>::iterator_category,
        std::random_access_iterator_tag
    >,
    std::size_t>
fast_size(const Container& c) {
    std::cout << "[O(n)] ";
    return std::distance(c.begin(), c.end());
}

int main() {
    std::cout << halve(100) << "\n";   // 정수 버전: 50 (비트 이동)
    std::cout << halve(3.14) << "\n";  // double 버전: 1.57

    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int>   lst = {1, 2, 3, 4, 5};

    std::cout << fast_size(vec) << "\n";  // [O(1)] 5
    std::cout << fast_size(lst) << "\n";  // [O(n)] 5

    return 0;
}

// filename: concepts_demo.cpp
// g++ -std=c++20 -O2 -o concepts_demo concepts_demo.cpp

#include <iostream>
#include <concepts>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <ranges>

// Concept 정의
template<typename T>
concept SignedInteger = std::integral<T> && std::signed_integral<T>;

template<typename T>
concept Printable = requires(T x) {
    { std::cout << x } -> std::same_as<std::ostream&>;
};

// Sortable: <, swap, random access iterator 모두 필요
template<typename Container>
concept Sortable = requires(Container c) {
    { c.begin() } -> std::random_access_iterator;
    { c.end()   } -> std::random_access_iterator;
    requires std::totally_ordered<typename Container::value_type>;
};

// Container: begin/end/size를 가진 타입
template<typename T>
concept Container = requires(T c) {
    { c.begin() };
    { c.end()   };
    { c.size()  } -> std::convertible_to<std::size_t>;
    typename T::value_type;
};

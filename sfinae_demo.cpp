// filename: sfinae_demo.cpp
// g++ -std=c++17 -O2 -o sfinae_demo sfinae_demo.cpp

#include <iostream>
#include <type_traits>
#include <string>

// 기본 SFINAE 예제
// enable_if<조건, 반환타입>::type — 조건이 false면 type이 정의되지 않음 → 치환 실패
template<typename T>
typename std::enable_if<std::is_integral<T>::value, std::string>::type
describe(T value) {
    return "정수형: " + std::to_string(value);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, std::string>::type
describe(T value) {
    return "부동소수점: " + std::to_string(value);
}

// C++14 단축형 _t, _v 접미사 사용
template<typename T>
std::enable_if_t<std::is_pointer_v<T>, void>
print_address(T ptr) {
    std::cout << "주소: " << (void*)ptr << "\n";
}

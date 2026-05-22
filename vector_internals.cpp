// filename: vector_internals.cpp
// g++ -std=c++17 -O2 -Wall -o vector_internals vector_internals.cpp
#include <iostream>
#include <vector>
#include <string>

template<typename T>
void print_vector_state(const std::string& label, const std::vector<T>& v) {
    std::cout << label << "\n";
    std::cout << "  size:     " << v.size()     << "\n";
    std::cout << "  capacity: " << v.capacity() << "\n";
    std::cout << "  data():   " << static_cast<const void*>(v.data()) << "\n";

    if (!v.empty()) {
        std::cout << "  &v[0]:    " << static_cast<const void*>(&v[0]) << "\n";
        std::cout << "  &v.back():" << static_cast<const void*>(&v.back()) << "\n";
    }
    std::cout << "\n";
}

int main() {
    // 기본 생성: size=0, capacity=0
    std::vector<int> v;
    print_vector_state("기본 생성 후", v);

    // push_back으로 원소 추가
    for (int i = 1; i <= 5; ++i) {
        v.push_back(i * 10);
        std::cout << "push_back(" << i * 10 << ") -> "
                  << "size=" << v.size()
                  << " capacity=" << v.capacity() << "\n";
    }
    std::cout << "\n";

    // 연속 메모리 확인
    std::cout << "연속 메모리 확인:\n";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << "  &v[" << i << "] = "
                  << static_cast<void*>(&v[i])
                  << " (v.data()+" << i << " = "
                  << static_cast<void*>(v.data() + i) << ")\n";
    }
    std::cout << "\n";

    // vector<bool> 함정 시연
    std::vector<bool> vb = {true, false, true};
    std::cout << "vector<bool> sizeof: " << sizeof(vb) << "\n";
    auto elem = vb[0];  // bool이 아닌 프록시 객체
    std::cout << "auto elem = vb[0] -> type is proxy: "
              << std::boolalpha << (elem == true) << "\n";

    // 대안: vector<uint8_t>
    std::vector<uint8_t> vu8 = {1, 0, 1};
    uint8_t& ref = vu8[0];  // 정상적인 참조
    std::cout << "vector<uint8_t>: &vu8[0] = "
              << static_cast<void*>(&vu8[0]) << " (정상)\n";

    return 0;
}

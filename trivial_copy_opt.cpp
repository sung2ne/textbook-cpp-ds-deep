// filename: trivial_copy_opt.cpp
// g++ -std=c++17 -O2 -o trivial_copy_opt trivial_copy_opt.cpp

#include <iostream>
#include <type_traits>
#include <cstring>

struct Point { float x, y; };
struct ComplexObj { std::string name; int value; };

template<typename T>
void copy_elements(T* dst, const T* src, std::size_t n) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        // memcpy 사용 — O(n) 바이트 복사, 생성자 없음
        std::cout << "memcpy 사용 (" << n * sizeof(T) << " bytes)\n";
        std::memcpy(dst, src, n * sizeof(T));
    } else {
        // 원소별 복사 생성자 호출
        std::cout << "복사 생성자 사용 (" << n << "개)\n";
        for (std::size_t i = 0; i < n; ++i) {
            new (dst + i) T(src[i]);
        }
    }
}

int main() {
    static_assert(std::is_trivially_copyable_v<Point>);
    static_assert(!std::is_trivially_copyable_v<ComplexObj>);
    static_assert(std::is_trivially_copyable_v<int>);

    Point points[5] = {{1,2},{3,4},{5,6},{7,8},{9,10}};
    Point copies[5];
    copy_elements(copies, points, 5);  // memcpy 사용

    ComplexObj objs[3] = {{"Alice",1},{"Bob",2},{"Carol",3}};
    ComplexObj objs2[3];
    copy_elements(objs2, objs, 3);     // 복사 생성자 사용

    return 0;
}

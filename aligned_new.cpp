// filename: aligned_new.cpp
// g++ -std=c++17 -O2 -o aligned_new aligned_new.cpp

#include <iostream>
#include <new>
#include <cstdint>

// 32바이트 정렬 필요 (SIMD 연산용)
struct alignas(32) SimdVector {
    float data[8];  // 256-bit AVX 레지스터에 대응
};

int main() {
    // alignas(32)가 있으면 컴파일러가 자동으로
    // operator new(sizeof(SimdVector), std::align_val_t{32}) 를 호출
    SimdVector* v = new SimdVector;

    uintptr_t addr = reinterpret_cast<uintptr_t>(v);
    std::cout << "주소: " << v << "\n";
    std::cout << "32바이트 정렬 여부: "
              << ((addr % 32 == 0) ? "OK" : "실패") << "\n";

    delete v;

    // 수동으로 정렬된 메모리 확보
    void* raw = ::operator new(sizeof(SimdVector), std::align_val_t{32});
    SimdVector* v2 = new (raw) SimdVector;
    v2->~SimdVector();
    ::operator delete(raw, std::align_val_t{32});

    return 0;
}

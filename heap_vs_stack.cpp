// filename: heap_vs_stack.cpp
// g++ -std=c++17 -O2 -o heap_vs_stack heap_vs_stack.cpp

#include <iostream>
#include <chrono>
#include <vector>

constexpr int N = 1'000'000;

int main() {
    // 스택 변수 (배열 크기 주의 — 너무 크면 스택 오버플로우)
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int x = i * 2;  // 스택 할당, 즉시 사용
        (void)x;
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    // 힙 변수
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int* p = new int(i * 2);  // 힙 할당
        delete p;
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    using ms = std::chrono::microseconds;
    std::cout << "스택 루프: "
              << std::chrono::duration_cast<ms>(t2 - t1).count() << " μs\n";
    std::cout << "힙 루프:   "
              << std::chrono::duration_cast<ms>(t4 - t3).count() << " μs\n";
    std::cout << "힙 할당이 스택 대비 약 "
              << std::chrono::duration_cast<ms>(t4 - t3).count() /
                 std::max(1LL, std::chrono::duration_cast<ms>(t2 - t1).count())
              << "배 느림\n";

    return 0;
}

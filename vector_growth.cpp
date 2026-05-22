// filename: vector_growth.cpp
// g++ -std=c++17 -O2 -Wall -o vector_growth vector_growth.cpp
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

// capacity 변화를 추적하며 push_back
void track_capacity_growth(int n) {
    std::vector<int> v;
    size_t prev_capacity = 0;
    int realloc_count = 0;

    std::cout << "=== " << n << "개 push_back capacity 추적 ===\n";
    for (int i = 0; i < n; ++i) {
        v.push_back(i);
        if (v.capacity() != prev_capacity) {
            std::cout << "  재할당: size=" << v.size()
                      << " capacity=" << v.capacity()
                      << " (이전: " << prev_capacity << ")\n";
            prev_capacity = v.capacity();
            ++realloc_count;
        }
    }
    std::cout << "  총 재할당 횟수: " << realloc_count << "\n\n";
}

// reserve 유무에 따른 성능 비교
void benchmark_reserve(int n) {
    using Clock = std::chrono::high_resolution_clock;

    // reserve 없이
    auto t1 = Clock::now();
    {
        std::vector<std::string> v;
        for (int i = 0; i < n; ++i) {
            v.push_back("item_" + std::to_string(i));
        }
    }
    auto t2 = Clock::now();
    auto ms_no_reserve = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    // reserve 사용
    auto t3 = Clock::now();
    {
        std::vector<std::string> v;
        v.reserve(n);  // 미리 확보
        for (int i = 0; i < n; ++i) {
            v.push_back("item_" + std::to_string(i));
        }
    }
    auto t4 = Clock::now();
    auto ms_with_reserve = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    std::cout << "=== reserve 성능 비교 (n=" << n << ") ===\n";
    std::cout << "  reserve 없이: " << ms_no_reserve << " μs\n";
    std::cout << "  reserve 사용: " << ms_with_reserve << " μs\n";
    std::cout << "  속도 향상: "
              << std::fixed << std::setprecision(1)
              << (double)ms_no_reserve / ms_with_reserve << "x\n\n";
}

// shrink_to_fit 동작 시연
void demo_shrink() {
    std::vector<int> v(1000, 0);
    std::cout << "=== shrink_to_fit 시연 ===\n";
    std::cout << "  초기: size=" << v.size() << " cap=" << v.capacity() << "\n";

    v.resize(10);
    std::cout << "  resize(10) 후: size=" << v.size() << " cap=" << v.capacity() << "\n";

    v.shrink_to_fit();
    std::cout << "  shrink_to_fit() 후: size=" << v.size() << " cap=" << v.capacity() << "\n";

    // swap trick
    std::vector<int>(v).swap(v);
    std::cout << "  swap trick 후: size=" << v.size() << " cap=" << v.capacity() << "\n";
}

int main() {
    track_capacity_growth(20);
    track_capacity_growth(1000);
    benchmark_reserve(100000);
    demo_shrink();
    return 0;
}

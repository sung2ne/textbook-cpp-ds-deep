// filename: atomic_basics.cpp
// g++ -std=c++17 -O2 -pthread -o atomic_basics atomic_basics.cpp

#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

// 가장 단순한 atomic 카운터
std::atomic<int> atomic_counter{0};
int              unsafe_counter = 0; // 비교용

void increment_atomic(int n) {
    for (int i = 0; i < n; ++i)
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
        // 또는 단순히: ++atomic_counter;
}

void increment_unsafe(int n) {
    for (int i = 0; i < n; ++i)
        ++unsafe_counter; // race condition!
}

int main()
{
    constexpr int N = 5'000'000;

    // atomic 버전
    {
        atomic_counter = 0;
        auto t0 = std::chrono::high_resolution_clock::now();
        std::thread t1(increment_atomic, N);
        std::thread t2(increment_atomic, N);
        t1.join(); t2.join();
        auto t1e = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1e - t0).count();
        std::cout << "atomic:   " << atomic_counter.load() << " (" << ms << " ms)\n";
    }

    // unsafe 버전 (race condition 시연)
    {
        unsafe_counter = 0;
        auto t0 = std::chrono::high_resolution_clock::now();
        std::thread t1(increment_unsafe, N);
        std::thread t2(increment_unsafe, N);
        t1.join(); t2.join();
        auto t1e = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1e - t0).count();
        std::cout << "unsafe:   " << unsafe_counter << " (기대: " << 2*N << ", " << ms << " ms)\n";
    }

    return 0;
}

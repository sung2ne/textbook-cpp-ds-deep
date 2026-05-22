// filename: false_sharing.cpp
// g++ -std=c++17 -O2 -pthread -o false_sharing false_sharing.cpp

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

// 나쁜 설계: 카운터들이 한 배열에 붙어 있음
struct BadCounters {
    long long c[8]; // 8 × 8바이트 = 64바이트 — 딱 한 cache line!
};

// 좋은 설계: 각 카운터가 독립된 cache line에 위치
struct alignas(64) GoodCounter {
    long long value;
    // 64바이트 정렬이므로 이 구조체는 독립된 cache line을 차지
};

constexpr long long ITER = 100'000'000LL;

void increment_bad(BadCounters& counters, int idx)
{
    for (long long i = 0; i < ITER; ++i)
        counters.c[idx]++;
}

void increment_good(GoodCounter& counter)
{
    for (long long i = 0; i < ITER; ++i)
        counter.value++;
}

template<typename F>
double bench_ms(F&& fn)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main()
{
    const int NTHREADS = 4;

    // False Sharing 시연
    BadCounters bad{};
    double t_bad = bench_ms([&]{
        std::vector<std::thread> threads;
        for (int i = 0; i < NTHREADS; ++i)
            threads.emplace_back(increment_bad, std::ref(bad), i);
        for (auto& t : threads) t.join();
    });

    // 올바른 설계
    std::vector<GoodCounter> good(NTHREADS);
    double t_good = bench_ms([&]{
        std::vector<std::thread> threads;
        for (int i = 0; i < NTHREADS; ++i)
            threads.emplace_back(increment_good, std::ref(good[i]));
        for (auto& t : threads) t.join();
    });

    std::cout << "False Sharing (나쁜 설계): " << t_bad  << " ms\n";
    std::cout << "Padded (좋은 설계)       : " << t_good << " ms\n";
    std::cout << "Speedup: " << t_bad / t_good << "x\n";

    // sizeof 확인
    std::cout << "sizeof(GoodCounter) = " << sizeof(GoodCounter) << " bytes\n";
    return 0;
}

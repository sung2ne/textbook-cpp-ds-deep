// filename: prefetch_demo.cpp
// g++ -std=c++17 -O2 -o prefetch_demo prefetch_demo.cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

// 프리페치 없이 연결 리스트 순회
long long traverse_no_prefetch(const std::vector<int>& next,
                                const std::vector<int>& data,
                                int start, int count)
{
    long long sum = 0;
    int cur = start;
    for (int i = 0; i < count; ++i) {
        sum += data[cur];
        cur = next[cur];
    }
    return sum;
}

// 미리 2단계 앞 노드를 프리페치
long long traverse_with_prefetch(const std::vector<int>& next,
                                  const std::vector<int>& data,
                                  int start, int count)
{
    long long sum = 0;
    int cur = start;
    // 2단계 앞을 프리페치
    int ahead1 = next[cur];
    int ahead2 = next[ahead1];

    for (int i = 0; i < count; ++i) {
        __builtin_prefetch(&data[ahead2], 0, 1);  // 읽기, L2 보존
        __builtin_prefetch(&next[ahead2], 0, 1);
        sum += data[cur];
        cur = next[cur];
        ahead1 = next[ahead1];
        if (i + 2 < count) ahead2 = next[ahead2];
    }
    return sum;
}

int main()
{
    constexpr int N     = 2'000'000;
    constexpr int STEPS = 1'000'000;

    // 랜덤 순서 연결 리스트 구성
    std::vector<int> next(N), data(N, 1);
    std::iota(next.begin(), next.end(), 1);
    next.back() = 0;

    std::mt19937 rng(42);
    std::shuffle(next.begin(), next.end(), rng);
    int start = static_cast<int>(rng() % N);

    auto bench = [&](auto fn, const char* label) {
        auto t0 = std::chrono::high_resolution_clock::now();
        volatile long long s = fn(next, data, start, STEPS);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << label << ": " << ms << " ms\n";
    };

    bench(traverse_no_prefetch,   "프리페치 없음");
    bench(traverse_with_prefetch, "프리페치 있음");
    return 0;
}

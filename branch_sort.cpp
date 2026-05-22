// filename: branch_sort.cpp
// g++ -std=c++17 -O2 -o branch_sort branch_sort.cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>

long long conditional_sum(const std::vector<int>& v, int threshold)
{
    long long sum = 0;
    for (int x : v)
        if (x >= threshold) sum += x; // 조건 분기
    return sum;
}

int main()
{
    constexpr int N = 1'000'000;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);

    std::vector<int> data(N);
    for (auto& x : data) x = dist(rng);

    auto bench = [&](const std::vector<int>& v, const char* label) {
        auto t0 = std::chrono::high_resolution_clock::now();
        long long sum = 0;
        for (int rep = 0; rep < 10; ++rep)
            sum += conditional_sum(v, 128);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << label << ": " << ms << " ms (sum=" << sum << ")\n";
    };

    bench(data, "랜덤(정렬 없음)");

    std::sort(data.begin(), data.end());
    bench(data, "정렬 후");

    return 0;
}

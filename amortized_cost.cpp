// filename: amortized_cost.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o amortized_cost amortized_cost.cpp
#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>

int main() {
    const int N = 1024;
    std::vector<long long> costs;
    costs.reserve(N);

    std::vector<int> v;
    size_t prev_cap = 0;

    for (int i = 0; i < N; i++) {
        auto t0 = std::chrono::steady_clock::now();
        v.push_back(i);
        auto t1 = std::chrono::steady_clock::now();

        long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        costs.push_back(ns);

        if (v.capacity() != prev_cap) {
            std::cout << "i=" << i
                      << " cap " << prev_cap << "->" << v.capacity()
                      << " cost=" << ns << " ns\n";
            prev_cap = v.capacity();
        }
    }

    // 비용 통계
    long long total = 0;
    for (long long c : costs) total += c;
    long long max_cost = *std::max_element(costs.begin(), costs.end());

    std::cout << "\ntotal N=" << N << " push_backs\n";
    std::cout << "total cost: " << total << " ns\n";
    std::cout << "amortized:  " << total / N << " ns/op\n";
    std::cout << "max single: " << max_cost << " ns (재할당 발생 시)\n";

    return 0;
}

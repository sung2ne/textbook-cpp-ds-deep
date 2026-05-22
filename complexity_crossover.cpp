// filename: complexity_crossover.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o complexity_crossover complexity_crossover.cpp
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

static long long now_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(
        steady_clock::now().time_since_epoch()).count();
}

// 삽입 정렬: O(n²) but small constant
static void insertion_sort(std::vector<int>& v) {
    for (size_t i = 1; i < v.size(); i++) {
        int key = v[i];
        int j = (int)i - 1;
        while (j >= 0 && v[j] > key) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = key;
    }
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 100000);

    for (int n : {8, 16, 32, 64, 128, 256, 512}) {
        std::vector<int> base(n);
        for (auto& x : base) x = dist(rng);

        // insertion sort
        std::vector<int> v1 = base;
        auto t0 = now_us();
        for (int r = 0; r < 10000; r++) {
            v1 = base;
            insertion_sort(v1);
        }
        auto t1 = now_us();

        // std::sort (introsort, ~O(n log n))
        std::vector<int> v2 = base;
        auto t2 = now_us();
        for (int r = 0; r < 10000; r++) {
            v2 = base;
            std::sort(v2.begin(), v2.end());
        }
        auto t3 = now_us();

        std::cout << "n=" << n
                  << "  insertion_sort: " << (t1 - t0) / 10000.0 << " us"
                  << "  std::sort: "      << (t3 - t2) / 10000.0 << " us\n";
    }
    return 0;
}

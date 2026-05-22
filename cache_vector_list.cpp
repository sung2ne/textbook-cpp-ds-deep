// filename: cache_vector_list.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o cache_vector_list cache_vector_list.cpp
#include <vector>
#include <list>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>

static long long now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(
        steady_clock::now().time_since_epoch()).count();
}

int main() {
    const int N = 1'000'000;
    const int ROUNDS = 10;

    // vector 구성 및 순회
    {
        std::vector<int> v(N);
        std::iota(v.begin(), v.end(), 0);

        long long total_sum = 0;
        auto t0 = now_ns();
        for (int r = 0; r < ROUNDS; r++) {
            long long sum = 0;
            for (int x : v) sum += x;
            total_sum += sum;
        }
        auto t1 = now_ns();
        std::cout << "vector<int> traversal (" << N << " elements x " << ROUNDS << " rounds):\n";
        std::cout << "  time:  " << (t1 - t0) / 1e6 << " ms\n";
        std::cout << "  sum:   " << total_sum / ROUNDS << "\n";
        std::cout << "  ns/op: " << (double)(t1 - t0) / (N * ROUNDS) << " ns\n\n";
    }

    // list 구성 및 순회
    {
        std::list<int> lst;
        for (int i = 0; i < N; i++) lst.push_back(i);

        long long total_sum = 0;
        auto t0 = now_ns();
        for (int r = 0; r < ROUNDS; r++) {
            long long sum = 0;
            for (int x : lst) sum += x;
            total_sum += sum;
        }
        auto t1 = now_ns();
        std::cout << "list<int> traversal (" << N << " elements x " << ROUNDS << " rounds):\n";
        std::cout << "  time:  " << (t1 - t0) / 1e6 << " ms\n";
        std::cout << "  sum:   " << total_sum / ROUNDS << "\n";
        std::cout << "  ns/op: " << (double)(t1 - t0) / (N * ROUNDS) << " ns\n";
    }

    return 0;
}

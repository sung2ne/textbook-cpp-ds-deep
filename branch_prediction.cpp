// filename: branch_prediction.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o branch_prediction branch_prediction.cpp
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>

static long long now_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(
        steady_clock::now().time_since_epoch()).count();
}

int main() {
    const int N = 1'000'000;
    const int ROUNDS = 20;

    // 랜덤 배열 생성
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<int> random_arr(N);
    for (auto& x : random_arr) x = dist(rng);

    // 정렬된 배열
    std::vector<int> sorted_arr = random_arr;
    std::sort(sorted_arr.begin(), sorted_arr.end());

    // 랜덤 배열에서 128 이상인 원소 합산
    {
        long long total = 0;
        auto t0 = now_us();
        for (int r = 0; r < ROUNDS; r++) {
            long long sum = 0;
            for (int x : random_arr) {
                if (x >= 128) sum += x;
            }
            total += sum;
        }
        auto t1 = now_us();
        std::cout << "Random array sum (x>=128): " << total / ROUNDS
                  << "  time: " << (t1 - t0) / ROUNDS << " us\n";
    }

    // 정렬된 배열에서 128 이상인 원소 합산
    {
        long long total = 0;
        auto t0 = now_us();
        for (int r = 0; r < ROUNDS; r++) {
            long long sum = 0;
            for (int x : sorted_arr) {
                if (x >= 128) sum += x;
            }
            total += sum;
        }
        auto t1 = now_us();
        std::cout << "Sorted array sum (x>=128): " << total / ROUNDS
                  << "  time: " << (t1 - t0) / ROUNDS << " us\n";
    }

    // __builtin_expect 사용: 컴파일러에 예측 힌트
    {
        long long total = 0;
        auto t0 = now_us();
        for (int r = 0; r < ROUNDS; r++) {
            long long sum = 0;
            for (int x : random_arr) {
                // x >= 128인 경우가 약 50% — 힌트가 도움 안 됨
                if (__builtin_expect(x >= 128, 1)) sum += x;
            }
            total += sum;
        }
        auto t1 = now_us();
        std::cout << "Random + builtin_expect: " << total / ROUNDS
                  << "  time: " << (t1 - t0) / ROUNDS << " us\n";
    }

    return 0;
}

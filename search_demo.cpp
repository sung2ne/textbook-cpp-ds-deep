// filename: search_demo.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o search_demo search_demo.cpp
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>

static long long now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(
        steady_clock::now().time_since_epoch()).count();
}

int main() {
    const int N = 1000;
    const int ROUNDS = 100'000;
    const int TARGET = N / 2;  // 중간 값

    // 컨테이너 초기화
    std::vector<int> vec(N);
    std::iota(vec.begin(), vec.end(), 0);

    std::list<int> lst(vec.begin(), vec.end());

    // vector 선형 탐색
    long long found_v = 0;
    auto t0 = now_ns();
    for (int r = 0; r < ROUNDS; r++) {
        auto it = std::find(vec.begin(), vec.end(), TARGET);
        if (it != vec.end()) found_v++;
    }
    auto t1 = now_ns();
    std::cout << "vector<int> find: " << (t1 - t0) / 1e6 << " ms"
              << " (found " << found_v << " times)\n";

    // list 선형 탐색
    long long found_l = 0;
    auto t2 = now_ns();
    for (int r = 0; r < ROUNDS; r++) {
        auto it = std::find(lst.begin(), lst.end(), TARGET);
        if (it != lst.end()) found_l++;
    }
    auto t3 = now_ns();
    std::cout << "list<int>   find: " << (t3 - t2) / 1e6 << " ms"
              << " (found " << found_l << " times)\n";

    return 0;
}

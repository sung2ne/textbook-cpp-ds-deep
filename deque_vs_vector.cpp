// filename: deque_vs_vector.cpp
// g++ -std=c++17 -O2 -Wall -o deque_vs_vector deque_vs_vector.cpp
#include <iostream>
#include <vector>
#include <deque>
#include <chrono>
#include <algorithm>

using Clock = std::chrono::high_resolution_clock;
using us = std::chrono::microseconds;

void benchmark_push_front(int n) {
    // deque push_front
    auto t1 = Clock::now();
    {
        std::deque<int> dq;
        for (int i = 0; i < n; ++i) {
            dq.push_front(i);
        }
    }
    auto t2 = Clock::now();
    long long deque_us = std::chrono::duration_cast<us>(t2 - t1).count();

    // vector push_front (insert at begin = O(n))
    auto t3 = Clock::now();
    {
        std::vector<int> v;
        v.reserve(n);
        for (int i = 0; i < n; ++i) {
            v.insert(v.begin(), i);  // O(n) 삽입!
        }
    }
    auto t4 = Clock::now();
    long long vector_us = std::chrono::duration_cast<us>(t4 - t3).count();

    std::cout << "=== push_front " << n << "회 ===\n";
    std::cout << "  deque:  " << deque_us << " μs\n";
    std::cout << "  vector: " << vector_us << " μs\n";
    std::cout << "  deque가 " << vector_us / std::max(deque_us, 1LL) << "배 빠름\n\n";
}

void benchmark_traverse(int n) {
    std::deque<int> dq;
    std::vector<int> v;
    for (int i = 0; i < n; ++i) {
        dq.push_back(i);
        v.push_back(i);
    }

    long long sum_dq = 0, sum_v = 0;

    auto t1 = Clock::now();
    for (int x : dq) sum_dq += x;
    auto t2 = Clock::now();
    long long deque_us = std::chrono::duration_cast<us>(t2 - t1).count();

    auto t3 = Clock::now();
    for (int x : v) sum_v += x;
    auto t4 = Clock::now();
    long long vector_us = std::chrono::duration_cast<us>(t4 - t3).count();

    std::cout << "=== 순회 " << n << "개 (sum=" << sum_dq << ") ===\n";
    std::cout << "  deque:  " << deque_us  << " μs\n";
    std::cout << "  vector: " << vector_us << " μs\n";
    std::cout << "  vector가 " << deque_us / std::max(vector_us, 1LL) << "배 빠름\n\n";
}

void show_deque_structure() {
    std::deque<int> dq;

    // 양쪽에서 원소 추가
    for (int i = 0; i < 5; ++i) dq.push_back(i * 10);
    for (int i = 1; i <= 3; ++i) dq.push_front(i * -10);

    std::cout << "deque 내용 (앞←[원소들]→뒤):\n  ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "size=" << dq.size() << "\n\n";
}

int main() {
    show_deque_structure();
    benchmark_push_front(10000);
    benchmark_traverse(1000000);
    return 0;
}

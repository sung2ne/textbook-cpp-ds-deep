// filename: heap_algorithms.cpp
// g++ -std=c++17 -O2 -Wall -o heap_algorithms heap_algorithms.cpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <functional>
#include <chrono>
#include <random>

void print_vec(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    print_vec("원본", v);

    // === make_heap: O(n) ===
    std::make_heap(v.begin(), v.end());
    print_vec("make_heap 후", v);     // [9, 6, 4, 5, 5, 3, 2, 1, 1, 3]
    std::cout << "최댓값(v[0]): " << v.front() << "\n";  // 9

    // === push_heap: 새 원소 추가 ===
    v.push_back(10);  // 벡터 끝에 추가
    std::push_heap(v.begin(), v.end());  // heap 복원 O(log n)
    print_vec("10 push_heap 후", v);    // [10, 6, 9, 5, 5, 4, 2, 1, 1, 3, 3]
    std::cout << "최댓값: " << v.front() << "\n";  // 10

    // === pop_heap: 최댓값 꺼내기 ===
    std::pop_heap(v.begin(), v.end());   // 최댓값을 맨 뒤로 이동
    int max_val = v.back();
    v.pop_back();                        // 실제로 제거
    std::cout << "pop_heap 결과: " << max_val << "\n";  // 10
    print_vec("pop_heap 후", v);

    // === sort_heap: heap → 정렬 ===
    std::make_heap(v.begin(), v.end());
    std::sort_heap(v.begin(), v.end());  // O(n log n)
    print_vec("sort_heap 후(오름차순)", v);

    std::cout << "\n=== 최소 힙 (greater<int>) ===\n";
    std::vector<int> min_heap = {5, 3, 8, 1, 9, 2};
    std::make_heap(min_heap.begin(), min_heap.end(), std::greater<int>{});
    std::cout << "최솟값: " << min_heap.front() << "\n";  // 1

    std::cout << "\n=== priority_queue vs 수동 heap ===\n";
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 100000);
    const int N = 100000;
    std::vector<int> data(N);
    std::generate(data.begin(), data.end(), [&]{ return dist(rng); });

    using ms = std::chrono::duration<double, std::milli>;

    // priority_queue 사용
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::priority_queue<int> pq;
        for (int x : data) pq.push(x);
        long long sum = 0;
        while (!pq.empty()) { sum += pq.top(); pq.pop(); }
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "priority_queue: " << ms(t2-t1).count() << " ms (sum=" << sum << ")\n";
    }

    // 수동 heap 사용
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::vector<int> heap;
        heap.reserve(N);
        for (int x : data) {
            heap.push_back(x);
            std::push_heap(heap.begin(), heap.end());
        }
        long long sum = 0;
        while (!heap.empty()) {
            sum += heap.front();
            std::pop_heap(heap.begin(), heap.end());
            heap.pop_back();
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "수동 heap:      " << ms(t2-t1).count() << " ms (sum=" << sum << ")\n";
    }

    std::cout << "\n=== Dijkstra에서 힙의 역할 ===\n";
    // 최소 힙으로 (거리, 노드) 우선순위 큐 구현
    using pii = std::pair<int,int>;
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> min_pq;

    min_pq.push({0, 0});    // (거리=0, 노드=0)
    min_pq.push({5, 1});
    min_pq.push({3, 2});
    min_pq.push({10, 3});

    std::cout << "Dijkstra 순서:\n";
    while (!min_pq.empty()) {
        auto [dist, node] = min_pq.top(); min_pq.pop();
        std::cout << "  거리=" << dist << ", 노드=" << node << "\n";
    }
    // 거리 오름차순: 0, 3, 5, 10

    return 0;
}

// filename: heap_sort.cpp
// g++ -std=c++17 -O2 -Wall -o heap_sort heap_sort.cpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

// 직접 구현한 힙 정렬
void sift_down(std::vector<int>& v, int n, int i) {
    while (true) {
        int largest = i;
        int left    = 2 * i + 1;
        int right   = 2 * i + 2;
        if (left  < n && v[left]  > v[largest]) largest = left;
        if (right < n && v[right] > v[largest]) largest = right;
        if (largest == i) break;
        std::swap(v[i], v[largest]);
        i = largest;
    }
}

void heap_sort_manual(std::vector<int>& v) {
    int n = static_cast<int>(v.size());

    // Phase 1: make_heap (Floyd's algorithm)
    // 리프 노드는 이미 heap 성질을 만족 → 내부 노드만 처리
    for (int i = n / 2 - 1; i >= 0; --i) {
        sift_down(v, n, i);
    }

    // Phase 2: sort_heap
    for (int size = n - 1; size > 0; --size) {
        std::swap(v[0], v[size]);     // 최댓값을 끝으로
        sift_down(v, size, 0);        // 남은 구간 heap 복원
    }
}

// STL을 이용한 힙 정렬
void heap_sort_stl(std::vector<int>& v) {
    std::make_heap(v.begin(), v.end());   // O(n)
    std::sort_heap(v.begin(), v.end());   // O(n log n)
}

// STL의 priority_queue처럼 사용
void heap_sort_push(std::vector<int>& v) {
    // 순차 삽입: O(n log n) — make_heap보다 느림
    std::vector<int> heap;
    heap.reserve(v.size());
    for (int x : v) {
        heap.push_back(x);
        std::push_heap(heap.begin(), heap.end());  // O(log n)
    }
    for (int i = static_cast<int>(v.size()) - 1; i >= 0; --i) {
        v[i] = heap.front();
        std::pop_heap(heap.begin(), heap.end());   // O(log n)
        heap.pop_back();
    }
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 1000000);

    for (int n : {10000, 100000, 1000000}) {
        std::vector<int> base(n);
        std::generate(base.begin(), base.end(), [&]{ return dist(rng); });

        auto measure = [&](const std::string& name, auto fn, std::vector<int> data) {
            auto t1 = std::chrono::high_resolution_clock::now();
            fn(data);
            auto t2 = std::chrono::high_resolution_clock::now();
            std::cout << "  " << name << ": "
                      << std::chrono::duration<double, std::milli>(t2-t1).count()
                      << " ms\n";
        };

        std::cout << "n=" << n << "\n";
        measure("힙정렬(직접구현)", heap_sort_manual, base);
        measure("힙정렬(STL)",      heap_sort_stl,    base);
        measure("std::sort",
                [](std::vector<int>& v){ std::sort(v.begin(), v.end()); }, base);
    }

    // make_heap O(n) vs 순차 삽입 O(n log n) 확인
    std::cout << "\n--- make_heap O(n) vs 순차 push_heap O(n log n) ---\n";
    std::vector<int> big(1000000);
    std::generate(big.begin(), big.end(), [&]{ return dist(rng); });

    {
        auto v = big;
        auto t1 = std::chrono::high_resolution_clock::now();
        std::make_heap(v.begin(), v.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "  make_heap O(n):          "
                  << std::chrono::duration<double, std::milli>(t2-t1).count()
                  << " ms\n";
    }
    {
        std::vector<int> h;
        h.reserve(big.size());
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int x : big) {
            h.push_back(x);
            std::push_heap(h.begin(), h.end());
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "  순차 push_heap O(n logn): "
                  << std::chrono::duration<double, std::milli>(t2-t1).count()
                  << " ms\n";
    }

    return 0;
}

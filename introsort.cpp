// filename: introsort.cpp
// g++ -std=c++17 -O2 -Wall -o introsort introsort.cpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>

// --- 삽입 정렬 (작은 구간용) ---
void insertion_sort_range(std::vector<int>& v, int left, int right) {
    for (int i = left + 1; i <= right; ++i) {
        int key = v[i];
        int j = i - 1;
        while (j >= left && v[j] > key) {
            v[j + 1] = v[j];
            --j;
        }
        v[j + 1] = key;
    }
}

// --- sift_down (힙 정렬용) ---
void sift_down(std::vector<int>& v, int root, int end) {
    while (true) {
        int child = 2 * root + 1;
        if (child > end) break;
        if (child + 1 <= end && v[child] < v[child + 1]) ++child;
        if (v[root] >= v[child]) break;
        std::swap(v[root], v[child]);
        root = child;
    }
}

void heap_sort_range(std::vector<int>& v, int left, int right) {
    // 0-based 오프셋으로 처리
    int n = right - left + 1;
    auto get = [&](int i) -> int& { return v[left + i]; };

    // make_heap
    for (int i = n / 2 - 1; i >= 0; --i) {
        int root = i;
        while (true) {
            int child = 2 * root + 1;
            if (child >= n) break;
            if (child + 1 < n && get(child) < get(child + 1)) ++child;
            if (get(root) >= get(child)) break;
            std::swap(get(root), get(child));
            root = child;
        }
    }
    // sort_heap
    for (int end = n - 1; end > 0; --end) {
        std::swap(get(0), get(end));
        int root = 0;
        while (true) {
            int child = 2 * root + 1;
            if (child >= end) break;
            if (child + 1 < end && get(child) < get(child + 1)) ++child;
            if (get(root) >= get(child)) break;
            std::swap(get(root), get(child));
            root = child;
        }
    }
}

// --- Median-of-Three partition ---
int partition_introsort(std::vector<int>& v, int left, int right) {
    int mid = left + (right - left) / 2;
    if (v[left] > v[mid])   std::swap(v[left], v[mid]);
    if (v[left] > v[right]) std::swap(v[left], v[right]);
    if (v[mid]  > v[right]) std::swap(v[mid],  v[right]);
    // pivot = v[mid], 이를 right에 저장
    std::swap(v[mid], v[right - 1]);
    int pivot = v[right - 1];

    int i = left, j = right - 1;
    while (true) {
        while (v[++i] < pivot) {}
        while (v[--j] > pivot) {}
        if (i >= j) break;
        std::swap(v[i], v[j]);
    }
    std::swap(v[i], v[right - 1]);
    return i;
}

// --- Introsort 본체 ---
constexpr int THRESHOLD = 16;

void introsort_impl(std::vector<int>& v, int left, int right, int depth_limit) {
    while (right - left > THRESHOLD) {
        if (depth_limit == 0) {
            // 재귀 깊이 초과 → 힙 정렬
            heap_sort_range(v, left, right);
            return;
        }
        --depth_limit;

        // 퀵 정렬 단계
        if (right - left < 3) {
            if (v[left] > v[right]) std::swap(v[left], v[right]);
            return;
        }
        int p = partition_introsort(v, left, right);

        // 오른쪽 구간 재귀, 왼쪽 구간 루프 (tail-call 최적화)
        introsort_impl(v, p + 1, right, depth_limit);
        right = p - 1;
    }
}

void introsort(std::vector<int>& v) {
    if (v.size() <= 1) return;
    int n = static_cast<int>(v.size());
    int depth_limit = 2 * static_cast<int>(std::log2(n));

    introsort_impl(v, 0, n - 1, depth_limit);

    // 전체 배열 삽입 정렬 (이미 거의 정렬된 상태)
    insertion_sort_range(v, 0, n - 1);
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 1000000);

    std::cout << "=== Introsort vs std::sort 성능 비교 ===\n";
    for (int n : {10000, 100000, 1000000}) {
        std::vector<int> base(n);
        std::generate(base.begin(), base.end(), [&]{ return dist(rng); });

        auto v1 = base, v2 = base;

        auto t1 = std::chrono::high_resolution_clock::now();
        introsort(v1);
        auto t2 = std::chrono::high_resolution_clock::now();

        std::sort(v2.begin(), v2.end());
        auto t3 = std::chrono::high_resolution_clock::now();

        using ms = std::chrono::duration<double, std::milli>;
        std::cout << "n=" << n << "\n";
        std::cout << "  Introsort(직접): " << ms(t2-t1).count() << " ms\n";
        std::cout << "  std::sort:       " << ms(t3-t2).count() << " ms\n";
        std::cout << "  결과 일치: " << (v1 == v2 ? "OK" : "FAIL") << "\n\n";
    }

    // 최악 케이스 시뮬레이션: 힙 정렬 전환 확인
    std::cout << "=== 힙 정렬 전환 시뮬레이션 (인위적으로 depth_limit=0) ===\n";
    std::vector<int> test = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    heap_sort_range(test, 0, static_cast<int>(test.size()) - 1);
    std::cout << "힙 정렬 결과: ";
    for (int x : test) std::cout << x << " ";
    std::cout << "\n";

    return 0;
}

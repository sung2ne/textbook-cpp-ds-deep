// filename: pdqsort_demo.cpp
// g++ -std=c++17 -O2 -Wall -o pdqsort_demo pdqsort_demo.cpp
// 참고: 실제 pdqsort는 https://github.com/orlp/pdqsort

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>

// pdqsort 핵심 아이디어 1: 거의 정렬 감지
bool is_nearly_sorted(const std::vector<int>& v, int left, int right, int limit) {
    int inversions = 0;
    for (int i = left; i < right; ++i) {
        if (v[i] > v[i + 1]) {
            if (++inversions > limit) return false;
        }
    }
    return true;
}

// 3-way partition (Dutch National Flag)
std::pair<int,int> partition_3way(std::vector<int>& v, int left, int right) {
    int pivot = v[left + (right - left) / 2];
    int lt = left, eq = left, gt = right;
    while (eq <= gt) {
        if      (v[eq] < pivot) std::swap(v[lt++], v[eq++]);
        else if (v[eq] > pivot) std::swap(v[eq], v[gt--]);
        else                    ++eq;
    }
    return {lt, gt};
}

// 간소화된 pdqsort 아이디어 시연
void pdq_like_sort(std::vector<int>& v, int left, int right, int depth_limit) {
    if (right <= left) return;
    int n = right - left + 1;

    // 작은 구간: 삽입 정렬
    if (n <= 16) {
        for (int i = left + 1; i <= right; ++i) {
            int key = v[i], j = i - 1;
            while (j >= left && v[j] > key) { v[j+1] = v[j]; --j; }
            v[j+1] = key;
        }
        return;
    }

    // 거의 정렬된 패턴 감지
    if (is_nearly_sorted(v, left, right, n / 8)) {
        // 삽입 정렬로 마무리
        for (int i = left + 1; i <= right; ++i) {
            int key = v[i], j = i - 1;
            while (j >= left && v[j] > key) { v[j+1] = v[j]; --j; }
            v[j+1] = key;
        }
        return;
    }

    // 재귀 깊이 초과: 힙 정렬
    if (depth_limit == 0) {
        std::sort(v.begin() + left, v.begin() + right + 1);  // 데모용 std::sort
        return;
    }

    // 3-way partition (중복 처리)
    auto [lo, hi] = partition_3way(v, left, right);
    pdq_like_sort(v, left, lo - 1, depth_limit - 1);
    pdq_like_sort(v, hi + 1, right, depth_limit - 1);
}

int main() {
    std::mt19937 rng(42);
    const int N = 100000;

    auto measure = [](const std::string& name, auto fn, std::vector<int> data) {
        auto t1 = std::chrono::high_resolution_clock::now();
        fn(data);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "  " << name << ": "
                  << std::chrono::duration<double, std::milli>(t2-t1).count()
                  << " ms\n";
    };

    // 1. 이미 정렬된 배열
    std::vector<int> sorted(N);
    std::iota(sorted.begin(), sorted.end(), 0);
    std::cout << "=== 이미 정렬된 배열 (n=" << N << ") ===\n";
    measure("pdq_like (패턴 감지)", [](std::vector<int>& v){
        pdq_like_sort(v, 0, static_cast<int>(v.size())-1, 40);
    }, sorted);
    measure("std::sort", [](std::vector<int>& v){ std::sort(v.begin(), v.end()); }, sorted);
    measure("std::ranges::sort", [](std::vector<int>& v){
        std::ranges::sort(v);
    }, sorted);

    // 2. 중복이 많은 배열
    std::vector<int> dup(N);
    std::uniform_int_distribution<int> dist(0, 9);  // 0~9 값만
    std::generate(dup.begin(), dup.end(), [&]{ return dist(rng); });
    std::cout << "\n=== 중복 많은 배열 (값 범위 0~9, n=" << N << ") ===\n";
    measure("pdq_like (3-way partition)", [](std::vector<int>& v){
        pdq_like_sort(v, 0, static_cast<int>(v.size())-1, 40);
    }, dup);
    measure("std::sort", [](std::vector<int>& v){ std::sort(v.begin(), v.end()); }, dup);
    measure("std::ranges::sort", [](std::vector<int>& v){
        std::ranges::sort(v);
    }, dup);

    return 0;
}

// filename: merge_sort.cpp
// g++ -std=c++17 -O2 -Wall -o merge_sort merge_sort.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>

// merge: [left, mid)와 [mid, right)를 병합
void merge(std::vector<int>& v, int left, int mid, int right,
           std::vector<int>& temp) {
    int i = left, j = mid, k = left;
    while (i < mid && j < right) {
        // <= 이므로 안정 정렬 보장
        if (v[i] <= v[j]) temp[k++] = v[i++];
        else              temp[k++] = v[j++];
    }
    while (i < mid)   temp[k++] = v[i++];
    while (j < right) temp[k++] = v[j++];
    // 결과를 원본 배열로 복사
    for (int x = left; x < right; ++x) v[x] = temp[x];
}

// 재귀 병합 정렬 (Top-down)
void merge_sort_recursive(std::vector<int>& v, int left, int right,
                          std::vector<int>& temp) {
    if (right - left <= 1) return;
    int mid = left + (right - left) / 2;
    merge_sort_recursive(v, left, mid, temp);
    merge_sort_recursive(v, mid, right, temp);
    merge(v, left, mid, right, temp);
}

void merge_sort(std::vector<int>& v) {
    std::vector<int> temp(v.size());
    merge_sort_recursive(v, 0, static_cast<int>(v.size()), temp);
}

// 반복 병합 정렬 (Bottom-up)
void merge_sort_iterative(std::vector<int>& v) {
    int n = static_cast<int>(v.size());
    std::vector<int> temp(n);
    // 크기 1인 구간부터 시작해 2배씩 키우며 병합
    for (int width = 1; width < n; width *= 2) {
        for (int left = 0; left < n; left += 2 * width) {
            int mid   = std::min(left + width, n);
            int right = std::min(left + 2 * width, n);
            if (mid < right) {
                merge(v, left, mid, right, temp);
            }
        }
    }
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 100000);

    for (int n : {1000, 100000, 1000000}) {
        std::vector<int> data(n);
        std::generate(data.begin(), data.end(), [&]{ return dist(rng); });

        std::vector<int> v1 = data, v2 = data, v3 = data;

        auto t1 = std::chrono::high_resolution_clock::now();
        merge_sort(v1);
        auto t2 = std::chrono::high_resolution_clock::now();

        merge_sort_iterative(v2);
        auto t3 = std::chrono::high_resolution_clock::now();

        std::sort(v3.begin(), v3.end());
        auto t4 = std::chrono::high_resolution_clock::now();

        using ms = std::chrono::duration<double, std::milli>;
        std::cout << "n=" << n << "\n";
        std::cout << "  Merge(재귀):   " << ms(t2-t1).count() << " ms\n";
        std::cout << "  Merge(반복):   " << ms(t3-t2).count() << " ms\n";
        std::cout << "  std::sort:     " << ms(t4-t3).count() << " ms\n";

        // 결과 검증
        bool ok = (v1 == v2) && (v1 == v3);
        std::cout << "  결과 일치: " << (ok ? "OK" : "FAIL") << "\n\n";
    }
    return 0;
}

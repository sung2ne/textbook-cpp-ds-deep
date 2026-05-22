// filename: basic_sorts.cpp
// g++ -std=c++17 -O2 -Wall -o basic_sorts basic_sorts.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <string>

// 버블 정렬
void bubble_sort(std::vector<int>& v) {
    int n = static_cast<int>(v.size());
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; ++j) {
            if (v[j] > v[j + 1]) {
                std::swap(v[j], v[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;  // 조기 종료 최적화
    }
}

// 선택 정렬
void selection_sort(std::vector<int>& v) {
    int n = static_cast<int>(v.size());
    for (int i = 0; i < n - 1; ++i) {
        int min_idx = i;
        for (int j = i + 1; j < n; ++j) {
            if (v[j] < v[min_idx]) min_idx = j;
        }
        if (min_idx != i) std::swap(v[i], v[min_idx]);
    }
}

// 삽입 정렬
void insertion_sort(std::vector<int>& v) {
    int n = static_cast<int>(v.size());
    for (int i = 1; i < n; ++i) {
        int key = v[i];
        int j = i - 1;
        while (j >= 0 && v[j] > key) {
            v[j + 1] = v[j];
            --j;
        }
        v[j + 1] = key;
    }
}

// 성능 측정 헬퍼
using SortFn = void(*)(std::vector<int>&);

double measure(SortFn fn, std::vector<int> data) {
    auto t1 = std::chrono::high_resolution_clock::now();
    fn(data);
    auto t2 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t2 - t1).count();
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 100000);

    for (int n : {100, 1000, 5000, 10000}) {
        std::vector<int> data(n);
        std::generate(data.begin(), data.end(), [&]{ return dist(rng); });

        double t_bubble    = measure(bubble_sort,    data);
        double t_selection = measure(selection_sort, data);
        double t_insertion = measure(insertion_sort, data);

        std::cout << "n=" << n << "\n";
        std::cout << "  Bubble:    " << t_bubble    << " ms\n";
        std::cout << "  Selection: " << t_selection << " ms\n";
        std::cout << "  Insertion: " << t_insertion << " ms\n";
    }

    // 거의 정렬된 배열에서 삽입 정렬 성능 확인
    std::cout << "\n--- 거의 정렬된 배열 (n=10000, 100개만 무작위 교환) ---\n";
    std::vector<int> nearly_sorted(10000);
    std::iota(nearly_sorted.begin(), nearly_sorted.end(), 0);
    for (int i = 0; i < 100; ++i) {
        int a = dist(rng) % 10000, b = dist(rng) % 10000;
        std::swap(nearly_sorted[a], nearly_sorted[b]);
    }
    std::cout << "  Bubble:    " << measure(bubble_sort,    nearly_sorted) << " ms\n";
    std::cout << "  Selection: " << measure(selection_sort, nearly_sorted) << " ms\n";
    std::cout << "  Insertion: " << measure(insertion_sort, nearly_sorted) << " ms\n";

    return 0;
}

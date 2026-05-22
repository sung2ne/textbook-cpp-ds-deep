// filename: quick_sort.cpp
// g++ -std=c++17 -O2 -Wall -o quick_sort quick_sort.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <functional>

// --- Lomuto + 랜덤 pivot ---
std::mt19937 rng_global(12345);

int partition_random(std::vector<int>& v, int left, int right) {
    std::uniform_int_distribution<int> dist(left, right);
    int rand_idx = dist(rng_global);
    std::swap(v[rand_idx], v[right]);  // 랜덤 원소를 마지막으로

    int pivot = v[right];
    int i = left - 1;
    for (int j = left; j < right; ++j) {
        if (v[j] <= pivot) std::swap(v[++i], v[j]);
    }
    std::swap(v[i + 1], v[right]);
    return i + 1;
}

void quick_sort_random(std::vector<int>& v, int left, int right) {
    if (left >= right) return;
    int p = partition_random(v, left, right);
    quick_sort_random(v, left, p - 1);
    quick_sort_random(v, p + 1, right);
}

// --- Median-of-Three + Lomuto ---
int partition_median(std::vector<int>& v, int left, int right) {
    if (right - left < 2) {
        if (v[left] > v[right]) std::swap(v[left], v[right]);
        return left;
    }
    int mid = left + (right - left) / 2;
    if (v[left] > v[mid])   std::swap(v[left], v[mid]);
    if (v[left] > v[right]) std::swap(v[left], v[right]);
    if (v[mid]  > v[right]) std::swap(v[mid],  v[right]);
    std::swap(v[mid], v[right]);  // pivot을 마지막으로

    int pivot = v[right];
    int i = left - 1;
    for (int j = left; j < right; ++j) {
        if (v[j] <= pivot) std::swap(v[++i], v[j]);
    }
    std::swap(v[i + 1], v[right]);
    return i + 1;
}

void quick_sort_median(std::vector<int>& v, int left, int right) {
    if (left >= right) return;
    int p = partition_median(v, left, right);
    quick_sort_median(v, left, p - 1);
    quick_sort_median(v, p + 1, right);
}

// 성능 측정
void measure(const std::string& name, std::function<void(std::vector<int>&)> fn,
             std::vector<int> data) {
    auto t1 = std::chrono::high_resolution_clock::now();
    fn(data);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "  " << name << ": "
              << std::chrono::duration<double, std::milli>(t2-t1).count()
              << " ms\n";
}

int main() {
    const int N = 100000;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N);

    // 무작위 배열
    std::vector<int> random_data(N);
    std::generate(random_data.begin(), random_data.end(), [&]{ return dist(rng); });

    // 이미 정렬된 배열
    std::vector<int> sorted_data(N);
    std::iota(sorted_data.begin(), sorted_data.end(), 0);

    // 역정렬 배열
    std::vector<int> reverse_data(N);
    std::iota(reverse_data.rbegin(), reverse_data.rend(), 0);

    auto wrap_random = [](std::vector<int>& v) {
        quick_sort_random(v, 0, static_cast<int>(v.size()) - 1);
    };
    auto wrap_median = [](std::vector<int>& v) {
        quick_sort_median(v, 0, static_cast<int>(v.size()) - 1);
    };
    auto wrap_std = [](std::vector<int>& v) {
        std::sort(v.begin(), v.end());
    };

    std::cout << "=== 무작위 배열 (n=" << N << ") ===\n";
    measure("퀵정렬(랜덤pivot)", wrap_random, random_data);
    measure("퀵정렬(median-3)", wrap_median, random_data);
    measure("std::sort",        wrap_std,    random_data);

    std::cout << "\n=== 이미 정렬된 배열 ===\n";
    measure("퀵정렬(랜덤pivot)", wrap_random, sorted_data);
    measure("퀵정렬(median-3)", wrap_median, sorted_data);
    measure("std::sort",        wrap_std,    sorted_data);

    std::cout << "\n=== 역정렬 배열 ===\n";
    measure("퀵정렬(랜덤pivot)", wrap_random, reverse_data);
    measure("퀵정렬(median-3)", wrap_median, reverse_data);
    measure("std::sort",        wrap_std,    reverse_data);

    return 0;
}

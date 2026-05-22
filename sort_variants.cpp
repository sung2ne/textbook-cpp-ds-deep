// filename: sort_variants.cpp
// g++ -std=c++17 -O2 -Wall -o sort_variants sort_variants.cpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>

void print_vec(const std::string& label, const std::vector<int>& v, int limit = 10) {
    std::cout << label << ": [";
    for (int i = 0; i < std::min(limit, (int)v.size()); ++i) {
        std::cout << v[i];
        if (i < (int)v.size() - 1 && i < limit - 1) std::cout << ", ";
    }
    if ((int)v.size() > limit) std::cout << ", ...";
    std::cout << "]\n";
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 10000);

    const int N = 1000000;
    const int K = 100;  // 상위 K개

    std::vector<int> base(N);
    std::generate(base.begin(), base.end(), [&]{ return dist(rng); });

    using ms = std::chrono::duration<double, std::milli>;

    // 1. std::sort — 전체 정렬
    {
        auto v = base;
        auto t1 = std::chrono::high_resolution_clock::now();
        std::sort(v.begin(), v.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "std::sort (전체 정렬): " << ms(t2-t1).count() << " ms\n";
        print_vec("최솟값 10개", v);
    }

    // 2. std::partial_sort — 상위 K개만 정렬
    {
        auto v = base;
        auto t1 = std::chrono::high_resolution_clock::now();
        std::partial_sort(v.begin(), v.begin() + K, v.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "\nstd::partial_sort (상위 " << K << "개): "
                  << ms(t2-t1).count() << " ms\n";
        print_vec("최솟값 " + std::to_string(K) + "개", v, K);
    }

    // 3. std::nth_element — 중앙값 찾기
    {
        auto v = base;
        auto t1 = std::chrono::high_resolution_clock::now();
        auto median_it = v.begin() + N / 2;
        std::nth_element(v.begin(), median_it, v.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "\nstd::nth_element (중앙값): " << ms(t2-t1).count() << " ms\n";
        std::cout << "중앙값: " << *median_it << "\n";
    }

    // 4. std::nth_element — Top-K 분리 (K번째보다 작은 것들)
    {
        auto v = base;
        auto t1 = std::chrono::high_resolution_clock::now();
        std::nth_element(v.begin(), v.begin() + K, v.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "\nstd::nth_element (Top-" << K << " 분리): "
                  << ms(t2-t1).count() << " ms\n";
        std::cout << v[K] << "번째 원소 (이 값보다 작은 것 " << K << "개 존재)\n";
    }

    // 5. std::is_sorted — 정렬 여부 확인
    {
        auto v = base;
        std::cout << "\nstd::is_sorted (정렬 전): "
                  << (std::is_sorted(v.begin(), v.end()) ? "정렬됨" : "미정렬") << "\n";
        std::sort(v.begin(), v.end());
        std::cout << "std::is_sorted (정렬 후): "
                  << (std::is_sorted(v.begin(), v.end()) ? "정렬됨" : "미정렬") << "\n";
    }

    // 6. std::is_sorted_until — 어디까지 정렬됐는지
    {
        std::vector<int> partial = {1, 2, 3, 4, 5, 3, 7, 8};
        auto it = std::is_sorted_until(partial.begin(), partial.end());
        std::cout << "\nstd::is_sorted_until: "
                  << std::distance(partial.begin(), it) << "번째 인덱스까지 정렬\n";
        // 출력: 5번째 인덱스까지 정렬 (5→3에서 역전)
    }

    return 0;
}

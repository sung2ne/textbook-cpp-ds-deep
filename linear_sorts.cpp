// filename: linear_sorts.cpp
// g++ -std=c++17 -O2 -Wall -o linear_sorts linear_sorts.cpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <cassert>

// === Counting Sort ===
void counting_sort(std::vector<int>& v, int max_val) {
    std::vector<int> count(max_val + 1, 0);
    for (int x : v) ++count[x];

    // 누적합
    for (int i = 1; i <= max_val; ++i) count[i] += count[i-1];

    // 배치 (안정 정렬을 위해 뒤에서부터)
    std::vector<int> output(v.size());
    for (int i = static_cast<int>(v.size()) - 1; i >= 0; --i) {
        output[--count[v[i]]] = v[i];
    }
    v = std::move(output);
}

// === Radix Sort (LSD, 기수 256) ===
void radix_sort(std::vector<int>& v) {
    const int BITS = 8;   // 한 번에 8비트(0~255) 처리
    const int BUCKETS = 1 << BITS;  // 256
    int n = static_cast<int>(v.size());

    std::vector<int> output(n);

    // 양수 전용 (음수는 별도 처리 필요)
    for (int shift = 0; shift < 32; shift += BITS) {
        std::vector<int> count(BUCKETS, 0);

        // 각 자릿수 카운팅
        for (int x : v) ++count[(x >> shift) & (BUCKETS - 1)];

        // 누적합
        for (int i = 1; i < BUCKETS; ++i) count[i] += count[i-1];

        // 뒤에서부터 배치 (안정 정렬)
        for (int i = n - 1; i >= 0; --i) {
            int bucket = (v[i] >> shift) & (BUCKETS - 1);
            output[--count[bucket]] = v[i];
        }
        v = output;
    }
}

// === Bucket Sort (0.0~1.0 실수용) ===
void bucket_sort(std::vector<double>& v) {
    int n = static_cast<int>(v.size());
    if (n <= 1) return;

    std::vector<std::vector<double>> buckets(n);

    // 분배
    for (double x : v) {
        int idx = static_cast<int>(x * n);
        if (idx == n) idx = n - 1;  // x == 1.0 엣지 케이스
        buckets[idx].push_back(x);
    }

    // 각 버킷 정렬 후 연결
    int pos = 0;
    for (auto& bucket : buckets) {
        std::sort(bucket.begin(), bucket.end());  // 삽입 정렬도 가능
        for (double x : bucket) v[pos++] = x;
    }
}

int main() {
    std::mt19937 rng(42);

    // === Counting Sort 테스트 ===
    std::cout << "=== Counting Sort ===\n";
    {
        const int N = 1000000;
        const int MAX = 1000;  // 키 범위 0~1000
        std::uniform_int_distribution<int> dist(0, MAX);
        std::vector<int> data(N);
        std::generate(data.begin(), data.end(), [&]{ return dist(rng); });

        auto v1 = data, v2 = data;

        auto t1 = std::chrono::high_resolution_clock::now();
        counting_sort(v1, MAX);
        auto t2 = std::chrono::high_resolution_clock::now();

        std::sort(v2.begin(), v2.end());
        auto t3 = std::chrono::high_resolution_clock::now();

        using ms = std::chrono::duration<double, std::milli>;
        std::cout << "n=" << N << ", 키 범위 0~" << MAX << "\n";
        std::cout << "  Counting Sort: " << ms(t2-t1).count() << " ms\n";
        std::cout << "  std::sort:     " << ms(t3-t2).count() << " ms\n";
        std::cout << "  결과 일치: " << (v1 == v2 ? "OK" : "FAIL") << "\n\n";
    }

    // === Radix Sort 테스트 ===
    std::cout << "=== Radix Sort ===\n";
    {
        const int N = 1000000;
        std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
        std::vector<int> data(N);
        std::generate(data.begin(), data.end(), [&]{ return dist(rng); });

        auto v1 = data, v2 = data;

        auto t1 = std::chrono::high_resolution_clock::now();
        radix_sort(v1);
        auto t2 = std::chrono::high_resolution_clock::now();

        std::sort(v2.begin(), v2.end());
        auto t3 = std::chrono::high_resolution_clock::now();

        using ms = std::chrono::duration<double, std::milli>;
        std::cout << "n=" << N << ", 키 범위 전체 int\n";
        std::cout << "  Radix Sort: " << ms(t2-t1).count() << " ms\n";
        std::cout << "  std::sort:  " << ms(t3-t2).count() << " ms\n";
        std::cout << "  결과 일치: " << (v1 == v2 ? "OK" : "FAIL") << "\n\n";
    }

    // === Bucket Sort 테스트 ===
    std::cout << "=== Bucket Sort ===\n";
    {
        const int N = 100000;
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        std::vector<double> data(N);
        std::generate(data.begin(), data.end(), [&]{ return dist(rng); });

        auto v1 = data, v2 = data;

        auto t1 = std::chrono::high_resolution_clock::now();
        bucket_sort(v1);
        auto t2 = std::chrono::high_resolution_clock::now();

        std::sort(v2.begin(), v2.end());
        auto t3 = std::chrono::high_resolution_clock::now();

        using ms = std::chrono::duration<double, std::milli>;
        std::cout << "n=" << N << ", 균등 분포 0.0~1.0\n";
        std::cout << "  Bucket Sort: " << ms(t2-t1).count() << " ms\n";
        std::cout << "  std::sort:   " << ms(t3-t2).count() << " ms\n";
        std::cout << "  결과 일치: " << (v1 == v2 ? "OK" : "FAIL") << "\n";
    }

    return 0;
}

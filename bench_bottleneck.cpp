// filename: bench_bottleneck.cpp
// g++ -std=c++17 -O2 -g -fno-omit-frame-pointer -o bench_bottleneck bench_bottleneck.cpp
//
// 프로파일링:
// perf stat ./bench_bottleneck
// perf record -g ./bench_bottleneck && perf report

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>

// ============================================================
// 캐시 친화적 버전: vector + 선형 순회
// ============================================================
long long sum_vector(int n) {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);

    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    // 연속 메모리: 프리패치 효과, L1/L2 캐시 히트
    for (int x : v) sum += x;
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "vector sum (" << n << "): "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << " us, sum=" << sum << '\n';
    return sum;
}

// ============================================================
// 캐시 비친화적 버전: list + 포인터 추적
// ============================================================
long long sum_list(int n) {
    std::list<int> l;
    for (int i = 0; i < n; ++i) l.push_back(i);

    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    // 비연속 메모리: 각 노드마다 캐시 미스 발생
    for (int x : l) sum += x;
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "list   sum (" << n << "): "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << " us, sum=" << sum << '\n';
    return sum;
}

// ============================================================
// 랜덤 접근: 캐시 thrashing 유발
// ============================================================
long long random_access_vector(int n) {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);

    // 랜덤 인덱스 생성
    std::vector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 rng(42);
    std::shuffle(indices.begin(), indices.end(), rng);

    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    // 랜덤 순서 접근 → 캐시 미스 빈발
    for (int idx : indices) sum += v[idx];
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "vector random (" << n << "): "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << " us, sum=" << sum << '\n';
    return sum;
}

int main() {
    constexpr int N = 1'000'000;

    std::cout << "=== 접근 패턴별 성능 비교 ===\n";
    sum_vector(N);         // 연속 접근: 빠름
    sum_list(N);           // 포인터 추적: 느림
    random_access_vector(N);  // 랜덤 접근: 중간

    // perf stat로 cache-misses를 비교하면 차이가 명확히 보임:
    // perf stat -e cache-misses,cache-references ./bench_bottleneck
    return 0;
}

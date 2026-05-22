// filename: unordered_map_internals.cpp
// g++ -std=c++17 -O2 -Wall -o unordered_map_internals unordered_map_internals.cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// 버킷 분포를 출력하는 유틸리티
void print_bucket_stats(const std::unordered_map<std::string, int>& m) {
    std::cout << "원소 수:      " << m.size()         << '\n';
    std::cout << "버킷 수:      " << m.bucket_count() << '\n';
    std::cout << "load_factor:  " << m.load_factor()  << '\n';
    std::cout << "max_load_fac: " << m.max_load_factor() << '\n';

    // 버킷별 원소 수 분포
    std::vector<std::size_t> dist(m.bucket_count());
    for (std::size_t i = 0; i < m.bucket_count(); ++i)
        dist[i] = m.bucket_size(i);

    std::size_t empty = std::count(dist.begin(), dist.end(), 0);
    std::size_t max_chain = *std::max_element(dist.begin(), dist.end());
    std::cout << "빈 버킷:      " << empty   << " / " << m.bucket_count() << '\n';
    std::cout << "최장 체인:    " << max_chain << '\n';
}

int main() {
    std::unordered_map<std::string, int> m;
    m.reserve(16);  // 최소 16개 버킷 확보, rehash 횟수 줄이기

    std::cout << "=== 초기 상태 ===\n";
    print_bucket_stats(m);

    // 원소 삽입
    const std::string keys[] = {
        "alpha", "beta", "gamma", "delta", "epsilon",
        "zeta",  "eta",  "theta", "iota",  "kappa"
    };
    for (int i = 0; auto& k : keys) m[k] = ++i;

    std::cout << "\n=== 10개 삽입 후 ===\n";
    print_bucket_stats(m);

    // 특정 키가 어느 버킷에 있는지 확인
    std::cout << "\n=== 버킷 탐색 ===\n";
    for (const auto& k : keys) {
        std::size_t b = m.bucket(k);
        std::cout << k << " → bucket[" << b << "]\n";
    }

    // 버킷 직접 순회
    std::cout << "\n=== 버킷 [0] 내용 직접 순회 ===\n";
    for (std::size_t b = 0; b < m.bucket_count(); ++b) {
        if (m.bucket_size(b) == 0) continue;
        std::cout << "bucket[" << b << "]: ";
        for (auto it = m.begin(b); it != m.end(b); ++it)
            std::cout << it->first << "=" << it->second << " ";
        std::cout << '\n';
    }

    // max_load_factor 낮추면 더 많은 버킷, 충돌 감소
    std::cout << "\n=== max_load_factor 0.5로 줄인 후 ===\n";
    m.max_load_factor(0.5f);
    m.rehash(0);  // max_load_factor에 맞춰 rehash 강제 실행
    print_bucket_stats(m);

    return 0;
}

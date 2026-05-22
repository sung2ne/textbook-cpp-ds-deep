// filename: cache_latency.cpp
// g++ -std=c++17 -O2 -o cache_latency cache_latency.cpp
// stride별 접근 시간 측정으로 캐시 계층 크기 추정

#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <cstdint>

// 배열 크기: 최대 64MB (L3 캐시를 넘도록)
static constexpr size_t MAX_SIZE  = 64 * 1024 * 1024; // bytes
static constexpr int    REPEAT    = 16;

// stride(요소 개수) 간격으로 배열을 순회하며 접근 시간 측정
double measure_ns_per_access(const std::vector<uint8_t>& arr,
                              size_t stride_bytes,
                              size_t total_accesses)
{
    volatile uint8_t sink = 0; // 최적화 방지
    size_t idx = 0;
    size_t n   = arr.size();

    auto t0 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < total_accesses; ++i) {
        sink += arr[idx];
        idx  += stride_bytes;
        if (idx >= n) idx -= n;
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
    return ns / static_cast<double>(total_accesses);
}

int main()
{
    // 배열 초기화
    std::vector<uint8_t> arr(MAX_SIZE, 1);

    std::cout << "stride_bytes\tns_per_access\n";

    // stride를 64바이트(cache line 크기)에서 시작해 2배씩 증가
    for (size_t stride = 64; stride <= MAX_SIZE / 2; stride *= 2) {
        size_t accesses = std::max<size_t>(1'000'000, MAX_SIZE / stride * REPEAT);

        double ns = measure_ns_per_access(arr, stride, accesses);
        std::cout << stride << "\t\t" << ns << "\n";
    }

    return 0;
}

// filename: simd_add.cpp
// g++ -std=c++17 -O2 -mavx2 -mfma -o simd_add simd_add.cpp
// ARM 계열: g++ -std=c++17 -O2 -o simd_add simd_add.cpp (NEON 자동 활성화)

#include <immintrin.h>  // AVX2 (x86-64 전용)
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cassert>
#include <cstdlib>      // posix_memalign

// 정렬된 메모리 할당 헬퍼
struct AlignedBuffer {
    float* data;
    AlignedBuffer(size_t n, size_t align = 32) {
        if (posix_memalign(reinterpret_cast<void**>(&data), align, n * sizeof(float)) != 0)
            throw std::bad_alloc{};
    }
    ~AlignedBuffer() { free(data); }
    float& operator[](size_t i) { return data[i]; }
    const float& operator[](size_t i) const { return data[i]; }
};

// 스칼라 버전
void add_scalar(const float* a, const float* b, float* c, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        c[i] = a[i] + b[i];
}

// AVX2 버전 (8× float 동시 처리)
void add_avx2(const float* a, const float* b, float* c, size_t n)
{
    size_t i = 0;
    // 8개 단위 처리
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_load_ps(a + i); // 32바이트 정렬 가정
        __m256 vb = _mm256_load_ps(b + i);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_store_ps(c + i, vc);
    }
    // 나머지 처리 (tail)
    for (; i < n; ++i)
        c[i] = a[i] + b[i];
}

template<typename F>
double bench_ms(F&& fn, int iter)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; ++i) fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main()
{
    // N은 8의 배수 (tail 처리 없이 깔끔하게)
    constexpr size_t N    = 1 << 24; // 16M floats = 64MB
    constexpr int    ITER = 20;

    AlignedBuffer a(N), b(N), c(N);
    for (size_t i = 0; i < N; ++i) { a[i] = static_cast<float>(i); b[i] = 1.0f; }

    double t_scalar = bench_ms([&]{ add_scalar(a.data, b.data, c.data, N); }, ITER);
    double t_avx2   = bench_ms([&]{ add_avx2  (a.data, b.data, c.data, N); }, ITER);

    std::cout << "Scalar: " << t_scalar << " ms\n";
    std::cout << "AVX2  : " << t_avx2   << " ms\n";
    std::cout << "Speedup: " << t_scalar / t_avx2 << "x\n";

    // 정확도 검증 (첫 번째 원소)
    add_scalar(a.data, b.data, c.data, N);
    float ref = c[0];
    add_avx2(a.data, b.data, c.data, N);
    assert(c[0] == ref && "결과 불일치");
    std::cout << "결과 검증 통과\n";
    return 0;
}

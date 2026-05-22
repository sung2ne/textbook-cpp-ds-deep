// filename: parallel_transform.cpp
// g++ -std=c++20 -O3 -march=native -o parallel_transform parallel_transform.cpp -ltbb
// (GCC on Linux: Intel TBB가 par_unseq 백엔드)
// macOS(Clang 15+): -L/opt/homebrew/lib -ltbb
// Windows(MSVC): ppl.h 기반, -ltbb 불필요

#include <algorithm>
#include <execution>
#include <numeric>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>

struct Vec3 { float x, y, z; };

// 시간 측정 헬퍼
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
    constexpr size_t N    = 4'000'000;
    constexpr int    ITER = 50;
    const float      dt   = 0.016f;

    // SoA 스타일로 x, vx 분리
    std::vector<float> px(N, 0.0f), py(N, 0.0f), pz(N, 0.0f);
    std::vector<float> vx(N, 1.0f), vy(N, 0.5f), vz(N, 0.25f);
    std::vector<size_t> indices(N);
    std::iota(indices.begin(), indices.end(), 0);

    // 순차 실행
    auto seq_fn = [&]{
        std::transform(std::execution::seq,
                       indices.begin(), indices.end(), indices.begin(),
                       [&](size_t i){
                           px[i] += vx[i] * dt;
                           py[i] += vy[i] * dt;
                           pz[i] += vz[i] * dt;
                           return i;
                       });
    };

    // par_unseq 실행
    auto par_fn = [&]{
        std::transform(std::execution::par_unseq,
                       indices.begin(), indices.end(), indices.begin(),
                       [&](size_t i){
                           px[i] += vx[i] * dt;
                           py[i] += vy[i] * dt;
                           pz[i] += vz[i] * dt;
                           return i;
                       });
    };

    double t_seq = bench_ms(seq_fn, ITER);
    double t_par = bench_ms(par_fn, ITER);

    std::cout << "seq     : " << t_seq << " ms\n";
    std::cout << "par_unseq: " << t_par << " ms\n";
    std::cout << "Speedup : " << t_seq / t_par << "x\n";

    // 검증
    std::cout << "px[0]=" << px[0] << " (기대: " << 1.0f * dt * ITER * 2 << ")\n";
    return 0;
}

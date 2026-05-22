// filename: soa_vs_aos.cpp
// g++ -std=c++17 -O3 -march=native -o soa_vs_aos soa_vs_aos.cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <numeric>

// ---------- AoS ----------
struct ParticleAoS {
    float x, y, z;
    float vx, vy, vz;
    float mass;
    float _pad; // 32바이트 정렬
};

void update_aos(std::vector<ParticleAoS>& p, float dt)
{
    for (auto& q : p) {
        q.x += q.vx * dt;
        q.y += q.vy * dt;
        q.z += q.vz * dt;
    }
}

// ---------- SoA ----------
struct ParticlesSoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;

    explicit ParticlesSoA(size_t n)
        : x(n), y(n), z(n), vx(n), vy(n), vz(n), mass(n) {}
};

void update_soa(ParticlesSoA& p, float dt)
{
    size_t n = p.x.size();
    for (size_t i = 0; i < n; ++i) {
        p.x[i] += p.vx[i] * dt;
        p.y[i] += p.vy[i] * dt;
        p.z[i] += p.vz[i] * dt;
    }
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
    constexpr size_t N    = 1'000'000;
    constexpr int    ITER = 200;
    const float      dt   = 0.016f;

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> d(-1.0f, 1.0f);

    // AoS 초기화
    std::vector<ParticleAoS> aos(N);
    for (auto& p : aos) {
        p = { d(rng), d(rng), d(rng), d(rng), d(rng), d(rng), 1.0f, 0.0f };
    }

    // SoA 초기화
    ParticlesSoA soa(N);
    for (size_t i = 0; i < N; ++i) {
        soa.x[i]  = d(rng); soa.y[i]  = d(rng); soa.z[i]  = d(rng);
        soa.vx[i] = d(rng); soa.vy[i] = d(rng); soa.vz[i] = d(rng);
        soa.mass[i] = 1.0f;
    }

    double t_aos = bench_ms([&]{ update_aos(aos, dt); }, ITER);
    double t_soa = bench_ms([&]{ update_soa(soa, dt); }, ITER);

    std::cout << "AoS " << ITER << " iterations: " << t_aos << " ms\n";
    std::cout << "SoA " << ITER << " iterations: " << t_soa << " ms\n";
    std::cout << "Speedup: " << t_aos / t_soa << "x\n";

    // 결과 동일성 확인 (최초값만)
    std::cout << "AoS x[0]=" << aos[0].x << "  SoA x[0]=" << soa.x[0] << "\n";
    return 0;
}

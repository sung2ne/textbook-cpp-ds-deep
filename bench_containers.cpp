// filename: bench_containers.cpp
// g++ -std=c++17 -O2 -o bench_containers bench_containers.cpp -lbenchmark -lpthread
// 또는 CMake 빌드 사용

#include <benchmark/benchmark.h>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <numeric>
#include <random>

// ============================================================
// 1. push_back 벤치마크 (N개 삽입)
// ============================================================
static void BM_vector_push_back(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(N);   // 재할당 없음
        for (int i = 0; i < N; ++i)
            v.push_back(i);
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_vector_push_back)->Range(64, 1 << 16);

static void BM_list_push_back(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::list<int> l;
        for (int i = 0; i < N; ++i)
            l.push_back(i);
        benchmark::DoNotOptimize(l.size());
    }
}
BENCHMARK(BM_list_push_back)->Range(64, 1 << 16);

static void BM_deque_push_back(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::deque<int> d;
        for (int i = 0; i < N; ++i)
            d.push_back(i);
        benchmark::DoNotOptimize(d.size());
    }
}
BENCHMARK(BM_deque_push_back)->Range(64, 1 << 16);

// ============================================================
// 2. 순회 (iteration) 벤치마크
// ============================================================
static void BM_vector_iterate(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);

    for (auto _ : state) {
        long long sum = 0;
        for (int x : v) sum += x;
        benchmark::DoNotOptimize(sum);
    }
    state.SetBytesProcessed(state.iterations() * N * sizeof(int));
}
BENCHMARK(BM_vector_iterate)->Range(64, 1 << 20);

static void BM_list_iterate(benchmark::State& state) {
    const int N = state.range(0);
    std::list<int> l;
    for (int i = 0; i < N; ++i) l.push_back(i);

    for (auto _ : state) {
        long long sum = 0;
        for (int x : l) sum += x;
        benchmark::DoNotOptimize(sum);
    }
    state.SetBytesProcessed(state.iterations() * N * sizeof(int));
}
BENCHMARK(BM_list_iterate)->Range(64, 1 << 20);

// ============================================================
// 3. 정렬 벤치마크 (랜덤 데이터)
// ============================================================
class SortFixture : public benchmark::Fixture {
public:
    void SetUp(const benchmark::State& state) override {
        int N = state.range(0);
        data.resize(N);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, N * 10);
        for (int& x : data) x = dist(rng);
    }

    std::vector<int> data;
};

BENCHMARK_DEFINE_F(SortFixture, VectorSort)(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();   // 복사 시간 제외
        std::vector<int> copy = data;
        state.ResumeTiming();

        std::sort(copy.begin(), copy.end());
        benchmark::DoNotOptimize(copy.data());
    }
}
BENCHMARK_REGISTER_F(SortFixture, VectorSort)->Range(64, 1 << 18);

// ============================================================
// 4. map vs unordered_map 조회 비교
// ============================================================
static void BM_map_lookup(benchmark::State& state) {
    const int N = state.range(0);
    std::map<int, int> m;
    for (int i = 0; i < N; ++i) m[i] = i * 2;

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> dist(0, N - 1);

    for (auto _ : state) {
        auto it = m.find(dist(rng));
        benchmark::DoNotOptimize(it);
    }
}
BENCHMARK(BM_map_lookup)->Range(64, 1 << 18);

static void BM_unordered_map_lookup(benchmark::State& state) {
    const int N = state.range(0);
    std::unordered_map<int, int> m;
    m.reserve(N);
    for (int i = 0; i < N; ++i) m[i] = i * 2;

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> dist(0, N - 1);

    for (auto _ : state) {
        auto it = m.find(dist(rng));
        benchmark::DoNotOptimize(it);
    }
}
BENCHMARK(BM_unordered_map_lookup)->Range(64, 1 << 18);

BENCHMARK_MAIN();

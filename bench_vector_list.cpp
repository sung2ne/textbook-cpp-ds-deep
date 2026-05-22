// filename: bench_vector_list.cpp
// g++ -std=c++17 -O2 -o bench bench_vector_list.cpp -lbenchmark -lpthread
// vcpkg 사용 시: cmake로 빌드 (CMakeLists.txt에서 target_link_libraries 필요)
#include <benchmark/benchmark.h>
#include <vector>
#include <list>
#include <numeric>

// ─── push_back 벤치마크 ───────────────────────────────────────

static void BM_VectorPushBack(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(N);  // 재할당 제거 — push_back 자체 비용만 측정
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_VectorPushBackNoReserve(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::vector<int> v;
        // reserve 없음 — 재할당 포함
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_ListPushBack(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::list<int> lst;
        for (int i = 0; i < N; i++) {
            lst.push_back(i);
        }
        benchmark::DoNotOptimize(&lst);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// ─── 순회 벤치마크 ───────────────────────────────────────────

static void BM_VectorTraversal(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);

    for (auto _ : state) {
        long long sum = 0;
        for (int x : v) sum += x;
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_ListTraversal(benchmark::State& state) {
    const int N = state.range(0);
    std::list<int> lst;
    for (int i = 0; i < N; i++) lst.push_back(i);

    for (auto _ : state) {
        long long sum = 0;
        for (int x : lst) sum += x;
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// ─── 등록 및 실행 ────────────────────────────────────────────

BENCHMARK(BM_VectorPushBack)         ->RangeMultiplier(4)->Range(256, 65536);
BENCHMARK(BM_VectorPushBackNoReserve)->RangeMultiplier(4)->Range(256, 65536);
BENCHMARK(BM_ListPushBack)           ->RangeMultiplier(4)->Range(256, 65536);
BENCHMARK(BM_VectorTraversal)        ->RangeMultiplier(4)->Range(256, 65536);
BENCHMARK(BM_ListTraversal)          ->RangeMultiplier(4)->Range(256, 65536);

BENCHMARK_MAIN();

// filename: thread_basics.cpp
// g++ -std=c++17 -O2 -pthread -o thread_basics thread_basics.cpp

#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <chrono>

// 범위 합산 함수 (스레드에서 실행)
long long partial_sum(const std::vector<int>& v, size_t begin, size_t end)
{
    long long s = 0;
    for (size_t i = begin; i < end; ++i)
        s += v[i];
    return s;
}

int main()
{
    constexpr size_t N = 100'000'000;
    std::vector<int> data(N, 1); // 모두 1로 채움

    // 논리 코어 수 확인
    unsigned int n_cores = std::thread::hardware_concurrency();
    std::cout << "논리 코어 수: " << n_cores << "\n";

    // 결과를 담을 배열 (race condition 없이 — 각 스레드가 독립 슬롯 사용)
    std::vector<long long> results(n_cores, 0);
    std::vector<std::thread> threads;

    size_t chunk = N / n_cores;

    auto t0 = std::chrono::high_resolution_clock::now();

    for (unsigned i = 0; i < n_cores; ++i) {
        size_t begin = i * chunk;
        size_t end   = (i + 1 == n_cores) ? N : begin + chunk;
        // 람다로 스레드에 캡처
        threads.emplace_back([&data, &results, i, begin, end]{
            results[i] = partial_sum(data, begin, end);
        });
    }

    // 모든 스레드가 끝날 때까지 대기
    for (auto& t : threads)
        t.join(); // join() 없이 소멸하면 std::terminate 호출!

    auto t1 = std::chrono::high_resolution_clock::now();

    long long total = std::accumulate(results.begin(), results.end(), 0LL);
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "합산 결과: " << total << " (기대: " << N << ")\n";
    std::cout << "소요 시간: " << ms << " ms\n";
    return 0;
}

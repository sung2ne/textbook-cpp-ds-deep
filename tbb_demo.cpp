// filename: tbb_demo.cpp
// g++ -std=c++17 -O2 -o tbb_demo tbb_demo.cpp -ltbb

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <cmath>

#include <tbb/tbb.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <tbb/global_control.h>

// ============================================================
// 1. tbb::parallel_for — 배열 병렬 처리
// ============================================================
void demo_parallel_for() {
    constexpr int N = 10'000'000;
    std::vector<double> data(N);
    std::iota(data.begin(), data.end(), 0.0);

    // 직렬 버전
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        data[i] = std::sqrt(data[i]) * std::log(data[i] + 1);
    auto end = std::chrono::high_resolution_clock::now();
    auto serial_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // 초기화 다시
    std::iota(data.begin(), data.end(), 0.0);

    // TBB parallel_for 버전
    start = std::chrono::high_resolution_clock::now();
    tbb::parallel_for(
        tbb::blocked_range<int>(0, N),      // [0, N) 범위
        [&data](const tbb::blocked_range<int>& r) {
            for (int i = r.begin(); i < r.end(); ++i)
                data[i] = std::sqrt(data[i]) * std::log(data[i] + 1);
        }
        // 그레인 크기 지정 가능: tbb::blocked_range<int>(0, N, 1000)
    );
    end = std::chrono::high_resolution_clock::now();
    auto parallel_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "parallel_for (" << N << " 요소):\n";
    std::cout << "  직렬: " << serial_ms << " ms\n";
    std::cout << "  TBB:  " << parallel_ms << " ms\n";
    if (parallel_ms > 0)
        std::cout << "  속도 향상: " << static_cast<double>(serial_ms) / parallel_ms << "x\n";
}

// ============================================================
// 2. tbb::parallel_reduce — 병렬 집계
// ============================================================
void demo_parallel_reduce() {
    constexpr int N = 100'000'000;
    std::vector<long long> data(N);
    std::iota(data.begin(), data.end(), 1LL);

    // TBB parallel_reduce
    auto start = std::chrono::high_resolution_clock::now();
    long long total = tbb::parallel_reduce(
        tbb::blocked_range<int>(0, N),
        0LL,   // 초기값
        // reduce 함수: 부분 합 계산
        [&data](const tbb::blocked_range<int>& r, long long init) -> long long {
            for (int i = r.begin(); i < r.end(); ++i)
                init += data[i];
            return init;
        },
        // join 함수: 두 결과 합치기
        [](long long a, long long b) { return a + b; }
    );
    auto end = std::chrono::high_resolution_clock::now();

    long long expected = (long long)N * (N + 1) / 2;
    std::cout << "\nparallel_reduce sum(1.." << N << "):\n";
    std::cout << "  결과: " << total << " (기대: " << expected << ")\n";
    std::cout << "  시간: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";
}

// ============================================================
// 3. tbb::concurrent_hash_map
// ============================================================
void demo_concurrent_hash_map() {
    using MyMap = tbb::concurrent_hash_map<int, int>;
    MyMap cmap;

    constexpr int N = 1'000'000;

    // 멀티스레드 삽입
    tbb::parallel_for(0, N, [&cmap](int i) {
        MyMap::accessor acc;  // 쓰기 잠금
        if (cmap.insert(acc, i)) {
            acc->second = i * 2;  // 새 키
        } else {
            acc->second += 1;     // 기존 키 업데이트
        }
    });

    // 멀티스레드 조회
    std::atomic<long long> sum{0};
    tbb::parallel_for(0, N, [&cmap, &sum](int i) {
        MyMap::const_accessor acc;  // 읽기 잠금 (공유 가능)
        if (cmap.find(acc, i)) {
            sum += acc->second;
        }
    });

    std::cout << "\nconcurrent_hash_map 결과:\n";
    std::cout << "  크기: " << cmap.size() << " (기대: " << N << ")\n";
    std::cout << "  합계: " << sum.load() << '\n';
}

// ============================================================
// 4. tbb::concurrent_queue — 멀티프로듀서-멀티컨슈머
// ============================================================
void demo_concurrent_queue() {
    tbb::concurrent_queue<int> queue;
    constexpr int ITEMS = 100'000;
    std::atomic<long long> consumed_sum{0};
    std::atomic<int> consumed_count{0};

    // 생산자: 병렬로 삽입
    tbb::parallel_for(0, ITEMS, [&queue](int i) {
        queue.push(i);
    });

    // 소비자: 병렬로 꺼내기
    tbb::parallel_for(0, 8, [&queue, &consumed_sum, &consumed_count, ITEMS](int) {
        int val;
        while (queue.try_pop(val)) {
            consumed_sum += val;
            ++consumed_count;
        }
    });

    long long expected = (long long)ITEMS * (ITEMS - 1) / 2;
    std::cout << "\nconcurrent_queue:\n";
    std::cout << "  소비된 항목 수: " << consumed_count.load() << " (기대: " << ITEMS << ")\n";
    std::cout << "  합계: " << consumed_sum.load() << " (기대: " << expected << ")\n";
}

// ============================================================
// 5. TBB 스레드 수 제어
// ============================================================
void demo_thread_control() {
    int hw_threads = tbb::info::default_concurrency();
    std::cout << "\n하드웨어 스레드 수: " << hw_threads << '\n';

    // 스레드 수를 2로 제한
    {
        tbb::global_control gc{
            tbb::global_control::max_allowed_parallelism, 2
        };
        std::cout << "제한된 병렬 처리 수: "
                  << tbb::global_control::active_value(
                         tbb::global_control::max_allowed_parallelism) << '\n';

        // 이 블록 안에서는 최대 2개 스레드만 사용
        tbb::parallel_for(0, 100, [](int i) {
            // 가벼운 작업
            volatile int x = i * i;
            (void)x;
        });
    }
    // global_control 소멸 → 원래 스레드 수로 복원
}

int main() {
    std::cout << "=== Intel TBB 데모 ===\n\n";

    demo_parallel_for();
    demo_parallel_reduce();
    demo_concurrent_hash_map();
    demo_concurrent_queue();
    demo_thread_control();

    return 0;
}

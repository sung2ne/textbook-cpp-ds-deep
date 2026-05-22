// filename: cas_counter.cpp
// g++ -std=c++17 -O2 -pthread -o cas_counter cas_counter.cpp
#include <atomic>
#include <thread>
#include <iostream>

// CAS 기반 lock-free 최댓값 업데이트
void update_max(std::atomic<int>& maximum, int candidate)
{
    int current = maximum.load(std::memory_order_relaxed);
    // 후보가 현재 최댓값보다 클 때만 업데이트
    while (candidate > current) {
        // CAS: current가 여전히 maximum이면 candidate로 교체
        if (maximum.compare_exchange_weak(current, candidate,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed)) {
            break; // 성공
        }
        // 실패 시 current는 최신 maximum 값으로 자동 갱신 → 루프 반복
    }
}

int main()
{
    std::atomic<int> max_val{0};
    std::vector<std::thread> threads;

    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&, i]{
            for (int v = i; v < 10000; v += 8)
                update_max(max_val, v);
        });
    }
    for (auto& t : threads) t.join();

    std::cout << "최댓값: " << max_val.load() << " (기대: 9999)\n";
    return 0;
}

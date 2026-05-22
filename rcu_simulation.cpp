// filename: rcu_simulation.cpp
// g++ -std=c++17 -O2 -pthread -o rcu_simulation rcu_simulation.cpp

#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <cassert>

// 설정 데이터 (복사 비용이 낮은 값 객체)
struct Config {
    int   max_connections;
    float timeout_sec;
    bool  debug_mode;
};

// RCU 스타일 설정 관리자
class RcuConfig {
    // shared_ptr: atomic 교체 + 참조 카운팅으로 Grace Period 에뮬레이션
    std::atomic<std::shared_ptr<Config>> current_;

public:
    explicit RcuConfig(Config init)
        : current_(std::make_shared<Config>(std::move(init))) {}

    // 읽기: lock 없음 — O(1)
    std::shared_ptr<const Config> read() const
    {
        // atomic load: 현재 포인터를 원자적으로 복사
        // shared_ptr 복사는 참조 카운트를 올려 해제를 방지
        return current_.load(std::memory_order_acquire);
    }

    // 쓰기: 새 복사본 생성 후 원자적 교체
    void update(Config new_config)
    {
        auto new_ptr = std::make_shared<Config>(std::move(new_config));
        // 이전 포인터 교체 — 이전 공유 포인터는 이 스코프를 벗어나면
        // 참조 카운트가 0이 되면 자동 해제 (Grace Period는 shared_ptr가 처리)
        current_.store(new_ptr, std::memory_order_release);
        // 실제 liburcu에서는 여기서 synchronize_rcu() 또는 call_rcu()를 호출
    }
};

// 성능 측정
int main()
{
    RcuConfig config(Config{100, 30.0f, false});
    constexpr int READERS  = 6;
    constexpr int WRITERS  = 1;
    constexpr int ITER     = 2'000'000;

    std::atomic<long long> read_ops{0};
    std::atomic<int>       write_ops{0};
    std::atomic<bool>      stop{false};

    // 읽기 스레드들 (lock-free 읽기)
    std::vector<std::thread> threads;
    for (int i = 0; i < READERS; ++i) {
        threads.emplace_back([&]{
            long long local = 0;
            while (!stop.load(std::memory_order_relaxed)) {
                auto cfg = config.read(); // lock-free
                // 실제 사용
                if (cfg->max_connections > 0) ++local;
            }
            read_ops.fetch_add(local, std::memory_order_relaxed);
        });
    }

    // 쓰기 스레드 (드물게 업데이트)
    threads.emplace_back([&]{
        for (int i = 0; i < 100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            config.update(Config{100 + i, 30.0f, (i % 10 == 0)});
            write_ops.fetch_add(1, std::memory_order_relaxed);
        }
        stop.store(true, std::memory_order_release);
    });

    for (auto& t : threads) t.join();

    std::cout << "읽기 연산: " << read_ops.load() << "\n";
    std::cout << "쓰기 연산: " << write_ops.load() << "\n";
    std::cout << "최종 max_connections: "
              << config.read()->max_connections << "\n";
    return 0;
}

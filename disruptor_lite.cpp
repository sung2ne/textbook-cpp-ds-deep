// filename: disruptor_lite.cpp
// g++ -std=c++17 -O3 -march=native -pthread -o disruptor_lite disruptor_lite.cpp

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstring>

// 캐시 라인 크기 (대부분의 x86_64: 64 bytes)
static constexpr std::size_t CACHE_LINE = 64;

// ============================================================
// SPSC Ring Buffer (Single Producer, Single Consumer)
// Disruptor-lite: CAS 없음, memory_order만 사용
// ============================================================
template <typename T, std::size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity는 2의 거듭제곱이어야 함");
    static_assert(std::is_trivially_copyable_v<T>,
                  "T는 trivially copyable이어야 함 (lock-free 환경)");

public:
    SPSCQueue() : head_(0), tail_(0) {
        std::memset(buffer_, 0, sizeof(buffer_));
    }

    // ---- 생산자 측 ----
    bool try_push(const T& val) noexcept {
        // head_: 생산자만 씀, 소비자는 읽기만
        const std::size_t current_head = head_.load(std::memory_order_relaxed);
        const std::size_t next_head    = (current_head + 1) & MASK;

        // 버퍼 꽉 찼으면 실패
        if (next_head == tail_.load(std::memory_order_acquire))
            return false;

        buffer_[current_head] = val;

        // release: buffer 쓰기가 head 업데이트 이전에 완료됨을 보장
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    // ---- 소비자 측 ----
    bool try_pop(T& val) noexcept {
        // tail_: 소비자만 씀, 생산자는 읽기만
        const std::size_t current_tail = tail_.load(std::memory_order_relaxed);

        // 버퍼 비어있으면 실패
        if (current_tail == head_.load(std::memory_order_acquire))
            return false;

        val = buffer_[current_tail];

        // release: buffer 읽기가 tail 업데이트 이전에 완료됨을 보장
        tail_.store((current_tail + 1) & MASK, std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return tail_.load(std::memory_order_acquire) ==
               head_.load(std::memory_order_acquire);
    }

    std::size_t size() const noexcept {
        std::size_t h = head_.load(std::memory_order_acquire);
        std::size_t t = tail_.load(std::memory_order_acquire);
        return (h - t + Capacity) & MASK;
    }

private:
    static constexpr std::size_t MASK = Capacity - 1;

    // ---- 캐시 라인 분리 (False Sharing 방지) ----
    // head_: 생산자 스레드가 쓰는 변수
    alignas(CACHE_LINE) std::atomic<std::size_t> head_;
    char head_pad_[CACHE_LINE - sizeof(std::atomic<std::size_t>)];

    // tail_: 소비자 스레드가 쓰는 변수
    alignas(CACHE_LINE) std::atomic<std::size_t> tail_;
    char tail_pad_[CACHE_LINE - sizeof(std::atomic<std::size_t>)];

    // 데이터 버퍼
    alignas(CACHE_LINE) T buffer_[Capacity];
};

// ============================================================
// HFT 시나리오: 시장 데이터 → 신호 계산 → 주문 전송
// ============================================================
struct MarketData {
    long long  timestamp_ns;   // 수신 시각 (나노초)
    double     bid;
    double     ask;
    int        symbol_id;
};

struct OrderSignal {
    long long  timestamp_ns;
    int        symbol_id;
    double     price;
    int        quantity;
    char       side;    // 'B'=매수, 'S'=매도
};

// HFT 파이프라인: market_data_queue → signal_queue
using MarketQueue = SPSCQueue<MarketData, 4096>;
using SignalQueue  = SPSCQueue<OrderSignal, 4096>;

// ============================================================
// 벤치마크: std::queue(mutex) vs SPSCQueue
// ============================================================
#include <queue>
#include <mutex>

long long bench_spsc(int n) {
    SPSCQueue<int, 1024> q;
    long long total = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producer([&q, n] {
        for (int i = 0; i < n; ++i) {
            while (!q.try_push(i)) {}  // busy spin until space
        }
    });

    for (int i = 0; i < n; ) {
        int val;
        if (q.try_pop(val)) {
            total += val;
            ++i;
        }
    }

    producer.join();
    auto end = std::chrono::high_resolution_clock::now();
    (void)total;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

long long bench_mutex_queue(int n) {
    std::queue<int> q;
    std::mutex      mu;
    long long total = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producer([&q, &mu, n] {
        for (int i = 0; i < n; ++i) {
            std::lock_guard lk{mu};
            q.push(i);
        }
    });

    for (int i = 0; i < n; ) {
        std::unique_lock lk{mu};
        if (!q.empty()) {
            total += q.front();
            q.pop();
            ++i;
        }
    }

    producer.join();
    auto end = std::chrono::high_resolution_clock::now();
    (void)total;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int main() {
    // --- SPSC 기본 동작 ---
    std::cout << "=== SPSC Queue 기본 동작 ===\n";
    {
        SPSCQueue<int, 8> q;
        std::cout << "초기 empty: " << q.empty() << '\n';

        for (int i = 1; i <= 5; ++i) q.try_push(i);
        std::cout << "5개 push 후 size: " << q.size() << '\n';

        int val;
        while (q.try_pop(val)) std::cout << val << ' ';
        std::cout << '\n';
        std::cout << "pop 후 empty: " << q.empty() << '\n';
    }

    // --- HFT 파이프라인 시뮬레이션 ---
    std::cout << "\n=== HFT 파이프라인 시뮬레이션 ===\n";
    {
        MarketQueue mq;
        SignalQueue  sq;
        constexpr int TICKS = 100'000;

        // 생산자: 시장 데이터 생성
        std::thread feed_thread([&mq, TICKS] {
            for (int i = 0; i < TICKS; ++i) {
                MarketData md;
                md.timestamp_ns = i * 1000;
                md.bid = 100.0 + (i % 10) * 0.01;
                md.ask = md.bid + 0.01;
                md.symbol_id = i % 16;
                while (!mq.try_push(md)) {}
            }
        });

        // 신호 처리기 + 주문 생성
        std::thread strategy_thread([&mq, &sq, TICKS] {
            int processed = 0;
            while (processed < TICKS) {
                MarketData md;
                if (mq.try_pop(md)) {
                    // 간단한 신호: bid-ask 스프레드가 충분하면 매수
                    if (md.ask - md.bid < 0.015) {
                        OrderSignal sig;
                        sig.timestamp_ns = md.timestamp_ns + 100;  // 100ns 처리 지연
                        sig.symbol_id    = md.symbol_id;
                        sig.price        = md.bid;
                        sig.quantity     = 100;
                        sig.side         = 'B';
                        while (!sq.try_push(sig)) {}
                    }
                    ++processed;
                }
            }
        });

        feed_thread.join();
        strategy_thread.join();

        std::cout << "처리된 틱: " << TICKS << '\n';
        std::cout << "생성된 신호: " << sq.size() << '\n';
    }

    // --- 성능 비교 ---
    std::cout << "\n=== 성능 비교: mutex vs lock-free ===\n";
    constexpr int N = 1'000'000;

    long long t_spsc  = bench_spsc(N);
    long long t_mutex = bench_mutex_queue(N);

    std::cout << "SPSC lock-free  : " << t_spsc  / N << " ns/item\n";
    std::cout << "mutex std::queue: " << t_mutex / N << " ns/item\n";
    std::cout << "속도 향상: " << static_cast<double>(t_mutex) / t_spsc << "x\n";

    return 0;
}

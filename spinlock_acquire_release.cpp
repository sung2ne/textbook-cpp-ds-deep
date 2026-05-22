// filename: spinlock_acquire_release.cpp
// g++ -std=c++17 -O2 -pthread -o spinlock_acquire_release spinlock_acquire_release.cpp
#include <atomic>
#include <thread>
#include <iostream>

class AcqRelSpinLock {
    std::atomic<bool> locked_{false};
public:
    void lock() {
        bool expected = false;
        // acquire: lock 이후의 임계 구역 접근이 앞으로 올라오지 않음
        while (!locked_.compare_exchange_weak(expected, true,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed)) {
            expected = false; // CAS 실패 시 expected가 true로 갱신되므로 리셋
            // 스핀 중 CPU에게 힌트 (x86: PAUSE, ARM: YIELD)
            // GCC/Clang에서는 __builtin_ia32_pause() 또는 std::this_thread::yield()
        }
    }
    void unlock() {
        // release: unlock 이전의 임계 구역 쓰기가 다른 스레드에게 보임
        locked_.store(false, std::memory_order_release);
    }
};

AcqRelSpinLock spin;
int safe_data = 0;

int main()
{
    constexpr int N = 2'000'000;
    auto worker = [&](int n){
        for (int i = 0; i < n; ++i) {
            spin.lock();
            ++safe_data;
            spin.unlock();
        }
    };
    std::thread t1(worker, N), t2(worker, N);
    t1.join(); t2.join();
    std::cout << "safe_data = " << safe_data << " (기대: " << 2*N << ")\n";
    return 0;
}

// filename: spinlock_atomicflag.cpp
// g++ -std=c++17 -O2 -pthread -o spinlock_atomicflag spinlock_atomicflag.cpp
#include <atomic>
#include <thread>
#include <iostream>

class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() {
        // test_and_set: 항상 true로 설정하고 이전 값 반환
        // 이전 값이 false였으면 내가 획득, true면 다른 스레드가 보유 중 → 계속 spin
        while (flag_.test_and_set(std::memory_order_acquire))
            ; // spin
    }
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

SpinLock spin;
int      data = 0;

void worker(int n) {
    for (int i = 0; i < n; ++i) {
        spin.lock();
        ++data;
        spin.unlock();
    }
}

int main()
{
    constexpr int N = 1'000'000;
    std::thread t1(worker, N);
    std::thread t2(worker, N);
    t1.join(); t2.join();
    std::cout << "data = " << data << " (기대: " << 2*N << ")\n";
    return 0;
}

// filename: mutex_raii.cpp
// g++ -std=c++17 -O2 -pthread -o mutex_raii mutex_raii.cpp
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>

class SafeCounter {
    int          value_ = 0;
    mutable std::mutex mtx_;
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx_); // 소멸 시 자동 unlock
        ++value_;
    }
    int get() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return value_;
    }
};

int main()
{
    SafeCounter counter;
    constexpr int N = 1'000'000;

    std::thread t1([&]{ for (int i = 0; i < N; ++i) counter.increment(); });
    std::thread t2([&]{ for (int i = 0; i < N; ++i) counter.increment(); });
    t1.join();
    t2.join();

    std::cout << "counter = " << counter.get()
              << " (기대: " << 2 * N << ")\n";
    return 0;
}

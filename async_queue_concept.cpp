// filename: async_queue_concept.cpp
// g++ -std=c++20 -O2 -pthread -o async_queue_concept async_queue_concept.cpp
// (단순화된 개념 코드 — 실제 async framework은 cppcoro, asio 등 사용)

#include <coroutine>
#include <queue>
#include <mutex>
#include <optional>
#include <iostream>
#include <thread>
#include <functional>

// 비동기 팝을 지원하는 채널 개념
// (실전에서는 cppcoro::channel 또는 asio::channel 사용)
template<typename T>
class AsyncChannel {
    std::queue<T>             queue_;
    std::mutex                mtx_;
    std::queue<std::function<void(T)>> waiters_; // 대기 중인 Coroutine 재개 콜백

public:
    void push(T value)
    {
        std::function<void(T)> waiter;
        {
            std::lock_guard lk(mtx_);
            if (!waiters_.empty()) {
                waiter = std::move(waiters_.front());
                waiters_.pop();
            } else {
                queue_.push(std::move(value));
                return;
            }
        }
        waiter(std::move(value)); // Coroutine 재개
    }

    // Awaitable: co_await channel.pop() 문법 지원
    struct PopAwaitable {
        AsyncChannel& ch;
        std::optional<T> result;

        bool await_ready() {
            std::lock_guard lk(ch.mtx_);
            if (!ch.queue_.empty()) {
                result = std::move(ch.queue_.front());
                ch.queue_.pop();
                return true; // 즉시 반환 (suspend 불필요)
            }
            return false; // 대기 필요
        }

        void await_suspend(std::coroutine_handle<> h) {
            // Coroutine을 나중에 재개하는 콜백 등록
            std::lock_guard lk(ch.mtx_);
            ch.waiters_.push([this, h](T val) mutable {
                result = std::move(val);
                h.resume(); // Coroutine 재개
            });
        }

        T await_resume() { return std::move(*result); }
    };

    PopAwaitable pop() { return PopAwaitable{*this}; }
};

// Task: co_await를 지원하는 단순 태스크 타입
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

AsyncChannel<int> channel;

// 소비자 Coroutine
Task consumer()
{
    for (int i = 0; i < 5; ++i) {
        int val = co_await channel.pop(); // 데이터가 올 때까지 중단
        std::cout << "수신: " << val << "\n";
    }
}

int main()
{
    consumer(); // Coroutine 시작 (initial_suspend가 never이므로 즉시 실행)

    // 생산자 스레드에서 값 전송
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "전송: " << i << "\n";
        channel.push(i);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}

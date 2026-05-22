// filename: coroutine_generator.cpp
// g++ -std=c++20 -O2 -o coroutine_generator coroutine_generator.cpp

#include <coroutine>
#include <iostream>
#include <optional>
#include <stdexcept>

// 간단한 Generator 타입 (C++23의 std::generator를 직접 구현)
template<typename T>
class Generator {
public:
    // Coroutine이 필요로 하는 promise_type
    struct promise_type {
        std::optional<T> current_value;
        std::exception_ptr exception;

        Generator get_return_object()
        {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()   noexcept { return {}; }

        // co_yield: 값을 current_value에 저장하고 중단
        std::suspend_always yield_value(T value)
        {
            current_value = std::move(value);
            return {};
        }

        void return_void() noexcept {}

        void unhandled_exception()
        {
            exception = std::current_exception();
        }
    };

    // 이터레이터: Generator를 range-for에서 사용할 수 있도록
    struct iterator {
        std::coroutine_handle<promise_type> handle;

        iterator& operator++()
        {
            handle.resume(); // Coroutine 재개
            return *this;
        }

        const T& operator*() const
        {
            if (handle.promise().exception)
                std::rethrow_exception(handle.promise().exception);
            return *handle.promise().current_value;
        }

        bool operator==(std::default_sentinel_t) const
        {
            return handle.done();
        }
    };

    iterator begin()
    {
        handle_.resume(); // 첫 번째 값까지 실행
        return iterator{handle_};
    }

    std::default_sentinel_t end() { return {}; }

    explicit Generator(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~Generator() { if (handle_) handle_.destroy(); }

    // non-copyable, movable
    Generator(const Generator&) = delete;
    Generator(Generator&& other) noexcept
        : handle_(std::exchange(other.handle_, {})) {}

private:
    std::coroutine_handle<promise_type> handle_;
};

// --- Coroutine 함수들 ---

// 무한 피보나치 수열 생성기
Generator<long long> fibonacci()
{
    long long a = 0, b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

// 범위 내 짝수 생성기
Generator<int> even_range(int from, int to)
{
    for (int i = from; i <= to; ++i) {
        if (i % 2 == 0)
            co_yield i;
    }
}

// 두 시퀀스를 합성하는 Coroutine
Generator<int> take(Generator<long long>& gen, int n)
{
    int count = 0;
    for (long long v : gen) {
        if (count++ >= n) co_return;
        co_yield static_cast<int>(v);
    }
}

int main()
{
    // 피보나치 수열 첫 10개 출력
    std::cout << "피보나치(첫 10개): ";
    auto fib = fibonacci();
    int count = 0;
    for (long long v : fib) {
        std::cout << v << " ";
        if (++count >= 10) break;
    }
    std::cout << "\n";

    // 짝수 범위
    std::cout << "1~20 짝수: ";
    for (int v : even_range(1, 20))
        std::cout << v << " ";
    std::cout << "\n";

    return 0;
}

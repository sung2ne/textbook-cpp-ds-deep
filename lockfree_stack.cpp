// filename: lockfree_stack.cpp
// g++ -std=c++17 -O2 -pthread -o lockfree_stack lockfree_stack.cpp

#include <atomic>
#include <optional>
#include <thread>
#include <iostream>
#include <vector>
#include <cassert>

// Treiber Stack — shared_ptr 기반 (ABA 문제 없음, 메모리 안전)
// 성능 중심이면 Ch 07의 Hazard Pointer 버전을 사용

template<typename T>
class LockFreeStack {
    struct Node {
        T                      value;
        std::shared_ptr<Node>  next;
        explicit Node(T v) : value(std::move(v)) {}
    };

    // C++20: std::atomic<std::shared_ptr<Node>>
    // C++17 호환: std::shared_ptr를 raw pointer + count로 분해
    // 여기서는 C++20 방식 사용 (GCC 12+, Clang 14+ 지원)
    std::atomic<std::shared_ptr<Node>> head_;

public:
    LockFreeStack() : head_(nullptr) {}

    // push: O(1) amortized, wait-free에 가까움
    void push(T value)
    {
        auto new_node = std::make_shared<Node>(std::move(value));
        // head_를 new_node->next로 설정하고, CAS로 head_를 new_node로 교체
        new_node->next = head_.load(std::memory_order_acquire);
        while (!head_.compare_exchange_weak(new_node->next, new_node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {
            // 실패 시 new_node->next가 최신 head로 자동 갱신됨
        }
    }

    // pop: O(1) amortized, lock-free
    std::optional<T> pop()
    {
        auto old_head = head_.load(std::memory_order_acquire);
        while (old_head) {
            if (head_.compare_exchange_weak(old_head, old_head->next,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed)) {
                return std::move(old_head->value);
            }
            // old_head는 최신 head로 자동 갱신됨
        }
        return std::nullopt; // 스택이 비어있음
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) == nullptr;
    }
};

// 테스트: N개 push, N개 pop, 순서는 LIFO (단 멀티스레드에서 순서 비결정적)
int main()
{
    LockFreeStack<int> stack;
    constexpr int N = 100'000;
    constexpr int NTHREADS = 4;

    std::atomic<int> pop_sum{0};

    // 프로듀서: N개 push
    auto producer = [&]{
        for (int i = 0; i < N; ++i)
            stack.push(1); // 모두 1로 넣어 합산으로 검증
    };

    // 컨슈머: 값 pop해서 합산
    auto consumer = [&]{
        int local = 0;
        for (int i = 0; i < N; ++i) {
            std::optional<int> v;
            while (!(v = stack.pop())) ; // 값이 생길 때까지 스핀
            local += *v;
        }
        pop_sum.fetch_add(local, std::memory_order_relaxed);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < NTHREADS; ++i)
        threads.emplace_back(producer);
    for (int i = 0; i < NTHREADS; ++i)
        threads.emplace_back(consumer);
    for (auto& t : threads) t.join();

    int expected = N * NTHREADS;
    std::cout << "pop_sum = " << pop_sum.load()
              << " (기대: " << expected << ")\n";
    assert(pop_sum.load() == expected);
    std::cout << "검증 통과\n";
    return 0;
}

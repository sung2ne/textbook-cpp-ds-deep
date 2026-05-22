// filename: pool_allocator.cpp
// g++ -std=c++17 -O2 -Wall -o pool_allocator pool_allocator.cpp

#include <iostream>
#include <chrono>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdlib>

// FixedPoolAllocator: T 타입 객체 전용, 슬롯 BlockCount개
template <typename T, std::size_t BlockCount = 1024>
class FixedPoolAllocator {
public:
    using value_type = T;

    static constexpr std::size_t SlotSize =
        sizeof(T) >= sizeof(void*) ? sizeof(T) : sizeof(void*);

    FixedPoolAllocator() {
        // 풀 메모리 할당 (정렬 보장)
        pool_ = static_cast<char*>(std::aligned_alloc(alignof(T), SlotSize * BlockCount));
        if (!pool_) throw std::bad_alloc{};

        // Free list 초기화: 슬롯들을 연결
        free_head_ = reinterpret_cast<Slot*>(pool_);
        Slot* cur = free_head_;
        for (std::size_t i = 0; i + 1 < BlockCount; ++i) {
            char* next_ptr = pool_ + SlotSize * (i + 1);
            cur->next = reinterpret_cast<Slot*>(next_ptr);
            cur = cur->next;
        }
        cur->next = nullptr;  // 마지막 슬롯
    }

    ~FixedPoolAllocator() {
        std::free(pool_);
    }

    // 복사 금지 (풀 소유권 이전 불명확)
    FixedPoolAllocator(const FixedPoolAllocator&) = delete;
    FixedPoolAllocator& operator=(const FixedPoolAllocator&) = delete;

    [[nodiscard]] T* allocate(std::size_t n = 1) {
        if (n != 1) throw std::bad_alloc{};  // 고정 크기 풀: 1개만 지원
        if (!free_head_) throw std::bad_alloc{};  // 풀 소진

        Slot* slot = free_head_;
        free_head_ = free_head_->next;
        ++allocated_count_;
        return reinterpret_cast<T*>(slot);
    }

    void deallocate(T* ptr, std::size_t n = 1) noexcept {
        assert(n == 1);
        Slot* slot = reinterpret_cast<Slot*>(ptr);
        slot->next = free_head_;
        free_head_ = slot;
        --allocated_count_;
    }

    // construct / destroy: allocator_traits가 기본 구현 사용
    // (placement new / ptr->~T() 호출)

    std::size_t available() const noexcept {
        std::size_t count = 0;
        Slot* cur = free_head_;
        while (cur) { ++count; cur = cur->next; }
        return count;
    }

    std::size_t allocated() const noexcept { return allocated_count_; }

private:
    struct Slot { Slot* next; };

    char*  pool_            = nullptr;
    Slot*  free_head_       = nullptr;
    std::size_t allocated_count_ = 0;
};

// 벤치마크 도우미
template <typename Func>
long long measure_ns(Func&& f, int repeat = 10000) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < repeat; ++i) f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / repeat;
}

struct Node {
    int   value;
    Node* prev;
    Node* next;
    char  padding[8];  // sizeof(Node) > sizeof(void*)
};

int main() {
    constexpr int N = 10000;
    constexpr int POOL_SIZE = N + 64;

    // --- 1) malloc 방식 ---
    long long malloc_ns = measure_ns([&]{
        Node* ptr = static_cast<Node*>(std::malloc(sizeof(Node)));
        ptr->value = 42;
        std::free(ptr);
    }, N);

    // --- 2) Pool Allocator 방식 ---
    FixedPoolAllocator<Node, POOL_SIZE> pool;
    long long pool_ns = measure_ns([&]{
        Node* ptr = pool.allocate();
        ptr->value = 42;
        pool.deallocate(ptr);
    }, N);

    std::cout << "=== 단일 할당/해제 성능 비교 (" << N << "회 평균) ===\n";
    std::cout << "malloc/free   : " << malloc_ns << " ns/op\n";
    std::cout << "PoolAllocator : " << pool_ns   << " ns/op\n";
    std::cout << "속도 향상      : " << static_cast<double>(malloc_ns) / pool_ns << "x\n";

    // --- 3) 사용 예 ---
    std::cout << "\n=== FixedPoolAllocator 사용 예 ===\n";
    FixedPoolAllocator<Node, 16> small_pool;
    std::cout << "초기 가용 슬롯: " << small_pool.available() << '\n';

    Node* a = small_pool.allocate();
    Node* b = small_pool.allocate();
    new (a) Node{1, nullptr, b, {}};
    new (b) Node{2, a, nullptr, {}};
    std::cout << "2개 할당 후 가용 슬롯: " << small_pool.available() << '\n';

    a->~Node();
    small_pool.deallocate(a);
    std::cout << "1개 반환 후 가용 슬롯: " << small_pool.available() << '\n';

    b->~Node();
    small_pool.deallocate(b);

    return 0;
}

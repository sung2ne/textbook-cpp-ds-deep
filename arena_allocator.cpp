// filename: arena_allocator.cpp
// g++ -std=c++17 -O2 -Wall -o arena_allocator arena_allocator.cpp

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <vector>
#include <new>

// ------------------------------------------------------------
// BumpAllocator: 단일 블록, 앞으로만 할당
// ------------------------------------------------------------
class BumpAllocator {
public:
    explicit BumpAllocator(std::size_t capacity)
        : capacity_(capacity)
    {
        base_ = static_cast<char*>(std::malloc(capacity));
        if (!base_) throw std::bad_alloc{};
        bump_ = base_;
    }

    ~BumpAllocator() { std::free(base_); }

    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator& operator=(const BumpAllocator&) = delete;

    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t align = alignof(std::max_align_t)) {
        // 정렬: bump를 align 배수로 올림
        std::uintptr_t cur = reinterpret_cast<std::uintptr_t>(bump_);
        std::uintptr_t aligned = (cur + align - 1) & ~(align - 1);
        char* new_bump = reinterpret_cast<char*>(aligned) + bytes;

        if (new_bump > base_ + capacity_)
            throw std::bad_alloc{};  // 블록 소진

        bump_ = new_bump;
        return reinterpret_cast<void*>(aligned);
    }

    // 개별 해제 없음 — 아무것도 하지 않음
    void deallocate(void*, std::size_t) noexcept {}

    // 전체 리셋
    void reset() noexcept { bump_ = base_; }

    std::size_t used() const noexcept {
        return static_cast<std::size_t>(bump_ - base_);
    }
    std::size_t available() const noexcept { return capacity_ - used(); }

private:
    char*       base_     = nullptr;
    char*       bump_     = nullptr;
    std::size_t capacity_ = 0;
};

// ------------------------------------------------------------
// ArenaAllocator: 블록이 소진되면 새 블록 추가 (체인 구조)
// ------------------------------------------------------------
class ArenaAllocator {
public:
    explicit ArenaAllocator(std::size_t block_size = 64 * 1024)
        : block_size_(block_size)
    {
        add_block();
    }

    ~ArenaAllocator() {
        for (auto& b : blocks_) std::free(b.data);
    }

    [[nodiscard]] void* allocate(std::size_t bytes,
                                 std::size_t align = alignof(std::max_align_t)) {
        Block& cur = blocks_.back();
        std::uintptr_t pos = reinterpret_cast<std::uintptr_t>(cur.data) + cur.used;
        std::uintptr_t aligned = (pos + align - 1) & ~(align - 1);
        std::size_t   offset  = static_cast<std::size_t>(
                                    aligned - reinterpret_cast<std::uintptr_t>(cur.data));

        if (offset + bytes > cur.capacity) {
            // 현재 블록 소진 → 새 블록
            std::size_t new_cap = std::max(block_size_, bytes + align);
            add_block(new_cap);
            return allocate(bytes, align);  // 새 블록에서 재시도
        }

        cur.used = offset + bytes;
        return reinterpret_cast<void*>(aligned);
    }

    void deallocate(void*, std::size_t) noexcept {}

    void reset() noexcept {
        // 첫 블록만 남기고 나머지 해제
        while (blocks_.size() > 1) {
            std::free(blocks_.back().data);
            blocks_.pop_back();
        }
        blocks_.front().used = 0;
    }

    std::size_t block_count() const noexcept { return blocks_.size(); }

private:
    struct Block {
        char*       data;
        std::size_t capacity;
        std::size_t used;
    };

    void add_block(std::size_t cap = 0) {
        std::size_t actual = cap ? cap : block_size_;
        char* data = static_cast<char*>(std::malloc(actual));
        if (!data) throw std::bad_alloc{};
        blocks_.push_back({data, actual, 0});
    }

    std::vector<Block> blocks_;
    std::size_t        block_size_;
};

// ------------------------------------------------------------
// 벤치마크
// ------------------------------------------------------------
struct RequestData {
    int    id;
    double value;
    char   tag[32];
};

long long bench_malloc(int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto* p = static_cast<RequestData*>(std::malloc(sizeof(RequestData)));
        p->id = i;
        std::free(p);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

long long bench_bump(int iterations) {
    BumpAllocator arena(sizeof(RequestData) * iterations + 64);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto* p = static_cast<RequestData*>(
            arena.allocate(sizeof(RequestData), alignof(RequestData)));
        p->id = i;
        // deallocate 없음
    }
    arena.reset();  // 일괄 해제
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int main() {
    constexpr int N = 100000;

    long long t_malloc = bench_malloc(N);
    long long t_bump   = bench_bump(N);

    std::cout << "=== malloc vs BumpAllocator (" << N << "회) ===\n";
    std::cout << "malloc      : " << t_malloc / N << " ns/op  (total: " << t_malloc / 1000 << " us)\n";
    std::cout << "BumpAlloc   : " << t_bump   / N << " ns/op  (total: " << t_bump   / 1000 << " us)\n";
    std::cout << "속도 향상    : " << static_cast<double>(t_malloc) / t_bump << "x\n\n";

    // ArenaAllocator 사용 예
    std::cout << "=== ArenaAllocator 체인 블록 예 ===\n";
    ArenaAllocator arena(256);  // 256바이트 블록으로 시작 (작은 크기 의도)

    for (int i = 0; i < 20; ++i) {
        auto* p = static_cast<RequestData*>(
            arena.allocate(sizeof(RequestData), alignof(RequestData)));
        p->id = i;
    }
    std::cout << "블록 수: " << arena.block_count() << '\n';

    arena.reset();
    std::cout << "reset 후 블록 수: " << arena.block_count() << '\n';

    return 0;
}

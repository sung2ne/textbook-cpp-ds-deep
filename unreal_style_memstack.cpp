// filename: unreal_style_memstack.cpp
// g++ -std=c++17 -O2 -Wall -o unreal_style_memstack unreal_style_memstack.cpp

#include <iostream>
#include <vector>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <new>

// ============================================================
// FMemStack 에뮬레이션: Bump Allocator + Mark/Pop
// ============================================================
class FMemStack {
public:
    struct FMemMark {
        FMemStack* stack;
        char*      saved_bump;
        FMemMark(FMemStack* s) : stack(s), saved_bump(s->bump_) {}
        ~FMemMark() { stack->bump_ = saved_bump; }  // 소멸 시 자동 롤백
    };

    explicit FMemStack(std::size_t capacity = 4 * 1024 * 1024)
        : capacity_(capacity)
    {
        base_ = static_cast<char*>(std::malloc(capacity));
        if (!base_) throw std::bad_alloc{};
        bump_ = base_;
    }

    ~FMemStack() { std::free(base_); }

    // Unreal의 PushBytes와 동일
    void* PushBytes(std::size_t bytes, std::size_t align = alignof(std::max_align_t)) {
        std::uintptr_t cur = reinterpret_cast<std::uintptr_t>(bump_);
        std::uintptr_t aligned = (cur + align - 1) & ~(align - 1);
        char* new_bump = reinterpret_cast<char*>(aligned) + bytes;
        if (new_bump > base_ + capacity_) throw std::bad_alloc{};
        bump_ = new_bump;
        return reinterpret_cast<void*>(aligned);
    }

    FMemMark Mark() { return FMemMark{this}; }

    std::size_t used() const noexcept {
        return static_cast<std::size_t>(bump_ - base_);
    }

    // 스레드로컬 글로벌 인스턴스 (Unreal의 FMemStack::Get() 에뮬레이션)
    static FMemStack& Get() {
        thread_local FMemStack instance{4 * 1024 * 1024};
        return instance;
    }

private:
    char*       base_     = nullptr;
    char*       bump_     = nullptr;
    std::size_t capacity_ = 0;

    friend class FMemMark;
};

// Unreal의 new(FMemStack::Get()) T[] 에뮬레이션
template <typename T>
T* NewOnStack(FMemStack& stack, std::size_t count = 1) {
    void* mem = stack.PushBytes(sizeof(T) * count, alignof(T));
    return static_cast<T*>(mem);  // 초기화는 placement new로
}

// ============================================================
// TInlineAllocator 에뮬레이션
// ============================================================
template <typename T, std::size_t InlineCapacity>
class TInlineArray {
public:
    TInlineArray() : size_(0), using_heap_(false) {}
    ~TInlineArray() { if (using_heap_) delete[] heap_data_; }

    void push_back(const T& val) {
        if (size_ < InlineCapacity) {
            new (&inline_data_[size_]) T(val);
        } else {
            if (!using_heap_) {
                // 스택 → 힙 이전
                heap_data_ = new T[InlineCapacity * 2];
                for (std::size_t i = 0; i < size_; ++i)
                    heap_data_[i] = *std::launder(reinterpret_cast<T*>(&inline_data_[i]));
                heap_capacity_ = InlineCapacity * 2;
                using_heap_ = true;
            } else if (size_ >= heap_capacity_) {
                // 힙 확장
                T* new_heap = new T[heap_capacity_ * 2];
                for (std::size_t i = 0; i < size_; ++i) new_heap[i] = heap_data_[i];
                delete[] heap_data_;
                heap_data_ = new_heap;
                heap_capacity_ *= 2;
            }
            heap_data_[size_] = val;
        }
        ++size_;
    }

    T& operator[](std::size_t i) {
        if (using_heap_) return heap_data_[i];
        return *std::launder(reinterpret_cast<T*>(&inline_data_[i]));
    }

    std::size_t size() const noexcept { return size_; }
    bool is_inline() const noexcept { return !using_heap_; }

private:
    alignas(T) char inline_data_[sizeof(T) * InlineCapacity];
    T*          heap_data_     = nullptr;
    std::size_t heap_capacity_ = 0;
    std::size_t size_;
    bool        using_heap_;
};

// ============================================================
// 사용 예
// ============================================================
struct FRenderCommand {
    int   mesh_id;
    float transform[16];
    int   material_id;
};

int main() {
    std::cout << "=== FMemStack 시뮬레이션 ===\n";
    auto& stack = FMemStack::Get();
    std::cout << "초기 사용량: " << stack.used() << " bytes\n";

    // 프레임 시작
    {
        auto mark = stack.Mark();  // 현재 위치 저장

        // 렌더링 임시 데이터 할당
        FRenderCommand* cmds = NewOnStack<FRenderCommand>(stack, 1000);
        for (int i = 0; i < 1000; ++i) {
            new (&cmds[i]) FRenderCommand{i, {}, i % 16};
        }

        std::cout << "1000개 커맨드 후 사용량: " << stack.used() << " bytes\n";

        // 더 많은 임시 데이터
        float* temp_buf = static_cast<float*>(stack.PushBytes(4096, alignof(float)));
        (void)temp_buf;

        std::cout << "추가 버퍼 후 사용량: " << stack.used() << " bytes\n";

        // mark 소멸자: 이 블록 시작 지점으로 롤백
    }
    std::cout << "Mark 소멸 후 사용량: " << stack.used() << " bytes (롤백됨)\n";

    // --- TInlineArray 시뮬레이션 ---
    std::cout << "\n=== TInlineArray<int, 8> ===\n";
    TInlineArray<int, 8> arr;

    std::cout << "8개까지 인라인:\n";
    for (int i = 0; i < 8; ++i) {
        arr.push_back(i * 10);
        std::cout << "  size=" << arr.size() << " inline=" << arr.is_inline() << '\n';
    }

    std::cout << "9번째 (힙으로 이동):\n";
    arr.push_back(80);
    std::cout << "  size=" << arr.size() << " inline=" << arr.is_inline() << '\n';

    return 0;
}

// filename: pmr_basic.cpp
// g++ -std=c++17 -O2 -Wall -o pmr_basic pmr_basic.cpp

#include <iostream>
#include <memory_resource>   // C++17 PMR 헤더
#include <vector>
#include <map>
#include <string>

// memory_resource를 직접 구현하는 커스텀 예제
class LoggingResource : public std::pmr::memory_resource {
public:
    explicit LoggingResource(std::pmr::memory_resource* upstream =
                             std::pmr::get_default_resource())
        : upstream_(upstream) {}

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        void* ptr = upstream_->allocate(bytes, alignment);
        std::cout << "[LOG ALLOC]   " << bytes << " bytes @ " << ptr << '\n';
        return ptr;
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override {
        std::cout << "[LOG DEALLOC] " << bytes << " bytes @ " << ptr << '\n';
        upstream_->deallocate(ptr, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

private:
    std::pmr::memory_resource* upstream_;
};

void process(std::pmr::vector<int>& v) {
    // 함수 시그니처에 allocator 타입이 노출되지 않음
    for (int x : v) std::cout << x << ' ';
    std::cout << '\n';
}

int main() {
    // --- 1) monotonic_buffer_resource (스택 버퍼 기반) ---
    std::cout << "=== monotonic_buffer_resource ===\n";
    {
        char buffer[4096];
        std::pmr::monotonic_buffer_resource mono{buffer, sizeof(buffer)};
        std::pmr::vector<int> v{&mono};

        for (int i = 0; i < 10; ++i)
            v.push_back(i);

        process(v);
        // mono 소멸 시 buffer 자체는 스택에 있으므로 별도 해제 없음
    }

    // --- 2) unsynchronized_pool_resource ---
    std::cout << "\n=== unsynchronized_pool_resource ===\n";
    {
        std::pmr::unsynchronized_pool_resource pool;
        std::pmr::vector<std::pmr::string> sv{&pool};
        sv.emplace_back("alpha");
        sv.emplace_back("beta");
        sv.emplace_back("gamma");
        for (auto& s : sv) std::cout << s << ' ';
        std::cout << '\n';
    }

    // --- 3) LoggingResource (커스텀 구현) ---
    std::cout << "\n=== LoggingResource ===\n";
    {
        LoggingResource lr;
        std::pmr::vector<int> v{&lr};
        v.reserve(4);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
    }

    // --- 4) null_memory_resource (할당 시 예외) ---
    std::cout << "\n=== null_memory_resource ===\n";
    {
        // 스택 버퍼로 충분하다고 가정 — 버퍼 초과 시 예외 발생
        char buffer[256];
        std::pmr::monotonic_buffer_resource mono{
            buffer, sizeof(buffer),
            std::pmr::null_memory_resource()  // upstream: null → 초과하면 throw
        };
        std::pmr::vector<int> v{&mono};
        try {
            for (int i = 0; i < 1000; ++i)
                v.push_back(i);   // 256바이트 초과 시 bad_alloc
        } catch (const std::bad_alloc& e) {
            std::cout << "bad_alloc 발생 (예상된 동작): " << e.what() << '\n';
        }
    }

    return 0;
}

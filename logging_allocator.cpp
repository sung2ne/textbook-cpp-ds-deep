// filename: logging_allocator.cpp
// g++ -std=c++17 -O2 -Wall -o logging_allocator logging_allocator.cpp

#include <iostream>
#include <memory>
#include <vector>
#include <string>

template <typename T>
struct LoggingAllocator {
    using value_type = T;

    LoggingAllocator() noexcept = default;

    // 다른 타입의 LoggingAllocator로부터 변환 생성자
    template <typename U>
    LoggingAllocator(const LoggingAllocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        std::size_t bytes = n * sizeof(T);
        T* ptr = static_cast<T*>(::operator new(bytes));
        std::cout << "[ALLOC]   " << n << " x " << sizeof(T)
                  << " bytes = " << bytes << " bytes @ " << ptr << '\n';
        return ptr;
    }

    void deallocate(T* ptr, std::size_t n) noexcept {
        std::size_t bytes = n * sizeof(T);
        std::cout << "[DEALLOC] " << n << " x " << sizeof(T)
                  << " bytes = " << bytes << " bytes @ " << ptr << '\n';
        ::operator delete(ptr);
    }
};

// 두 LoggingAllocator는 항상 서로의 메모리를 해제할 수 있음
template <typename T, typename U>
bool operator==(const LoggingAllocator<T>&, const LoggingAllocator<U>&) noexcept {
    return true;
}

template <typename T, typename U>
bool operator!=(const LoggingAllocator<T>&, const LoggingAllocator<U>&) noexcept {
    return false;
}

int main() {
    std::cout << "=== vector<int> with LoggingAllocator ===\n";
    {
        std::vector<int, LoggingAllocator<int>> v;
        v.reserve(4);        // 처음부터 4개 공간 확보

        std::cout << "--- push_back x 4 ---\n";
        for (int i = 0; i < 4; ++i)
            v.push_back(i * 10);

        std::cout << "--- push_back x 1 (재할당 발생) ---\n";
        v.push_back(40);     // capacity 초과 → 재할당
    }                        // 소멸자: deallocate 호출

    std::cout << "\n=== vector<std::string> with LoggingAllocator ===\n";
    {
        // LoggingAllocator<std::string>: std::string 한 개 크기(보통 24~32 bytes)씩 할당
        std::vector<std::string, LoggingAllocator<std::string>> sv;
        sv.emplace_back("hello");
        sv.emplace_back("world");
    }

    return 0;
}

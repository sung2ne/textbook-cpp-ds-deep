// filename: operator_new_demo.cpp
// g++ -std=c++17 -O2 -o operator_new_demo operator_new_demo.cpp

#include <iostream>
#include <cstdlib>
#include <new>

// 글로벌 operator new 오버로딩
void* operator new(std::size_t size) {
    std::cout << "[전역] operator new(" << size << " bytes) 호출\n";
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc{};
    return ptr;
}

void operator delete(void* ptr) noexcept {
    std::cout << "[전역] operator delete 호출\n";
    std::free(ptr);
}

struct PoolObject {
    int value;
    static char pool[1024];
    static std::size_t offset;

    // 클래스 수준 operator new — 전역보다 우선
    static void* operator new(std::size_t size) {
        std::cout << "[Pool] operator new(" << size << " bytes)\n";
        if (offset + size > sizeof(pool))
            throw std::bad_alloc{};
        void* ptr = pool + offset;
        offset += size;
        return ptr;
    }

    static void operator delete(void* ptr) noexcept {
        // 풀 방식에서는 개별 해제를 지원하지 않을 수 있음
        std::cout << "[Pool] operator delete (no-op)\n";
    }
};

char PoolObject::pool[1024] = {};
std::size_t PoolObject::offset = 0;

int main() {
    std::cout << "=== 일반 int 할당 ===\n";
    int* p = new int(42);
    delete p;

    std::cout << "\n=== PoolObject 할당 ===\n";
    PoolObject* obj1 = new PoolObject{100};
    PoolObject* obj2 = new PoolObject{200};
    std::cout << "obj1->value = " << obj1->value << "\n";
    std::cout << "obj2->value = " << obj2->value << "\n";
    delete obj1;
    delete obj2;

    return 0;
}

// filename: my_unique_ptr.cpp
// g++ -std=c++17 -O2 -o my_unique_ptr my_unique_ptr.cpp

#include <iostream>
#include <utility>   // std::exchange, std::forward
#include <cstdio>    // FILE

// 기본 Deleter
template<typename T>
struct DefaultDelete {
    void operator()(T* p) const noexcept {
        delete p;
    }
};

template<typename T>
struct DefaultDelete<T[]> {
    void operator()(T* p) const noexcept {
        delete[] p;
    }
};

// 간략화된 unique_ptr 구현
template<typename T, typename Deleter = DefaultDelete<T>>
class MyUniquePtr {
public:
    using pointer     = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // 기본 생성자
    constexpr MyUniquePtr() noexcept : ptr_(nullptr) {}
    constexpr MyUniquePtr(std::nullptr_t) noexcept : ptr_(nullptr) {}

    // 포인터로 생성
    explicit MyUniquePtr(pointer p) noexcept : ptr_(p) {}

    // 소멸자 — RAII 핵심
    ~MyUniquePtr() {
        if (ptr_) {
            Deleter{}(ptr_);  // 커스텀 Deleter 호출
        }
    }

    // 복사 금지 — unique 소유권 보장
    MyUniquePtr(const MyUniquePtr&)            = delete;
    MyUniquePtr& operator=(const MyUniquePtr&) = delete;

    // 이동 생성자 — 소유권 이전
    MyUniquePtr(MyUniquePtr&& other) noexcept
        : ptr_(std::exchange(other.ptr_, nullptr)) {}

    // 이동 대입 — 기존 리소스 해제 후 소유권 이전
    MyUniquePtr& operator=(MyUniquePtr&& other) noexcept {
        if (this != &other) {
            reset();  // 기존 포인터 해제
            ptr_ = std::exchange(other.ptr_, nullptr);
        }
        return *this;
    }

    // nullptr 대입
    MyUniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // 역참조 연산자
    T& operator*()  const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }

    // 명시적 bool 변환
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // 원시 포인터 접근
    pointer get() const noexcept { return ptr_; }

    // 소유권 포기 (해제 없이 포인터 반환)
    pointer release() noexcept {
        return std::exchange(ptr_, nullptr);
    }

    // 새 포인터로 교체 (기존 해제)
    void reset(pointer p = nullptr) noexcept {
        if (ptr_) Deleter{}(ptr_);
        ptr_ = p;
    }

    // swap
    void swap(MyUniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

private:
    pointer ptr_;
};

// make_unique 구현 — new 직접 사용보다 예외 안전
template<typename T, typename... Args>
MyUniquePtr<T> make_my_unique(Args&&... args) {
    return MyUniquePtr<T>(new T(std::forward<Args>(args)...));
}

// 테스트용 클래스
struct Widget {
    int id;
    Widget(int i) : id(i) { std::cout << "Widget(" << id << ") 생성\n"; }
    ~Widget()              { std::cout << "Widget(" << id << ") 소멸\n"; }
    void draw() const      { std::cout << "Widget(" << id << ") draw\n"; }
};

// FILE* 관리용 커스텀 Deleter
struct FileDeleter {
    void operator()(FILE* f) const noexcept {
        if (f) {
            std::fclose(f);
            std::cout << "[FileDeleter] fclose 호출\n";
        }
    }
};

int main() {
    std::cout << "=== 기본 사용 ===\n";
    {
        auto w = make_my_unique<Widget>(1);
        w->draw();

        // 복사는 불가
        // auto w2 = w;  // ❌ 컴파일 에러

        // 이동은 가능
        auto w2 = std::move(w);
        std::cout << "이동 후 w는 null: " << (w ? "false" : "true") << "\n";
        w2->draw();
    }  // 스코프 종료 → Widget(1) 소멸

    std::cout << "\n=== 커스텀 Deleter ===\n";
    {
        MyUniquePtr<FILE, FileDeleter> file(std::fopen("/tmp/test.txt", "w"));
        if (file) {
            std::fputs("unique_ptr 테스트\n", file.get());
        }
    }  // FileDeleter 호출

    std::cout << "\n=== release()와 reset() ===\n";
    {
        auto w = make_my_unique<Widget>(2);
        Widget* raw = w.release();  // 소유권 포기
        std::cout << "release 후 w는 null: " << (!w ? "true" : "false") << "\n";
        delete raw;  // 이제 호출자가 책임

        auto w2 = make_my_unique<Widget>(3);
        w2.reset(new Widget(4));  // Widget(3) 소멸, Widget(4) 소유
    }

    return 0;
}

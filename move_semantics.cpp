// filename: move_semantics.cpp
// g++ -std=c++17 -O2 -o move_semantics move_semantics.cpp

#include <iostream>
#include <chrono>
#include <cstring>
#include <utility>
#include <cassert>

class MyString {
public:
    // 기본 생성자
    MyString() : data_(nullptr), size_(0) {}

    // 문자열 리터럴로 생성
    explicit MyString(const char* str) {
        size_ = std::strlen(str);
        data_ = new char[size_ + 1];
        std::memcpy(data_, str, size_ + 1);
    }

    // 소멸자
    ~MyString() {
        delete[] data_;
    }

    // 복사 생성자 — 깊은 복사 O(n)
    MyString(const MyString& other) {
        size_ = other.size_;
        data_ = new char[size_ + 1];
        std::memcpy(data_, other.data_, size_ + 1);
        // std::cout << "[복사 생성] " << size_ << " 바이트\n";
    }

    // 복사 대입 — 깊은 복사 O(n)
    MyString& operator=(const MyString& other) {
        if (this == &other) return *this;
        delete[] data_;
        size_ = other.size_;
        data_ = new char[size_ + 1];
        std::memcpy(data_, other.data_, size_ + 1);
        return *this;
    }

    // 이동 생성자 — 포인터 이전 O(1)
    MyString(MyString&& other) noexcept
        : data_(other.data_), size_(other.size_)
    {
        other.data_ = nullptr;  // 원본을 안전한 빈 상태로
        other.size_ = 0;
        // std::cout << "[이동 생성] 포인터 이전\n";
    }

    // 이동 대입 — 포인터 이전 O(1)
    MyString& operator=(MyString&& other) noexcept {
        if (this == &other) return *this;
        delete[] data_;           // 기존 리소스 해제
        data_ = other.data_;      // 포인터 탈취
        size_ = other.size_;
        other.data_ = nullptr;    // 원본 무효화
        other.size_ = 0;
        return *this;
    }

    // 접근자
    std::size_t size() const noexcept { return size_; }
    const char* c_str() const noexcept { return data_ ? data_ : ""; }
    bool empty() const noexcept { return size_ == 0; }

private:
    char*       data_;
    std::size_t size_;
};

// 이동 의미론의 혜택: 값으로 반환해도 O(1)
MyString buildLargeString(std::size_t n) {
    MyString result;
    // 실제로는 뭔가를 구성...
    result = MyString(std::string(n, 'A').c_str());
    return result;  // NRVO 또는 이동 적용
}

int main() {
    const int ITERATIONS = 100'000;
    const int STR_SIZE = 1000;

    std::string prototype(STR_SIZE, 'X');
    MyString original(prototype.c_str());

    // 복사 성능 측정
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        MyString copy = original;  // 복사 생성자
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    // 이동 성능 측정
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        MyString temp = original;              // 복사 (원본 보존)
        MyString moved = std::move(temp);      // 이동 생성자
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    using us = std::chrono::microseconds;
    auto copy_us = std::chrono::duration_cast<us>(t2 - t1).count();
    auto move_us = std::chrono::duration_cast<us>(t4 - t3).count();

    std::cout << "복사 " << ITERATIONS << "회: " << copy_us << " μs\n";
    std::cout << "이동 " << ITERATIONS << "회: " << move_us << " μs\n";
    std::cout << "이동이 복사 대비: ~"
              << (copy_us / std::max(1LL, move_us - copy_us)) << "배 빠름\n";

    // 이동 후 상태 확인
    MyString s1("Hello");
    MyString s2 = std::move(s1);
    std::cout << "\n이동 후 s1 empty: " << s1.empty() << "\n";  // true
    std::cout << "s2: " << s2.c_str() << "\n";

    return 0;
}

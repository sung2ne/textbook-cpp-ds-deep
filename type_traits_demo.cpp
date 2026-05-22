// filename: type_traits_demo.cpp
// g++ -std=c++17 -O2 -o type_traits_demo type_traits_demo.cpp

#include <iostream>
#include <type_traits>
#include <string>
#include <vector>

// 도우미 매크로
#define SHOW(trait, type) \
    std::cout << #trait "<" #type ">: " << trait<type>::value << "\n"

struct Trivial {
    int x;
    // 컴파일러 생성 특수 함수만 사용
};

struct NonTrivial {
    std::string name;  // std::string이 비trivial
    NonTrivial() {}
    ~NonTrivial() {}
};

struct MoveOnly {
    MoveOnly() = default;
    MoveOnly(MoveOnly&&) = default;
    MoveOnly(const MoveOnly&) = delete;
};

struct Abstract {
    virtual void doSomething() = 0;
    virtual ~Abstract() = default;
};

int main() {
    std::cout << "=== 기본 타입 분류 ===\n";
    SHOW(std::is_integral,       int);        // 1
    SHOW(std::is_integral,       float);      // 0
    SHOW(std::is_floating_point, double);     // 1
    SHOW(std::is_floating_point, int);        // 0
    SHOW(std::is_arithmetic,     int);        // 1 (integral + floating_point)
    SHOW(std::is_arithmetic,     std::string);// 0
    SHOW(std::is_pointer,        int*);       // 1
    SHOW(std::is_pointer,        int);        // 0
    SHOW(std::is_reference,      int&);       // 1
    SHOW(std::is_reference,      int&&);      // 1
    SHOW(std::is_lvalue_reference, int&);    // 1
    SHOW(std::is_rvalue_reference, int&&);   // 1
    SHOW(std::is_void,           void);       // 1
    SHOW(std::is_array,          int[10]);    // 1

    std::cout << "\n=== 클래스/구조체 특성 ===\n";
    SHOW(std::is_class,          std::string);  // 1
    SHOW(std::is_class,          int);          // 0
    SHOW(std::is_abstract,       Abstract);     // 1
    SHOW(std::is_abstract,       Trivial);      // 0
    SHOW(std::is_trivial,        Trivial);      // 1
    SHOW(std::is_trivial,        NonTrivial);   // 0

    std::cout << "\n=== 생성/소멸 가능 여부 ===\n";
    SHOW(std::is_constructible<int COMMA int>,       int);        // 실제로는 다르게 사용
    SHOW(std::is_default_constructible,   int);           // 1
    SHOW(std::is_default_constructible,   MoveOnly);      // 1
    SHOW(std::is_copy_constructible,      std::string);   // 1
    SHOW(std::is_copy_constructible,      MoveOnly);      // 0
    SHOW(std::is_move_constructible,      MoveOnly);      // 1
    SHOW(std::is_move_constructible,      int);           // 1
    SHOW(std::is_destructible,            std::string);   // 1
    SHOW(std::is_trivially_copyable,      Trivial);       // 1
    SHOW(std::is_trivially_copyable,      NonTrivial);    // 0

    std::cout << "\n=== noexcept 여부 ===\n";
    SHOW(std::is_nothrow_move_constructible, int);       // 1
    SHOW(std::is_nothrow_move_constructible, std::vector<int>); // 1 (보통)
    SHOW(std::is_nothrow_destructible,       int);       // 1

    return 0;
}

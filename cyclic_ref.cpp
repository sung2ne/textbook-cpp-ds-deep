// filename: cyclic_ref.cpp
// g++ -std=c++17 -O2 -o cyclic_ref cyclic_ref.cpp

#include <iostream>
#include <memory>

struct Child;

struct Parent {
    std::shared_ptr<Child> child;
    Parent()  { std::cout << "Parent 생성\n"; }
    ~Parent() { std::cout << "Parent 소멸\n"; }
};

struct Child {
    std::shared_ptr<Parent> parent;  // ← 여기가 문제
    Child()  { std::cout << "Child 생성\n"; }
    ~Child() { std::cout << "Child 소멸\n"; }
};

int main() {
    std::cout << "=== 순환 참조 (누수 발생) ===\n";
    {
        auto parent = std::make_shared<Parent>();  // Parent use_count: 1
        auto child  = std::make_shared<Child>();   // Child  use_count: 1

        parent->child  = child;   // Child  use_count: 2
        child->parent  = parent;  // Parent use_count: 2
    }
    // parent 소멸 → Parent use_count: 1 (child->parent가 유지)
    // child  소멸 → Child  use_count: 1 (parent->child가 유지)
    // 둘 다 use_count > 0 → 소멸자 호출 안 됨 → 메모리 누수!
    std::cout << "=== 스코프 종료 (소멸자가 호출되지 않음) ===\n\n";

    return 0;
}

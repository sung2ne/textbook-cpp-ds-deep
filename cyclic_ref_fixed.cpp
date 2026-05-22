// filename: cyclic_ref_fixed.cpp
// g++ -std=c++17 -O2 -o cyclic_ref_fixed cyclic_ref_fixed.cpp

#include <iostream>
#include <memory>

struct Child;

struct Parent {
    std::shared_ptr<Child> child;  // 부모가 자식을 소유
    Parent()  { std::cout << "Parent 생성\n"; }
    ~Parent() { std::cout << "Parent 소멸\n"; }

    void printChild();
};

struct Child {
    std::weak_ptr<Parent> parent;  // ← weak_ptr로 변경! 소유하지 않음
    int id;

    Child(int i) : id(i) { std::cout << "Child(" << id << ") 생성\n"; }
    ~Child()              { std::cout << "Child(" << id << ") 소멸\n"; }

    void notifyParent() {
        // lock()으로 임시 shared_ptr 획득
        if (auto p = parent.lock()) {
            std::cout << "Child(" << id << "): 부모에게 알림\n";
        } else {
            std::cout << "Child(" << id << "): 부모가 이미 없음\n";
        }
    }
};

void Parent::printChild() {
    if (child) {
        std::cout << "Parent: 자식 id = " << child->id << "\n";
    }
}

int main() {
    std::cout << "=== weak_ptr 해결책 ===\n";
    {
        auto parent = std::make_shared<Parent>();  // Parent use_count: 1
        auto child  = std::make_shared<Child>(42); // Child  use_count: 1

        parent->child = child;   // Child  use_count: 2
        child->parent = parent;  // Parent use_count: 1 (weak_ptr은 카운트 안 올림)

        parent->printChild();
        child->notifyParent();
    }
    // parent 소멸 → Parent use_count: 1→0 → Parent 소멸자 호출
    //   → parent->child(shared_ptr) 소멸 → Child use_count: 2→1
    // child 소멸 → Child  use_count: 1→0 → Child  소멸자 호출
    std::cout << "=== 스코프 종료 (정상 소멸) ===\n";

    return 0;
}

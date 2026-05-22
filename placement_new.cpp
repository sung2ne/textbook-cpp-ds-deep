// filename: placement_new.cpp
// g++ -std=c++17 -O2 -o placement_new placement_new.cpp

#include <iostream>
#include <new>       // std::launder
#include <cstring>   // memset
#include <memory>    // std::destroy_at (C++17)

struct Widget {
    int id;
    std::string name;

    Widget(int i, const std::string& n) : id(i), name(n) {
        std::cout << "Widget(" << id << ", " << name << ") 생성\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id << ", " << name << ") 소멸\n";
    }
};

int main() {
    // 1) 원시 메모리 확보 (alignas로 정렬 보장)
    alignas(Widget) unsigned char buf[sizeof(Widget)];

    // 2) placement new — buf 위에 Widget 객체 구성
    Widget* w = new (buf) Widget(1, "Button");

    std::cout << "w->id = " << w->id << ", w->name = " << w->name << "\n";

    // 3) 소멸자를 명시적으로 호출 (delete는 사용 불가 — buf는 스택)
    w->~Widget();
    // C++17: std::destroy_at(w);  // 위와 동일

    // 4) buf는 스택이므로 별도 해제 불필요

    std::cout << "\n--- 힙 메모리에 placement new ---\n";
    void* raw = ::operator new(sizeof(Widget));  // 메모리만 확보
    Widget* w2 = new (raw) Widget(2, "Label");
    std::cout << "w2->id = " << w2->id << "\n";
    w2->~Widget();
    ::operator delete(raw);  // 메모리 해제

    return 0;
}

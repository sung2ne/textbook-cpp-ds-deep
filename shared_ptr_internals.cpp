// filename: shared_ptr_internals.cpp
// g++ -std=c++17 -O2 -o shared_ptr_internals shared_ptr_internals.cpp

#include <iostream>
#include <memory>
#include <cstdint>

struct Widget {
    int id;
    Widget(int i) : id(i) {
        std::cout << "Widget(" << id << ") 생성 @ " << this << "\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id << ") 소멸\n";
    }
};

int main() {
    std::cout << "=== make_shared: 객체와 control block이 인접 ===\n";
    {
        auto sp = std::make_shared<Widget>(1);
        std::cout << "shared_ptr 크기: " << sizeof(sp) << " bytes\n";
        std::cout << "use_count: " << sp.use_count() << "\n";

        auto sp2 = sp;  // 복사 → use_count 증가
        std::cout << "복사 후 use_count: " << sp.use_count() << "\n";

        // sp, sp2 둘 다 같은 control block의 use_count를 참조
        auto sp3 = std::move(sp2);  // 이동 → use_count 변화 없음
        std::cout << "이동 후 use_count: " << sp.use_count() << "\n";
        std::cout << "sp2 사용 가능: " << (sp2 ? "true" : "false") << "\n";
    }
    std::cout << "(스코프 종료 → Widget 소멸)\n\n";

    std::cout << "=== new + shared_ptr: 두 번 할당 ===\n";
    {
        Widget* raw = new Widget(2);
        std::cout << "Widget 주소: " << raw << "\n";
        std::shared_ptr<Widget> sp(raw);
        // control block은 raw와 다른 주소에 할당됨
        std::cout << "use_count: " << sp.use_count() << "\n";
    }

    std::cout << "\n=== use_count 변화 추적 ===\n";
    std::shared_ptr<Widget> outer;
    {
        auto sp = std::make_shared<Widget>(3);
        std::cout << "초기 use_count: " << sp.use_count() << "\n";

        outer = sp;  // outer가 소유권 공유
        std::cout << "outer 추가 후: " << sp.use_count() << "\n";

        {
            auto inner = sp;
            std::cout << "inner 추가 후: " << sp.use_count() << "\n";
        }  // inner 소멸 → use_count 감소
        std::cout << "inner 소멸 후: " << sp.use_count() << "\n";
    }  // sp 소멸 → use_count 감소, outer만 남음
    std::cout << "내부 스코프 종료 후: " << outer.use_count() << "\n";
    // outer가 마지막 소유자 → 여기서 Widget 소멸 (outer가 소멸될 때)

    return 0;
}

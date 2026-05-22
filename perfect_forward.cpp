// filename: perfect_forward.cpp
// g++ -std=c++17 -O2 -o perfect_forward perfect_forward.cpp

#include <iostream>
#include <string>
#include <utility>  // std::forward, std::move

struct Widget {
    Widget(int id)                 { std::cout << "Widget(" << id << ") 생성\n"; }
    Widget(const Widget&)          { std::cout << "복사 생성\n"; }
    Widget(Widget&&) noexcept      { std::cout << "이동 생성\n"; }
};

void process(Widget& w)  { std::cout << "process(lvalue ref)\n"; }
void process(Widget&& w) { std::cout << "process(rvalue ref)\n"; }

// ❌ 단순 래퍼 — rvalue가 전달되어도 w는 이름이 있어 lvalue
void bad_wrapper(Widget&& w) {
    process(w);  // 항상 lvalue 버전 호출 — 이동 기회 상실
}

// ❌ 오버로드로 해결 시도 — 조합 폭발
void wrapper_copy(Widget& w)  { process(w); }
void wrapper_move(Widget&& w) { process(std::move(w)); }
// 파라미터가 2개면 4가지 조합, 3개면 8가지... 불가능

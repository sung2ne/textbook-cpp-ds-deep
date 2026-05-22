// filename: iterator_invalidation.cpp
// g++ -std=c++20 -O2 -Wall -o iterator_invalidation iterator_invalidation.cpp
#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <string>

void demo_vector_invalidation() {
    std::cout << "=== vector iterator invalidation ===\n";
    std::vector<int> v = {10, 20, 30, 40, 50};
    v.reserve(20);  // 재할당 방지

    auto it_begin = v.begin();
    auto it_mid = v.begin() + 2;  // 30
    std::cout << "초기: *it_mid = " << *it_mid << "\n";

    // 중간에 삽입: it_mid 이후 iterator 무효화
    v.insert(v.begin() + 1, 15);
    // it_mid는 이제 무효 (한 칸 밀림)
    std::cout << "insert 후 it_mid 역참조는 undefined! 새로 얻어야 함\n";
    auto new_it = v.begin() + 2;
    std::cout << "새 iterator: *new_it = " << *new_it << "\n\n";  // 20

    // 올바른 erase 루프
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    std::cout << "짝수 제거 전: ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n";

    for (auto it = data.begin(); it != data.end(); ) {
        if (*it % 2 == 0) {
            it = data.erase(it);
        } else {
            ++it;
        }
    }
    std::cout << "짝수 제거 후: ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n\n";

    // erase-remove idiom
    std::vector<int> data2 = {1, 2, 3, 4, 5, 6, 7, 8};
    std::erase_if(data2, [](int x) { return x % 2 == 0; });
    std::cout << "erase_if 결과: ";
    for (int x : data2) std::cout << x << " ";
    std::cout << "\n\n";
}

void demo_deque_invalidation() {
    std::cout << "=== deque iterator invalidation ===\n";
    std::deque<int> dq = {10, 20, 30, 40, 50};
    auto it = dq.begin() + 2;  // 30
    std::cout << "초기: *it = " << *it << "\n";

    dq.push_back(60);   // iterator 무효화!
    // it를 역참조하면 undefined behavior
    std::cout << "push_back 후 it는 무효화됨\n";
    auto new_it = dq.begin() + 2;
    std::cout << "새 iterator: *new_it = " << *new_it << "\n\n";
}

void demo_list_stability() {
    std::cout << "=== list iterator stability ===\n";
    std::list<int> lst = {10, 20, 30, 40, 50};
    auto it30 = std::next(lst.begin(), 2);  // 30
    auto it40 = std::next(lst.begin(), 3);  // 40
    std::cout << "초기: *it30=" << *it30 << " *it40=" << *it40 << "\n";

    lst.insert(lst.begin(), 5);    // 맨 앞에 삽입
    lst.erase(lst.begin());        // 맨 앞 삭제
    lst.push_back(60);             // 맨 뒤에 추가

    // it30, it40은 여전히 유효!
    std::cout << "여러 삽입/삭제 후: *it30=" << *it30
              << " *it40=" << *it40 << " (여전히 유효)\n\n";

    // 루프 중 안전한 삭제
    std::list<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    for (auto it = data.begin(); it != data.end(); ) {
        if (*it % 2 == 0) {
            it = data.erase(it);  // list도 erase 반환값 사용
        } else {
            ++it;
        }
    }
    std::cout << "list 짝수 제거: ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    demo_vector_invalidation();
    demo_deque_invalidation();
    demo_list_stability();
    return 0;
}

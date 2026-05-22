// filename: set_map_demo.cpp
// g++ -std=c++17 -O2 -Wall -o set_map_demo set_map_demo.cpp

#include <iostream>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <vector>

int main() {
    // === std::set ===
    std::set<int> s = {5, 3, 8, 1, 9, 3, 7};  // 중복 3 무시
    std::cout << "set 크기: " << s.size() << "\n";  // 6

    // Red-Black Tree이므로 항상 정렬된 순서로 순회
    std::cout << "set 순회: ";
    for (int x : s) std::cout << x << " ";
    std::cout << "\n";  // 1 3 5 7 8 9

    // O(log n) 검색
    std::cout << "7 포함: " << s.count(7) << "\n";  // 1
    std::cout << "4 포함: " << s.count(4) << "\n";  // 0

    // lower_bound, upper_bound — O(log n)
    auto lb = s.lower_bound(5);  // ≥ 5인 첫 번째: iterator to 5
    auto ub = s.upper_bound(5);  // > 5인 첫 번째: iterator to 7

    std::cout << "lower_bound(5): " << *lb << "\n";  // 5
    std::cout << "upper_bound(5): " << *ub << "\n";  // 7

    // equal_range: lower_bound와 upper_bound 동시
    auto [first, last] = s.equal_range(5);
    std::cout << "equal_range(5): [" << *first << ", " << *last << ")\n";
    // [5, 7) → 5만 포함

    // 범위 내 원소 개수
    int count_in_range = std::distance(s.lower_bound(3), s.upper_bound(8));
    std::cout << "[3, 8] 범위 원소 수: " << count_in_range << "\n";  // 4 (3,5,7,8)

    std::cout << "\n";

    // === std::multiset ===
    std::multiset<int> ms = {3, 1, 4, 1, 5, 9, 2, 6, 5};
    std::cout << "multiset: ";
    for (int x : ms) std::cout << x << " ";
    std::cout << "\n";  // 1 1 2 3 4 5 5 6 9

    std::cout << "5의 개수: " << ms.count(5) << "\n";  // 2

    // 5 하나만 삭제 (전체 삭제 방지)
    auto it = ms.find(5);
    if (it != ms.end()) ms.erase(it);  // 하나만 삭제
    std::cout << "5 하나 삭제 후: ";
    for (int x : ms) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\n";

    // === std::map ===
    std::map<std::string, int> word_count;
    std::vector<std::string> words = {"apple", "banana", "apple", "cherry", "banana", "apple"};
    for (auto& w : words) ++word_count[w];  // operator[] 자동 삽입 (초기값 0)

    std::cout << "단어 빈도:\n";
    for (auto& [word, cnt] : word_count)
        std::cout << "  " << word << ": " << cnt << "\n";
    // apple: 3, banana: 2, cherry: 1 (알파벳 순)

    // find vs operator[]: operator[]는 없으면 삽입함!
    std::cout << "\nfind 사용:\n";
    auto found = word_count.find("banana");
    if (found != word_count.end())
        std::cout << "banana: " << found->second << "\n";

    // at()은 없으면 std::out_of_range 예외
    try {
        std::cout << word_count.at("grape") << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "grape 없음: " << e.what() << "\n";
    }

    std::cout << "\n";

    // === lower_bound 이진 탐색 응용 ===
    // 배열 [1, 3, 5, 7, 9]에서 6 이상인 첫 번째 수
    std::set<int> sorted_set = {1, 3, 5, 7, 9};
    auto pos = sorted_set.lower_bound(6);
    std::cout << "6 이상인 첫 번째 수: " << *pos << "\n";  // 7

    // floor(6) — 6 이하인 마지막 수
    if (pos != sorted_set.begin()) {
        --pos;
        std::cout << "6 이하인 마지막 수: " << *pos << "\n";  // 5
    }

    return 0;
}

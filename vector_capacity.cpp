// filename: vector_capacity.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o vector_capacity vector_capacity.cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v;
    size_t prev_cap = 0;

    std::cout << "size | capacity | 재할당 발생\n";
    std::cout << "-----+----------+------------\n";

    for (int i = 0; i < 33; i++) {
        v.push_back(i);
        size_t cap = v.capacity();
        if (cap != prev_cap) {
            std::cout << v.size() << "    | " << cap << "        | <-- 재할당\n";
            prev_cap = cap;
        }
    }

    std::cout << "\n총 원소 수: " << v.size()
              << ", 현재 capacity: " << v.capacity() << "\n";
    return 0;
}

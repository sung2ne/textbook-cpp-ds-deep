// filename: recursion_depth.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o recursion_depth recursion_depth.cpp
#include <iostream>

// 선형 그래프의 DFS — 깊이가 n에 달함
static int dfs_depth(int n, int current = 0) {
    if (current == n) return 0;
    return 1 + dfs_depth(n, current + 1);
}

int main() {
    // 얕은 재귀: 문제없음
    std::cout << "depth 1000: " << dfs_depth(1000) << "\n";

    // 깊은 재귀: 시스템에 따라 스택 오버플로우 발생 가능
    // 보통 8MB 스택에서 int 재귀는 ~50만 ~ 100만 수준이 한계
    std::cout << "depth 10000: " << dfs_depth(10000) << "\n";

    // 이 줄은 스택 오버플로우 가능성 있음 — 주석 처리
    // std::cout << "depth 1000000: " << dfs_depth(1000000) << "\n";

    std::cout << "스택 프레임 1개 크기(추정): ~수십~수백 bytes (로컬 변수에 따라 다름)\n";
    return 0;
}

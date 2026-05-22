// filename: adapter_demo.cpp
// g++ -std=c++17 -O2 -Wall -o adapter_demo adapter_demo.cpp
#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <deque>
#include <list>
#include <chrono>
#include <string>

// stack 사용 예제: 괄호 검사
bool check_brackets(const std::string& expr) {
    std::stack<char, std::vector<char>> stk;  // vector 기반 스택
    for (char c : expr) {
        if (c == '(' || c == '[' || c == '{') {
            stk.push(c);
        } else if (c == ')' || c == ']' || c == '}') {
            if (stk.empty()) return false;
            char top = stk.top();
            stk.pop();
            if ((c == ')' && top != '(') ||
                (c == ']' && top != '[') ||
                (c == '}' && top != '{')) {
                return false;
            }
        }
    }
    return stk.empty();
}

// queue 사용 예제: BFS 레벨 출력
void bfs_demo() {
    // 간단한 트리: 1→[2,3], 2→[4,5], 3→[6]
    // adjacency list
    std::vector<std::vector<int>> adj = {
        {},       // 0 (미사용)
        {2, 3},   // 1
        {4, 5},   // 2
        {6},      // 3
        {},       // 4
        {},       // 5
        {},       // 6
    };

    std::queue<std::pair<int,int>> q;  // {노드, 레벨}
    q.push({1, 0});

    std::cout << "BFS 순서:\n";
    while (!q.empty()) {
        auto [node, level] = q.front();
        q.pop();
        std::cout << "  레벨 " << level << ": 노드 " << node << "\n";
        for (int next : adj[node]) {
            q.push({next, level + 1});
        }
    }
}

// 성능 비교: deque 기반 vs vector 기반 stack
void benchmark_stack(int n) {
    using Clock = std::chrono::high_resolution_clock;
    using us = std::chrono::microseconds;

    auto t1 = Clock::now();
    {
        std::stack<int, std::deque<int>> s;
        for (int i = 0; i < n; ++i) s.push(i);
        while (!s.empty()) s.pop();
    }
    auto t2 = Clock::now();
    long long deque_us = std::chrono::duration_cast<us>(t2 - t1).count();

    auto t3 = Clock::now();
    {
        std::stack<int, std::vector<int>> s;
        s.c.reserve(n);  // 내부 vector reserve (container는 protected)
        for (int i = 0; i < n; ++i) s.push(i);
        while (!s.empty()) s.pop();
    }
    auto t4 = Clock::now();
    long long vector_us = std::chrono::duration_cast<us>(t4 - t3).count();

    std::cout << "=== stack 성능 비교 (n=" << n << ") ===\n";
    std::cout << "  deque 기반:  " << deque_us  << " μs\n";
    std::cout << "  vector 기반: " << vector_us << " μs\n\n";
}

int main() {
    // 괄호 검사
    std::cout << "=== 괄호 검사 ===\n";
    std::vector<std::string> exprs = {
        "(a + b) * [c - {d}]",
        "(((",
        "{[()]}"
    };
    for (const auto& e : exprs) {
        std::cout << "  \"" << e << "\": "
                  << (check_brackets(e) ? "올바름" : "잘못됨") << "\n";
    }
    std::cout << "\n";

    // BFS 시연
    bfs_demo();
    std::cout << "\n";

    // 성능 비교
    benchmark_stack(1000000);

    return 0;
}

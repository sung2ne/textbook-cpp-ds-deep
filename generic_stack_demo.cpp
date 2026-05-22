// filename: generic_stack_demo.cpp
// g++ -std=c++17 -O2 -Wall -o generic_stack generic_stack_demo.cpp
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include "generic_stack.hpp"

struct Point {
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
};

int main() {
    // int 스택
    Stack<int> int_stack;
    for (int i : {5, 3, 8, 1, 9}) int_stack.push(i);

    std::cout << "=== Stack<int> ===\n";
    std::cout << "size=" << int_stack.size() << " top=" << int_stack.top() << "\n";

    // iterator로 STL 알고리즘 사용
    std::cout << "내용 (바닥→꼭대기): ";
    for (int x : int_stack) std::cout << x << " ";
    std::cout << "\n";

    auto sum = std::accumulate(int_stack.begin(), int_stack.end(), 0);
    auto max = *std::max_element(int_stack.begin(), int_stack.end());
    std::cout << "합=" << sum << " 최댓값=" << max << "\n\n";

    // string 스택
    Stack<std::string> str_stack;
    str_stack.push("hello");
    str_stack.push("world");
    str_stack.emplace("C++");  // 직접 생성

    std::cout << "=== Stack<string> ===\n";
    while (!str_stack.empty()) {
        std::cout << str_stack.top() << " ";
        str_stack.pop();
    }
    std::cout << "\n\n";

    // Point 스택 (사용자 정의 타입)
    Stack<Point> pt_stack;
    pt_stack.emplace(1.0, 2.0);
    pt_stack.emplace(3.0, 4.0);

    std::cout << "=== Stack<Point> ===\n";
    while (!pt_stack.empty()) {
        auto& p = pt_stack.top();
        std::cout << "(" << p.x << ", " << p.y << ") ";
        pt_stack.pop();
    }
    std::cout << "\n\n";

    // Queue<int> 테스트
    Queue<int> q;
    for (int i : {10, 20, 30, 40, 50}) q.push(i);

    std::cout << "=== Queue<int> ===\n";
    std::cout << "size=" << q.size() << " front=" << q.front() << "\n";
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << "\n";

    return 0;
}

// filename: ranges_sort.cpp
// g++ -std=c++20 -O2 -Wall -o ranges_sort ranges_sort.cpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <string>
#include <numeric>

struct Employee {
    std::string name;
    int age;
    double salary;
};

int main() {
    std::vector<int> nums = {5, 3, 1, 4, 2};

    // std::sort 방식 (C++17)
    std::sort(nums.begin(), nums.end());

    // std::ranges::sort — 컨테이너 직접 전달
    std::ranges::sort(nums);

    // 내림차순
    std::ranges::sort(nums, std::greater<int>{});

    // 구조체 정렬: projection으로 정렬 기준 지정
    std::vector<Employee> employees = {
        {"Alice", 30, 5000.0},
        {"Bob",   25, 6500.0},
        {"Carol", 35, 4800.0},
        {"Dave",  28, 7200.0},
    };

    // 나이로 정렬 — projection 사용
    std::ranges::sort(employees, {}, &Employee::age);
    std::cout << "나이 순: ";
    for (auto& e : employees) std::cout << e.name << "(" << e.age << ") ";
    std::cout << "\n";
    // 출력: Bob(25) Dave(28) Alice(30) Carol(35)

    // 급여 내림차순 — projection + comparator 조합
    std::ranges::sort(employees,
                      [](double a, double b){ return a > b; },
                      &Employee::salary);
    std::cout << "급여 내림차순: ";
    for (auto& e : employees) std::cout << e.name << "($" << e.salary << ") ";
    std::cout << "\n";
    // 출력: Dave($7200) Bob($6500) Alice($5000) Carol($4800)

    // 범위(subrange) 정렬
    std::vector<int> data = {8, 3, 5, 1, 9, 2, 7, 4, 6};
    // 인덱스 2~6만 정렬
    std::ranges::sort(data.begin() + 2, data.begin() + 7);
    std::cout << "부분 정렬(인덱스 2-6): ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n";
    // 출력: 8 3 1 2 5 9 7 4 6  (인덱스 2-6만 정렬됨)

    // 문자열 길이로 정렬
    std::vector<std::string> words = {"banana", "apple", "cherry", "fig", "elderberry"};
    std::ranges::sort(words, {}, &std::string::size);
    std::cout << "길이 순: ";
    for (auto& w : words) std::cout << w << " ";
    std::cout << "\n";
    // 출력: fig apple banana cherry elderberry

    // iota view + sort 조합
    std::vector<int> indices(10);
    std::iota(indices.begin(), indices.end(), 0);
    std::vector<int> values = {50, 20, 80, 10, 60, 30, 90, 40, 70, 15};

    // 값 기준으로 인덱스 정렬 (간접 정렬)
    std::ranges::sort(indices, [&](int a, int b){
        return values[a] < values[b];
    });
    std::cout << "값 기준 인덱스 순서: ";
    for (int i : indices) std::cout << i << "(" << values[i] << ") ";
    std::cout << "\n";

    return 0;
}

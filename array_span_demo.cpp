// filename: array_span_demo.cpp
// g++ -std=c++20 -O2 -Wall -o array_span_demo array_span_demo.cpp
#include <iostream>
#include <array>
#include <vector>
#include <span>
#include <algorithm>
#include <numeric>

// span으로 통일된 인터페이스
void print_span(std::string_view label, std::span<const int> data) {
    std::cout << label << ": [";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << data[i];
        if (i + 1 < data.size()) std::cout << ", ";
    }
    std::cout << "] size=" << data.size() << "\n";
}

void scale_inplace(std::span<int> data, int factor) {
    for (int& x : data) x *= factor;
}

double mean(std::span<const int> data) {
    if (data.empty()) return 0.0;
    return static_cast<double>(std::accumulate(data.begin(), data.end(), 0))
           / data.size();
}

int main() {
    // std::array: 스택 할당, 크기 타입 포함
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    std::cout << "sizeof(std::array<int,5>) = " << sizeof(arr) << " bytes\n";
    std::cout << "sizeof(int[5]) = " << sizeof(int[5]) << " bytes\n\n";

    // STL 알고리즘 완전 호환
    std::sort(arr.begin(), arr.end(), std::greater<int>());
    print_span("정렬 후 array", arr);

    // 복사 가능 (C 배열은 불가)
    std::array<int, 5> arr_copy = arr;
    std::cout << "복사 성공, arr_copy[0]=" << arr_copy[0] << "\n\n";

    // span으로 다양한 타입 통합 처리
    std::vector<int> vec = {10, 20, 30, 40, 50};
    int raw_arr[] = {100, 200, 300};

    print_span("vector", vec);
    print_span("raw array", raw_arr);
    print_span("array", arr);

    // span으로 부분 범위
    print_span("vec의 처음 3개", std::span<int>(vec).first(3));
    print_span("vec의 마지막 2개", std::span<int>(vec).last(2));
    print_span("vec[1..3]", std::span<int>(vec).subspan(1, 3));

    // 제자리 수정
    scale_inplace(vec, 2);
    print_span("vec * 2", vec);

    // 평균 계산 (read-only)
    std::cout << "\n평균: " << mean(vec) << "\n";
    std::cout << "평균: " << mean(arr) << "\n";
    std::cout << "평균: " << mean(raw_arr) << "\n";

    // constexpr std::array
    constexpr std::array<int, 4> PRIMES = {2, 3, 5, 7};
    std::cout << "\nconstexpr primes: ";
    for (int p : PRIMES) std::cout << p << " ";
    std::cout << "\n";

    // 정적 extent span
    std::span<const int, 4> static_span = PRIMES;
    std::cout << "static span size (compile-time): " << static_span.size() << "\n";

    return 0;
}

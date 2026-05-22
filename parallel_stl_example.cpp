// filename: parallel_stl_example.cpp
// g++ -std=c++17 -O2 -ltbb -o parallel_stl parallel_stl_example.cpp

#include <algorithm>
#include <execution>
#include <vector>
#include <numeric>
#include <iostream>

int main() {
    std::vector<double> v(1 << 20);  // 1M elements
    std::iota(v.begin(), v.end(), 0.0);

    // 병렬 정렬 (C++17) — TBB 백엔드 사용
    std::sort(std::execution::par_unseq, v.begin(), v.end(),
              std::greater<double>{});

    // 병렬 reduce
    double sum = std::reduce(std::execution::par_unseq,
                             v.begin(), v.end(), 0.0);

    std::cout << "sum = " << sum << "\n";
    return 0;
}

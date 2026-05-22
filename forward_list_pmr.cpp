// filename: forward_list_pmr.cpp
// g++ -std=c++17 -O2 -Wall -o forward_list_pmr forward_list_pmr.cpp
#include <iostream>
#include <forward_list>
#include <memory_resource>
#include <chrono>
#include <vector>
#include <numeric>

void demo_forward_list() {
    std::cout << "=== forward_list 기본 사용 ===\n";
    std::forward_list<int> fl = {3, 1, 4, 1, 5, 9, 2, 6};

    // forward_list에는 size() 없음 — std::distance로 계산
    auto cnt = std::distance(fl.begin(), fl.end());
    std::cout << "크기 (std::distance): " << cnt << "\n";

    // 정렬 (내장 sort 사용, std::sort 불가 — forward iterator)
    fl.sort();
    std::cout << "정렬 후: ";
    for (int x : fl) std::cout << x << " ";
    std::cout << "\n";

    // 중복 제거 (정렬 후 unique)
    fl.unique();
    std::cout << "unique 후: ";
    for (int x : fl) std::cout << x << " ";
    std::cout << "\n\n";

    // insert_after로 짝수 뒤에 0 삽입
    std::forward_list<int> fl2 = {1, 2, 3, 4, 5};
    auto it = fl2.before_begin();
    for (auto cur = fl2.begin(); cur != fl2.end(); ) {
        if (*cur % 2 == 0) {
            auto inserted = fl2.insert_after(cur, 0);
            ++cur; ++cur;  // 0 건너뜀
            it = inserted;
        } else {
            it = cur;
            ++cur;
        }
    }
    std::cout << "짝수 뒤 0 삽입: ";
    for (int x : fl2) std::cout << x << " ";
    std::cout << "\n\n";

    // splice_after: 다른 리스트에서 노드 이동
    std::forward_list<int> a = {10, 20, 30};
    std::forward_list<int> b = {1, 2, 3};
    a.splice_after(a.begin(), b);  // a의 10 다음에 b 전체 이동
    std::cout << "splice_after 후 a: ";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "b 비워짐: " << (b.empty() ? "yes" : "no") << "\n\n";
}

void benchmark_pmr() {
    using Clock = std::chrono::high_resolution_clock;
    using us = std::chrono::microseconds;
    const int N = 100000;

    // 일반 forward_list (힙 할당)
    auto t1 = Clock::now();
    {
        for (int trial = 0; trial < 100; ++trial) {
            std::forward_list<int> fl;
            for (int i = 0; i < N / 100; ++i) fl.push_front(i);
        }
    }
    auto t2 = Clock::now();
    long long heap_us = std::chrono::duration_cast<us>(t2 - t1).count();

    // PMR forward_list (monotonic buffer)
    auto t3 = Clock::now();
    {
        for (int trial = 0; trial < 100; ++trial) {
            std::vector<char> buf(N * 20);  // 충분한 크기
            std::pmr::monotonic_buffer_resource pool(buf.data(), buf.size());
            std::pmr::forward_list<int> fl(&pool);
            for (int i = 0; i < N / 100; ++i) fl.push_front(i);
        }
    }
    auto t4 = Clock::now();
    long long pmr_us = std::chrono::duration_cast<us>(t4 - t3).count();

    std::cout << "=== PMR 성능 비교 (100회 × " << N/100 << "개 삽입) ===\n";
    std::cout << "  일반 heap:  " << heap_us << " μs\n";
    std::cout << "  PMR buffer: " << pmr_us  << " μs\n";
    if (pmr_us > 0)
        std::cout << "  PMR가 " << std::fixed << heap_us / std::max(pmr_us, 1LL)
                  << "배 빠름\n\n";
}

void demo_pmr_pool() {
    std::cout << "=== PMR synchronized_pool_resource ===\n";
    // pool_resource: 크기별 블록 재활용, 일반적인 allocator 대체
    std::pmr::synchronized_pool_resource pool;
    std::pmr::forward_list<std::string> fl(&pool);

    fl.push_front("world");
    fl.push_front("hello");
    fl.push_front("C++17");

    for (const auto& s : fl) std::cout << s << " ";
    std::cout << "\n";
}

int main() {
    demo_forward_list();
    benchmark_pmr();
    demo_pmr_pool();
    return 0;
}

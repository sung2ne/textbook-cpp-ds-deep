// filename: acquire_release.cpp
// g++ -std=c++17 -O2 -pthread -o acquire_release acquire_release.cpp
#include <atomic>
#include <thread>
#include <cassert>
#include <iostream>

std::atomic<bool> ready{false};
int               data = 0;

void producer()
{
    data = 42;                                      // (1) 일반 쓰기
    ready.store(true, std::memory_order_release);   // (2) release: (1)이 (2) 이전으로 보장
    // "release" = 이전의 모든 쓰기가 이 store 이전에 완료됨을 보장
}

void consumer()
{
    // (3) acquire: (4) 이후의 읽기가 이 load 이전으로 앞당겨지지 않음
    while (!ready.load(std::memory_order_acquire)) ;
    int v = data;                                   // (4) 일반 읽기
    assert(v == 42); // 항상 성립
    std::cout << "data = " << v << "\n";
}

int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join(); t2.join();
    return 0;
}

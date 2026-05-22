// filename: stack_depth.cpp
// g++ -std=c++17 -O0 -o stack_depth stack_depth.cpp
// 주의: 실행 시 스택 오버플로우 발생 후 segfault

#include <iostream>

int depth = 0;

void recurse() {
    int local[64];  // 각 호출마다 256 바이트 소비
    local[0] = depth;
    ++depth;

    if (depth % 1000 == 0) {
        std::cout << "깊이: " << depth
                  << ", 스택 변수 주소: " << (void*)local << "\n";
    }
    recurse();
}

int main() {
    try {
        recurse();
    } catch (...) {
        std::cout << "예외 발생 (최대 깊이: " << depth << ")\n";
    }
    return 0;
}

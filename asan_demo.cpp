// filename: asan_demo.cpp
// g++ -std=c++17 -g -fsanitize=address -fno-omit-frame-pointer -o asan_demo asan_demo.cpp
// 주의: 각 섹션을 별도로 실행해야 함 (첫 오류에서 종료)

#include <iostream>
#include <vector>
#include <memory>
#include <cstring>

// ============================================================
// 1. Heap buffer overflow
// ============================================================
void heap_overflow() {
    int* arr = new int[4]{0, 1, 2, 3};
    // ASan 리포트: heap-buffer-overflow
    arr[4] = 99;   // 범위 초과! (인덱스는 0~3)
    delete[] arr;
}

// ============================================================
// 2. Use-after-free
// ============================================================
void use_after_free() {
    int* p = new int(42);
    delete p;
    // ASan 리포트: heap-use-after-free
    std::cout << *p << '\n';   // 해제된 메모리 읽기!
}

// ============================================================
// 3. Stack buffer overflow
// ============================================================
void stack_overflow() {
    char buf[8];
    // ASan 리포트: stack-buffer-overflow
    std::memset(buf, 0, 16);   // buf 크기(8)를 초과!
}

// ============================================================
// 4. Double-free
// ============================================================
void double_free() {
    int* p = new int(10);
    delete p;
    // ASan 리포트: attempting double-free
    delete p;   // 이미 해제한 메모리를 또 해제!
}

// ============================================================
// 5. Memory leak (LeakSanitizer)
// ============================================================
void memory_leak() {
    int* p = new int[100];
    p[0] = 1;
    // delete[] p; 누락!
    // 함수 종료 후 LSan이 "leak" 보고
}

// ============================================================
// 올바른 버전들
// ============================================================
void correct_heap() {
    std::vector<int> arr = {0, 1, 2, 3};
    // arr.at(4);  // std::out_of_range 예외 → 안전
    arr[3] = 99;   // 유효한 접근
}

void correct_ownership() {
    auto p = std::make_unique<int>(42);
    std::cout << "value: " << *p << '\n';
    // unique_ptr 소멸 시 자동 delete
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "사용법: ./asan_demo <test>\n";
        std::cout << "  1: heap_overflow\n";
        std::cout << "  2: use_after_free\n";
        std::cout << "  3: stack_overflow\n";
        std::cout << "  4: double_free\n";
        std::cout << "  5: memory_leak\n";
        std::cout << "  6: correct (ASan 통과)\n";
        return 0;
    }

    switch (argv[1][0]) {
        case '1': heap_overflow();  break;
        case '2': use_after_free(); break;
        case '3': stack_overflow(); break;
        case '4': double_free();    break;
        case '5': memory_leak();    break;
        case '6':
            correct_heap();
            correct_ownership();
            std::cout << "ASan 통과\n";
            break;
    }
    return 0;
}

// filename: ubsan_demo.cpp
// g++ -std=c++17 -g -fsanitize=undefined -o ubsan_demo ubsan_demo.cpp

#include <iostream>
#include <climits>
#include <cstdint>

// ============================================================
// 1. 부호 있는 정수 오버플로 (UB)
// ============================================================
void signed_overflow() {
    int x = INT_MAX;
    // UBSan: runtime error: signed integer overflow
    int y = x + 1;   // 정의되지 않은 동작!
    std::cout << y << '\n';
}

// ============================================================
// 2. Null 포인터 역참조 (UB)
// ============================================================
void null_deref() {
    int* p = nullptr;
    // UBSan: runtime error: load of null pointer of type 'int'
    std::cout << *p << '\n';
}

// ============================================================
// 3. 정렬 오류 (UB)
// ============================================================
void alignment_violation() {
    char buf[8] = {0};
    // 4-byte 정렬된 포인터가 필요한 곳에 1-byte offset 사용
    int* p = reinterpret_cast<int*>(buf + 1);
    // UBSan: runtime error: load of misaligned address 0x... for type 'int'
    std::cout << *p << '\n';
}

// ============================================================
// 4. VLA (Variable Length Array) 음수 크기 (UB)
// ============================================================
void vla_negative(int n) {
    // UBSan: runtime error: variable length array bound evaluates to non-positive value
    // 주석 처리 (컴파일러에 따라 -Wvla 경고)
    // int arr[n];
    (void)n;
}

// ============================================================
// 5. 올바른 버전: 부호 없는 정수 사용
// ============================================================
void no_overflow() {
    uint32_t x = UINT32_MAX;
    uint32_t y = x + 1;  // 부호 없는 오버플로: 0 (defined behavior)
    std::cout << "wrap: " << y << '\n';
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "사용법: ./ubsan_demo <1|2|3|4>\n";
        return 0;
    }
    switch (argv[1][0]) {
        case '1': signed_overflow();     break;
        case '2': null_deref();          break;
        case '3': alignment_violation(); break;
        case '4': no_overflow();         break;
    }
    return 0;
}

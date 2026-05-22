// filename: double_free.cpp
// g++ -std=c++17 -O0 -fsanitize=address -o double_free double_free.cpp
// (AddressSanitizer로 실행하면 정확한 진단 출력)

#include <iostream>

int main() {
    int* p = new int(42);
    delete p;
    // p는 여전히 같은 주소를 가리키지만, 그 메모리는 이미 해제됨

    delete p;  // ❌ double free — 미정의 동작
    // 실행 결과: 크래시, 힙 손상, 보안 취약점
    return 0;
}

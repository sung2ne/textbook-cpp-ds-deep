// filename: sso_threshold.cpp
// g++ -std=c++17 -O2 -Wall -o sso_threshold sso_threshold.cpp
#include <iostream>
#include <string>
#include <cstdlib>

// 힙 할당 여부를 감지하는 트릭:
// 문자열이 객체 내부에 있으면 &s[0]이 &s 근처를 가리킴
// 힙에 있으면 &s[0]과 &s의 주소 차이가 큼
bool is_sso(const std::string& s) {
    const char* str_ptr = s.data();
    const char* obj_ptr = reinterpret_cast<const char*>(&s);
    ptrdiff_t diff = str_ptr - obj_ptr;
    // 객체 크기(sizeof(string)) 이내라면 SSO
    return diff >= 0 && diff < static_cast<ptrdiff_t>(sizeof(std::string));
}

int main() {
    std::cout << "sizeof(std::string) = " << sizeof(std::string) << " bytes\n\n";

    std::cout << "길이별 SSO 여부:\n";
    for (int len = 0; len <= 30; ++len) {
        std::string s(len, 'x');
        std::cout << "  len=" << len
                  << " : " << (is_sso(s) ? "SSO (힙 없음)" : "힙 할당")
                  << "\n";
    }

    // string_view 비교
    std::cout << "\nsizeof(std::string_view) = " << sizeof(std::string_view) << " bytes\n";
    std::cout << "  (포인터 8 + 크기 8 = 16바이트, 항상 비소유)\n";

    // SSO 문자열의 복사 비용
    std::string short_s = "Hello";       // SSO
    std::string long_s(100, 'x');        // 힙

    auto copy_short = short_s;  // memcpy만, 힙 없음
    auto copy_long  = long_s;   // 힙 할당 + memcpy

    std::cout << "\n짧은 문자열 복사: SSO=" << is_sso(copy_short)
              << " (힙 할당 없음)\n";
    std::cout << "긴 문자열 복사:   SSO=" << is_sso(copy_long)
              << " (힙 할당 발생)\n";

    return 0;
}

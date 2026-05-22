// filename: dangling_ptr.cpp
// g++ -std=c++17 -O0 -fsanitize=address -o dangling_ptr dangling_ptr.cpp

#include <iostream>
#include <string>

int* createDangling() {
    int local = 99;
    return &local;  // ❌ 지역 변수 주소 반환 — 함수 종료 후 스택 해제
}

int main() {
    // 패턴 1: 지역 변수 주소 반환
    int* p1 = createDangling();
    std::cout << *p1 << "\n";  // 미정의 동작 — 스택 프레임 이미 해제

    // 패턴 2: delete 후 사용
    int* p2 = new int(42);
    delete p2;
    std::cout << *p2 << "\n";  // ❌ use-after-free

    // 패턴 3: 컨테이너 재할당 후 이터레이터 무효화
    std::vector<int> v = {1, 2, 3};
    int* ptr = &v[0];
    v.push_back(4);  // 재할당 발생 → ptr이 가리키던 메모리 해제
    std::cout << *ptr << "\n";  // ❌ dangling pointer

    return 0;
}

// filename: weak_ptr_demo.cpp
// g++ -std=c++17 -O2 -o weak_ptr_demo weak_ptr_demo.cpp

#include <iostream>
#include <memory>

struct Resource {
    int id;
    Resource(int i) : id(i) {
        std::cout << "Resource(" << id << ") 생성\n";
    }
    ~Resource() { std::cout << "Resource(" << id << ") 소멸\n"; }
    void use() { std::cout << "Resource(" << id << ") 사용 중\n"; }
};

int main() {
    std::weak_ptr<Resource> weak;

    {
        auto shared = std::make_shared<Resource>(42);
        weak = shared;  // weak_ptr 연결 (use_count 변화 없음)

        std::cout << "use_count: " << shared.use_count() << "\n";  // 1
        std::cout << "weak expired: " << weak.expired() << "\n";   // false

        // lock()으로 임시 shared_ptr 획득
        if (auto locked = weak.lock()) {
            locked->use();
            std::cout << "lock 후 use_count: " << shared.use_count() << "\n"; // 2
        }  // locked 소멸 → use_count 다시 1
    }  // shared 소멸 → Resource 소멸, use_count 0

    std::cout << "스코프 종료 후 weak expired: " << weak.expired() << "\n";  // true

    // 소멸된 객체에 접근 시도
    if (auto locked = weak.lock()) {
        locked->use();  // 실행되지 않음
    } else {
        std::cout << "객체가 이미 소멸됨\n";
    }

    return 0;
}

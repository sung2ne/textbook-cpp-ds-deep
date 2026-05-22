// filename: object_slicing.cpp
// g++ -std=c++17 -O2 -o object_slicing object_slicing.cpp

#include <iostream>
#include <vector>
#include <memory>

struct Animal {
    std::string name;
    Animal(const std::string& n) : name(n) {}
    virtual std::string sound() const { return "..."; }
    virtual ~Animal() = default;
};

struct Dog : Animal {
    Dog(const std::string& n) : Animal(n) {}
    std::string sound() const override { return "멍멍"; }
};

struct Cat : Animal {
    Cat(const std::string& n) : Animal(n) {}
    std::string sound() const override { return "야옹"; }
};

int main() {
    Dog dog("Rex");
    Cat cat("Nabi");

    // ❌ 슬라이싱 발생 — Dog/Cat 부분이 잘려나감
    Animal a1 = dog;  // Dog → Animal 값 복사, Dog 전용 데이터 손실
    Animal a2 = cat;
    std::cout << a1.sound() << "\n";  // "..." (Dog::sound가 아님)
    std::cout << a2.sound() << "\n";  // "..." (Cat::sound가 아님)

    // ❌ 컨테이너에 값으로 저장 시 슬라이싱
    std::vector<Animal> animals;
    animals.push_back(dog);  // Dog → Animal 슬라이싱
    animals.push_back(cat);
    for (const auto& a : animals) {
        std::cout << a.name << ": " << a.sound() << "\n";  // 모두 "..."
    }

    std::cout << "\n=== 올바른 방법: 포인터 사용 ===\n";
    std::vector<std::unique_ptr<Animal>> polymorphic;
    polymorphic.push_back(std::make_unique<Dog>("Rex"));
    polymorphic.push_back(std::make_unique<Cat>("Nabi"));
    for (const auto& a : polymorphic) {
        std::cout << a->name << ": " << a->sound() << "\n";  // 다형성 정상 동작
    }

    return 0;
}

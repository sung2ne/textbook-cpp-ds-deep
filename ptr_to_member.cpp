// filename: ptr_to_member.cpp
// g++ -std=c++17 -O2 -o ptr_to_member ptr_to_member.cpp

#include <iostream>
#include <string>

struct Employee {
    std::string name;
    int salary;
    int bonus;

    int total() const { return salary + bonus; }
    void print() const {
        std::cout << name << ": " << total() << "\n";
    }
};

int main() {
    Employee emp{"Alice", 5000, 1000};

    // 멤버 변수 포인터 — Employee 클래스의 int 멤버를 가리킴
    int Employee::* pSalary = &Employee::salary;
    int Employee::* pBonus  = &Employee::bonus;

    std::cout << "급여: " << emp.*pSalary << "\n";    // 5000
    emp.*pBonus = 2000;  // bonus 수정
    std::cout << "보너스 수정 후 합계: " << emp.total() << "\n";  // 7000

    // 멤버 함수 포인터
    void (Employee::*pPrint)() const = &Employee::print;
    (emp.*pPrint)();  // emp.print() 호출

    int (Employee::*pTotal)() const = &Employee::total;
    std::cout << "total(): " << (emp.*pTotal)() << "\n";

    // 포인터를 통한 접근
    Employee* ptr = &emp;
    std::cout << "포인터로 급여: " << ptr->*pSalary << "\n";
    (ptr->*pPrint)();

    return 0;
}

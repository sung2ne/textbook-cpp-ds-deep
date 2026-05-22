// filename: sizeof_containers.cpp
// g++ -std=c++17 -O2 -Wall -Wextra -o sizeof_containers sizeof_containers.cpp
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>
#include <array>
#include <forward_list>
#include <set>
#include <stack>
#include <queue>

int main() {
    std::cout << "=== STL 컨테이너 sizeof ===\n\n";

    // 시퀀스 컨테이너
    std::cout << "[ 시퀀스 컨테이너 ]\n";
    std::cout << "sizeof(vector<int>)         = " << sizeof(std::vector<int>) << "\n";
    std::cout << "sizeof(vector<double>)      = " << sizeof(std::vector<double>) << "\n";
    std::cout << "sizeof(vector<string>)      = " << sizeof(std::vector<std::string>) << "\n";
    std::cout << "sizeof(deque<int>)          = " << sizeof(std::deque<int>) << "\n";
    std::cout << "sizeof(list<int>)           = " << sizeof(std::list<int>) << "\n";
    std::cout << "sizeof(forward_list<int>)   = " << sizeof(std::forward_list<int>) << "\n";
    std::cout << "sizeof(array<int,10>)       = " << sizeof(std::array<int, 10>) << "\n";
    std::cout << "sizeof(array<int,100>)      = " << sizeof(std::array<int, 100>) << "\n";

    // 문자열
    std::cout << "\n[ 문자열 ]\n";
    std::cout << "sizeof(string)              = " << sizeof(std::string) << "\n";
    std::string s_short = "hi";
    std::string s_long  = "this string is longer than SSO threshold";
    std::cout << "short string capacity       = " << s_short.capacity() << "\n";
    std::cout << "long  string capacity       = " << s_long.capacity()  << "\n";

    // 연관 컨테이너
    std::cout << "\n[ 연관 컨테이너 ]\n";
    std::cout << "sizeof(set<int>)            = " << sizeof(std::set<int>) << "\n";
    std::cout << "sizeof(map<int,int>)        = " << sizeof(std::map<int,int>) << "\n";
    std::cout << "sizeof(unordered_map<int,int>) = " << sizeof(std::unordered_map<int,int>) << "\n";

    // 컨테이너 어댑터
    std::cout << "\n[ 어댑터 ]\n";
    std::cout << "sizeof(stack<int>)          = " << sizeof(std::stack<int>) << "\n";
    std::cout << "sizeof(queue<int>)          = " << sizeof(std::queue<int>) << "\n";
    std::cout << "sizeof(priority_queue<int>) = " << sizeof(std::priority_queue<int>) << "\n";

    // list 노드 크기 추정
    std::cout << "\n[ list 노드 크기 추정 ]\n";
    std::list<int> lst;
    for (int i = 0; i < 1000; i++) lst.push_back(i);
    // list는 노드별 할당 — 노드 = (prev ptr) + (next ptr) + (value)
    // 64비트 시스템: 8 + 8 + 4 = 20 bytes, 정렬로 인해 보통 24 bytes
    std::cout << "list<int> 노드 추정 크기    = " << sizeof(void*) * 2 + sizeof(int)
              << " ~ " << sizeof(void*) * 2 + sizeof(int) + 4 << " bytes\n";

    return 0;
}

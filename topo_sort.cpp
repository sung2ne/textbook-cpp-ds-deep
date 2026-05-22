// filename: topo_sort.cpp
// g++ -std=c++17 -O2 -Wall -o topo_sort topo_sort.cpp
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class DependencyResolver {
    std::vector<std::string>              names_;
    std::unordered_map<std::string, int>  name_to_id_;
    std::vector<std::vector<int>>         deps_;      // deps_[u] = u가 의존하는 패키지들
    int V_ = 0;

public:
    int add_package(const std::string& name) {
        if (auto it = name_to_id_.find(name); it != name_to_id_.end())
            return it->second;
        int id = V_++;
        names_.push_back(name);
        name_to_id_[name] = id;
        deps_.push_back({});
        return id;
    }

    // A depends_on B: A를 설치하려면 B가 먼저 필요
    void add_dependency(const std::string& A, const std::string& B) {
        int a = add_package(A);
        int b = add_package(B);
        deps_[a].push_back(b);
    }

    // Kahn's Algorithm으로 설치 순서 결정
    std::vector<std::string> install_order() const {
        // in-degree: 각 패키지가 몇 개의 다른 패키지에게 필요한지
        // = "이 패키지를 먼저 설치해야 하는 패키지 수"
        // 방향 그래프: A→B (A depends on B)
        // in-degree[B] += 1 when A→B exists

        std::vector<int> indegree(V_, 0);
        // 간선 방향: A→B (A가 B에 의존) → B의 out-degree 역방향
        // 위상 정렬용으로 역방향 그래프 구성
        std::vector<std::vector<int>> rev(V_);
        for (int a = 0; a < V_; ++a)
            for (int b : deps_[a]) {
                rev[b].push_back(a);
                ++indegree[a];
            }

        // in-degree=0인 패키지들 (의존성 없음 → 즉시 설치 가능)
        std::queue<int> q;
        for (int i = 0; i < V_; ++i)
            if (indegree[i] == 0) q.push(i);

        std::vector<std::string> order;
        while (!q.empty()) {
            int pkg = q.front(); q.pop();
            order.push_back(names_[pkg]);
            for (int dependent : rev[pkg]) {
                --indegree[dependent];
                if (indegree[dependent] == 0) q.push(dependent);
            }
        }

        if (static_cast<int>(order.size()) != V_)
            throw std::runtime_error("순환 의존성 탐지! 설치 불가.");

        return order;
    }

    void print_dependencies() const {
        std::cout << "의존성 그래프:\n";
        for (int i = 0; i < V_; ++i) {
            std::cout << "  " << names_[i];
            if (!deps_[i].empty()) {
                std::cout << " → depends on: ";
                for (int j = 0; auto d : deps_[i]) {
                    if (j++) std::cout << ", ";
                    std::cout << names_[d];
                }
            }
            std::cout << '\n';
        }
    }
};

int main() {
    // 패키지 의존성 시나리오
    // myapp → fastapi → starlette → anyio → sniffio
    //                  → httpx   → httpcore → h11
    //       → uvicorn → httptools
    //                 → websockets
    DependencyResolver resolver;

    resolver.add_dependency("myapp",      "fastapi");
    resolver.add_dependency("myapp",      "uvicorn");
    resolver.add_dependency("fastapi",    "starlette");
    resolver.add_dependency("fastapi",    "httpx");
    resolver.add_dependency("starlette",  "anyio");
    resolver.add_dependency("anyio",      "sniffio");
    resolver.add_dependency("httpx",      "httpcore");
    resolver.add_dependency("httpcore",   "h11");
    resolver.add_dependency("uvicorn",    "httptools");
    resolver.add_dependency("uvicorn",    "websockets");

    std::cout << "=== 패키지 의존성 ===\n";
    resolver.print_dependencies();

    std::cout << "\n=== 설치 순서 (위상 정렬) ===\n";
    try {
        auto order = resolver.install_order();
        std::cout << "총 " << order.size() << "개 패키지:\n";
        for (int i = 0; auto& pkg : order)
            std::cout << ++i << ". " << pkg << '\n';
    } catch (const std::exception& e) {
        std::cout << "오류: " << e.what() << '\n';
    }

    // 순환 의존성 테스트
    std::cout << "\n=== 순환 의존성 테스트 ===\n";
    DependencyResolver cyclic;
    cyclic.add_dependency("A", "B");
    cyclic.add_dependency("B", "C");
    cyclic.add_dependency("C", "A");  // 순환!
    try {
        cyclic.install_order();
    } catch (const std::exception& e) {
        std::cout << "예외 발생: " << e.what() << '\n';
    }

    // 빌드 시스템 예시: CMake 타겟 의존성
    std::cout << "\n=== 빌드 시스템 의존성 ===\n";
    DependencyResolver build;
    build.add_dependency("main.exe",   "libapp.a");
    build.add_dependency("main.exe",   "libui.a");
    build.add_dependency("libapp.a",   "libcore.a");
    build.add_dependency("libapp.a",   "libdb.a");
    build.add_dependency("libui.a",    "libcore.a");
    build.add_dependency("libcore.a",  "libmath.a");
    build.add_dependency("libdb.a",    "libmath.a");

    auto build_order = build.install_order();
    std::cout << "빌드 순서:\n";
    for (int i = 0; auto& t : build_order)
        std::cout << ++i << ". " << t << '\n';

    return 0;
}

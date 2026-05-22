// filename: dfs.cpp
// g++ -std=c++17 -O2 -Wall -o dfs dfs.cpp
#include <algorithm>
#include <iostream>
#include <numeric>
#include <stack>
#include <utility>
#include <vector>

using Graph = std::vector<std::vector<std::pair<int,int>>>;

// --- 재귀 DFS ---

void dfs_recursive(const Graph& g, int u,
                   std::vector<bool>& visited,
                   std::vector<int>& order) {
    visited[u] = true;
    order.push_back(u);
    for (const auto& [v, w] : g[u]) {
        if (!visited[v])
            dfs_recursive(g, v, visited, order);
    }
}

// --- 반복 DFS (명시적 스택) ---

std::vector<int> dfs_iterative(const Graph& g, int start) {
    int V = static_cast<int>(g.size());
    std::vector<bool> visited(V, false);
    std::vector<int>  order;
    std::stack<int>   stk;

    stk.push(start);
    while (!stk.empty()) {
        int u = stk.top(); stk.pop();
        if (visited[u]) continue;
        visited[u] = true;
        order.push_back(u);
        // 역순으로 push해야 재귀와 같은 순서 (스택이므로 역전)
        for (auto it = g[u].rbegin(); it != g[u].rend(); ++it)
            if (!visited[it->first]) stk.push(it->first);
    }
    return order;
}

// --- DFS 응용 1: 연결 요소 탐색 ---

int count_components(const Graph& g) {
    int V = static_cast<int>(g.size());
    std::vector<bool> visited(V, false);
    int components = 0;
    for (int i = 0; i < V; ++i) {
        if (!visited[i]) {
            std::vector<int> dummy;
            dfs_recursive(g, i, visited, dummy);
            ++components;
        }
    }
    return components;
}

// --- DFS 응용 2: 사이클 탐지 (무방향 그래프) ---

bool has_cycle_helper(const Graph& g, int u, int parent,
                      std::vector<bool>& visited) {
    visited[u] = true;
    for (const auto& [v, w] : g[u]) {
        if (!visited[v]) {
            if (has_cycle_helper(g, v, u, visited)) return true;
        } else if (v != parent) {
            return true;  // 방문한 정점이 부모가 아니면 사이클
        }
    }
    return false;
}

bool has_cycle(const Graph& g) {
    int V = static_cast<int>(g.size());
    std::vector<bool> visited(V, false);
    for (int i = 0; i < V; ++i)
        if (!visited[i] && has_cycle_helper(g, i, -1, visited))
            return true;
    return false;
}

// --- DFS 응용 3: 위상 정렬 (방향 그래프 DAG) ---

void topo_dfs(const Graph& g, int u,
              std::vector<bool>& visited,
              std::vector<int>& result) {
    visited[u] = true;
    for (const auto& [v, w] : g[u])
        if (!visited[v]) topo_dfs(g, v, visited, result);
    result.push_back(u);  // 후위 순서로 추가
}

std::vector<int> topological_sort(const Graph& g) {
    int V = static_cast<int>(g.size());
    std::vector<bool> visited(V, false);
    std::vector<int>  result;
    for (int i = 0; i < V; ++i)
        if (!visited[i]) topo_dfs(g, i, visited, result);
    std::reverse(result.begin(), result.end());
    return result;
}

int main() {
    // 무방향 그래프 (6개 정점)
    const int V = 6;
    Graph undirected(V);
    auto add_ue = [&](int u, int v) {
        undirected[u].emplace_back(v, 1);
        undirected[v].emplace_back(u, 1);
    };
    add_ue(0, 1); add_ue(0, 2); add_ue(1, 3); add_ue(2, 4); add_ue(3, 4); add_ue(4, 5);

    std::vector<bool> visited(V, false);
    std::vector<int>  rec_order;
    dfs_recursive(undirected, 0, visited, rec_order);

    std::cout << "=== 무방향 그래프 DFS ===\n";
    std::cout << "재귀 DFS 방문 순서: ";
    for (int v : rec_order) std::cout << v << " ";
    std::cout << '\n';

    std::vector<int> iter_order = dfs_iterative(undirected, 0);
    std::cout << "반복 DFS 방문 순서: ";
    for (int v : iter_order) std::cout << v << " ";
    std::cout << '\n';

    std::cout << "연결 요소 수: " << count_components(undirected) << '\n';
    std::cout << "사이클 존재: " << (has_cycle(undirected) ? "예" : "아니오") << '\n';

    // DAG (방향 비순환 그래프) — 빌드 의존성 예시
    // 0: main.cpp, 1: utils.cpp, 2: io.cpp, 3: math.cpp, 4: config.cpp, 5: logger.cpp
    Graph dag(6);
    dag[0].emplace_back(1, 1);  // main → utils
    dag[0].emplace_back(2, 1);  // main → io
    dag[1].emplace_back(3, 1);  // utils → math
    dag[2].emplace_back(4, 1);  // io → config
    dag[3].emplace_back(5, 1);  // math → logger
    dag[4].emplace_back(5, 1);  // config → logger

    std::cout << "\n=== DAG 위상 정렬 (빌드 순서) ===\n";
    const std::string names[] = {"main", "utils", "io", "math", "config", "logger"};
    std::vector<int> topo = topological_sort(dag);
    std::cout << "빌드 순서: ";
    for (int v : topo) std::cout << names[v] << " → ";
    std::cout << "(완료)\n";

    return 0;
}

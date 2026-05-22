// filename: bfs.cpp
// g++ -std=c++17 -O2 -Wall -o bfs bfs.cpp
#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using Graph = std::vector<std::vector<std::pair<int,int>>>;
constexpr int INF = std::numeric_limits<int>::max();

// --- 기본 BFS (방문 순서 반환) ---

std::vector<int> bfs(const Graph& g, int start) {
    int V = static_cast<int>(g.size());
    std::vector<bool> visited(V, false);
    std::vector<int>  order;
    std::queue<int>   q;

    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (const auto& [v, w] : g[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
    return order;
}

// --- BFS 최단 경로 (가중치 없는 그래프) ---

struct BFSResult {
    std::vector<int> dist;   // 시작점으로부터 거리
    std::vector<int> prev;   // 경로 복원용 이전 정점
};

BFSResult bfs_shortest(const Graph& g, int start) {
    int V = static_cast<int>(g.size());
    BFSResult result;
    result.dist.assign(V, INF);
    result.prev.assign(V, -1);

    result.dist[start] = 0;
    std::queue<int> q;
    q.push(start);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (const auto& [v, w] : g[u]) {
            if (result.dist[v] == INF) {
                result.dist[v] = result.dist[u] + 1;
                result.prev[v] = u;
                q.push(v);
            }
        }
    }
    return result;
}

std::vector<int> reconstruct_path(const std::vector<int>& prev, int target) {
    std::vector<int> path;
    for (int v = target; v != -1; v = prev[v])
        path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
}

// --- BFS 응용: 이분 그래프 판별 ---

// 이분 그래프: 정점을 두 그룹으로 나눌 수 있고
// 같은 그룹의 정점끼리는 간선이 없음
bool is_bipartite(const Graph& g) {
    int V = static_cast<int>(g.size());
    std::vector<int> color(V, -1);  // -1: 미방문, 0/1: 두 색

    for (int start = 0; start < V; ++start) {
        if (color[start] != -1) continue;
        color[start] = 0;
        std::queue<int> q;
        q.push(start);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (const auto& [v, w] : g[u]) {
                if (color[v] == -1) {
                    color[v] = 1 - color[u];  // 반대 색
                    q.push(v);
                } else if (color[v] == color[u]) {
                    return false;  // 같은 색끼리 간선 → 이분 그래프 아님
                }
            }
        }
    }
    return true;
}

// --- BFS 레벨별 탐색 ---

void bfs_by_level(const Graph& g, int start) {
    int V = static_cast<int>(g.size());
    std::vector<bool> visited(V, false);
    std::queue<int>   q;
    visited[start] = true;
    q.push(start);
    int level = 0;

    while (!q.empty()) {
        int size = static_cast<int>(q.size());
        std::cout << "레벨 " << level << ": ";
        for (int i = 0; i < size; ++i) {
            int u = q.front(); q.pop();
            std::cout << u << " ";
            for (const auto& [v, w] : g[u])
                if (!visited[v]) { visited[v] = true; q.push(v); }
        }
        std::cout << '\n';
        ++level;
    }
}

int main() {
    const int V = 7;
    Graph g(V);
    auto add_ue = [&](int u, int v) {
        g[u].emplace_back(v, 1);
        g[v].emplace_back(u, 1);
    };
    add_ue(0,1); add_ue(0,2); add_ue(1,3); add_ue(1,4);
    add_ue(2,5); add_ue(2,6);

    std::cout << "=== BFS 방문 순서 ===\n";
    auto order = bfs(g, 0);
    for (int v : order) std::cout << v << " ";
    std::cout << '\n';

    std::cout << "\n=== 레벨별 탐색 ===\n";
    bfs_by_level(g, 0);

    std::cout << "\n=== 최단 경로 (0에서 각 정점) ===\n";
    auto [dist, prev] = bfs_shortest(g, 0);
    for (int v = 0; v < V; ++v) {
        std::cout << "0→" << v << ": 거리=" << dist[v] << "  경로: ";
        for (int p : reconstruct_path(prev, v)) std::cout << p << " ";
        std::cout << '\n';
    }

    // 이분 그래프 판별
    Graph bipartite_g(4);
    bipartite_g[0].emplace_back(1, 1); bipartite_g[1].emplace_back(0, 1);
    bipartite_g[0].emplace_back(3, 1); bipartite_g[3].emplace_back(0, 1);
    bipartite_g[2].emplace_back(1, 1); bipartite_g[1].emplace_back(2, 1);
    bipartite_g[2].emplace_back(3, 1); bipartite_g[3].emplace_back(2, 1);

    Graph non_bipartite_g(3);
    non_bipartite_g[0].emplace_back(1, 1); non_bipartite_g[1].emplace_back(0, 1);
    non_bipartite_g[1].emplace_back(2, 1); non_bipartite_g[2].emplace_back(1, 1);
    non_bipartite_g[2].emplace_back(0, 1); non_bipartite_g[0].emplace_back(2, 1);

    std::cout << "\n=== 이분 그래프 판별 ===\n";
    std::cout << "사각형 그래프(4정점): "
              << (is_bipartite(bipartite_g) ? "이분 그래프" : "아님") << '\n';
    std::cout << "삼각형 그래프(3정점): "
              << (is_bipartite(non_bipartite_g) ? "이분 그래프" : "아님") << '\n';

    return 0;
}

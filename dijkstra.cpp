// filename: dijkstra.cpp
// g++ -std=c++17 -O2 -Wall -o dijkstra dijkstra.cpp
#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using Graph = std::vector<std::vector<std::pair<int,int>>>;
constexpr int INF = std::numeric_limits<int>::max() / 2;

struct DijkstraResult {
    std::vector<int> dist;
    std::vector<int> prev;
};

DijkstraResult dijkstra(const Graph& g, int start) {
    int V = static_cast<int>(g.size());
    DijkstraResult result;
    result.dist.assign(V, INF);
    result.prev.assign(V, -1);
    result.dist[start] = 0;

    // min-heap: (거리, 정점)
    // greater<>를 사용해 최소 힙으로 만듦
    using PII = std::pair<int,int>;
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;
    pq.emplace(0, start);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();

        // 이미 더 짧은 경로로 처리된 경우 스킵 (느린 삭제)
        if (d > result.dist[u]) continue;

        for (const auto& [v, w] : g[u]) {
            int new_dist = result.dist[u] + w;
            if (new_dist < result.dist[v]) {
                result.dist[v] = new_dist;
                result.prev[v] = u;
                pq.emplace(new_dist, v);
            }
        }
    }
    return result;
}

std::vector<int> get_path(const std::vector<int>& prev, int target) {
    std::vector<int> path;
    for (int v = target; v != -1; v = prev[v])
        path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
}

// 음수 간선에서의 실패를 보여주는 예시
void demonstrate_negative_edge_failure() {
    // 정점 3개, 간선: 0→1(5), 0→2(2), 2→1(-4)
    // 실제 최단 경로 0→2→1: 2 + (-4) = -2
    // 다익스트라는 이를 찾지 못함
    Graph g(3);
    g[0].emplace_back(1, 5);
    g[0].emplace_back(2, 2);
    g[2].emplace_back(1, -4);  // 음수 간선

    auto [dist, prev] = dijkstra(g, 0);
    std::cout << "음수 간선 그래프 (다익스트라 잘못된 결과):\n";
    std::cout << "  0→1 다익스트라: " << dist[1]
              << " (실제 최단: " << -2 << ")\n";
    std::cout << "  → 음수 간선에는 벨만-포드를 사용해야 함\n";
}

int main() {
    // 6개 정점의 방향 가중치 그래프
    // (도시 간 거리 예시)
    const int V = 6;
    Graph g(V);
    // add_directed_edge
    auto add = [&](int u, int v, int w) { g[u].emplace_back(v, w); };

    add(0,1,7);  add(0,2,9);  add(0,5,14);
    add(1,2,10); add(1,3,15);
    add(2,3,11); add(2,5,2);
    add(3,4,6);
    add(4,5,9);

    const std::string cities[] = {"서울","수원","천안","대전","대구","부산"};

    std::cout << "=== 다익스트라 최단 경로 (서울 출발) ===\n";
    auto [dist, prev] = dijkstra(g, 0);

    for (int v = 0; v < V; ++v) {
        std::cout << cities[0] << " → " << cities[v] << ": ";
        if (dist[v] == INF) {
            std::cout << "도달 불가\n";
        } else {
            std::cout << "거리=" << dist[v] << "  경로: ";
            for (int p : get_path(prev, v))
                std::cout << cities[p] << " ";
            std::cout << '\n';
        }
    }

    std::cout << "\n";
    demonstrate_negative_edge_failure();

    // 대규모 희소 그래프에서의 성능 시연
    const int BIG_V = 10000;
    Graph big_g(BIG_V);
    // 각 정점에서 무작위 5개 간선 생성
    std::srand(42);
    for (int u = 0; u < BIG_V; ++u) {
        for (int i = 0; i < 5; ++i) {
            int v = std::rand() % BIG_V;
            int w = 1 + std::rand() % 100;
            if (v != u) big_g[u].emplace_back(v, w);
        }
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    auto [big_dist, big_prev] = dijkstra(big_g, 0);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "\n10000개 정점 다익스트라: " << ms << " ms\n";
    // chrono 포함 필요
    return 0;
}

// filename: dijkstra_full.cpp
// g++ -std=c++17 -O2 -Wall -o dijkstra_full dijkstra_full.cpp
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using Graph = std::vector<std::vector<std::pair<int,int>>>;
constexpr int INF = std::numeric_limits<int>::max() / 2;

std::pair<std::vector<int>, std::vector<int>> dijkstra(const Graph& g, int start) {
    int V = static_cast<int>(g.size());
    std::vector<int> dist(V, INF), prev(V, -1);
    dist[start] = 0;
    using PII = std::pair<int,int>;
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;
    pq.emplace(0, start);
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (const auto& [v, w] : g[u])
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.emplace(dist[v], v);
            }
    }
    return {dist, prev};
}

int main() {
    const int V = 6;
    Graph g(V);
    auto add = [&](int u, int v, int w) { g[u].emplace_back(v, w); };
    add(0,1,7); add(0,2,9); add(0,5,14);
    add(1,2,10); add(1,3,15);
    add(2,3,11); add(2,5,2);
    add(3,4,6); add(4,5,9);

    auto [dist, prev] = dijkstra(g, 0);
    for (int v = 0; v < V; ++v)
        std::cout << "0→" << v << ": " << (dist[v]==INF ? -1 : dist[v]) << '\n';
    return 0;
}

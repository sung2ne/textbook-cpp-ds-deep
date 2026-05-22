// filename: bellman_ford.cpp
// g++ -std=c++17 -O2 -Wall -o bellman_ford bellman_ford.cpp
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include <algorithm>

constexpr int INF = std::numeric_limits<int>::max() / 2;

struct Edge {
    int u, v, w;
};

struct BFResult {
    std::vector<int> dist;
    std::vector<int> prev;
    bool has_negative_cycle;
};

BFResult bellman_ford(int V, const std::vector<Edge>& edges, int start) {
    BFResult result;
    result.dist.assign(V, INF);
    result.prev.assign(V, -1);
    result.has_negative_cycle = false;
    result.dist[start] = 0;

    // V-1번 완화
    for (int i = 0; i < V - 1; ++i) {
        bool updated = false;
        for (const auto& [u, v, w] : edges) {
            if (result.dist[u] != INF && result.dist[u] + w < result.dist[v]) {
                result.dist[v] = result.dist[u] + w;
                result.prev[v] = u;
                updated = true;
            }
        }
        if (!updated) break;  // 조기 종료 최적화
    }

    // V번째 완화: 음수 사이클 탐지
    for (const auto& [u, v, w] : edges) {
        if (result.dist[u] != INF && result.dist[u] + w < result.dist[v]) {
            result.has_negative_cycle = true;
            break;
        }
    }

    return result;
}

// --- 플로이드-워셜 알고리즘 ---

struct FWResult {
    std::vector<std::vector<int>> dist;
    std::vector<std::vector<int>> next;  // 경로 복원용
};

FWResult floyd_warshall(int V, const std::vector<Edge>& edges) {
    FWResult result;
    result.dist.assign(V, std::vector<int>(V, INF));
    result.next.assign(V, std::vector<int>(V, -1));

    // 자기 자신으로의 거리는 0
    for (int i = 0; i < V; ++i) result.dist[i][i] = 0;

    // 간선 정보 초기화
    for (const auto& [u, v, w] : edges) {
        if (w < result.dist[u][v]) {
            result.dist[u][v] = w;
            result.next[u][v] = v;
        }
    }

    // DP: 경유지 k를 늘려가며 최단 경로 갱신
    for (int k = 0; k < V; ++k)
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                if (result.dist[i][k] != INF && result.dist[k][j] != INF &&
                    result.dist[i][k] + result.dist[k][j] < result.dist[i][j]) {
                    result.dist[i][j] = result.dist[i][k] + result.dist[k][j];
                    result.next[i][j] = result.next[i][k];
                }

    return result;
}

std::vector<int> fw_path(const FWResult& fw, int u, int v) {
    if (fw.next[u][v] == -1) return {};
    std::vector<int> path = {u};
    while (u != v) {
        u = fw.next[u][v];
        path.push_back(u);
    }
    return path;
}

int main() {
    // --- 벨만-포드 예시 ---
    // 음수 간선 포함 방향 그래프
    const int V = 5;
    std::vector<Edge> edges = {
        {0,1, 6}, {0,3, 7},
        {1,2, 5}, {1,3, 8}, {1,4,-4},
        {2,1,-2},
        {3,2,-3}, {3,4, 9},
        {4,0, 2}, {4,2, 7}
    };

    std::cout << "=== 벨만-포드 (음수 간선 포함) ===\n";
    auto bf = bellman_ford(V, edges, 0);
    std::cout << "음수 사이클: " << (bf.has_negative_cycle ? "있음" : "없음") << '\n';
    for (int v = 0; v < V; ++v)
        std::cout << "0→" << v << ": "
                  << (bf.dist[v] == INF ? "도달불가" : std::to_string(bf.dist[v]))
                  << '\n';

    // 음수 사이클 탐지 예시
    std::vector<Edge> neg_cycle_edges = {
        {0,1,1}, {1,2,-3}, {2,0,1}  // 0→1→2→0: 1-3+1 = -1 (음수 사이클)
    };
    auto bf2 = bellman_ford(3, neg_cycle_edges, 0);
    std::cout << "\n음수 사이클 그래프: "
              << (bf2.has_negative_cycle ? "사이클 탐지!" : "사이클 없음") << '\n';

    // --- 플로이드-워셜 예시 ---
    // 4개 도시 간 이동 비용 (무방향)
    const int FV = 4;
    std::vector<Edge> fw_edges = {
        {0,1,3}, {1,0,3},
        {0,3,7}, {3,0,7},
        {1,2,2}, {2,1,2},
        {1,3,1}, {3,1,1},
        {2,3,4}, {3,2,4}
    };

    std::cout << "\n=== 플로이드-워셜 (모든 쌍 최단 경로) ===\n";
    auto fw = floyd_warshall(FV, fw_edges);

    const std::string cities[] = {"A", "B", "C", "D"};
    for (int i = 0; i < FV; ++i)
        for (int j = 0; j < FV; ++j) {
            if (i == j) continue;
            std::cout << cities[i] << "→" << cities[j] << ": ";
            if (fw.dist[i][j] == INF) {
                std::cout << "도달불가\n";
            } else {
                std::cout << "거리=" << fw.dist[i][j] << "  경로: ";
                for (int p : fw_path(fw, i, j)) std::cout << cities[p] << " ";
                std::cout << '\n';
            }
        }

    // 음수 사이클 탐지 (대각선 원소 확인)
    std::cout << "\n대각선 음수 여부(음수 사이클 지표): ";
    bool neg = false;
    for (int i = 0; i < FV; ++i) if (fw.dist[i][i] < 0) { neg = true; break; }
    std::cout << (neg ? "있음" : "없음") << '\n';

    return 0;
}

// filename: graph_representation.cpp
// g++ -std=c++17 -O2 -Wall -o graph_representation graph_representation.cpp
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

// --- 인접 행렬 ---

class AdjacencyMatrix {
    int V_;
    std::vector<std::vector<int>> mat_;
    static constexpr int INF = std::numeric_limits<int>::max() / 2;

public:
    explicit AdjacencyMatrix(int V)
        : V_(V), mat_(V, std::vector<int>(V, 0)) {}

    void add_edge(int u, int v, int w = 1, bool directed = false) {
        mat_[u][v] = w;
        if (!directed) mat_[v][u] = w;
    }

    bool has_edge(int u, int v) const { return mat_[u][v] != 0; }
    int  weight  (int u, int v) const { return mat_[u][v]; }
    int  vertices()             const { return V_; }

    void print() const {
        std::cout << "인접 행렬:\n   ";
        for (int i = 0; i < V_; ++i) std::cout << std::setw(4) << i;
        std::cout << '\n';
        for (int i = 0; i < V_; ++i) {
            std::cout << std::setw(3) << i;
            for (int j = 0; j < V_; ++j)
                std::cout << std::setw(4) << mat_[i][j];
            std::cout << '\n';
        }
    }

    std::size_t memory_bytes() const {
        return static_cast<std::size_t>(V_) * V_ * sizeof(int);
    }
};

// --- 인접 리스트 ---

class AdjacencyList {
    int V_;
    std::vector<std::vector<std::pair<int,int>>> adj_;  // adj_[u] = [(v, w), ...]

public:
    explicit AdjacencyList(int V) : V_(V), adj_(V) {}

    void add_edge(int u, int v, int w = 1, bool directed = false) {
        adj_[u].emplace_back(v, w);
        if (!directed) adj_[v].emplace_back(u, w);
    }

    const std::vector<std::pair<int,int>>& neighbors(int u) const {
        return adj_[u];
    }

    bool has_edge(int u, int v) const {
        for (const auto& [nv, nw] : adj_[u])
            if (nv == v) return true;
        return false;
    }

    int vertices() const { return V_; }
    int edges() const {
        return std::accumulate(adj_.begin(), adj_.end(), 0,
            [](int s, const auto& v) { return s + static_cast<int>(v.size()); }) / 2;
    }

    void print() const {
        std::cout << "인접 리스트:\n";
        for (int u = 0; u < V_; ++u) {
            std::cout << u << " → ";
            for (const auto& [v, w] : adj_[u])
                std::cout << "(" << v << "," << w << ") ";
            std::cout << '\n';
        }
    }

    std::size_t memory_bytes() const {
        std::size_t total = adj_.capacity() * sizeof(std::vector<std::pair<int,int>>);
        for (const auto& vec : adj_)
            total += vec.capacity() * sizeof(std::pair<int,int>);
        return total;
    }
};

int main() {
    // 7개 정점, 9개 간선의 무방향 가중치 그래프
    const int V = 7;
    const std::vector<std::tuple<int,int,int>> edges = {
        {0,1,4}, {0,2,3}, {1,2,1}, {1,3,2},
        {2,4,6}, {3,4,2}, {3,5,3}, {4,6,5}, {5,6,1}
    };

    AdjacencyMatrix am(V);
    AdjacencyList   al(V);

    for (const auto& [u, v, w] : edges) {
        am.add_edge(u, v, w);
        al.add_edge(u, v, w);
    }

    am.print();
    std::cout << '\n';
    al.print();

    std::cout << "\n=== 메모리 사용량 비교 ===\n";
    std::cout << "인접 행렬: " << am.memory_bytes() << " bytes ("
              << V << "×" << V << " int 배열)\n";
    std::cout << "인접 리스트: ~" << al.memory_bytes() << " bytes ("
              << V << " 정점, " << al.edges() << " 간선)\n";

    std::cout << "\n=== 간선 확인 속도 차이 ===\n";
    std::cout << "행렬 has_edge(0,6): O(1) = " << am.has_edge(0,6) << '\n';
    std::cout << "리스트 has_edge(0,6): O(degree) = " << al.has_edge(0,6) << '\n';
    std::cout << "행렬 has_edge(0,1): O(1) = " << am.has_edge(0,1) << '\n';
    std::cout << "리스트 has_edge(0,1): O(degree) = " << al.has_edge(0,1) << '\n';

    return 0;
}

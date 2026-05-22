// filename: kruskal_mst.cpp
// g++ -std=c++17 -O2 -Wall -o kruskal_mst kruskal_mst.cpp
#include <algorithm>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

// --- Union-Find (Disjoint Set Union) ---

class DSU {
    std::vector<int> parent_, rank_;

public:
    explicit DSU(int n) : parent_(n), rank_(n, 0) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    int find(int x) {
        if (parent_[x] != x)
            parent_[x] = find(parent_[x]);  // path compression
        return parent_[x];
    }

    bool unite(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;  // 이미 같은 집합 (사이클)
        // union by rank
        if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
        parent_[ry] = rx;
        if (rank_[rx] == rank_[ry]) ++rank_[rx];
        return true;
    }

    bool same(int x, int y) { return find(x) == find(y); }
};

struct Edge {
    int u, v, w;
    bool operator<(const Edge& o) const { return w < o.w; }
};

// 크루스칼 MST
std::vector<Edge> kruskal(int V, std::vector<Edge> edges) {
    std::sort(edges.begin(), edges.end());  // 가중치 오름차순 정렬
    DSU dsu(V);
    std::vector<Edge> mst;
    mst.reserve(V - 1);

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            mst.push_back(e);
            if (static_cast<int>(mst.size()) == V - 1) break;  // MST 완성
        }
    }
    return mst;
}

// --- 프림 알고리즘 ---

using Graph = std::vector<std::vector<std::pair<int,int>>>;

std::vector<Edge> prim(const Graph& g) {
    int V = static_cast<int>(g.size());
    std::vector<bool> in_mst(V, false);
    // (가중치, to, from)
    using Triple = std::tuple<int,int,int>;
    std::priority_queue<Triple, std::vector<Triple>, std::greater<Triple>> pq;

    in_mst[0] = true;
    for (const auto& [v, w] : g[0]) pq.emplace(w, v, 0);

    std::vector<Edge> mst;
    while (!pq.empty() && static_cast<int>(mst.size()) < V - 1) {
        auto [w, v, u] = pq.top(); pq.pop();
        if (in_mst[v]) continue;
        in_mst[v] = true;
        mst.push_back({u, v, w});
        for (const auto& [nv, nw] : g[v])
            if (!in_mst[nv]) pq.emplace(nw, nv, v);
    }
    return mst;
}

int main() {
    // 7개 정점의 무방향 가중치 그래프
    const int V = 7;
    std::vector<Edge> edges = {
        {0,1,7}, {0,3,5},
        {1,2,8}, {1,3,9}, {1,4,7},
        {2,4,5},
        {3,4,15}, {3,5,6},
        {4,5,8}, {4,6,9},
        {5,6,11}
    };

    // 크루스칼
    std::cout << "=== 크루스칼 MST ===\n";
    auto mst_k = kruskal(V, edges);
    int total_k = 0;
    for (const auto& e : mst_k) {
        std::cout << e.u << " — " << e.v << "  (가중치 " << e.w << ")\n";
        total_k += e.w;
    }
    std::cout << "총 비용: " << total_k << '\n';

    // 인접 리스트 구성
    Graph g(V);
    for (const auto& e : edges) {
        g[e.u].emplace_back(e.v, e.w);
        g[e.v].emplace_back(e.u, e.w);
    }

    // 프림
    std::cout << "\n=== 프림 MST ===\n";
    auto mst_p = prim(g);
    int total_p = 0;
    for (const auto& e : mst_p) {
        std::cout << e.u << " — " << e.v << "  (가중치 " << e.w << ")\n";
        total_p += e.w;
    }
    std::cout << "총 비용: " << total_p << '\n';

    std::cout << "\n두 알고리즘의 총 비용 일치: "
              << (total_k == total_p ? "예" : "아니오") << '\n';

    // Union-Find 단독 테스트
    std::cout << "\n=== Union-Find 테스트 ===\n";
    DSU dsu(5);
    dsu.unite(0, 1);
    dsu.unite(1, 2);
    std::cout << "0과 2 같은 집합: " << (dsu.same(0,2) ? "예" : "아니오") << '\n';
    std::cout << "0과 3 같은 집합: " << (dsu.same(0,3) ? "예" : "아니오") << '\n';
    std::cout << "0과 1 unite(사이클 체크): " << (dsu.unite(0,2) ? "성공" : "사이클") << '\n';

    return 0;
}

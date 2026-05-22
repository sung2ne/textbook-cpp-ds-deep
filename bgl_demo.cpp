// filename: bgl_demo.cpp
// Boost 설치 필요: sudo apt install libboost-dev  또는
//   brew install boost  (macOS)
// g++ -std=c++17 -O2 -o bgl_demo bgl_demo.cpp -I/usr/include/boost
// (Boost는 헤더 전용이므로 별도 링크 불필요)

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>

// 가중치 간선을 가진 무방향 그래프 타입 정의
using Graph = boost::adjacency_list<
    boost::vecS,        // 간선 컨테이너: std::vector
    boost::vecS,        // 정점 컨테이너: std::vector
    boost::undirectedS, // 무방향
    boost::no_property, // 정점 속성 없음
    boost::property<boost::edge_weight_t, int>  // 간선 속성: 가중치(int)
>;

using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
using Edge   = boost::graph_traits<Graph>::edge_descriptor;

// DFS Visitor 예시: 방문 순서 기록
class MyDFSVisitor : public boost::default_dfs_visitor {
public:
    explicit MyDFSVisitor(std::vector<Vertex>& order) : order_(order) {}

    void discover_vertex(Vertex v, const Graph&) {
        order_.push_back(v);
    }
private:
    std::vector<Vertex>& order_;
};

int main() {
    // 그래프 구성 (6개 정점)
    Graph g;
    auto weight_map = boost::get(boost::edge_weight, g);

    // 간선 추가: add_edge(u, v, graph) → 간선 디스크립터 반환
    auto add = [&](int u, int v, int w) {
        auto [e, ok] = boost::add_edge(u, v, g);
        weight_map[e] = w;
    };

    add(0,1,7); add(0,2,9); add(0,5,14);
    add(1,2,10); add(1,3,15);
    add(2,3,11); add(2,5,2);
    add(3,4,6);  add(4,5,9);

    int V = static_cast<int>(boost::num_vertices(g));
    std::cout << "정점 수: " << V
              << "  간선 수: " << boost::num_edges(g) << '\n';

    // --- 다익스트라 최단 경로 ---
    std::vector<int>    dist(V, 0);
    std::vector<Vertex> prev(V, 0);

    boost::dijkstra_shortest_paths(g, 0,
        boost::distance_map(boost::make_iterator_property_map(
            dist.begin(), boost::get(boost::vertex_index, g)))
        .predecessor_map(boost::make_iterator_property_map(
            prev.begin(), boost::get(boost::vertex_index, g)))
    );

    std::cout << "\n=== BGL 다익스트라 (정점 0 출발) ===\n";
    for (int v = 0; v < V; ++v)
        std::cout << "0→" << v << ": " << dist[v] << '\n';

    // --- MST (크루스칼) ---
    std::vector<Edge> spanning_tree;
    boost::kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));

    std::cout << "\n=== BGL 크루스칼 MST ===\n";
    int total = 0;
    for (const auto& e : spanning_tree) {
        int u = static_cast<int>(boost::source(e, g));
        int v = static_cast<int>(boost::target(e, g));
        int w = weight_map[e];
        std::cout << u << " — " << v << "  (가중치 " << w << ")\n";
        total += w;
    }
    std::cout << "총 비용: " << total << '\n';

    // --- DFS Visitor ---
    std::vector<Vertex> dfs_order;
    MyDFSVisitor visitor(dfs_order);
    boost::depth_first_search(g, boost::visitor(visitor));

    std::cout << "\n=== BGL DFS 방문 순서 ===\n";
    for (Vertex v : dfs_order) std::cout << v << " ";
    std::cout << '\n';

    return 0;
}

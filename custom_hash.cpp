// filename: custom_hash.cpp
// g++ -std=c++17 -O2 -Wall -o custom_hash custom_hash.cpp
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <tuple>
#include <utility>

// --- 해시 결합 유틸리티 ---

template <typename T>
void hash_combine(std::size_t& seed, const T& val) {
    seed ^= std::hash<T>{}(val)
         + 0x9e3779b9ULL
         + (seed << 6)
         + (seed >> 2);
}

template <typename... Args>
std::size_t make_hash(const Args&... args) {
    std::size_t seed = 0;
    (hash_combine(seed, args), ...);  // C++17 fold expression
    return seed;
}

// --- 사용자 정의 타입 ---

struct Point {
    int x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};

struct Edge {
    std::string from, to;
    double weight;
    bool operator==(const Edge& o) const {
        return from == o.from && to == o.to && weight == o.weight;
    }
};

// --- 방법 1: std::hash 특수화 (std namespace에 주입) ---

namespace std {
    template <>
    struct hash<Point> {
        std::size_t operator()(const Point& p) const noexcept {
            std::size_t seed = 0;
            hash_combine(seed, p.x);
            hash_combine(seed, p.y);
            return seed;
        }
    };

    template <>
    struct hash<Edge> {
        std::size_t operator()(const Edge& e) const noexcept {
            return make_hash(e.from, e.to, e.weight);
        }
    };
}

// --- 방법 2: 커스텀 해시 함수 객체 (템플릿 인자로 전달) ---

struct PointHash {
    std::size_t operator()(const Point& p) const noexcept {
        return make_hash(p.x, p.y);
    }
};

// --- pair 해시 (표준은 pair 해시 미제공) ---

struct PairHash {
    template <typename A, typename B>
    std::size_t operator()(const std::pair<A, B>& p) const noexcept {
        return make_hash(p.first, p.second);
    }
};

// --- C++20 방식: 해시 결합 + 범용 튜플 해시 ---

struct TupleHash {
    template <typename... Ts>
    std::size_t operator()(const std::tuple<Ts...>& t) const noexcept {
        std::size_t seed = 0;
        std::apply([&seed](const auto&... args) {
            (hash_combine(seed, args), ...);
        }, t);
        return seed;
    }
};

int main() {
    // std::hash 특수화로 unordered_map에 Point 키 사용
    std::unordered_map<Point, std::string> grid;
    grid[{0, 0}] = "origin";
    grid[{1, 0}] = "east";
    grid[{0, 1}] = "north";
    grid[{1, 2}] = "northeast";

    std::cout << "=== Point 키 unordered_map ===\n";
    for (const auto& [pt, name] : grid)
        std::cout << "(" << pt.x << "," << pt.y << ") → " << name << '\n';

    // (1,2)와 (2,1)의 해시가 다른지 확인
    std::hash<Point> h;
    std::cout << "\nhash(1,2) = " << h({1, 2}) << '\n';
    std::cout << "hash(2,1) = " << h({2, 1}) << '\n';
    std::cout << "해시 값 다름: " << (h({1,2}) != h({2,1}) ? "예" : "아니오") << '\n';

    // 커스텀 해시 함수 객체 사용
    std::unordered_set<Point, PointHash> point_set;
    point_set.insert({3, 4});
    point_set.insert({4, 3});
    point_set.insert({3, 4});  // 중복
    std::cout << "\npoint_set 크기 (중복 제거): " << point_set.size() << '\n';

    // pair 키 unordered_map
    std::unordered_map<std::pair<int,int>, std::string, PairHash> pair_map;
    pair_map[{10, 20}] = "A";
    pair_map[{20, 10}] = "B";
    std::cout << "\npair_map[{10,20}] = " << pair_map[{10,20}] << '\n';
    std::cout << "pair_map[{20,10}] = " << pair_map[{20,10}] << '\n';

    // tuple 키 unordered_map (3개 필드)
    using Key3 = std::tuple<int, std::string, double>;
    std::unordered_map<Key3, int, TupleHash> tuple_map;
    tuple_map[{1, "hello", 3.14}] = 42;
    std::cout << "\ntuple_map[{1,\"hello\",3.14}] = "
              << tuple_map[{1, "hello", 3.14}] << '\n';

    // Edge 해시 (std::hash 특수화)
    std::unordered_set<Edge> edge_set;
    edge_set.insert({"A", "B", 1.5});
    edge_set.insert({"B", "A", 1.5});  // from/to 반대 → 다른 키
    edge_set.insert({"A", "B", 1.5});  // 완전 중복
    std::cout << "\nedge_set 크기: " << edge_set.size() << " (A→B와 B→A는 다름)\n";

    return 0;
}

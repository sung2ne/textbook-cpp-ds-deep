// filename: maze_bfs.cpp
// g++ -std=c++17 -O2 -Wall -o maze_bfs maze_bfs.cpp
#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
#include <string>

using Grid   = std::vector<std::vector<int>>;
using Pos    = std::pair<int,int>;
constexpr int WALL    = 1;
constexpr int EMPTY   = 0;
constexpr int INF     = 1e9;
constexpr int PATH    = 2;

// 상하좌우 이동 방향
const int DR[] = {-1, 1, 0, 0};
const int DC[] = { 0, 0,-1, 1};

struct BFSMazeResult {
    int              dist;   // 최단 거리 (-1: 도달 불가)
    std::vector<Pos> path;   // 최단 경로 좌표 목록
};

BFSMazeResult bfs_maze(const Grid& maze, Pos start, Pos goal) {
    int R = static_cast<int>(maze.size());
    int C = static_cast<int>(maze[0].size());

    std::vector<std::vector<int>>  d(R, std::vector<int>(C, INF));
    std::vector<std::vector<Pos>>  prev(R, std::vector<Pos>(C, {-1,-1}));
    std::queue<Pos> q;

    d[start.first][start.second] = 0;
    q.push(start);

    while (!q.empty()) {
        auto [r, c] = q.front(); q.pop();
        if (Pos{r,c} == goal) break;

        for (int dir = 0; dir < 4; ++dir) {
            int nr = r + DR[dir];
            int nc = c + DC[dir];
            if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;
            if (maze[nr][nc] == WALL) continue;
            if (d[nr][nc] != INF) continue;
            d[nr][nc] = d[r][c] + 1;
            prev[nr][nc] = {r, c};
            q.emplace(nr, nc);
        }
    }

    if (d[goal.first][goal.second] == INF) return {-1, {}};

    // 경로 복원
    std::vector<Pos> path;
    for (Pos cur = goal; cur != Pos{-1,-1}; cur = prev[cur.first][cur.second])
        path.push_back(cur);
    std::reverse(path.begin(), path.end());

    return {d[goal.first][goal.second], path};
}

void print_maze(Grid maze, const std::vector<Pos>& path) {
    // 경로 표시
    for (const auto& [r, c] : path)
        if (maze[r][c] == EMPTY) maze[r][c] = PATH;

    for (const auto& row : maze) {
        for (int cell : row) {
            if (cell == WALL)  std::cout << "██";
            else if (cell == PATH) std::cout << "··";
            else               std::cout << "  ";
        }
        std::cout << '\n';
    }
}

int main() {
    // 10×15 미로 (0=통로, 1=벽)
    Grid maze = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
        {1,1,1,0,1,0,1,1,1,0,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,1,0,1,0,0,0,1},
        {1,0,1,1,1,1,1,0,1,0,0,0,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    Pos start = {1, 1};
    Pos goal  = {8, 13};

    auto result = bfs_maze(maze, start, goal);

    std::cout << "=== 미로 BFS 최단 경로 ===\n";
    if (result.dist < 0) {
        std::cout << "경로 없음\n";
    } else {
        std::cout << "최단 거리: " << result.dist << " 칸\n";
        std::cout << "경로 좌표 수: " << result.path.size() << '\n';
        std::cout << "\n";
        print_maze(maze, result.path);
    }

    return 0;
}

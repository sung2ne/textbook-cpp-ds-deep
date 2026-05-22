// filename: oop_monster.cpp
// g++ -std=c++17 -O2 -o oop_monster oop_monster.cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

struct Monster {
    // 자주 쓰는 필드 (핫 데이터)
    float x, y, z;      // 위치
    float vx, vy, vz;   // 속도
    float hp;           // 체력
    bool  alive;

    // 드물게 쓰는 필드 (콜드 데이터)
    char  name[64];     // 이름
    int   sprite_id;    // 스프라이트 ID
    float drop_table[8]; // 드롭 테이블 (잘 변하지 않음)
    // 총 구조체 크기: 약 128바이트 (cache line 2개)
};

void update_oop(std::vector<Monster>& monsters, float dt)
{
    for (auto& m : monsters) {
        if (!m.alive) continue;
        m.x += m.vx * dt;
        m.y += m.vy * dt;
        m.z += m.vz * dt;
    }
}

int main()
{
    constexpr int N = 1'000'000;
    std::vector<Monster> monsters(N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (auto& m : monsters) {
        m.x = dist(rng); m.y = dist(rng); m.z = dist(rng);
        m.vx = dist(rng); m.vy = dist(rng); m.vz = dist(rng);
        m.hp = 100.0f;
        m.alive = true;
    }

    float dt = 0.016f; // 60fps
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int frame = 0; frame < 100; ++frame)
        update_oop(monsters, dt);
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "OOP: " << ms << " ms (100 frames, " << N << " monsters)\n";
    return 0;
}

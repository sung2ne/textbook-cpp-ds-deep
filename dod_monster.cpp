// filename: dod_monster.cpp
// g++ -std=c++17 -O2 -o dod_monster dod_monster.cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

// 핫 데이터: 매 프레임 접근
struct MonsterHot {
    float x, y, z;
    float vx, vy, vz;
    float hp;
    bool  alive;
    // 크기: 29바이트 → 패딩 포함 약 32바이트
};

// 콜드 데이터: 이벤트 시에만 접근
struct MonsterCold {
    char  name[64];
    int   sprite_id;
    float drop_table[8];
};

void update_dod(std::vector<MonsterHot>& hot, float dt)
{
    for (auto& m : hot) {
        if (!m.alive) continue;
        m.x += m.vx * dt;
        m.y += m.vy * dt;
        m.z += m.vz * dt;
    }
}

int main()
{
    constexpr int N = 1'000'000;
    std::vector<MonsterHot>  hot(N);
    std::vector<MonsterCold> cold(N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (int i = 0; i < N; ++i) {
        hot[i] = { dist(rng), dist(rng), dist(rng),
                   dist(rng), dist(rng), dist(rng),
                   100.0f, true };
    }

    float dt = 0.016f;
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int frame = 0; frame < 100; ++frame)
        update_dod(hot, dt);
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "DOD: " << ms << " ms (100 frames, " << N << " monsters)\n";
    return 0;
}

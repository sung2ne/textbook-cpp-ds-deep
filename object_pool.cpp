// filename: object_pool.cpp
// g++ -std=c++17 -O2 -Wall -o object_pool object_pool.cpp

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <cassert>
#include <functional>

// ------------------------------------------------------------
// ObjectPool<T>: acquire/release 패턴
// ------------------------------------------------------------
template <typename T>
class ObjectPool {
public:
    explicit ObjectPool(std::size_t initial_size = 32) {
        expand(initial_size);
    }

    ~ObjectPool() {
        // 모든 슬롯 해제 (free list 순회 불필요 — 블록별 해제)
        for (auto& block : blocks_)
            std::free(block);
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // T 객체 하나를 얻음 (생성자 인자 완벽 전달)
    template <typename... Args>
    [[nodiscard]] T* acquire(Args&&... args) {
        if (!free_head_) expand(capacity_);  // 자동 확장

        Slot* slot = free_head_;
        free_head_ = free_head_->next;
        ++in_use_;

        // placement new: 기존 메모리에 T 객체 생성
        return new (slot) T(std::forward<Args>(args)...);
    }

    // T 객체를 풀에 반환
    void release(T* ptr) noexcept {
        assert(ptr != nullptr);
        ptr->~T();  // 소멸자 명시적 호출

        // 반환된 메모리를 free list에 연결
        Slot* slot = reinterpret_cast<Slot*>(ptr);
        slot->next = free_head_;
        free_head_ = slot;
        --in_use_;
    }

    std::size_t in_use()    const noexcept { return in_use_; }
    std::size_t capacity()  const noexcept { return capacity_; }
    std::size_t available() const noexcept { return capacity_ - in_use_; }

private:
    // Slot: T와 같은 공간을 next 포인터로 재사용
    union Slot {
        Slot* next;
        alignas(T) char storage[sizeof(T)];
    };
    static_assert(sizeof(Slot) >= sizeof(T));

    void expand(std::size_t count) {
        std::size_t bytes = sizeof(Slot) * count;
        Slot* block = static_cast<Slot*>(std::aligned_alloc(alignof(Slot), bytes));
        if (!block) throw std::bad_alloc{};
        blocks_.push_back(block);

        // 새 슬롯들을 free list 앞에 연결
        for (std::size_t i = 0; i < count; ++i) {
            block[i].next = free_head_;
            free_head_ = &block[i];
        }
        capacity_ += count;
    }

    Slot*              free_head_ = nullptr;
    std::vector<Slot*> blocks_;
    std::size_t        capacity_  = 0;
    std::size_t        in_use_    = 0;
};

// ------------------------------------------------------------
// 게임 파티클 시뮬레이션 예제
// ------------------------------------------------------------
struct Particle {
    float x, y, z;          // 위치
    float vx, vy, vz;       // 속도
    float life;              // 남은 수명
    int   color;

    Particle() = default;
    Particle(float x_, float y_, float z_, float vx_, float vy_, float vz_,
             float life_, int color_)
        : x(x_), y(y_), z(z_), vx(vx_), vy(vy_), vz(vz_)
        , life(life_), color(color_) {}

    void update(float dt) {
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;
        life -= dt;
    }

    bool is_dead() const noexcept { return life <= 0.0f; }
};

int main() {
    // --- ObjectPool 기본 동작 ---
    std::cout << "=== ObjectPool<Particle> ===\n";
    ObjectPool<Particle> particle_pool{64};

    std::cout << "초기 capacity: " << particle_pool.capacity() << '\n';
    std::cout << "초기 available: " << particle_pool.available() << '\n';

    // 파티클 발사
    std::vector<Particle*> active;
    for (int i = 0; i < 10; ++i) {
        Particle* p = particle_pool.acquire(
            0.0f, 0.0f, 0.0f,          // 위치
            static_cast<float>(i) * 0.1f, 1.0f, 0.0f,  // 속도
            2.0f + i * 0.1f,            // 수명
            i * 100                      // 색상
        );
        active.push_back(p);
    }

    std::cout << "10개 발사 후 in_use: " << particle_pool.in_use() << '\n';

    // 1프레임 업데이트 + 죽은 파티클 반환
    float dt = 0.5f;
    auto it = active.begin();
    while (it != active.end()) {
        (*it)->update(dt);
        if ((*it)->is_dead()) {
            particle_pool.release(*it);  // 풀에 반환
            it = active.erase(it);
        } else {
            ++it;
        }
    }

    std::cout << "0.5초 후 살아있는 파티클: " << active.size() << '\n';
    std::cout << "반환 후 available: " << particle_pool.available() << '\n';

    // 반환된 슬롯 재사용
    Particle* recycled = particle_pool.acquire(1.0f, 1.0f, 1.0f,
                                               0.0f, -1.0f, 0.0f,
                                               3.0f, 0xFF0000);
    std::cout << "재활용 파티클 life: " << recycled->life << '\n';
    particle_pool.release(recycled);

    // --- 성능 비교: new/delete vs ObjectPool ---
    std::cout << "\n=== new/delete vs ObjectPool 성능 비교 ===\n";
    constexpr int N = 100000;

    auto bench = [&](const char* label, auto alloc_fn, auto free_fn) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto* p = alloc_fn(i);
            free_fn(p);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << label << ": " << ns / N << " ns/op\n";
        return ns;
    };

    long long t_new = bench("new/delete ",
        [](int i) { return new Particle(0, 0, 0, i*0.1f, 1, 0, 2, i); },
        [](Particle* p) { delete p; }
    );

    ObjectPool<Particle> bench_pool{N + 16};
    long long t_pool = bench("ObjectPool ",
        [&](int i) { return bench_pool.acquire(0, 0, 0, i*0.1f, 1, 0, 2, i); },
        [&](Particle* p) { bench_pool.release(p); }
    );

    std::cout << "속도 향상: " << static_cast<double>(t_new) / t_pool << "x\n";

    // 나머지 active 파티클 반환
    for (auto* p : active) particle_pool.release(p);

    return 0;
}

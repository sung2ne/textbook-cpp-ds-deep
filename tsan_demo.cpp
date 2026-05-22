// filename: tsan_demo.cpp
// g++ -std=c++17 -g -fsanitize=thread -o tsan_demo tsan_demo.cpp -pthread

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

// ============================================================
// 예제 1: 명백한 데이터 레이스
// ============================================================
int global_counter = 0;  // 보호 없음!

void racy_increment(int n) {
    for (int i = 0; i < n; ++i) {
        ++global_counter;  // 비원자적 read-modify-write: 레이스!
    }
}

void demo_racy() {
    std::cout << "[demo_racy] 시작\n";
    global_counter = 0;

    std::thread t1(racy_increment, 100000);
    std::thread t2(racy_increment, 100000);
    t1.join();
    t2.join();

    // 기대: 200000, 실제: 무작위 (레이스로 인한 손실)
    std::cout << "[demo_racy] 결과: " << global_counter
              << " (기대: 200000, 손실 가능)\n";
}

// ============================================================
// 예제 2: 수정된 버전 — std::atomic
// ============================================================
std::atomic<int> atomic_counter{0};

void atomic_increment(int n) {
    for (int i = 0; i < n; ++i)
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
}

void demo_atomic() {
    std::cout << "[demo_atomic] 시작\n";
    atomic_counter = 0;

    std::thread t1(atomic_increment, 100000);
    std::thread t2(atomic_increment, 100000);
    t1.join();
    t2.join();

    std::cout << "[demo_atomic] 결과: " << atomic_counter.load()
              << " (기대: 200000)\n";
}

// ============================================================
// 예제 3: 더 미묘한 레이스 — 초기화 완료 전 읽기
// ============================================================
struct Config {
    int timeout = 0;
    std::string host;
    bool ready = false;
};

Config g_config;

void producer() {
    g_config.timeout = 5000;
    g_config.host    = "localhost";
    g_config.ready   = true;  // 여기가 문제: 다른 쓰기들이 아직 전파 안 됐을 수 있음
}

void consumer() {
    // g_config.ready를 확인하지만 동기화가 없음
    while (!g_config.ready) {}           // spin-wait
    std::cout << "host: " << g_config.host << '\n';  // 레이스!
}

// 수정 버전: std::atomic<bool>로 ready flag
std::atomic<bool> g_ready{false};
Config g_config2;

void safe_producer() {
    g_config2.timeout = 5000;
    g_config2.host    = "localhost";
    g_ready.store(true, std::memory_order_release);  // release: 이전 쓰기 모두 보임
}

void safe_consumer() {
    while (!g_ready.load(std::memory_order_acquire)) {}  // acquire: producer의 쓰기 모두 보임
    std::cout << "safe host: " << g_config2.host << '\n';
}

// ============================================================
// 예제 4: mutex로 보호된 카운터 (TSan 통과)
// ============================================================
int mutex_counter = 0;
std::mutex counter_mutex;

void safe_increment(int n) {
    for (int i = 0; i < n; ++i) {
        std::lock_guard<std::mutex> lk{counter_mutex};
        ++mutex_counter;
    }
}

void demo_mutex() {
    std::cout << "[demo_mutex] 시작\n";
    mutex_counter = 0;

    std::thread t1(safe_increment, 10000);
    std::thread t2(safe_increment, 10000);
    t1.join();
    t2.join();

    std::cout << "[demo_mutex] 결과: " << mutex_counter
              << " (기대: 20000)\n";
}

int main() {
    // 주의: demo_racy()는 TSan 실행 시 레포트를 출력하고 종료할 수 있음
    // TSan 리포트를 보려면 이 함수만 실행:
    // demo_racy();

    demo_atomic();
    demo_mutex();

    // safe producer/consumer
    {
        std::thread t1(safe_producer);
        std::thread t2(safe_consumer);
        t1.join();
        t2.join();
    }

    return 0;
}

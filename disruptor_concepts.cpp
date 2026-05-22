// filename: disruptor_concepts.cpp
// g++ -std=c++17 -O2 -o disruptor disruptor_concepts.cpp
// PART 13에서 다룬 HFT 패턴의 핵심 아이디어

#include <atomic>
#include <array>
#include <cstddef>

// 1. Ring Buffer (2^N 크기 — 모듈로 연산을 AND로 대체)
constexpr std::size_t BUFFER_SIZE = 1 << 16;  // 65536
constexpr std::size_t MASK = BUFFER_SIZE - 1;

// 2. Sequence (atomic, cache-line 패딩)
struct alignas(64) Sequence {
    std::atomic<long> value{-1};
    // 64 - sizeof(atomic<long>) = 56 bytes padding
    // 한 캐시 라인을 독점해 false sharing 차단
    char padding[64 - sizeof(std::atomic<long>)];
};

// 3. 링 버퍼 슬롯
template<typename T>
struct alignas(64) Slot {
    T data;
    char padding[64 - sizeof(T) % 64 == 0 ? 64 : 64 - sizeof(T) % 64];
};

// 4. Wait Strategy — BusySpin이 가장 빠름 (CPU 점유 100%)
// Yielding, Sleeping 전략도 있으나 지연 증가
// 5. Barrier — 다운스트림 소비자 Sequence 추적

int main() {
    Sequence producer_seq;
    producer_seq.value.store(0, std::memory_order_release);
    return 0;
}

// filename: pmr_parser.cpp
// g++ -std=c++17 -O2 -Wall -o pmr_parser pmr_parser.cpp

#include <iostream>
#include <memory_resource>
#include <vector>
#include <string>
#include <variant>
#include <chrono>

// JSON 파서 시뮬레이션: AST 노드 타입
struct JsonNull   {};
struct JsonBool   { bool value; };
struct JsonNumber { double value; };
struct JsonString { std::pmr::string value; };

struct JsonValue;

using JsonArray  = std::pmr::vector<JsonValue>;
using JsonObject = std::pmr::vector<std::pair<std::pmr::string, JsonValue>>;

struct JsonValue {
    using Storage = std::variant<JsonNull, JsonBool, JsonNumber,
                                 JsonString, JsonArray, JsonObject>;
    Storage data;

    // PMR allocator를 전파하는 생성자
    explicit JsonValue(std::pmr::memory_resource* mr)
        : data{JsonNull{}} {}
};

// 간단한 벤치마크: N개 노드 생성
long long bench_heap(int n) {
    auto start = std::chrono::high_resolution_clock::now();
    {
        std::vector<JsonValue*> nodes;
        nodes.reserve(n);
        for (int i = 0; i < n; ++i) {
            // heap에서 직접 할당
            nodes.push_back(new JsonValue{std::pmr::get_default_resource()});
            nodes.back()->data = JsonNumber{static_cast<double>(i)};
        }
        for (auto* p : nodes) delete p;
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

long long bench_monotonic(int n) {
    auto start = std::chrono::high_resolution_clock::now();
    {
        // 스택 버퍼 + monotonic_buffer_resource
        std::vector<char> stack_buf(sizeof(JsonValue) * n * 2);
        std::pmr::monotonic_buffer_resource mr{
            stack_buf.data(), stack_buf.size()
        };

        std::pmr::vector<JsonValue> nodes{&mr};
        nodes.reserve(n);
        for (int i = 0; i < n; ++i) {
            nodes.push_back(JsonValue{&mr});
            nodes.back().data = JsonNumber{static_cast<double>(i)};
        }
        // 소멸: mr 소멸로 일괄 해제 (stack_buf도 함께 해제)
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

int main() {
    // --- monotonic_buffer_resource 기본 활용 ---
    std::cout << "=== monotonic_buffer_resource 기본 예 ===\n";
    {
        char buf[1024];
        std::pmr::monotonic_buffer_resource mr{buf, sizeof(buf)};

        // pmr::vector와 pmr::string이 같은 mr에서 할당
        std::pmr::vector<std::pmr::string> words{&mr};
        words.emplace_back("apple");
        words.emplace_back("banana");
        words.emplace_back("cherry");

        for (auto& w : words) std::cout << w << ' ';
        std::cout << '\n';
        // mr 소멸 → buf 자동 반환 (buf는 스택이므로 아무것도 안 해도 됨)
    }

    // --- 체인 업스트림: 스택 버퍼 소진 후 heap ---
    std::cout << "\n=== 스택 버퍼 + heap upstream 체인 ===\n";
    {
        char small_buf[64];  // 의도적으로 작게
        std::pmr::monotonic_buffer_resource mr{
            small_buf, sizeof(small_buf),
            std::pmr::new_delete_resource()  // 소진 시 heap
        };

        std::pmr::vector<int> v{&mr};
        for (int i = 0; i < 100; ++i)  // 64바이트 이상 필요
            v.push_back(i);

        std::cout << "크기: " << v.size() << ", 합: ";
        long long sum = 0;
        for (int x : v) sum += x;
        std::cout << sum << '\n';
    }

    // --- 성능 비교 ---
    std::cout << "\n=== 성능 비교 ===\n";
    constexpr int N = 1000;

    // warm-up
    bench_heap(100);
    bench_monotonic(100);

    long long t_heap = bench_heap(N);
    long long t_mono = bench_monotonic(N);

    std::cout << "heap  alloc " << N << " JsonValue: " << t_heap << " us\n";
    std::cout << "mono  alloc " << N << " JsonValue: " << t_mono << " us\n";
    if (t_mono > 0)
        std::cout << "속도 향상: " << static_cast<double>(t_heap) / t_mono << "x\n";

    // --- unsynchronized_pool_resource ---
    std::cout << "\n=== unsynchronized_pool_resource ===\n";
    {
        std::pmr::unsynchronized_pool_resource pool;
        std::pmr::vector<std::pmr::string> sv{&pool};

        for (int i = 0; i < 5; ++i) {
            sv.emplace_back("item_" + std::to_string(i));
        }

        // 중간 요소 erase (pool_resource는 개별 해제 지원)
        sv.erase(sv.begin() + 2);

        for (auto& s : sv) std::cout << s << ' ';
        std::cout << '\n';
    }

    return 0;
}

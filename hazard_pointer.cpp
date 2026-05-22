// filename: hazard_pointer.cpp
// g++ -std=c++17 -O2 -pthread -o hazard_pointer hazard_pointer.cpp

#include <atomic>
#include <thread>
#include <vector>
#include <unordered_set>
#include <functional>
#include <iostream>
#include <optional>
#include <cassert>

// 최대 스레드 수 (단순화를 위해 상수로)
constexpr int MAX_THREADS = 32;
constexpr int RETIRE_THRESHOLD = 64; // retire list가 이 이상이면 scan

// 전역 Hazard Pointer 테이블
struct HazardRecord {
    std::atomic<void*> ptr{nullptr};
};
HazardRecord hp_table[MAX_THREADS];

// 스레드 로컬 상태
thread_local int       thread_id = -1;
thread_local int       next_hp_slot = 0;
std::atomic<int>       thread_count{0};

int get_thread_id()
{
    if (thread_id == -1)
        thread_id = thread_count.fetch_add(1, std::memory_order_relaxed);
    return thread_id;
}

// Hazard Pointer 획득 (주소를 공표)
void* hazard_acquire(void* ptr)
{
    int tid = get_thread_id();
    hp_table[tid].ptr.store(ptr, std::memory_order_seq_cst);
    // seq_cst: 이 store가 다른 스레드의 load보다 앞에 보이도록
    return ptr;
}

// Hazard Pointer 해제
void hazard_release()
{
    int tid = get_thread_id();
    hp_table[tid].ptr.store(nullptr, std::memory_order_release);
}

// retire list (스레드 로컬)
thread_local std::vector<void*> retire_list;
thread_local std::vector<std::function<void(void*)>> retire_deleters;

void retire(void* ptr, std::function<void(void*)> deleter)
{
    retire_list.push_back(ptr);
    retire_deleters.push_back(std::move(deleter));

    if (static_cast<int>(retire_list.size()) >= RETIRE_THRESHOLD) {
        // 현재 모든 스레드의 위험 포인터 수집
        std::unordered_set<void*> hazards;
        int n = thread_count.load(std::memory_order_acquire);
        for (int i = 0; i < n; ++i) {
            void* p = hp_table[i].ptr.load(std::memory_order_acquire);
            if (p) hazards.insert(p);
        }

        // 위험하지 않은 포인터만 즉시 해제
        std::vector<void*>                      new_list;
        std::vector<std::function<void(void*)>> new_deleters;
        for (size_t i = 0; i < retire_list.size(); ++i) {
            if (hazards.count(retire_list[i]) == 0) {
                retire_deleters[i](retire_list[i]); // 안전하게 해제
            } else {
                new_list.push_back(retire_list[i]);
                new_deleters.push_back(std::move(retire_deleters[i]));
            }
        }
        retire_list    = std::move(new_list);
        retire_deleters = std::move(new_deleters);
    }
}

// Hazard Pointer 기반 Treiber Stack
template<typename T>
class HPStack {
    struct Node {
        T      value;
        Node*  next;
    };
    std::atomic<Node*> head_{nullptr};

public:
    void push(T val)
    {
        Node* new_node = new Node{std::move(val), nullptr};
        new_node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(new_node->next, new_node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) ;
    }

    std::optional<T> pop()
    {
        while (true) {
            Node* old_top = head_.load(std::memory_order_acquire);
            if (!old_top) return std::nullopt;

            // Hazard Pointer 등록: "old_top 사용 중"
            hazard_acquire(static_cast<void*>(old_top));

            // 재확인: head가 바뀌지 않았는지
            if (old_top != head_.load(std::memory_order_seq_cst)) {
                hazard_release();
                continue; // head가 바뀜 → 재시도
            }

            Node* new_top = old_top->next;
            if (head_.compare_exchange_weak(old_top, new_top,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed)) {
                hazard_release(); // 더 이상 old_top 참조 안 함
                T val = std::move(old_top->value);
                // 즉시 delete 대신 retire에 등록
                retire(static_cast<void*>(old_top),
                       [](void* p){ delete static_cast<Node*>(p); });
                return val;
            }
            hazard_release();
        }
    }

    ~HPStack()
    {
        // 남은 노드 직접 해제 (단일 스레드 소멸)
        Node* n = head_.load();
        while (n) { Node* next = n->next; delete n; n = next; }
    }
};

int main()
{
    HPStack<int> stack;
    constexpr int N       = 20'000;
    constexpr int NTHREAD = 4;
    std::atomic<int> total{0};

    auto prod = [&]{ for (int i = 0; i < N; ++i) stack.push(1); };
    auto cons = [&]{
        int local = 0;
        for (int i = 0; i < N; ++i) {
            std::optional<int> v;
            while (!(v = stack.pop())) ;
            local += *v;
        }
        total.fetch_add(local, std::memory_order_relaxed);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < NTHREAD; ++i) threads.emplace_back(prod);
    for (int i = 0; i < NTHREAD; ++i) threads.emplace_back(cons);
    for (auto& t : threads) t.join();

    int expected = N * NTHREAD;
    std::cout << "total = " << total.load() << " (기대: " << expected << ")\n";
    assert(total.load() == expected);
    std::cout << "검증 통과\n";
    return 0;
}

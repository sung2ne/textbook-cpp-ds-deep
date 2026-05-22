// filename: autocomplete.cpp
// g++ -std=c++17 -O2 -Wall -o autocomplete autocomplete.cpp

#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>

// 자동완성 시스템: Trie + 빈도수
class AutoComplete {
    struct TrieNode {
        std::array<TrieNode*, 26> children{};
        int frequency = 0;  // 이 단어로 끝나는 검색 횟수
        bool is_word = false;
    };

    TrieNode* root_;
    int total_words_ = 0;

    // DFS로 모든 단어 수집 (prefix + 빈도수)
    void collect_(TrieNode* node, std::string& buf,
                  std::vector<std::pair<int, std::string>>& results) const {
        if (node->is_word)
            results.push_back({node->frequency, buf});
        for (int i = 0; i < 26; ++i) {
            if (node->children[i]) {
                buf.push_back('a' + i);
                collect_(node->children[i], buf, results);
                buf.pop_back();
            }
        }
    }

    void delete_(TrieNode* node) {
        if (!node) return;
        for (auto* c : node->children) delete_(c);
        delete node;
    }

public:
    AutoComplete() : root_(new TrieNode()) {}
    ~AutoComplete() { delete_(root_); }

    // 단어 삽입 또는 빈도 증가
    void record_search(const std::string& word) {
        TrieNode* curr = root_;
        for (char c : word) {
            int idx = c - 'a';
            if (!curr->children[idx])
                curr->children[idx] = new TrieNode();
            curr = curr->children[idx];
        }
        if (!curr->is_word) {
            curr->is_word = true;
            ++total_words_;
        }
        ++curr->frequency;
    }

    // 자동완성: prefix로 시작하는 단어 중 빈도 상위 N개
    std::vector<std::string> suggest(const std::string& prefix, int top_n = 5) const {
        // prefix 노드까지 이동
        TrieNode* curr = root_;
        for (char c : prefix) {
            int idx = c - 'a';
            if (!curr->children[idx]) return {};
            curr = curr->children[idx];
        }

        // 모든 후보 수집
        std::vector<std::pair<int, std::string>> candidates;
        std::string buf = prefix;
        collect_(curr, buf, candidates);

        // 빈도 내림차순 정렬
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b){ return a.first > b.first; });

        // 상위 N개 반환
        std::vector<std::string> result;
        for (int i = 0; i < std::min(top_n, (int)candidates.size()); ++i)
            result.push_back(candidates[i].second);
        return result;
    }

    int word_count() const { return total_words_; }
};

// === 프로젝트 2: 이벤트 스케줄러 ===
struct Event {
    long long fire_time_ms;  // 실행 시각 (밀리초)
    int priority;            // 같은 시각일 때 우선순위 (낮을수록 먼저)
    std::string name;
    std::function<void()> callback;

    // 최소 힙: fire_time이 작을수록 먼저
    bool operator>(const Event& other) const {
        if (fire_time_ms != other.fire_time_ms)
            return fire_time_ms > other.fire_time_ms;
        return priority > other.priority;
    }
};

class EventScheduler {
    using MinHeap = std::priority_queue<Event, std::vector<Event>, std::greater<Event>>;
    MinHeap heap_;
    long long current_time_ms_ = 0;

public:
    // 현재 시각에서 delay_ms 후에 이벤트 등록
    void schedule(const std::string& name, long long delay_ms,
                  int priority, std::function<void()> cb) {
        heap_.push({current_time_ms_ + delay_ms, priority, name, std::move(cb)});
    }

    // current_time까지 실행해야 할 이벤트 모두 처리
    void tick(long long new_time_ms) {
        current_time_ms_ = new_time_ms;
        while (!heap_.empty() && heap_.top().fire_time_ms <= current_time_ms_) {
            Event ev = heap_.top(); heap_.pop();
            std::cout << "[t=" << ev.fire_time_ms << "ms] 이벤트 실행: " << ev.name << "\n";
            ev.callback();
        }
    }

    // 전체 이벤트 처리 (시뮬레이션 완료까지)
    void run_all() {
        while (!heap_.empty()) {
            long long next_time = heap_.top().fire_time_ms;
            tick(next_time);
        }
    }

    bool empty() const { return heap_.empty(); }
    int pending_count() const { return static_cast<int>(heap_.size()); }
    long long current_time() const { return current_time_ms_; }
};

// === 메인 ===
int main() {
    // === 자동완성 시스템 테스트 ===
    std::cout << "===== 자동완성 시스템 =====\n";
    AutoComplete ac;

    // 검색 기록 시뮬레이션
    struct SearchLog { std::string word; int count; };
    std::vector<SearchLog> logs = {
        {"apple", 150}, {"application", 80}, {"apply", 45},
        {"app", 200},   {"banana", 120},    {"ban", 30},
        {"band", 60},   {"bandana", 15},    {"apple", 50},  // apple 추가 50번
        {"apt", 25},    {"application", 20}  // application 추가 20번
    };

    for (auto& log : logs)
        for (int i = 0; i < log.count; ++i)
            ac.record_search(log.word);

    std::cout << "총 등록 단어 수: " << ac.word_count() << "\n\n";

    for (auto& prefix : {"app", "a", "ban"}) {
        auto suggestions = ac.suggest(prefix, 5);
        std::cout << "'" << prefix << "' 검색어 추천:\n";
        for (int i = 0; i < (int)suggestions.size(); ++i)
            std::cout << "  " << (i+1) << ". " << suggestions[i] << "\n";
        std::cout << "\n";
    }
    // 'app' → app(200), apple(200), application(100), apply(45), apt(25) 순

    // === 이벤트 스케줄러 테스트 ===
    std::cout << "===== 이벤트 스케줄러 (게임 루프 시뮬레이션) =====\n";
    EventScheduler scheduler;

    // 이벤트 등록
    scheduler.schedule("플레이어_스폰",    0,   1, []{ std::cout << "  → 플레이어 생성\n"; });
    scheduler.schedule("몬스터_웨이브_1",  500, 2, []{ std::cout << "  → 1차 몬스터 웨이브\n"; });
    scheduler.schedule("아이템_드롭",      500, 3, []{ std::cout << "  → 아이템 드롭 (웨이브 후)\n"; });
    scheduler.schedule("체력_회복",        200, 1, []{ std::cout << "  → 자동 체력 회복\n"; });
    scheduler.schedule("몬스터_웨이브_2", 1000, 2, []{ std::cout << "  → 2차 몬스터 웨이브\n"; });
    scheduler.schedule("보스_등장",       2000, 1, []{ std::cout << "  → 보스 등장!\n"; });
    scheduler.schedule("저장",             800, 5, []{ std::cout << "  → 자동 저장\n"; });

    std::cout << "등록된 이벤트 수: " << scheduler.pending_count() << "\n\n";

    // 게임 루프 시뮬레이션 (100ms 단위로 tick)
    for (long long t = 0; t <= 2100; t += 100) {
        scheduler.tick(t);
    }

    // === 이벤트 스케줄러: 동적 이벤트 추가 ===
    std::cout << "\n===== 동적 이벤트 추가 테스트 =====\n";
    EventScheduler dyn;
    dyn.tick(0);  // t=0으로 초기화

    // t=0에 등록
    dyn.schedule("A", 100, 1, [&]{
        std::cout << "  → A 처리: B 이벤트를 200ms 후에 추가\n";
        // 이벤트 핸들러 내에서 새 이벤트 등록
        dyn.schedule("B (A에서 추가)", 200, 1, []{
            std::cout << "  → B 처리\n";
        });
    });
    dyn.schedule("C", 150, 1, []{ std::cout << "  → C 처리\n"; });

    dyn.run_all();

    return 0;
}

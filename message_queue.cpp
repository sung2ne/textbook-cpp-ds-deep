// filename: message_queue.cpp
// g++ -std=c++17 -O2 -Wall -o message_queue message_queue.cpp
#include <iostream>
#include <queue>
#include <priority_queue>
#include <map>
#include <string>
#include <functional>
#include <chrono>
#include <optional>
#include <variant>
#include <sstream>

// 메시지 타입
enum class MsgType {
    INPUT,      // 사용자 입력
    NETWORK,    // 네트워크 수신
    TIMER,      // 타이머 이벤트
    SHUTDOWN,   // 종료 신호
};

struct Message {
    MsgType  type;
    int      priority;  // 높을수록 중요
    std::string payload;
    std::chrono::steady_clock::time_point timestamp;

    Message(MsgType t, int p, std::string data)
        : type(t), priority(p), payload(std::move(data))
        , timestamp(std::chrono::steady_clock::now()) {}

    std::string type_str() const {
        switch (type) {
            case MsgType::INPUT:    return "INPUT";
            case MsgType::NETWORK:  return "NETWORK";
            case MsgType::TIMER:    return "TIMER";
            case MsgType::SHUTDOWN: return "SHUTDOWN";
        }
        return "UNKNOWN";
    }
};

// 일반 FIFO 메시지 큐
class MessageQueue {
    std::queue<Message> queue_;
    size_t max_size_;

public:
    explicit MessageQueue(size_t max_size = 1000)
        : max_size_(max_size) {}

    bool push(Message msg) {
        if (queue_.size() >= max_size_) return false;  // 가득 참
        queue_.push(std::move(msg));
        return true;
    }

    std::optional<Message> pop() {
        if (queue_.empty()) return std::nullopt;
        Message msg = std::move(const_cast<Message&>(queue_.front()));
        queue_.pop();
        return msg;
    }

    bool empty() const { return queue_.empty(); }
    size_t size() const { return queue_.size(); }
};

// 우선순위 메시지 큐
class PriorityMessageQueue {
    struct Cmp {
        bool operator()(const Message& a, const Message& b) const {
            return a.priority < b.priority;  // 높은 priority가 먼저
        }
    };
    std::priority_queue<Message, std::vector<Message>, Cmp> pq_;

public:
    void push(Message msg) { pq_.push(std::move(msg)); }

    std::optional<Message> pop() {
        if (pq_.empty()) return std::nullopt;
        Message msg = pq_.top();
        pq_.pop();
        return msg;
    }

    bool empty() const { return pq_.empty(); }
    size_t size() const { return pq_.size(); }
};

// 작업 스케줄러
using TimePoint = std::chrono::steady_clock::time_point;
using Duration  = std::chrono::milliseconds;

struct Task {
    std::string name;
    int         priority;
    std::function<void()> action;

    Task(std::string n, int p, std::function<void()> a)
        : name(std::move(n)), priority(p), action(std::move(a)) {}
};

class TaskScheduler {
    // 우선순위 기반 즉시 실행 큐
    struct TaskCmp {
        bool operator()(const Task& a, const Task& b) const {
            return a.priority < b.priority;
        }
    };
    std::priority_queue<Task, std::vector<Task>, TaskCmp> ready_queue_;

    // 시간 기반 지연 실행 큐 (정렬된 실행 시각 → 작업)
    std::multimap<TimePoint, Task> scheduled_;

public:
    // 즉시 실행 큐에 추가
    void schedule_now(Task task) {
        ready_queue_.push(std::move(task));
    }

    // 지정 시간에 실행
    void schedule_at(TimePoint when, Task task) {
        scheduled_.emplace(when, std::move(task));
    }

    // 지금부터 delay 후에 실행
    void schedule_after(Duration delay, Task task) {
        schedule_at(std::chrono::steady_clock::now() + delay, std::move(task));
    }

    // 한 번의 tick — 실행 가능한 작업 처리
    int tick() {
        int executed = 0;
        auto now = std::chrono::steady_clock::now();

        // 시간이 된 예약 작업을 즉시 큐로 이동
        auto it = scheduled_.begin();
        while (it != scheduled_.end() && it->first <= now) {
            ready_queue_.push(std::move(it->second));
            it = scheduled_.erase(it);
        }

        // 즉시 큐에서 최대 N개 처리
        const int MAX_PER_TICK = 5;
        while (!ready_queue_.empty() && executed < MAX_PER_TICK) {
            Task task = ready_queue_.top();
            ready_queue_.pop();
            std::cout << "  [tick] 실행: " << task.name
                      << " (priority=" << task.priority << ")\n";
            task.action();
            ++executed;
        }

        return executed;
    }

    bool has_pending() const {
        return !ready_queue_.empty() || !scheduled_.empty();
    }

    size_t ready_count() const { return ready_queue_.size(); }
    size_t scheduled_count() const { return scheduled_.size(); }
};

// 이벤트 루프 시뮬레이션
void run_event_loop() {
    std::cout << "=== 이벤트 루프 시뮬레이션 ===\n\n";

    MessageQueue fifo_mq;
    PriorityMessageQueue prio_mq;
    TaskScheduler scheduler;

    // 메시지 생산
    fifo_mq.push({MsgType::NETWORK, 2, "HTTP 요청 수신"});
    fifo_mq.push({MsgType::INPUT,   1, "키보드 Enter"});
    fifo_mq.push({MsgType::TIMER,   1, "타이머 1초 경과"});
    fifo_mq.push({MsgType::SHUTDOWN,5, "종료 신호"});

    prio_mq.push({MsgType::NETWORK, 2, "일반 패킷"});
    prio_mq.push({MsgType::INPUT,   5, "긴급 입력"});
    prio_mq.push({MsgType::TIMER,   1, "낮은 우선순위 타이머"});
    prio_mq.push({MsgType::NETWORK, 4, "중요 패킷"});

    // FIFO 처리
    std::cout << "--- FIFO 메시지 처리 (도착 순서) ---\n";
    while (!fifo_mq.empty()) {
        auto msg = fifo_mq.pop();
        std::cout << "  처리: [" << msg->type_str() << "] "
                  << msg->payload << "\n";
        if (msg->type == MsgType::SHUTDOWN) break;
    }

    // 우선순위 처리
    std::cout << "\n--- 우선순위 메시지 처리 (높은 우선순위 먼저) ---\n";
    while (!prio_mq.empty()) {
        auto msg = prio_mq.pop();
        std::cout << "  처리: [" << msg->type_str()
                  << " p=" << msg->priority << "] " << msg->payload << "\n";
    }

    // 작업 스케줄러
    std::cout << "\n--- 작업 스케줄러 ---\n";
    scheduler.schedule_now({"렌더링", 3, []{ std::cout << "    → 화면 그리기\n"; }});
    scheduler.schedule_now({"물리 계산", 5, []{ std::cout << "    → 물리 엔진 업데이트\n"; }});
    scheduler.schedule_now({"오디오 처리", 2, []{ std::cout << "    → 사운드 믹싱\n"; }});
    scheduler.schedule_now({"AI 업데이트", 4, []{ std::cout << "    → 적 AI 계산\n"; }});

    // 50ms 후 실행 예약
    scheduler.schedule_after(Duration(50), {"지연 작업", 1, []{
        std::cout << "    → 50ms 후 실행됨\n";
    }});

    std::cout << "즉시 큐: " << scheduler.ready_count() << "개\n";
    std::cout << "예약 큐: " << scheduler.scheduled_count() << "개\n\n";

    // tick 실행 (즉시 작업 처리)
    std::cout << "[tick 1]\n";
    scheduler.tick();

    // 50ms 대기 시뮬레이션 (실제로는 sleep 없이 진행)
    std::cout << "[tick 2] (50ms 이후 가정)\n";
    // 예약 작업을 강제로 즉시 큐로 이동하여 시연
    // 실제 코드에서는 steady_clock::now()가 적절히 진행됨
}

// multimap 기반 시간 스케줄러 예제
void demo_multimap_scheduler() {
    std::cout << "\n=== multimap 기반 시간 스케줄러 ===\n";

    using TP = std::chrono::steady_clock::time_point;
    auto now = std::chrono::steady_clock::now();

    std::multimap<TP, std::string> schedule;

    // 다양한 시간에 이벤트 등록 (삽입 순서와 무관하게 시각 순 정렬)
    schedule.emplace(now + std::chrono::seconds(5), "5초 후 저장");
    schedule.emplace(now + std::chrono::seconds(1), "1초 후 갱신");
    schedule.emplace(now + std::chrono::seconds(3), "3초 후 동기화");
    schedule.emplace(now + std::chrono::seconds(1), "1초 후 알림");  // 같은 시각 2개

    std::cout << "등록된 이벤트 (시각 순):\n";
    for (const auto& [tp, name] : schedule) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp - now).count();
        std::cout << "  +" << ms << "ms: " << name << "\n";
    }
}

int main() {
    run_event_loop();
    demo_multimap_scheduler();
    return 0;
}

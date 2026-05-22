// filename: enable_shared.cpp
// g++ -std=c++17 -O2 -o enable_shared enable_shared.cpp

#include <iostream>
#include <memory>
#include <functional>
#include <vector>

class Worker : public std::enable_shared_from_this<Worker> {
public:
    int id;
    std::vector<std::function<void()>> callbacks;

    Worker(int i) : id(i) {
        std::cout << "Worker(" << id << ") 생성\n";
    }
    ~Worker() {
        std::cout << "Worker(" << id << ") 소멸\n";
    }

    void scheduleWork() {
        // shared_from_this(): this를 가리키는 공유된 shared_ptr 반환
        // 기존 control block을 사용하므로 안전
        auto self = shared_from_this();

        callbacks.push_back([self]() {
            std::cout << "Worker(" << self->id << ") 작업 실행 (비동기)\n";
        });

        std::cout << "Worker(" << id << ") 작업 예약됨, use_count: "
                  << self.use_count() << "\n";
    }

    void runCallbacks() {
        for (auto& cb : callbacks) cb();
    }
};

int main() {
    std::vector<std::function<void()>> deferred;

    {
        auto worker = std::make_shared<Worker>(1);
        worker->scheduleWork();

        // 콜백을 외부로 빼냄 (worker보다 오래 살 수 있음)
        for (auto& cb : worker->callbacks) {
            deferred.push_back(cb);
        }

        std::cout << "스코프 종료 직전 use_count: " << worker.use_count() << "\n";
    }  // worker 로컬 변수 소멸, 하지만 콜백이 self를 잡고 있음

    std::cout << "스코프 종료 후 deferred 실행:\n";
    for (auto& cb : deferred) cb();

    // deferred 소멸 → 람다 내 self 소멸 → Worker 소멸
    std::cout << "=== deferred 소멸 ===\n";

    return 0;
}

// filename: shared_mutex_demo.cpp
// g++ -std=c++17 -O2 -pthread -o shared_mutex_demo shared_mutex_demo.cpp
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>

class ThreadSafeCache {
    std::unordered_map<std::string, int> data_;
    mutable std::shared_mutex mtx_;

public:
    // 읽기: 여러 스레드 동시 가능
    std::optional<int> get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mtx_); // 공유 잠금
        auto it = data_.find(key);
        if (it == data_.end()) return std::nullopt;
        return it->second;
    }

    // 쓰기: 독점 접근
    void put(const std::string& key, int value) {
        std::unique_lock<std::shared_mutex> lock(mtx_); // 독점 잠금
        data_[key] = value;
    }
};

int main()
{
    ThreadSafeCache cache;
    cache.put("alpha", 1);
    cache.put("beta",  2);

    // 4개 스레드가 동시에 읽기
    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([&cache, i]{
            for (int j = 0; j < 100'000; ++j) {
                auto v = cache.get("alpha");
                (void)v;
            }
        });
    }

    // 쓰기 스레드 하나
    std::thread writer([&cache]{
        for (int j = 0; j < 1'000; ++j)
            cache.put("gamma", j);
    });

    for (auto& t : readers) t.join();
    writer.join();

    std::cout << "alpha = " << cache.get("alpha").value_or(-1) << "\n";
    return 0;
}

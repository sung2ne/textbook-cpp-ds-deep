// filename: separate_chaining.cpp
// g++ -std=c++17 -O2 -Wall -o separate_chaining separate_chaining.cpp
#include <forward_list>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
class SeparateChainingMap {
public:
    explicit SeparateChainingMap(std::size_t bucket_count = 16)
        : buckets_(bucket_count), size_(0) {}

    void insert(K key, V value) {
        auto idx = bucket_index(key);
        for (auto& [k, v] : buckets_[idx]) {
            if (k == key) { v = value; return; }  // 기존 키 업데이트
        }
        buckets_[idx].emplace_front(std::move(key), std::move(value));
        ++size_;
        if (load_factor() > max_load_factor_) rehash(buckets_.size() * 2);
    }

    std::optional<V> find(const K& key) const {
        auto idx = bucket_index(key);
        for (const auto& [k, v] : buckets_[idx]) {
            if (k == key) return v;
        }
        return std::nullopt;
    }

    bool erase(const K& key) {
        auto idx = bucket_index(key);
        auto& chain = buckets_[idx];
        auto prev = chain.before_begin();
        for (auto it = chain.begin(); it != chain.end(); ++it, ++prev) {
            if (it->first == key) {
                chain.erase_after(prev);
                --size_;
                return true;
            }
        }
        return false;
    }

    double load_factor() const {
        return static_cast<double>(size_) / buckets_.size();
    }

    std::size_t size()         const { return size_; }
    std::size_t bucket_count() const { return buckets_.size(); }

    void print_distribution() const {
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            std::cout << "bucket[" << i << "]: ";
            for (const auto& [k, v] : buckets_[i])
                std::cout << k << " ";
            std::cout << '\n';
        }
    }

private:
    std::size_t bucket_index(const K& key) const {
        return std::hash<K>{}(key) % buckets_.size();
    }

    void rehash(std::size_t new_count) {
        std::vector<std::forward_list<std::pair<K, V>>> new_buckets(new_count);
        for (auto& chain : buckets_)
            for (auto& [k, v] : chain) {
                auto idx = std::hash<K>{}(k) % new_count;
                new_buckets[idx].emplace_front(k, v);
            }
        buckets_ = std::move(new_buckets);
    }

    std::vector<std::forward_list<std::pair<K, V>>> buckets_;
    std::size_t size_;
    double max_load_factor_ = 1.0;
};

int main() {
    SeparateChainingMap<std::string, int> map(4);  // 버킷 4개로 시작

    map.insert("alpha", 1);
    map.insert("beta",  2);
    map.insert("gamma", 3);
    map.insert("delta", 4);
    map.insert("alpha", 99);  // 기존 키 업데이트

    std::cout << "=== 버킷 분포 ===\n";
    map.print_distribution();
    std::cout << "\nsize=" << map.size()
              << "  buckets=" << map.bucket_count()
              << "  load_factor=" << map.load_factor() << '\n';

    if (auto v = map.find("beta")) std::cout << "beta = " << *v << '\n';
    map.erase("beta");
    std::cout << "beta 삭제 후 exists = " << map.find("beta").has_value() << '\n';
}

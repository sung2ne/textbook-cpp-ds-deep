// filename: cuckoo_hashing.cpp
// g++ -std=c++17 -O2 -Wall -o cuckoo_hashing cuckoo_hashing.cpp
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
class CuckooMap {
    struct Slot {
        K    key;
        V    value;
        bool occupied = false;
    };

public:
    explicit CuckooMap(std::size_t cap = 16)
        : t1_(cap), t2_(cap), cap_(cap), size_(0), seed_(42) {}

    bool insert(K key, V value) {
        // 이미 존재하는 키 업데이트
        if (auto* v = find_ptr(key)) { *v = std::move(value); return true; }

        const int MAX_ITER = static_cast<int>(cap_ * 2);
        K   cur_key  = std::move(key);
        V   cur_val  = std::move(value);

        for (int i = 0; i < MAX_ITER; ++i) {
            // T1 시도
            std::size_t idx1 = h1(cur_key);
            if (!t1_[idx1].occupied) {
                t1_[idx1] = {std::move(cur_key), std::move(cur_val), true};
                ++size_;
                return true;
            }
            std::swap(cur_key, t1_[idx1].key);
            std::swap(cur_val, t1_[idx1].value);

            // T2 시도 (T1에서 퇴거된 원소)
            std::size_t idx2 = h2(cur_key);
            if (!t2_[idx2].occupied) {
                t2_[idx2] = {std::move(cur_key), std::move(cur_val), true};
                ++size_;
                return true;
            }
            std::swap(cur_key, t2_[idx2].key);
            std::swap(cur_val, t2_[idx2].value);
        }

        // 사이클 감지 → rehash 후 재삽입
        rehash();
        return insert(std::move(cur_key), std::move(cur_val));
    }

    std::optional<V> find(const K& key) const {
        if (const auto* v = find_ptr(key)) return *v;
        return std::nullopt;
    }

    bool erase(const K& key) {
        std::size_t idx1 = h1(key);
        if (t1_[idx1].occupied && t1_[idx1].key == key) {
            t1_[idx1].occupied = false;
            --size_;
            return true;
        }
        std::size_t idx2 = h2(key);
        if (t2_[idx2].occupied && t2_[idx2].key == key) {
            t2_[idx2].occupied = false;
            --size_;
            return true;
        }
        return false;
    }

    double load_factor() const {
        return static_cast<double>(size_) / (cap_ * 2);
    }

    std::size_t size()     const { return size_; }
    std::size_t capacity() const { return cap_; }

    void print_tables() const {
        std::cout << "T1:\n";
        for (std::size_t i = 0; i < cap_; ++i) {
            if (t1_[i].occupied)
                std::cout << "  [" << i << "] " << t1_[i].key
                          << " = " << t1_[i].value << '\n';
        }
        std::cout << "T2:\n";
        for (std::size_t i = 0; i < cap_; ++i) {
            if (t2_[i].occupied)
                std::cout << "  [" << i << "] " << t2_[i].key
                          << " = " << t2_[i].value << '\n';
        }
    }

private:
    const V* find_ptr(const K& key) const {
        std::size_t idx1 = h1(key);
        if (t1_[idx1].occupied && t1_[idx1].key == key)
            return &t1_[idx1].value;
        std::size_t idx2 = h2(key);
        if (t2_[idx2].occupied && t2_[idx2].key == key)
            return &t2_[idx2].value;
        return nullptr;
    }

    V* find_ptr(const K& key) {
        return const_cast<V*>(static_cast<const CuckooMap*>(this)->find_ptr(key));
    }

    std::size_t h1(const K& key) const {
        return std::hash<K>{}(key) % cap_;
    }

    std::size_t h2(const K& key) const {
        // 다른 시드를 사용해 독립적인 해시 함수 시뮬레이션
        std::size_t h = std::hash<K>{}(key) ^ (std::hash<std::size_t>{}(seed_) * 2654435761ULL);
        return h % cap_;
    }

    void rehash() {
        std::size_t new_cap = cap_ * 2;
        ++seed_;
        std::vector<Slot> old_t1 = std::move(t1_);
        std::vector<Slot> old_t2 = std::move(t2_);
        t1_.assign(new_cap, Slot{});
        t2_.assign(new_cap, Slot{});
        cap_  = new_cap;
        size_ = 0;
        for (auto& s : old_t1) if (s.occupied) insert(s.key, s.value);
        for (auto& s : old_t2) if (s.occupied) insert(s.key, s.value);
    }

    std::vector<Slot> t1_, t2_;
    std::size_t       cap_;
    std::size_t       size_;
    std::size_t       seed_;
};

int main() {
    CuckooMap<std::string, int> m(8);

    const std::string keys[] = {
        "router", "switch", "gateway", "firewall", "proxy", "dns"
    };
    for (int i = 0; auto& k : keys) m.insert(k, ++i);

    std::cout << "=== Cuckoo 해시 테이블 ===\n";
    m.print_tables();
    std::cout << "\nsize=" << m.size()
              << "  load_factor=" << m.load_factor() << '\n';

    std::cout << "\n=== 탐색 (최대 2번 메모리 접근) ===\n";
    for (const auto& k : keys)
        std::cout << k << " → " << m.find(k).value_or(-1) << '\n';

    m.erase("switch");
    std::cout << "\nswitch 삭제 후: " << m.find("switch").value_or(-1) << '\n';

    return 0;
}

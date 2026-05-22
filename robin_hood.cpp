// filename: robin_hood.cpp
// g++ -std=c++17 -O2 -Wall -o robin_hood robin_hood.cpp
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
class RobinHoodMap {
    struct Slot {
        K        key;
        V        value;
        int32_t  psl = -1;  // -1 = 비어있음

        bool empty() const { return psl < 0; }
    };

public:
    explicit RobinHoodMap(std::size_t cap = 16)
        : slots_(next_power_of_two(cap)), size_(0) {}

    void insert(K key, V value) {
        if (load_factor() > 0.75) grow();

        Slot incoming{std::move(key), std::move(value), 0};
        std::size_t idx = hash_index(incoming.key);

        while (true) {
            Slot& slot = slots_[idx];
            if (slot.empty()) {
                slot = std::move(incoming);
                ++size_;
                return;
            }
            if (slot.key == incoming.key) {
                slot.value = std::move(incoming.value);
                return;
            }
            // Robin Hood: 현재 슬롯의 PSL이 더 작으면 교환 (부자에게서 빼앗기)
            if (slot.psl < incoming.psl) {
                std::swap(slot, incoming);
            }
            ++incoming.psl;
            idx = (idx + 1) & (slots_.size() - 1);
        }
    }

    std::optional<V> find(const K& key) const {
        std::size_t idx = hash_index(key);
        int32_t psl = 0;
        while (true) {
            const Slot& slot = slots_[idx];
            if (slot.empty() || slot.psl < psl) return std::nullopt;
            if (slot.key == key) return slot.value;
            ++psl;
            idx = (idx + 1) & (slots_.size() - 1);
        }
    }

    bool erase(const K& key) {
        std::size_t idx = hash_index(key);
        int32_t psl = 0;
        while (true) {
            Slot& slot = slots_[idx];
            if (slot.empty() || slot.psl < psl) return false;
            if (slot.key == key) {
                // Backward Shift Deletion
                --size_;
                while (true) {
                    std::size_t next = (idx + 1) & (slots_.size() - 1);
                    Slot& next_slot = slots_[next];
                    if (next_slot.empty() || next_slot.psl == 0) {
                        slots_[idx] = Slot{};  // 비우기
                        return true;
                    }
                    slots_[idx] = std::move(next_slot);
                    --slots_[idx].psl;
                    idx = next;
                }
            }
            ++psl;
            idx = (idx + 1) & (slots_.size() - 1);
        }
    }

    double load_factor() const {
        return static_cast<double>(size_) / slots_.size();
    }

    std::size_t size() const { return size_; }

    void print_slots() const {
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            if (slots_[i].empty())
                std::cout << "[" << i << "] empty\n";
            else
                std::cout << "[" << i << "] " << slots_[i].key
                          << " (PSL=" << slots_[i].psl << ")\n";
        }
    }

private:
    std::size_t hash_index(const K& key) const {
        return std::hash<K>{}(key) & (slots_.size() - 1);
    }

    static std::size_t next_power_of_two(std::size_t n) {
        std::size_t p = 1;
        while (p < n) p <<= 1;
        return p;
    }

    void grow() {
        RobinHoodMap<K, V> bigger(slots_.size() * 2);
        for (auto& s : slots_)
            if (!s.empty()) bigger.insert(s.key, s.value);
        *this = std::move(bigger);
    }

    std::vector<Slot> slots_;
    std::size_t       size_;
};

int main() {
    RobinHoodMap<std::string, int> m(8);

    const std::string keys[] = {"apple", "banana", "cherry", "date", "elderberry"};
    for (int i = 0; auto& k : keys) m.insert(k, ++i);

    std::cout << "=== 슬롯 상태 (Robin Hood) ===\n";
    m.print_slots();

    std::cout << "\nfind(\"cherry\") = " << m.find("cherry").value_or(-1) << '\n';

    m.erase("banana");
    std::cout << "\n=== banana 삭제 후 ===\n";
    m.print_slots();

    return 0;
}

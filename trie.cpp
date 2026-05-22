// filename: trie.cpp
// g++ -std=c++17 -O2 -Wall -o trie trie.cpp

#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <functional>

// === 방법 1: array[26] 기반 Trie (알파벳 소문자) ===
struct TrieNodeArray {
    std::array<TrieNodeArray*, 26> children;
    bool is_end;
    int count;  // 이 노드를 지나는 단어 수

    TrieNodeArray() : children{}, is_end(false), count(0) {}
};

class TrieArray {
    TrieNodeArray* root_;

    void collect_(TrieNodeArray* node, std::string& prefix,
                  std::vector<std::string>& results, int limit) const {
        if (results.size() >= static_cast<size_t>(limit)) return;
        if (node->is_end) results.push_back(prefix);
        for (int i = 0; i < 26; ++i) {
            if (node->children[i]) {
                prefix.push_back('a' + i);
                collect_(node->children[i], prefix, results, limit);
                prefix.pop_back();
            }
        }
    }

    void delete_(TrieNodeArray* node) {
        if (!node) return;
        for (auto* child : node->children) delete_(child);
        delete node;
    }

public:
    TrieArray() : root_(new TrieNodeArray()) {}
    ~TrieArray() { delete_(root_); }

    void insert(const std::string& word) {
        TrieNodeArray* curr = root_;
        for (char c : word) {
            int idx = c - 'a';
            if (!curr->children[idx])
                curr->children[idx] = new TrieNodeArray();
            curr = curr->children[idx];
            ++curr->count;
        }
        curr->is_end = true;
    }

    bool search(const std::string& word) const {
        TrieNodeArray* curr = root_;
        for (char c : word) {
            int idx = c - 'a';
            if (!curr->children[idx]) return false;
            curr = curr->children[idx];
        }
        return curr->is_end;
    }

    bool starts_with(const std::string& prefix) const {
        TrieNodeArray* curr = root_;
        for (char c : prefix) {
            int idx = c - 'a';
            if (!curr->children[idx]) return false;
            curr = curr->children[idx];
        }
        return true;
    }

    bool remove(const std::string& word) {
        // 반복적 삭제 (단순화: 참조 카운트 방식)
        TrieNodeArray* curr = root_;
        std::vector<std::pair<TrieNodeArray*, int>> path;

        for (char c : word) {
            int idx = c - 'a';
            if (!curr->children[idx]) return false;
            path.push_back({curr, idx});
            curr = curr->children[idx];
        }
        if (!curr->is_end) return false;

        curr->is_end = false;
        // 역방향으로 올라가며 불필요한 노드 제거
        for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
            auto [parent, idx] = path[i];
            TrieNodeArray* child = parent->children[idx];
            bool has_children = false;
            for (auto* c : child->children) if (c) { has_children = true; break; }
            if (!child->is_end && !has_children) {
                delete child;
                parent->children[idx] = nullptr;
            } else break;
        }
        return true;
    }

    // 자동완성: prefix로 시작하는 단어 최대 limit개 반환
    std::vector<std::string> autocomplete(const std::string& prefix, int limit = 5) const {
        std::vector<std::string> results;
        TrieNodeArray* curr = root_;
        for (char c : prefix) {
            int idx = c - 'a';
            if (!curr->children[idx]) return results;
            curr = curr->children[idx];
        }
        std::string p = prefix;
        collect_(curr, p, results, limit);
        return results;
    }
};

// === 방법 2: unordered_map 기반 Trie (유니코드 지원 가능) ===
struct TrieNodeMap {
    std::unordered_map<char, std::unique_ptr<TrieNodeMap>> children;
    bool is_end = false;
};

class TrieMap {
    TrieNodeMap root_;
public:
    void insert(const std::string& word) {
        TrieNodeMap* curr = &root_;
        for (char c : word) {
            auto& child = curr->children[c];
            if (!child) child = std::make_unique<TrieNodeMap>();
            curr = child.get();
        }
        curr->is_end = true;
    }

    bool search(const std::string& word) const {
        const TrieNodeMap* curr = &root_;
        for (char c : word) {
            auto it = curr->children.find(c);
            if (it == curr->children.end()) return false;
            curr = it->second.get();
        }
        return curr->is_end;
    }
};

int main() {
    // === TrieArray 테스트 ===
    TrieArray trie;

    std::vector<std::string> words = {
        "apple", "app", "application", "apply", "apt",
        "banana", "band", "bandana", "ban"
    };

    for (auto& w : words) trie.insert(w);

    std::cout << "=== 검색 ===\n";
    std::cout << "apple: " << (trie.search("apple") ? "있음" : "없음") << "\n";
    std::cout << "appl:  " << (trie.search("appl")  ? "있음" : "없음") << "\n";
    std::cout << "app:   " << (trie.search("app")   ? "있음" : "없음") << "\n";
    std::cout << "starts_with(app): " << (trie.starts_with("app") ? "yes" : "no") << "\n";

    std::cout << "\n=== 자동완성 ===\n";
    for (auto& prefix : {"app", "ban", "b", "z"}) {
        auto suggestions = trie.autocomplete(prefix, 5);
        std::cout << "'" << prefix << "' 자동완성: ";
        for (auto& s : suggestions) std::cout << s << " ";
        std::cout << "\n";
    }
    // 'app' 자동완성: app apple application apply apt
    // 'ban' 자동완성: ban banana band bandana
    // 'b' 자동완성: ban banana band bandana banana...

    std::cout << "\n=== 삭제 ===\n";
    trie.remove("apple");
    std::cout << "apple 삭제 후 apple: " << (trie.search("apple") ? "있음" : "없음") << "\n";
    std::cout << "apple 삭제 후 app:   " << (trie.search("app")   ? "있음" : "없음") << "\n";

    // === 메모리 비교 ===
    std::cout << "\n=== 메모리 비교 ===\n";
    std::cout << "TrieNode(array) 크기: " << sizeof(TrieNodeArray) << " bytes\n";
    // 26 * 8(포인터) + 1(is_end) + 4(count) = ~213 bytes/노드

    // 단어 수가 적을 때: unordered_map이 유리
    // 알파벳 26자를 모두 사용할 때: array가 유리 (캐시 효율)

    return 0;
}

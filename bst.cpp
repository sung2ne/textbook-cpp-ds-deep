// filename: bst.cpp
// g++ -std=c++17 -O2 -Wall -o bst bst.cpp

#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>
#include <functional>

template<typename T>
class BST {
    struct Node {
        T val;
        std::unique_ptr<Node> left, right;
        explicit Node(const T& v) : val(v) {}
    };

    std::unique_ptr<Node> root_;
    int size_ = 0;

    // 내부 재귀 헬퍼들
    Node* insert_(std::unique_ptr<Node>& node, const T& val) {
        if (!node) {
            node = std::make_unique<Node>(val);
            ++size_;
            return node.get();
        }
        if (val < node->val) return insert_(node->left, val);
        if (val > node->val) return insert_(node->right, val);
        return node.get();  // 중복은 무시
    }

    bool search_(const Node* node, const T& val) const {
        if (!node) return false;
        if (val == node->val) return true;
        return val < node->val ? search_(node->left.get(), val)
                               : search_(node->right.get(), val);
    }

    // 서브트리에서 최솟값 노드 반환
    Node* find_min_(Node* node) const {
        while (node->left) node = node->left.get();
        return node;
    }

    // 최솟값 노드 제거 후 unique_ptr 반환
    std::unique_ptr<Node> remove_min_(std::unique_ptr<Node>& node) {
        if (!node->left) {
            std::unique_ptr<Node> right = std::move(node->right);
            node.reset();
            return right;
        }
        node->left = remove_min_(node->left);
        return nullptr;
    }

    bool remove_(std::unique_ptr<Node>& node, const T& val) {
        if (!node) return false;

        if (val < node->val) {
            return remove_(node->left, val);
        } else if (val > node->val) {
            return remove_(node->right, val);
        }

        // 찾음: 세 가지 경우
        --size_;
        if (!node->left && !node->right) {
            // 경우 1: 리프
            node.reset();
        } else if (!node->left) {
            // 경우 2: 오른쪽 자식만
            node = std::move(node->right);
        } else if (!node->right) {
            // 경우 2: 왼쪽 자식만
            node = std::move(node->left);
        } else {
            // 경우 3: 자식 둘 — inorder successor 사용
            Node* successor = find_min_(node->right.get());
            node->val = successor->val;  // 값 복사
            // successor 삭제 (재귀)
            // 실제로는 successor의 val을 다시 remove하는 방식
            ++size_;  // remove_에서 --size_ 되므로 보정
            remove_(node->right, successor->val);
        }
        return true;
    }

    void inorder_(const Node* node, std::vector<T>& result) const {
        if (!node) return;
        inorder_(node->left.get(), result);
        result.push_back(node->val);
        inorder_(node->right.get(), result);
    }

    int height_(const Node* node) const {
        if (!node) return -1;
        return 1 + std::max(height_(node->left.get()), height_(node->right.get()));
    }

public:
    void insert(const T& val) { insert_(root_, val); }
    bool search(const T& val) const { return search_(root_.get(), val); }
    bool remove(const T& val) { return remove_(root_, val); }
    int size() const { return size_; }
    int height() const { return height_(root_.get()); }

    std::vector<T> sorted() const {
        std::vector<T> result;
        inorder_(root_.get(), result);
        return result;
    }
};

int main() {
    BST<int> bst;

    // 삽입
    for (int x : {5, 3, 8, 1, 4, 9, 7, 2, 6}) {
        bst.insert(x);
    }

    std::cout << "크기: " << bst.size() << "\n";
    std::cout << "높이: " << bst.height() << "\n";

    auto sorted = bst.sorted();
    std::cout << "중위 순회(오름차순): ";
    for (int x : sorted) std::cout << x << " ";
    std::cout << "\n";
    // 출력: 1 2 3 4 5 6 7 8 9

    // 검색
    std::cout << "검색 4: " << (bst.search(4) ? "있음" : "없음") << "\n";  // 있음
    std::cout << "검색 10: " << (bst.search(10) ? "있음" : "없음") << "\n"; // 없음

    // 삭제
    bst.remove(5);  // 자식 둘인 경우
    sorted = bst.sorted();
    std::cout << "5 삭제 후: ";
    for (int x : sorted) std::cout << x << " ";
    std::cout << "\n";
    // 출력: 1 2 3 4 6 7 8 9

    bst.remove(1);  // 리프 삭제
    bst.remove(8);  // 자식 하나인 경우 (9만 있음)
    sorted = bst.sorted();
    std::cout << "1, 8 삭제 후: ";
    for (int x : sorted) std::cout << x << " ";
    std::cout << "\n";
    // 출력: 2 3 4 6 7 9

    return 0;
}

// filename: doubly_linked_list.cpp
// g++ -std=c++17 -O2 -Wall -o doubly_linked_list doubly_linked_list.cpp
#include <iostream>
#include <initializer_list>
#include <stdexcept>

template<typename T>
class DoublyLinkedList {
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
        Node() : prev(nullptr), next(nullptr) {}  // sentinel용
    };

    Node  sentinel_;  // 더미 노드: head이자 tail
    size_t size_;

    void link(Node* before, Node* node, Node* after) {
        before->next = node;
        node->prev   = before;
        node->next   = after;
        after->prev  = node;
    }

public:
    DoublyLinkedList() : size_(0) {
        sentinel_.prev = &sentinel_;
        sentinel_.next = &sentinel_;
    }

    ~DoublyLinkedList() { clear(); }

    void push_back(const T& val) {
        auto* node = new Node(val);
        link(sentinel_.prev, node, &sentinel_);
        ++size_;
    }

    void push_front(const T& val) {
        auto* node = new Node(val);
        link(&sentinel_, node, sentinel_.next);
        ++size_;
    }

    void pop_front() {
        if (empty()) throw std::runtime_error("list is empty");
        Node* to_del = sentinel_.next;
        sentinel_.next = to_del->next;
        to_del->next->prev = &sentinel_;
        delete to_del;
        --size_;
    }

    T& front() {
        if (empty()) throw std::runtime_error("list is empty");
        return sentinel_.next->data;
    }
    T& back() {
        if (empty()) throw std::runtime_error("list is empty");
        return sentinel_.prev->data;
    }

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    void clear() {
        Node* cur = sentinel_.next;
        while (cur != &sentinel_) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        sentinel_.prev = &sentinel_;
        sentinel_.next = &sentinel_;
        size_ = 0;
    }

    // 간단한 iterator
    class iterator {
        Node* node_;
    public:
        explicit iterator(Node* n) : node_(n) {}
        T& operator*() { return node_->data; }
        iterator& operator++() { node_ = node_->next; return *this; }
        iterator& operator--() { node_ = node_->prev; return *this; }
        bool operator==(const iterator& o) const { return node_ == o.node_; }
        bool operator!=(const iterator& o) const { return node_ != o.node_; }
        Node* get_node() { return node_; }
    };

    iterator begin() { return iterator(sentinel_.next); }
    iterator end()   { return iterator(&sentinel_); }

    // 특정 위치 앞에 삽입, 삽입된 원소의 iterator 반환
    iterator insert(iterator pos, const T& val) {
        auto* node = new Node(val);
        Node* after  = pos.get_node();
        Node* before = after->prev;
        link(before, node, after);
        ++size_;
        return iterator(node);
    }

    // 특정 위치 삭제, 다음 원소의 iterator 반환
    iterator erase(iterator pos) {
        Node* to_del = pos.get_node();
        Node* next = to_del->next;
        to_del->prev->next = to_del->next;
        to_del->next->prev = to_del->prev;
        delete to_del;
        --size_;
        return iterator(next);
    }

    void print() const {
        std::cout << "[";
        Node* cur = sentinel_.next;
        bool first = true;
        while (cur != &sentinel_) {
            if (!first) std::cout << " ↔ ";
            std::cout << cur->data;
            first = false;
            cur = cur->next;
        }
        std::cout << "] (size=" << size_ << ")\n";
    }
};

int main() {
    DoublyLinkedList<int> lst;

    // 기본 삽입
    for (int x : {1, 2, 3, 4, 5}) lst.push_back(x);
    std::cout << "초기: ";
    lst.print();

    // 앞에 삽입
    lst.push_front(0);
    std::cout << "push_front(0): ";
    lst.print();

    // 루프 중 짝수 삭제
    for (auto it = lst.begin(); it != lst.end(); ) {
        if (*it % 2 == 0) {
            it = lst.erase(it);
        } else {
            ++it;
        }
    }
    std::cout << "짝수 삭제 후: ";
    lst.print();

    // iterator stability 시연
    DoublyLinkedList<std::string> slst;
    slst.push_back("A");
    slst.push_back("B");
    slst.push_back("C");
    auto it_b = slst.begin();
    ++it_b;  // "B"를 가리킴

    slst.push_front("Z");
    slst.push_back("D");
    std::cout << "여러 삽입 후에도 it_b = \"" << *it_b << "\" (유효)\n";

    return 0;
}

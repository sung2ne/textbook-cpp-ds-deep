// filename: binary_heap.cpp
// g++ -std=c++17 -O2 -Wall -o binary_heap binary_heap.cpp
#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <stdexcept>

// 직접 구현한 max-heap
class MaxHeap {
    std::vector<int> data;

    void sift_up(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (data[parent] < data[i]) {
                std::swap(data[parent], data[i]);
                i = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(int i) {
        int n = static_cast<int>(data.size());
        while (true) {
            int left  = 2 * i + 1;
            int right = 2 * i + 2;
            int largest = i;

            if (left  < n && data[left]  > data[largest]) largest = left;
            if (right < n && data[right] > data[largest]) largest = right;

            if (largest != i) {
                std::swap(data[i], data[largest]);
                i = largest;
            } else {
                break;
            }
        }
    }

public:
    void push(int val) {
        data.push_back(val);
        sift_up(static_cast<int>(data.size()) - 1);
    }

    int top() const {
        if (data.empty()) throw std::runtime_error("heap is empty");
        return data[0];
    }

    void pop() {
        if (data.empty()) throw std::runtime_error("heap is empty");
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) sift_down(0);
    }

    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }

    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < data.size(); ++i) {
            std::cout << data[i];
            if (i + 1 < data.size()) std::cout << ", ";
        }
        std::cout << "]\n";
    }
};

// 커스텀 타입 priority_queue 예제
struct Task {
    int priority;
    std::string name;

    // min-heap을 최대 우선순위 큐로 쓰기 위해 역전
    bool operator>(const Task& other) const {
        return priority > other.priority;
    }
};

int main() {
    // 직접 구현 max-heap 테스트
    std::cout << "=== 직접 구현 MaxHeap ===\n";
    MaxHeap heap;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) {
        heap.push(x);
    }
    std::cout << "힙 배열: ";
    heap.print();
    std::cout << "top: " << heap.top() << "\n";

    std::cout << "pop 순서: ";
    while (!heap.empty()) {
        std::cout << heap.top() << " ";
        heap.pop();
    }
    std::cout << "\n\n";

    // std::priority_queue max-heap 비교
    std::cout << "=== std::priority_queue (max-heap) ===\n";
    std::priority_queue<int> pq;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) pq.push(x);
    std::cout << "pop 순서: ";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n\n";

    // min-heap
    std::cout << "=== std::priority_queue (min-heap) ===\n";
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_pq;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) min_pq.push(x);
    std::cout << "pop 순서: ";
    while (!min_pq.empty()) {
        std::cout << min_pq.top() << " ";
        min_pq.pop();
    }
    std::cout << "\n\n";

    // 커스텀 타입 (높은 priority 먼저)
    std::cout << "=== 작업 스케줄러 (높은 priority 먼저) ===\n";
    std::priority_queue<Task, std::vector<Task>, std::greater<Task>> task_pq;
    // greater + 역전된 operator> → 실제로는 큰 priority가 먼저
    task_pq.push({3, "일반 작업"});
    task_pq.push({1, "긴급 처리"});
    task_pq.push({2, "중요 업무"});
    task_pq.push({1, "즉시 처리"});

    while (!task_pq.empty()) {
        auto t = task_pq.top();
        task_pq.pop();
        std::cout << "  [priority=" << t.priority << "] " << t.name << "\n";
    }

    return 0;
}

// filename: tree_traversal.cpp
// g++ -std=c++17 -O2 -Wall -o tree_traversal tree_traversal.cpp

#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <memory>

struct Node {
    int val;
    std::unique_ptr<Node> left, right;
    Node(int v) : val(v) {}
};

// 재귀 순회
void preorder_recursive(const Node* node, std::vector<int>& result) {
    if (!node) return;
    result.push_back(node->val);
    preorder_recursive(node->left.get(), result);
    preorder_recursive(node->right.get(), result);
}

void inorder_recursive(const Node* node, std::vector<int>& result) {
    if (!node) return;
    inorder_recursive(node->left.get(), result);
    result.push_back(node->val);
    inorder_recursive(node->right.get(), result);
}

void postorder_recursive(const Node* node, std::vector<int>& result) {
    if (!node) return;
    postorder_recursive(node->left.get(), result);
    postorder_recursive(node->right.get(), result);
    result.push_back(node->val);
}

// 명시적 스택을 사용한 중위 순회 (반복)
std::vector<int> inorder_iterative(const Node* root) {
    std::vector<int> result;
    std::stack<const Node*> stack;
    const Node* curr = root;

    while (curr || !stack.empty()) {
        // 왼쪽 끝까지 이동
        while (curr) {
            stack.push(curr);
            curr = curr->left.get();
        }
        // 방문
        curr = stack.top(); stack.pop();
        result.push_back(curr->val);
        // 오른쪽으로 이동
        curr = curr->right.get();
    }
    return result;
}

// 명시적 스택을 사용한 전위 순회 (반복)
std::vector<int> preorder_iterative(const Node* root) {
    std::vector<int> result;
    if (!root) return result;
    std::stack<const Node*> stack;
    stack.push(root);

    while (!stack.empty()) {
        const Node* node = stack.top(); stack.pop();
        result.push_back(node->val);
        // 스택은 LIFO이므로 오른쪽 먼저 push
        if (node->right) stack.push(node->right.get());
        if (node->left)  stack.push(node->left.get());
    }
    return result;
}

// 레벨 순서 순회 (BFS)
std::vector<std::vector<int>> level_order(const Node* root) {
    std::vector<std::vector<int>> result;
    if (!root) return result;

    std::queue<const Node*> q;
    q.push(root);

    while (!q.empty()) {
        int level_size = static_cast<int>(q.size());
        std::vector<int> level;
        for (int i = 0; i < level_size; ++i) {
            const Node* node = q.front(); q.pop();
            level.push_back(node->val);
            if (node->left)  q.push(node->left.get());
            if (node->right) q.push(node->right.get());
        }
        result.push_back(std::move(level));
    }
    return result;
}

// 트리 높이
int height(const Node* node) {
    if (!node) return -1;  // 빈 트리: 높이 -1 (또는 0 정의에 따라)
    return 1 + std::max(height(node->left.get()), height(node->right.get()));
}

// 트리 구성 헬퍼
std::unique_ptr<Node> make_node(int v,
    std::unique_ptr<Node> left = nullptr,
    std::unique_ptr<Node> right = nullptr) {
    auto node = std::make_unique<Node>(v);
    node->left  = std::move(left);
    node->right = std::move(right);
    return node;
}

void print(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    //       1
    //      / \
    //     2   3
    //    / \   \
    //   4   5   6
    auto root = make_node(1,
        make_node(2,
            make_node(4),
            make_node(5)
        ),
        make_node(3,
            nullptr,
            make_node(6)
        )
    );

    // 재귀 순회
    std::vector<int> pre, in, post;
    preorder_recursive(root.get(), pre);
    inorder_recursive(root.get(), in);
    postorder_recursive(root.get(), post);
    print("전위(재귀)", pre);   // 1 2 4 5 3 6
    print("중위(재귀)", in);    // 4 2 5 1 3 6
    print("후위(재귀)", post);  // 4 5 2 6 3 1

    // 반복 순회
    print("전위(반복)", preorder_iterative(root.get()));  // 1 2 4 5 3 6
    print("중위(반복)", inorder_iterative(root.get()));   // 4 2 5 1 3 6

    // 레벨 순서
    auto levels = level_order(root.get());
    std::cout << "레벨 순서:\n";
    for (int l = 0; l < (int)levels.size(); ++l) {
        std::cout << "  레벨 " << l << ": ";
        for (int x : levels[l]) std::cout << x << " ";
        std::cout << "\n";
    }
    // 레벨 0: 1
    // 레벨 1: 2 3
    // 레벨 2: 4 5 6

    std::cout << "트리 높이: " << height(root.get()) << "\n";  // 2

    return 0;
}

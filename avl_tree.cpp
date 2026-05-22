// filename: avl_tree.cpp
// g++ -std=c++17 -O2 -Wall -o avl_tree avl_tree.cpp

#include <iostream>
#include <algorithm>
#include <vector>

struct AVLNode {
    int val, height;
    AVLNode *left, *right;
    explicit AVLNode(int v) : val(v), height(1), left(nullptr), right(nullptr) {}
};

int h(AVLNode* n) { return n ? n->height : 0; }
int bf(AVLNode* n) { return n ? h(n->left) - h(n->right) : 0; }

void update_height(AVLNode* n) {
    n->height = 1 + std::max(h(n->left), h(n->right));
}

// 오른쪽 회전 (LL 불균형 해소)
AVLNode* rotate_right(AVLNode* z) {
    AVLNode* y = z->left;
    AVLNode* T3 = y->right;

    y->right = z;
    z->left  = T3;

    update_height(z);
    update_height(y);
    return y;  // 새 루트
}

// 왼쪽 회전 (RR 불균형 해소)
AVLNode* rotate_left(AVLNode* z) {
    AVLNode* y = z->right;
    AVLNode* T2 = y->left;

    y->left  = z;
    z->right = T2;

    update_height(z);
    update_height(y);
    return y;  // 새 루트
}

// 균형 복원
AVLNode* rebalance(AVLNode* node) {
    update_height(node);
    int balance = bf(node);

    // LL
    if (balance > 1 && bf(node->left) >= 0)
        return rotate_right(node);

    // LR
    if (balance > 1 && bf(node->left) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    // RR
    if (balance < -1 && bf(node->right) <= 0)
        return rotate_left(node);

    // RL
    if (balance < -1 && bf(node->right) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;  // 이미 균형
}

AVLNode* insert(AVLNode* node, int val) {
    if (!node) return new AVLNode(val);
    if (val < node->val)  node->left  = insert(node->left, val);
    else if (val > node->val) node->right = insert(node->right, val);
    else return node;  // 중복 무시

    return rebalance(node);
}

AVLNode* find_min(AVLNode* node) {
    while (node->left) node = node->left;
    return node;
}

AVLNode* remove(AVLNode* node, int val) {
    if (!node) return nullptr;
    if (val < node->val)  node->left  = remove(node->left, val);
    else if (val > node->val) node->right = remove(node->right, val);
    else {
        // 삭제
        if (!node->left || !node->right) {
            AVLNode* temp = node->left ? node->left : node->right;
            delete node;
            return rebalance(temp);
        }
        AVLNode* successor = find_min(node->right);
        node->val  = successor->val;
        node->right = remove(node->right, successor->val);
    }
    return rebalance(node);
}

void inorder(AVLNode* node, std::vector<int>& v) {
    if (!node) return;
    inorder(node->left, v);
    v.push_back(node->val);
    inorder(node->right, v);
}

void free_tree(AVLNode* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    delete node;
}

int main() {
    AVLNode* root = nullptr;

    // 정렬 순서로 삽입 (일반 BST라면 최악 케이스)
    for (int x : {1, 2, 3, 4, 5, 6, 7}) {
        root = insert(root, x);
    }

    std::cout << "AVL 트리 높이 (n=7, 정렬 삽입): "
              << h(root) << "\n";  // 3 (완벽 균형)
    // 일반 BST라면 높이 = 6

    std::vector<int> sorted;
    inorder(root, sorted);
    std::cout << "중위 순회: ";
    for (int x : sorted) std::cout << x << " ";
    std::cout << "\n";

    root = remove(root, 4);
    sorted.clear();
    inorder(root, sorted);
    std::cout << "4 삭제 후: ";
    for (int x : sorted) std::cout << x << " ";
    std::cout << "\n";

    free_tree(root);
    return 0;
}

// filename: generic_stack.hpp
#pragma once
#include <vector>
#include <stdexcept>
#include <iterator>

template<typename T>
class Stack {
    std::vector<T> data_;

public:
    using value_type = T;
    using size_type  = std::size_t;

    void push(const T& val) { data_.push_back(val); }
    void push(T&& val)      { data_.push_back(std::move(val)); }

    template<typename... Args>
    void emplace(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

    T& top() {
        if (empty()) throw std::runtime_error("Stack::top() on empty stack");
        return data_.back();
    }
    const T& top() const {
        if (empty()) throw std::runtime_error("Stack::top() on empty stack");
        return data_.back();
    }

    void pop() {
        if (empty()) throw std::runtime_error("Stack::pop() on empty stack");
        data_.pop_back();
    }

    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    void reserve(size_type n) { data_.reserve(n); }

    // 바닥→꼭대기 순서로 접근하는 iterator
    class iterator {
        T* ptr_;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        explicit iterator(T* p) : ptr_(p) {}
        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }
        iterator& operator++() { ++ptr_; return *this; }
        iterator  operator++(int) { auto tmp = *this; ++ptr_; return tmp; }
        iterator& operator--() { --ptr_; return *this; }
        bool operator==(const iterator& o) const { return ptr_ == o.ptr_; }
        bool operator!=(const iterator& o) const { return ptr_ != o.ptr_; }
        difference_type operator-(const iterator& o) const { return ptr_ - o.ptr_; }
    };

    iterator begin() { return iterator(data_.data()); }
    iterator end()   { return iterator(data_.data() + data_.size()); }
};

template<typename T>
class Queue {
    std::deque<T> data_;

public:
    using value_type = T;
    using size_type  = std::size_t;

    void push(const T& val) { data_.push_back(val); }
    void push(T&& val)      { data_.push_back(std::move(val)); }

    template<typename... Args>
    void emplace(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

    T& front() {
        if (empty()) throw std::runtime_error("Queue::front() on empty queue");
        return data_.front();
    }
    const T& front() const {
        if (empty()) throw std::runtime_error("Queue::front() on empty queue");
        return data_.front();
    }

    T& back()       { return data_.back(); }
    const T& back() const { return data_.back(); }

    void pop() {
        if (empty()) throw std::runtime_error("Queue::pop() on empty queue");
        data_.pop_front();
    }

    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
};

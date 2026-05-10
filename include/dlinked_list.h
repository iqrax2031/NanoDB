#pragma once
// NanoDB Doubly Linked List
// Bidirectional list used by cache and queue-like subsystems needing O(1) splice/remove.

namespace nanodb {

// Generic doubly-linked list node
template <typename T>
struct DLLNode {
    T data;
    DLLNode<T>* prev;
    DLLNode<T>* next;
    
    DLLNode(const T& d) : data(d), prev(nullptr), next(nullptr) {}
};

// Generic doubly-linked list
template <typename T>
class DoublyLinkedList {
public:
    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}
    
    ~DoublyLinkedList() {
        clear();
    }
    
    void push_front(const T& data) {
        DLLNode<T>* node = new DLLNode<T>(data);
        if (!head_) {
            head_ = tail_ = node;
        } else {
            node->next = head_;
            head_->prev = node;
            head_ = node;
        }
        ++size_;
    }
    
    void push_back(const T& data) {
        DLLNode<T>* node = new DLLNode<T>(data);
        if (!tail_) {
            head_ = tail_ = node;
        } else {
            tail_->next = node;
            node->prev = tail_;
            tail_ = node;
        }
        ++size_;
    }
    
    T pop_front() {
        if (!head_) return T();
        DLLNode<T>* node = head_;
        T data = node->data;
        head_ = node->next;
        if (head_) head_->prev = nullptr;
        else tail_ = nullptr;
        delete node;
        --size_;
        return data;
    }
    
    T pop_back() {
        if (!tail_) return T();
        DLLNode<T>* node = tail_;
        T data = node->data;
        tail_ = node->prev;
        if (tail_) tail_->next = nullptr;
        else head_ = nullptr;
        delete node;
        --size_;
        return data;
    }
    
    // Remove a specific node (caller must provide the node pointer)
    void remove(DLLNode<T>* node) {
        if (!node) return;
        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == head_) head_ = node->next;
        if (node == tail_) tail_ = node->prev;
        delete node;
        --size_;
    }
    
    // Detach a node without deleting (for moving)
    void detach(DLLNode<T>* node) {
        if (!node) return;
        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == head_) head_ = node->next;
        if (node == tail_) tail_ = node->prev;
        node->prev = node->next = nullptr;
        --size_;
    }
    
    DLLNode<T>* front() const { return head_; }
    DLLNode<T>* back() const { return tail_; }
    
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    void clear() {
        while (head_) {
            DLLNode<T>* tmp = head_;
            head_ = head_->next;
            delete tmp;
        }
        tail_ = nullptr;
        size_ = 0;
    }
    
private:
    DLLNode<T>* head_;
    DLLNode<T>* tail_;
    int size_;
};

} // namespace nanodb

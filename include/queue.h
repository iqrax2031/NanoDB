#pragma once
// NanoDB Queue Primitives
// Custom FIFO queue and heap-based priority queue used by execution scheduling.

namespace nanodb {

// Generic Queue (FIFO)
template <typename T>
class Queue {
private:
    static const int DEFAULT_CAPACITY = 256;
    T* data_;
    int size_;
    int capacity_;
    int front_;
    
public:
    Queue(int capacity = DEFAULT_CAPACITY) 
        : size_(0), capacity_(capacity), front_(0) {
        data_ = new T[capacity_];
    }
    
    ~Queue() {
        delete[] data_;
    }
    
    void enqueue(const T& val) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        int rear = (front_ + size_) % capacity_;
        data_[rear] = val;
        ++size_;
    }
    
    T dequeue() {
        if (size_ <= 0) return T();
        T val = data_[front_];
        front_ = (front_ + 1) % capacity_;
        --size_;
        return val;
    }
    
    T& peek() {
        static T dummy;
        if (size_ <= 0) return dummy;
        return data_[front_];
    }
    
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    void clear() {
        size_ = 0;
        front_ = 0;
    }
    
private:
    void resize(int new_capacity) {
        T* new_data = new T[new_capacity];
        for (int i = 0; i < size_; ++i) {
            new_data[i] = data_[(front_ + i) % capacity_];
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
        front_ = 0;
    }
};

// Priority Queue (max-heap by default)
template <typename T>
class PriorityQueue {
private:
    static const int DEFAULT_CAPACITY = 256;
    T* data_;
    int size_;
    int capacity_;
    
    // Comparator: returns true if a should come before b (higher priority)
    typedef bool (*Comparator)(const T& a, const T& b);
    Comparator cmp_;
    
    void swap(int i, int j) {
        T tmp = data_[i];
        data_[i] = data_[j];
        data_[j] = tmp;
    }
    
    void bubble_up(int idx) {
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (cmp_(data_[idx], data_[parent])) {
                swap(idx, parent);
                idx = parent;
            } else {
                break;
            }
        }
    }
    
    void bubble_down(int idx) {
        while (true) {
            int smallest = idx;
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;
            
            if (left < size_ && cmp_(data_[left], data_[smallest])) {
                smallest = left;
            }
            if (right < size_ && cmp_(data_[right], data_[smallest])) {
                smallest = right;
            }
            
            if (smallest != idx) {
                swap(idx, smallest);
                idx = smallest;
            } else {
                break;
            }
        }
    }
    
public:
    PriorityQueue(Comparator cmp = nullptr, int capacity = DEFAULT_CAPACITY) 
        : size_(0), capacity_(capacity > 0 ? capacity : DEFAULT_CAPACITY), cmp_(cmp) {
        data_ = new T[capacity_];
        if (!cmp_) {
            // Keep deterministic fallback semantics when no comparator is provided.
            cmp_ = [](const T&, const T&) { return false; };
        }
    }
    
    ~PriorityQueue() {
        delete[] data_;
    }
    
    void push(const T& val) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_] = val;
        bubble_up(size_);
        ++size_;
    }
    
    T pop() {
        if (size_ <= 0) return T();
        T val = data_[0];
        data_[0] = data_[size_ - 1];
        --size_;
        if (size_ > 0) bubble_down(0);
        return val;
    }
    
    T& top() {
        static T dummy;
        if (size_ <= 0) return dummy;
        return data_[0];
    }
    
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    void clear() {
        size_ = 0;
    }
    
private:
    void resize(int new_capacity) {
        // Never shrink on push-triggered growth; keep amortized O(1) insertion behavior.
        if (new_capacity <= capacity_) {
            new_capacity = capacity_ > 0 ? capacity_ * 2 : DEFAULT_CAPACITY;
        }
        if (new_capacity <= 0) {
            new_capacity = DEFAULT_CAPACITY;
        }
        T* new_data = new T[new_capacity];
        for (int i = 0; i < size_; ++i) {
            new_data[i] = data_[i];
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }
};

} // namespace nanodb

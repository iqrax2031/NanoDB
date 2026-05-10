#pragma once
// NanoDB Stack
// Dynamic array-backed LIFO stack used by parser and evaluator internals.

namespace nanodb {

// Generic Stack (LIFO)
template <typename T>
class Stack {
private:
    static const int DEFAULT_CAPACITY = 256;
    T* data_;
    int size_;
    int capacity_;
    
public:
    Stack(int capacity = DEFAULT_CAPACITY) : size_(0), capacity_(capacity) {
        data_ = new T[capacity_];
    }
    
    ~Stack() {
        delete[] data_;
    }
    
    void push(const T& val) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = val;
    }
    
    T pop() {
        if (size_ <= 0) return T();
        return data_[--size_];
    }
    
    T& top() {
        static T dummy;
        if (size_ <= 0) return dummy;
        return data_[size_ - 1];
    }
    
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    void clear() {
        size_ = 0;
    }
    
private:
    void resize(int new_capacity) {
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

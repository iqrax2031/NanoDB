#pragma once

#include "dlinked_list.h"

namespace nanodb {

// Hash Map with chaining collision resolution
template <typename K, typename V>
class HashMap {
private:
    static const int DEFAULT_CAPACITY = 16;
    
    struct Entry {
        K key;
        V value;
        Entry(const K& k, const V& v) : key(k), value(v) {}
    };
    
    typedef DoublyLinkedList<Entry> Chain;
    Chain** buckets_;
    int capacity_;
    int size_;
    
    int hash_key(const K& key) const {
        // Simple hash function for common types
        return ((int)key) & (capacity_ - 1);
    }
    
    void resize(int new_capacity) {
        Chain** new_buckets = new Chain*[new_capacity];
        for (int i = 0; i < new_capacity; ++i) {
            new_buckets[i] = new Chain();
        }
        
        // Rehash all entries
        for (int i = 0; i < capacity_; ++i) {
            if (buckets_[i]) {
                DLLNode<Entry>* node = buckets_[i]->front();
                while (node) {
                    int idx = ((int)node->data.key) & (new_capacity - 1);
                    new_buckets[idx]->push_back(node->data);
                    node = node->next;
                }
                delete buckets_[i];
            }
        }
        delete[] buckets_;
        
        buckets_ = new_buckets;
        capacity_ = new_capacity;
    }
    
public:
    HashMap(int capacity = DEFAULT_CAPACITY) : capacity_(capacity), size_(0) {
        buckets_ = new Chain*[capacity_];
        for (int i = 0; i < capacity_; ++i) {
            buckets_[i] = new Chain();
        }
    }
    
    ~HashMap() {
        for (int i = 0; i < capacity_; ++i) {
            delete buckets_[i];
        }
        delete[] buckets_;
    }
    
    void insert(const K& key, const V& val) {
        if (size_ >= capacity_ / 2) {
            resize(capacity_ * 2);
        }
        
        int idx = hash_key(key);
        DLLNode<Entry>* node = buckets_[idx]->front();
        while (node) {
            if (node->data.key == key) {
                node->data.value = val; // Update
                return;
            }
            node = node->next;
        }
        
        buckets_[idx]->push_back(Entry(key, val));
        ++size_;
    }
    
    bool search(const K& key, V& result) {
        int idx = hash_key(key);
        DLLNode<Entry>* node = buckets_[idx]->front();
        while (node) {
            if (node->data.key == key) {
                result = node->data.value;
                return true;
            }
            node = node->next;
        }
        return false;
    }
    
    bool contains(const K& key) {
        int idx = hash_key(key);
        DLLNode<Entry>* node = buckets_[idx]->front();
        while (node) {
            if (node->data.key == key) {
                return true;
            }
            node = node->next;
        }
        return false;
    }
    
    bool remove(const K& key) {
        int idx = hash_key(key);
        DLLNode<Entry>* node = buckets_[idx]->front();
        while (node) {
            if (node->data.key == key) {
                buckets_[idx]->remove(node);
                --size_;
                return true;
            }
            node = node->next;
        }
        return false;
    }
    
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
};

} // namespace nanodb

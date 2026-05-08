#pragma once

namespace nanodb {

// AVL Tree Node
template <typename K, typename V>
struct AVLNode {
    K key;
    V value;
    AVLNode* left;
    AVLNode* right;
    int height;
    
    AVLNode(const K& k, const V& v) 
        : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
};

// AVL Balanced Tree (Self-balancing BST)
template <typename K, typename V>
class AVLTree {
private:
    AVLNode<K, V>* root_;
    int size_;
    
    int get_height(AVLNode<K, V>* node) {
        return node ? node->height : 0;
    }
    
    int get_balance(AVLNode<K, V>* node) {
        return node ? (get_height(node->left) - get_height(node->right)) : 0;
    }
    
    void update_height(AVLNode<K, V>* node) {
        if (node) {
            int lh = get_height(node->left);
            int rh = get_height(node->right);
            node->height = 1 + (lh > rh ? lh : rh);
        }
    }
    
    AVLNode<K, V>* rotate_right(AVLNode<K, V>* y) {
        AVLNode<K, V>* x = y->left;
        AVLNode<K, V>* T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        update_height(y);
        update_height(x);
        return x;
    }
    
    AVLNode<K, V>* rotate_left(AVLNode<K, V>* x) {
        AVLNode<K, V>* y = x->right;
        AVLNode<K, V>* T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        update_height(x);
        update_height(y);
        return y;
    }
    
    AVLNode<K, V>* insert_impl(AVLNode<K, V>* node, const K& key, const V& val, bool& inserted) {
        if (!node) {
            ++size_;
            inserted = true;
            return new AVLNode<K, V>(key, val);
        }
        
        if (key < node->key) {
            node->left = insert_impl(node->left, key, val, inserted);
        } else if (key > node->key) {
            node->right = insert_impl(node->right, key, val, inserted);
        } else {
            // Duplicate key: update value
            node->value = val;
            inserted = false;
            return node;
        }
        
        update_height(node);
        int balance = get_balance(node);
        
        // Left-left
        if (balance > 1 && key < node->left->key) {
            return rotate_right(node);
        }
        // Right-right
        if (balance < -1 && key > node->right->key) {
            return rotate_left(node);
        }
        // Left-right
        if (balance > 1 && key > node->left->key) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        // Right-left
        if (balance < -1 && key < node->right->key) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
    }
    
    bool search_impl(AVLNode<K, V>* node, const K& key, V& result) {
        if (!node) return false;
        
        if (key < node->key) {
            return search_impl(node->left, key, result);
        } else if (key > node->key) {
            return search_impl(node->right, key, result);
        } else {
            result = node->value;
            return true;
        }
    }
    
    AVLNode<K, V>* remove_impl(AVLNode<K, V>* node, const K& key, bool& removed) {
        if (!node) return nullptr;
        
        if (key < node->key) {
            node->left = remove_impl(node->left, key, removed);
        } else if (key > node->key) {
            node->right = remove_impl(node->right, key, removed);
        } else {
            removed = true;
            --size_;
            
            // Leaf
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            }
            // One child
            if (!node->left) {
                AVLNode<K, V>* tmp = node->right;
                delete node;
                return tmp;
            }
            if (!node->right) {
                AVLNode<K, V>* tmp = node->left;
                delete node;
                return tmp;
            }
            
            // Two children: find inorder successor
            AVLNode<K, V>* min_node = node->right;
            while (min_node->left) min_node = min_node->left;
            
            node->key = min_node->key;
            node->value = min_node->value;
            node->right = remove_impl(node->right, min_node->key, removed);
            removed = true; // We manually adjusted
        }
        
        if (node) {
            update_height(node);
            int balance = get_balance(node);
            
            if (balance > 1) {
                if (get_balance(node->left) >= 0) {
                    return rotate_right(node);
                } else {
                    node->left = rotate_left(node->left);
                    return rotate_right(node);
                }
            }
            if (balance < -1) {
                if (get_balance(node->right) <= 0) {
                    return rotate_left(node);
                } else {
                    node->right = rotate_right(node->right);
                    return rotate_left(node);
                }
            }
        }
        return node;
    }
    
    void clear_impl(AVLNode<K, V>* node) {
        if (!node) return;
        clear_impl(node->left);
        clear_impl(node->right);
        delete node;
    }
    
public:
    AVLTree() : root_(nullptr), size_(0) {}
    
    ~AVLTree() {
        clear_impl(root_);
    }
    
    void insert(const K& key, const V& val) {
        bool inserted = false;
        root_ = insert_impl(root_, key, val, inserted);
    }
    
    bool search(const K& key, V& result) {
        return search_impl(root_, key, result);
    }
    
    void remove(const K& key) {
        bool removed = false;
        root_ = remove_impl(root_, key, removed);
    }
    
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
};

} // namespace nanodb

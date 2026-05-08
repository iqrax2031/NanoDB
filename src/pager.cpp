#include "../include/pager.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <iostream>

static bool dir_exists(const std::string &p) {
    struct stat info;
    if (stat(p.c_str(), &info) != 0) return false;
    return (info.st_mode & S_IFDIR) != 0;
}

Pager::Pager(size_t page_size, size_t pool_size, const std::string &store_path)
    : page_size_(page_size), pool_size_(pool_size), store_path_(store_path) {
    buffer_ = new char[page_size_ * pool_size_];
    page_ids_ = new int[pool_size_];
    dirty_ = new bool[pool_size_];
    prev_ = new int[pool_size_];
    next_ = new int[pool_size_];
    for (size_t i = 0; i < pool_size_; ++i) {
        page_ids_[i] = -1;
        dirty_[i] = false;
        prev_[i] = next_[i] = -1;
    }
    head_ = tail_ = -1;
    free_count_ = (int)pool_size_;

    // hash capacity: next power of two *2
    hash_cap_ = 1;
    while (hash_cap_ < pool_size_ * 2) hash_cap_ <<= 1;
    hash_keys_ = new int[hash_cap_];
    hash_vals_ = new int[hash_cap_];
    for (size_t i = 0; i < hash_cap_; ++i) hash_keys_[i] = -1, hash_vals_[i] = -1;

    // ensure storage dir
    if (!dir_exists(store_path_)) {
#ifdef _WIN32
        _mkdir(store_path_.c_str());
#else
        mkdir(store_path_.c_str(), 0755);
#endif
    }
}

Pager::~Pager() {
    flush_all();
    delete[] buffer_;
    delete[] page_ids_;
    delete[] dirty_;
    delete[] prev_;
    delete[] next_;
    delete[] hash_keys_;
    delete[] hash_vals_;
}

int Pager::find_frame_for_page(int page_id) {
    size_t idx = (size_t)page_id & (hash_cap_ - 1);
    size_t start = idx;
    while (true) {
        if (hash_keys_[idx] == page_id) return hash_vals_[idx];
        if (hash_keys_[idx] == -1) return -1;
        idx = (idx + 1) & (hash_cap_ - 1);
        if (idx == start) return -1;
    }
}

void Pager::insert_mapping(int page_id, int frame) {
    size_t idx = (size_t)page_id & (hash_cap_ - 1);
    while (hash_keys_[idx] != -1) idx = (idx + 1) & (hash_cap_ - 1);
    hash_keys_[idx] = page_id;
    hash_vals_[idx] = frame;
}

void Pager::remove_mapping(int page_id) {
    size_t idx = (size_t)page_id & (hash_cap_ - 1);
    while (true) {
        if (hash_keys_[idx] == page_id) break;
        if (hash_keys_[idx] == -1) return;
        idx = (idx + 1) & (hash_cap_ - 1);
    }
    // remove and rehash subsequent cluster
    hash_keys_[idx] = -1;
    hash_vals_[idx] = -1;
    size_t next = (idx + 1) & (hash_cap_ - 1);
    while (hash_keys_[next] != -1) {
        int k = hash_keys_[next];
        int v = hash_vals_[next];
        hash_keys_[next] = -1; hash_vals_[next] = -1;
        insert_mapping(k, v);
        next = (next + 1) & (hash_cap_ - 1);
    }
}

void Pager::touch_frame(int frame) {
    if (frame < 0 || frame >= (int)pool_size_) return;
    if (head_ == frame) return;
    int p = prev_[frame];
    int n = next_[frame];
    // detach
    if (p != -1) next_[p] = n;
    if (n != -1) prev_[n] = p;
    if (tail_ == frame) tail_ = p;
    // insert at head
    prev_[frame] = -1;
    next_[frame] = head_;
    if (head_ != -1) prev_[head_] = frame;
    head_ = frame;
    if (tail_ == -1) tail_ = frame;
}

int Pager::evict_lru() {
    if (tail_ == -1) return -1;
    int victim = tail_;
    // write back if dirty
    if (dirty_[victim] && page_ids_[victim] != -1) {
        write_page_to_disk(page_ids_[victim], victim);
    }
    // remove mapping
    if (page_ids_[victim] != -1) remove_mapping(page_ids_[victim]);
    page_ids_[victim] = -1;
    dirty_[victim] = false;

    // detach victim from list
    int p = prev_[victim];
    if (p != -1) next_[p] = -1;
    tail_ = p;
    if (tail_ == -1) head_ = -1;
    prev_[victim] = next_[victim] = -1;
    ++free_count_;
    return victim;
}

void Pager::load_page_from_disk(int page_id, int frame) {
    std::ostringstream fname;
    fname << store_path_ << "/page_" << page_id << ".dat";
    std::ifstream f(fname.str(), std::ios::binary);
    char* dest = buffer_ + frame * page_size_;
    if (!f) {
        // zero-fill if no file
        std::memset(dest, 0, page_size_);
        return;
    }
    f.read(dest, page_size_);
    // if file smaller, rest remains
}

void Pager::write_page_to_disk(int page_id, int frame) {
    std::ostringstream fname;
    fname << store_path_ << "/page_" << page_id << ".dat";
    std::ofstream f(fname.str(), std::ios::binary);
    char* src = buffer_ + frame * page_size_;
    f.write(src, page_size_);
}

char* Pager::fetch_page(int page_id) {
    int frame = find_frame_for_page(page_id);
    if (frame != -1) {
        touch_frame(frame);
        return buffer_ + frame * page_size_;
    }

    // need to allocate a free frame
    int use_frame = -1;
    if (free_count_ > 0) {
        // find first free
        for (size_t i = 0; i < pool_size_; ++i) if (page_ids_[i] == -1) { use_frame = (int)i; break; }
    } else {
        use_frame = evict_lru();
    }
    if (use_frame == -1) return nullptr;

    // load page into frame
    load_page_from_disk(page_id, use_frame);
    page_ids_[use_frame] = page_id;
    dirty_[use_frame] = false;
    insert_mapping(page_id, use_frame);
    // insert at head
    prev_[use_frame] = -1;
    next_[use_frame] = head_;
    if (head_ != -1) prev_[head_] = use_frame;
    head_ = use_frame;
    if (tail_ == -1) tail_ = use_frame;
    --free_count_;
    return buffer_ + use_frame * page_size_;
}

void Pager::mark_dirty(int page_id) {
    int frame = find_frame_for_page(page_id);
    if (frame != -1) dirty_[frame] = true;
}

void Pager::flush_all() {
    for (size_t i = 0; i < pool_size_; ++i) {
        if (page_ids_[i] != -1 && dirty_[i]) write_page_to_disk(page_ids_[i], (int)i);
        dirty_[i] = false;
    }
}

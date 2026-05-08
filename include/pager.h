#pragma once

#include <string>

class Pager {
public:
    Pager(size_t page_size, size_t pool_size, const std::string &store_path);
    ~Pager();

    // Fetch a writable pointer to the page data for given page id.
    // Caller may modify the memory; call mark_dirty when done.
    char* fetch_page(int page_id);
    void mark_dirty(int page_id);
    void flush_all();

    size_t page_size() const { return page_size_; }

private:
    size_t page_size_;
    size_t pool_size_;
    std::string store_path_;

    char* buffer_; // contiguous pages: buffer_ + frame * page_size_

    int* page_ids_; // length pool_size_, page id or -1
    bool* dirty_;   // length pool_size_

    // LRU via doubly-linked list of frames (indices)
    int* prev_; // length pool_size_
    int* next_; // length pool_size_
    int head_; // most-recently used
    int tail_; // least-recently used
    int free_count_;

    // Simple open-addressing hash map from page_id -> frame index
    int* hash_keys_;
    int* hash_vals_;
    size_t hash_cap_;

    int find_frame_for_page(int page_id);
    void insert_mapping(int page_id, int frame);
    void remove_mapping(int page_id);
    void touch_frame(int frame);
    int evict_lru();
    void load_page_from_disk(int page_id, int frame);
    void write_page_to_disk(int page_id, int frame);
};

#pragma once
// NanoDB Logger
// Lightweight append-only execution logger for demo tracing and verification.

#include <fstream>
#include <ctime>
#include <chrono>
#include <cstring>

namespace nanodb {

// Execution logger for detailed logging
class Logger {
private:
    std::ofstream log_file_;
    bool console_output_;
    
    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return "[" + std::to_string(ms.count()) + "]";
    }
    
public:
    Logger(const char* log_path, bool console = true) : console_output_(console) {
        log_file_.open(log_path, std::ios::app);
    }
    
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }
    
    void log(const char* message) {
        std::string ts = get_timestamp();
        std::string msg = ts + " [LOG] " + message;
        
        if (log_file_.is_open()) {
            log_file_ << msg << std::endl;
            log_file_.flush();
        }
        
        if (console_output_) {
            // Only output to console for important messages
        }
    }
    
    void log_query(const char* query) {
        std::string msg = "[QUERY] " + std::string(query);
        if (log_file_.is_open()) {
            log_file_ << get_timestamp() << " " << msg << std::endl;
            log_file_.flush();
        }
    }
    
    void log_page_eviction(int page_id, int frame, const char* policy) {
        char buf[512];
        sprintf_s(buf, sizeof(buf), "Page %d evicted from frame %d via %s, written to disk", page_id, frame, policy);
        log(buf);
    }
    
    void log_index_search(int key, const char* index_type, double time_us) {
        char buf[512];
        sprintf_s(buf, sizeof(buf), "Index search (key=%d, type=%s) completed in %.2f microseconds", key, index_type, time_us);
        log(buf);
    }
    
    void log_sequential_scan(int records_scanned, double time_us) {
        char buf[512];
        sprintf_s(buf, sizeof(buf), "Sequential scan: %d records in %.2f microseconds", records_scanned, time_us);
        log(buf);
    }
    
    void log_join(const char* table1, const char* table2, int result_rows) {
        char buf[512];
        sprintf_s(buf, sizeof(buf), "Join %s <-> %s: %d result rows", table1, table2, result_rows);
        log(buf);
    }
    
    void log_infix_to_postfix(const char* infix, const char* postfix) {
        char buf[1024];
        sprintf_s(buf, sizeof(buf), "Infix '%s' converted to Postfix: '%s'", infix, postfix);
        log(buf);
    }
    
    void log_mst_path(const char* path) {
        char buf[512];
        sprintf_s(buf, sizeof(buf), "Multi-table join MST path: %s", path);
        log(buf);
    }
    
    void log_cache_stats(int total_pages, int hits, int misses, double hit_ratio) {
        char buf[512];
        sprintf_s(buf, sizeof(buf), "Cache stats - Total: %d, Hits: %d, Misses: %d, Hit ratio: %.2f%%", 
                  total_pages, hits, misses, hit_ratio * 100.0);
        log(buf);
    }
    
    void flush() {
        if (log_file_.is_open()) {
            log_file_.flush();
        }
    }
};

} // namespace nanodb

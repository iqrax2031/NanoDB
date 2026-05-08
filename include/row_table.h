#pragma once

#include "type.h"

namespace nanodb {

// Row: collection of fields (columns)
class Row {
private:
    Field* fields_;
    int field_count_;
    int capacity_;
    
public:
    Row(int num_fields = 0) : field_count_(num_fields), capacity_(num_fields > 0 ? num_fields : 8) {
        fields_ = new Field[capacity_];
    }
    
    Row(const Row& other) : field_count_(other.field_count_), capacity_(other.capacity_) {
        fields_ = new Field[capacity_];
        for (int i = 0; i < field_count_; ++i) {
            fields_[i] = other.fields_[i];
        }
    }
    
    Row& operator=(const Row& other) {
        if (this != &other) {
            if (field_count_ != other.field_count_) {
                delete[] fields_;
                field_count_ = other.field_count_;
                capacity_ = other.capacity_;
                fields_ = new Field[capacity_];
            }
            for (int i = 0; i < field_count_; ++i) {
                fields_[i] = other.fields_[i];
            }
        }
        return *this;
    }
    
    ~Row() {
        delete[] fields_;
    }
    
    void set_field(int idx, const Field& val) {
        if (idx >= 0 && idx < field_count_) {
            fields_[idx] = val;
        }
    }
    
    Field& get_field(int idx) {
        static Field dummy;
        if (idx >= 0 && idx < field_count_) {
            return fields_[idx];
        }
        return dummy;
    }
    
    const Field& get_field(int idx) const {
        static Field dummy;
        if (idx >= 0 && idx < field_count_) {
            return fields_[idx];
        }
        return dummy;
    }
    
    int field_count() const { return field_count_; }
    
    std::string to_string() const {
        std::string result;
        for (int i = 0; i < field_count_; ++i) {
            result += fields_[i].to_string();
            if (i + 1 < field_count_) result += " | ";
        }
        return result;
    }
};

// Table: collection of rows
class Table {
private:
    Row* rows_;
    int row_count_;
    int capacity_;
    char table_name_[256];
    
public:
    Table(const char* name = "table", int capacity = 1024) 
        : row_count_(0), capacity_(capacity) {
        strncpy_s(table_name_, sizeof(table_name_), name, sizeof(table_name_) - 1);
        rows_ = new Row[capacity_];
    }
    
    Table(const Table& other) : row_count_(other.row_count_), capacity_(other.capacity_) {
        strncpy_s(table_name_, sizeof(table_name_), other.table_name_, sizeof(table_name_) - 1);
        rows_ = new Row[capacity_];
        for (int i = 0; i < row_count_; ++i) {
            rows_[i] = other.rows_[i];
        }
    }
    
    Table& operator=(const Table& other) {
        if (this != &other) {
            delete[] rows_;
            strncpy_s(table_name_, sizeof(table_name_), other.table_name_, sizeof(table_name_) - 1);
            row_count_ = other.row_count_;
            capacity_ = other.capacity_;
            rows_ = new Row[capacity_];
            for (int i = 0; i < row_count_; ++i) {
                rows_[i] = other.rows_[i];
            }
        }
        return *this;
    }
    
    ~Table() {
        delete[] rows_;
    }
    
    void insert_row(const Row& row) {
        if (row_count_ >= capacity_) {
            resize(capacity_ * 2);
        }
        rows_[row_count_++] = row;
    }
    
    Row& get_row(int idx) {
        static Row dummy;
        if (idx >= 0 && idx < row_count_) {
            return rows_[idx];
        }
        return dummy;
    }
    
    const Row& get_row(int idx) const {
        static Row dummy;
        if (idx >= 0 && idx < row_count_) {
            return rows_[idx];
        }
        return dummy;
    }
    
    int row_count() const { return row_count_; }
    const char* name() const { return table_name_; }
    
private:
    void resize(int new_capacity) {
        Row* new_rows = new Row[new_capacity];
        for (int i = 0; i < row_count_; ++i) {
            new_rows[i] = rows_[i];
        }
        delete[] rows_;
        rows_ = new_rows;
        capacity_ = new_capacity;
    }
};

} // namespace nanodb

#pragma once
// NanoDB Type System
// Polymorphic scalar values and owning Field wrapper for heterogeneous row data.

#include <string>

namespace nanodb {

struct Value {
    virtual ~Value() {}
    virtual Value* clone() const = 0;
    virtual std::string to_string() const = 0;
    virtual int compare(const Value* other) const = 0; // -1,0,1
};

struct IntValue : public Value {
    int v;
    IntValue(int x): v(x) {}
    Value* clone() const override { return new IntValue(v); }
    std::string to_string() const override;
    int compare(const Value* other) const override;
};

struct FloatValue : public Value {
    double v;
    FloatValue(double x): v(x) {}
    Value* clone() const override { return new FloatValue(v); }
    std::string to_string() const override;
    int compare(const Value* other) const override;
};

struct StringValue : public Value {
    std::string v;
    StringValue(const std::string &s): v(s) {}
    Value* clone() const override { return new StringValue(v); }
    std::string to_string() const override;
    int compare(const Value* other) const override;
};

// A simple owning wrapper
struct Field {
    Value* val;
    Field(): val(nullptr) {}
    // Deep-copy ownership: every Field owns an independent Value clone.
    Field(const Field &o): val(o.val ? o.val->clone() : nullptr) {}
    Field(Field&& o) noexcept : val(o.val) { o.val = nullptr; }
    Field& operator=(const Field &o) {
        if (this != &o) {
            // Clone first to preserve strong exception safety for self state.
            Value* new_val = o.val ? o.val->clone() : nullptr;
            delete val;
            val = new_val;
        }
        return *this;
    }
    Field& operator=(Field&& o) noexcept {
        if (this != &o) {
            delete val;
            val = o.val;
            o.val = nullptr;
        }
        return *this;
    }
    Field(Value* v): val(v) {}
    ~Field() { delete val; }

    std::string to_string() const { return val?val->to_string():"NULL"; }
    int compare(const Field &o) const { if (!val || !o.val) return -2; return val->compare(o.val); }
};

} // ns

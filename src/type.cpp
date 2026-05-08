#include "../include/type.h"
#include <sstream>

using namespace nanodb;

std::string IntValue::to_string() const {
    std::ostringstream ss; ss << v; return ss.str();
}

int IntValue::compare(const Value* other) const {
    if (const IntValue* o = dynamic_cast<const IntValue*>(other)) {
        if (v < o->v) return -1; if (v > o->v) return 1; return 0;
    }
    if (const FloatValue* f = dynamic_cast<const FloatValue*>(other)) {
        double diff = (double)v - f->v; return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
    }
    return 0; // no-op for string
}

std::string FloatValue::to_string() const {
    std::ostringstream ss; ss << v; return ss.str();
}

int FloatValue::compare(const Value* other) const {
    if (const FloatValue* o = dynamic_cast<const FloatValue*>(other)) {
        if (v < o->v) return -1; if (v > o->v) return 1; return 0;
    }
    if (const IntValue* i = dynamic_cast<const IntValue*>(other)) {
        double diff = v - (double)i->v; return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
    }
    return 0;
}

std::string StringValue::to_string() const { return v; }

int StringValue::compare(const Value* other) const {
    if (const StringValue* s = dynamic_cast<const StringValue*>(other)) {
        if (v < s->v) return -1; if (v > s->v) return 1; return 0;
    }
    return 0;
}

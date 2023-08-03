#include "parser/config/rules/rules.h"

namespace alien::parser::rules {

    bool grammar_symbol::operator==(const grammar_symbol& other) const {
        return type == other.type && index == other.index;
    }

    bool grammar_symbol::operator<(const grammar_symbol& other) const {
        return type < other.type || index < other.index;
    }
}
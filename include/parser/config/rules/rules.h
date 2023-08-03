#ifndef ALIEN_PARSER_RULES_H
#define ALIEN_PARSER_RULES_H

#include <map>
#include <vector>

#include "nlohmann/json.hpp"

#include "util/u8string.h"

namespace alien::parser::rules {

    enum class symbol_type {
        TERMINAL,
        NON_TERMINAL
    };

    struct grammar_symbol {
        symbol_type type = symbol_type::TERMINAL;

        std::ptrdiff_t index = -2;

        bool operator==(const grammar_symbol& other) const;

        bool operator<(const grammar_symbol& other) const;
    };

    struct production {
        std::vector<grammar_symbol> symbols;

        std::ptrdiff_t prec = -1, assoc = -1;
        bool explicit_precedence = false, has_action = false;

        std::size_t length;
        util::u8string action;
    };

    struct rules {
        std::vector<std::vector<production>> ruleset;

        std::size_t start = 0;
    };

}

#endif //ALIEN_PARSER_RULES_H
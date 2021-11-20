#ifndef ALIEN_PARSER_CONFIG_RULES_H
#define ALIEN_PARSER_CONFIG_RULES_H

#include <vector>
#include <map>
#include "util/u8string.h"

namespace alien::parser::config::rules {

    struct grammar_symbol {
        util::u8string name, code_type;

        enum class symbol_type {
            NON_TERMINAL,
            TERMINAL,
        } type;

        int prec = -1, assoc = -1;

        bool operator<(const grammar_symbol& other) const {
            if (type == other.type) {
                return name < other.name;
            }

            return type < other.type;
        }
    };

    using alphabet = util::vecset<grammar_symbol>;

    struct production {
        std::vector<int> symbols;
        util::u8string code;

        int prec = -1, assoc = -1;

        bool given = false, has_action = false;
    };

    using nonterminal = std::vector<production>;

    struct rules {
        std::unordered_map<unsigned int, nonterminal> ruleset;

        unsigned int start;
    };
}

#endif //ALIEN_PARSER_CONFIG_RULES_H
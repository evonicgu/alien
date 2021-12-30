#ifndef ALIEN_NFA_H
#define ALIEN_NFA_H

#include <set>
#include <unordered_map>

#include "util/u8string.h"

namespace alien::lexer::automata::nfa {

    struct state {
        std::unordered_map<util::u8char, std::set<state*>> transitions;

        bool accepting;

        std::ptrdiff_t rule_number;

        bool operator<(const state& other) const {
            if (this->accepting) {
                if (other.accepting) {
                    return this->rule_number < other.rule_number;
                }
                return true;
            }

            if (other.accepting) {
                return false;
            }

            if (this->rule_number == other.rule_number) {
                return this < &other;
            }

            return this->rule_number < other.rule_number;
        }
    };

    struct simple_nfa {
        state *start = nullptr, *end = nullptr;
    };

}

#endif //ALIEN_NFA_H
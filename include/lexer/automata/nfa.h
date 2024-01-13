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

        state(const std::unordered_map<util::u8char, std::set<state*>>& transitions,
            bool accepting,
            std::ptrdiff_t rule_number)
            : transitions(transitions),
              accepting(accepting),
              rule_number(rule_number) {
        }

        bool operator<(const state& other) const;
    };

    struct simple_nfa {
        state *start = nullptr, *end = nullptr;
    };

}

#endif //ALIEN_NFA_H
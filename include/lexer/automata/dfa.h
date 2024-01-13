#ifndef ALIEN_DFA_H
#define ALIEN_DFA_H

#include <set>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"

#include "nfa.h"
#include "util/u8string.h"
#include "util/vecset.h"
#include "util/comparators.h"

namespace alien::lexer::automata::dfa {

    using nfa_set = std::set<nfa::state*, util::comparators::ptr_less<nfa::state>>;

    struct state {
        nfa_set nfa_states;
        std::vector<std::size_t> in_transitions, out_transitions;

        bool accepting = false;
        ptrdiff_t rule_number = -1;

        bool operator<(const state& other) const;
    };

    struct transition {
        std::size_t tail, head;

        util::u8char label;

        bool operator<(const transition& other) const;

        bool operator==(const transition& other) const;
    };

    struct dfa {
        std::vector<state> states;

        std::unordered_map<std::size_t, std::vector<std::size_t>> rulemap;
        std::vector<std::size_t> fstates;
        util::vecset<transition> transitions;

        std::size_t start_state;

        bool null_state_used = false, transitions_to_start = false;
    };

    void to_json(nlohmann::json& json, const dfa& automata);

}

#endif //ALIEN_DFA_H
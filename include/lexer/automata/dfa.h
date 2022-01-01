#ifndef ALIEN_DFA_H
#define ALIEN_DFA_H

#include <set>
#include <unordered_map>

#include "nlohmann/json.hpp"

#include "nfa.h"
#include "util/to_json.h"
#include "util/u8string.h"

namespace alien::lexer::automata::dfa {

    template<typename T>
    struct ptr_less {
        bool operator()(const T* lhs, const T* rhs) const {
            if (rhs == nullptr) {
                return false;
            }

            if (lhs == nullptr) {
                return true;
            }

            return *lhs < *rhs;
        }
    };

    using nfa_set = std::set<nfa::state*, ptr_less<nfa::state>>;

    struct state {
        nfa_set nfa_states;
        std::vector<std::size_t> in_transitions, out_transitions;

        bool accepting = false;
        ptrdiff_t rule_number = -1;

        bool operator<(const state& other) const {
            return nfa_states < other.nfa_states;
        }
    };

    struct transition {
        std::size_t tail, head;

        util::u8char label;

        bool operator<(const transition& other) const {
            if (label == other.label) {
                if (tail == other.tail) {
                    return head < other.head;
                }

                return tail < other.tail;
            }

            return label < other.label;
        }
    };

    struct dfa {
        std::vector<state> states;

        std::unordered_map<std::size_t, std::vector<std::size_t>> rulemap;
        std::vector<std::size_t> fstates;
        util::vecset<transition> transitions;

        std::size_t start_state;

        bool null_state_used = false;
    };

    void to_json(nlohmann::json& json, const dfa& automata) {
        json["states"] = util::to_json(automata.states, [](const state& state) {
            nlohmann::json json;

            json["transitions"] = state.out_transitions;
            json["accepting"] = state.accepting;
            json["rule_number"] = state.rule_number;

            return json;
        });

        json["transitions"] = util::to_json(automata.transitions, [](const transition& t) {
            nlohmann::json json;

            json["tail"] = t.tail;
            json["head"] = t.head;
            json["label"] = t.label;

            return json;
        });

        json["start_state"] = automata.start_state;

        json["null_state_used"] = automata.null_state_used;
    }

}

#endif //ALIEN_DFA_H
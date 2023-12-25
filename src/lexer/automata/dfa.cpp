#include "lexer/automata/dfa.h"

namespace alien::lexer::automata::dfa {

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

        json["transitions_to_start"] = automata.transitions_to_start;
    }

    bool state::operator<(const state& other) const {
        return nfa_states < other.nfa_states;
    }

    bool transition::operator<(const transition& other) const {
        if (label == other.label) {
            if (tail == other.tail) {
                return head < other.head;
            }

            return tail < other.tail;
        }

        return label < other.label;
    }

    bool transition::operator==(const transition& other) const {
        return tail == other.tail && head == other.head && label == other.label;
    }

}